/***************************************************************************
                          serial.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "serial.h"
#include "buffer.h"

#include <qapplication.h>
#include <qevent.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qdir.h>

#ifdef WIN32

#include <windows.h>

const unsigned SERIAL_TIMEOUT	= 1000;

class SerialEvent : public QEvent
{
public:
    SerialEvent(unsigned reason);
    unsigned reason() { return m_reason; }
protected:
    unsigned m_reason;
};

SerialEvent::SerialEvent(unsigned reason)
        : QEvent(User)
{
    m_reason = reason;
}

const unsigned EventComplete	= 0;
const unsigned EventError		= 1;
const unsigned EventTimeout		= 2;

enum PortState
{
    None,
    Read,
    Write,
    StartRead,
    StartWrite,
    Setup,
    Close
};

class SerialPortPrivate
{
public:
    SerialPortPrivate(SerialPort *port);
    ~SerialPortPrivate();
    void close();
    HANDLE		hPort;
    HANDLE		hEvent;
    HANDLE		hThread;
    OVERLAPPED	over;
    QTimer		*m_timer;
    SerialPort	*m_port;
    int			m_baudrate;
    bool		m_bXonXoff;
    string		m_line;
    PortState	m_state;
    Buffer		m_buff;
    int			m_time;
    int			m_read_time;
    char		m_char;
};

static DWORD __stdcall SerialThread(LPVOID lpParameter)
{
    log(L_DEBUG, "SerialThread: %X", GetCurrentThreadId());
    SerialPortPrivate *p = (SerialPortPrivate*)lpParameter;
    DWORD timeout = INFINITE;
    for (;;){
        DWORD res = WaitForSingleObject(p->hEvent, timeout);
        if ((res == WAIT_ABANDONED) || (p->m_state == Close))
            break;
        timeout = INFINITE;
        switch (p->m_state){
        case StartRead:{
                DWORD bytesReadn = 0;
                memset(&p->over, 0, sizeof(p->over));
                p->over.hEvent = p->hEvent;
                p->m_state = Read;
                if (ReadFile(p->hPort, &p->m_char, 1, &bytesReadn, &p->over))
                    break;
                DWORD err = GetLastError();
                if (err != ERROR_IO_PENDING){
                    p->m_state = None;
                    QApplication::postEvent(p->m_port, new SerialEvent(EventError));
                }else{
                    timeout = p->m_read_time;
                }
                break;
            }
        case StartWrite:{
                DWORD bytesWritten = 0;
                memset(&p->over, 0, sizeof(p->over));
                p->over.hEvent = p->hEvent;
                p->m_state = Write;
                if (WriteFile(p->hPort, p->m_line.c_str(), p->m_line.length(), &bytesWritten, &p->over))
                    break;
                DWORD err = GetLastError();
                if (err != ERROR_IO_PENDING){
                    p->m_state = None;
                    QApplication::postEvent(p->m_port, new SerialEvent(EventError));
                }else{
                    timeout = SERIAL_TIMEOUT;
                }
                break;
            }
        case Read:
        case Write:
            if (res == WAIT_TIMEOUT){
                QApplication::postEvent(p->m_port, new SerialEvent(EventTimeout));
            }else{
                QApplication::postEvent(p->m_port, new SerialEvent(EventComplete));
            }
            break;
        default:
            break;
        }
    }
    return 0;
}

SerialPort::SerialPort(QObject *parent)
        : QObject(parent)
{
    d = new SerialPortPrivate(this);
    connect(d->m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

SerialPortPrivate::SerialPortPrivate(SerialPort *port)
{
    hPort = INVALID_HANDLE_VALUE;
    hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_timer    = new QTimer(NULL);
    m_port     = port;
    m_state	   = None;
    DWORD threadId;
    hThread = CreateThread(NULL, 0, SerialThread, this, 0, &threadId);
}

SerialPortPrivate::~SerialPortPrivate()
{
    close();
    delete m_timer;
    m_state = Close;
    SetEvent(hEvent);
    CloseHandle(hEvent);
    WaitForSingleObject(hThread, INFINITE);
}

void SerialPortPrivate::close()
{
    if (hPort != INVALID_HANDLE_VALUE){
        CloseHandle(hPort);
        hPort = INVALID_HANDLE_VALUE;
        m_buff.init(0);
    }
    m_state = None;
}

SerialPort::~SerialPort()
{
    delete d;
}

void SerialPort::close()
{
    d->close();
}

bool SerialPort::open(const char *device, int baudrate, bool bXonXoff, int DTRtime)
{
    close();
    d->m_time	  = DTRtime;
    d->m_baudrate = baudrate;
    d->m_bXonXoff = bXonXoff;
    string port; // = "\\\\.\\";
    port += device;
    port += ":";
    d->hPort = CreateFileA(port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
    if (d->hPort == INVALID_HANDLE_VALUE){
        close();
        log(L_WARN, "Can' open %s", port.c_str());
        return false;
    }
    FlushFileBuffers(d->hPort);
    if (!EscapeCommFunction(d->hPort, CLRDTR)){
        close();
        log(L_WARN, "Clear DTR error");
        return false;
    }
    d->m_timer->start(d->m_time, true);
    return true;
}

void SerialPort::timeout()
{
    if (d->m_state == Setup){
        if (!SetupComm(d->hPort, 1024, 1024)){
            log(L_WARN, "SetupComm error");
            close();
            emit error();
            return;
        }
        PurgeComm(d->hPort, PURGE_RXABORT | PURGE_RXCLEAR);
        d->m_state = None;
        emit write_ready();
        return;
    }
    if (!EscapeCommFunction(d->hPort, SETDTR)){
        log(L_WARN, "Set DTR error");
        close();
        emit error();
        return;
    }
    DCB dcb;
    memset(&dcb, 0, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);
    if (GetCommState(d->hPort, &dcb) == 0){
        log(L_WARN, "Get status error");
        close();
        emit error();
        return;
    }
    dcb.fBinary  = TRUE;
    dcb.BaudRate = d->m_baudrate;
    dcb.fParity  = FALSE;
    dcb.Parity   = 0;
    dcb.ByteSize = 8;
    dcb.StopBits = 0;
    if (!d->m_bXonXoff)
    {
        dcb.fInX  = FALSE;
        dcb.fOutX = FALSE;
        dcb.fOutxDsrFlow = FALSE;
        dcb.fOutxCtsFlow = FALSE;
    }
    else
    {
        dcb.fInX  = TRUE;
        dcb.fOutX = TRUE;
        dcb.fOutxDsrFlow = FALSE;
        dcb.fOutxCtsFlow = FALSE;
    }
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    if (SetCommState(d->hPort, &dcb) == 0){
        log(L_WARN, "Set status error");
        close();
        emit error();
        return;
    }
    d->m_state = Setup;
    d->m_timer->start(d->m_time, true);
}

void SerialPort::writeLine(const char *data, unsigned read_time)
{
    if (d->hPort == INVALID_HANDLE_VALUE){
        emit error();
        return;
    }
    switch (d->m_state){
    case Read:
    case Write:
        CancelIo(d->hPort);
        break;
    default:
        break;
    }
    d->m_state		= StartWrite;
    d->m_line		= data;
    d->m_read_time	= read_time;
    FlushFileBuffers(d->hPort);
    SetEvent(d->hEvent);
}

void SerialPort::setTimeout(unsigned read_time)
{
    switch (d->m_state){
    case Read:
    case Write:
        CancelIo(d->hPort);
        break;
    default:
        break;
    }
    d->m_state		= StartRead;
    d->m_read_time	= read_time;
    SetEvent(d->hEvent);
}

string SerialPort::readLine()
{
    string res;
    if (d->hPort == INVALID_HANDLE_VALUE)
        return res;
    if (d->m_buff.scan("\n", res)){
        if (d->m_buff.readPos() == d->m_buff.writePos())
            d->m_buff.init(0);
    }
    return res;
}

bool SerialPort::event(QEvent *e)
{
    if (e->type() != QEvent::User)
        return QObject::event(e);
    switch (static_cast<SerialEvent*>(e)->reason()){
    case EventComplete:{
            DWORD bytes;
            if (GetOverlappedResult(d->hPort, &d->over, &bytes, true)){
                if (d->m_state == Read){
                    d->m_buff.pack(&d->m_char, 1);
                    if (d->m_char == '\n')
                        emit read_ready();
                }
                if (d->m_state == Write){
                    emit write_ready();
                    d->m_state = Read;
                }
                if (d->m_state == Read){
                    d->m_state = StartRead;
                    SetEvent(d->hEvent);
                }
                break;
            }
            close();
            emit error();
            break;
        }
    case EventTimeout:{
            log(L_WARN, "IO timeout");
            CancelIo(d->hPort);
            close();
            emit error();
            break;
        }
    case EventError:{
            log(L_WARN, "IO error");
            close();
            emit error();
        }
    }
    return true;
}

QStringList SerialPort::devices()
{
    QStringList res;
    for (unsigned i = 1; i <= 8; i++){
        string port = "COM";
        port += number(i);
        string fullPort = "\\\\.\\";
        fullPort += port;
        HANDLE hPort = CreateFileA(fullPort.c_str(),GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL );
        if (hPort == INVALID_HANDLE_VALUE)
            continue;
        res.append(port.c_str());
        CloseHandle(hPort);
    }
    return res;
}

#else

class SerialPortPrivate
{
public:
    SerialPortPrivate();
    ~SerialPortPrivate();
    void close();
    int fd;
};

SerialPortPrivate::SerialPortPrivate()
{
    fd = -1;
}

SerialPortPrivate::~SerialPortPrivate()
{
    close();
}

void SerialPortPrivate::close()
{
    if (fd == -1)
        return;
    ::close(fd);
    fd = -1;
}

SerialPort::SerialPort(QObject *parent)
        : QObject(parent)
{
    d = new SerialPortPrivate;
}

SerialPort::~SerialPort()
{
    delete d;
}

bool SerialPort::open(const char *device, int baudrate, bool bXonXoff, int DTRtime)
{
    return false;
}

void SerialPort::close()
{
}

void SerialPort::writeLine(const char *data, unsigned timeRead)
{
}

void SerialPort::setTimeout(unsigned timeRead)
{
}

string SerialPort::readLine()
{
    return "";
}

void SerialPort::timeout()
{
}

bool SerialPort::event(QEvent *e)
{
    return QObject::event(e);
}

QStringList SerialPort::devices()
{
    QStringList res;
    QDir dev("/dev");
    QStringList entries = dev.entryList("*", QDir::Drives);
    log(L_DEBUG, "Dev: %u", dev.exists());
    for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it){
	log(L_DEBUG, "?? %s", (const char*)(*it).latin1());
        res.append(*it);
    }
    return res;
}

#endif

#ifndef WIN32
#include "serial.moc"
#endif

