/***************************************************************************
                          message.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#include "simapi.h"

#include <time.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qstringlist.h>

#include <list>
#include <vector>

namespace SIM
{

using namespace std;

static DataDef	messageData[] =
    {
        { "Text", DATA_UTF, 1, 0 },
        { "Flags", DATA_ULONG, 1, 0 },
        { "Background", DATA_ULONG, 1, 0 },
        { "Foreground", DATA_ULONG, 1, 0 },
        { "Time", DATA_ULONG, 1, 0 },
        { "Font", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },			// Error
        { "", DATA_BOOL, 1, 0 },			// Retry
        { NULL, 0, 0, 0 }
    };

Message::Message(unsigned type, const char *cfg)
{
    m_type = type;
    m_id = 0;
    m_contact = 0;
    load_data(messageData, &data, cfg);
}

Message::~Message()
{
    free_data(messageData, &data);
}

QString Message::getPlainText()
{
    if (!(getFlags() & MESSAGE_RICHTEXT))
        return getText();
    string text;
    text = getText().utf8();
    Event e(EventDecodeText, &text);
    e.process();
    return QString::fromUtf8(SIM::unquoteText(text.c_str()).c_str());
}

QString Message::getRichText()
{
    if (getFlags() & MESSAGE_RICHTEXT)
        return getText();
    return quoteString(getText());
}

QString Message::presentation()
{
    QString res = getRichText();
    if (getBackground() != getForeground()){
        QString font;
        font.sprintf("<font color=\"#%06lX\">", getForeground() & 0xFFFFFF);
        res = font + res + "</font>";
    }
    return res;
}

void Message::setClient(const char *client)
{
    if (client == NULL)
        client = "";
    m_client = client;
}

string Message::save()
{
    if (getTime() == 0){
        time_t now;
        time(&now);
        setTime(now);
    }
    unsigned saveFlags = getFlags();
    setFlags(getFlags() & MESSAGE_SAVEMASK);
    string res = save_data(messageData, &data);
    setFlags(saveFlags);
    return res;
}

static DataDef messageSMSData[] =
    {
        { "Phone", DATA_UTF, 1, 0 },
        { "Network", DATA_UTF, 1, 0 },
        { NULL, 0, 0, 0 }
    };

SMSMessage::SMSMessage(const char *cfg)
        : Message(MessageSMS, cfg)
{
    load_data(messageSMSData, &data, cfg);
}

SMSMessage::~SMSMessage()
{
    free_data(messageSMSData, &data);
}

string SMSMessage::save()
{
    string s = Message::save();
    string s1 = save_data(messageSMSData, &data);
    if (!s1.empty()){
        if (!s.empty())
            s += '\n';
        s += s1;
    }
    return s;
}

QString SMSMessage::presentation()
{
    QString phone = quoteString(getPhone());
    QString net   = quoteString(getNetwork());
    if (!net.isEmpty())
        net = QString(" (") + net + ")";
    QString res = QString("<p><a href=\"sms:%1\"><img src=\"icon:cell\">%2%3</a></p>")
                  .arg(phone)
                  .arg(phone)
                  .arg(net);
    res += getRichText();
    return res;
}

class FileMessageIteratorPrivate : public vector<string>
{
public:
    FileMessageIteratorPrivate(const FileMessage &msg);
    vector<string>::iterator it;
    void add(const QString&);
};

FileMessageIteratorPrivate::FileMessageIteratorPrivate(const FileMessage &msg)
{
    QString files = ((FileMessage&)msg).getFile();
    while (!files.isEmpty()){
        add(getToken(files, ';'));
    }
    it = begin();
}

void FileMessageIteratorPrivate::add(const QString &str)
{
    QFileInfo f(str);
    if (!f.exists())
        return;
    if (!f.isDir()){
        push_back(string(QFile::encodeName(str)));
        return;
    }
    QDir d(str);
    QStringList l = d.entryList();
    for (QStringList::Iterator it = l.begin(); it != l.end(); ++it){
        QString p = str;
#ifdef WIN32
        p += "\\";
#else
        p += "/";
#endif
        p += *it;
        add(p);
    }
}

FileMessage::Iterator::Iterator(const FileMessage &m)
{
    p = new FileMessageIteratorPrivate(m);
}

FileMessage::Iterator::~Iterator()
{
    delete p;
}

const char *FileMessage::Iterator::operator[](unsigned n)
{
    if (n >= p->size())
        return NULL;
    return (*p)[n].c_str();
}

const char *FileMessage::Iterator::operator++()
{
    if (p->it == p->end())
        return NULL;
    const char *res = (*(p->it)).c_str();
    ++(p->it);
    return res;
}

void FileMessage::Iterator::reset()
{
    p->it = p->begin();
}

unsigned FileMessage::Iterator::count()
{
    return p->size();
}

static DataDef messageFileData[] =
    {
        { "File", DATA_UTF, 1, 0 },
        { "", DATA_UTF, 1, 0 },			// Description
        { "Size", DATA_ULONG, 1, 0 },
        { NULL, 0, 0, 0 }
    };

FileMessage::FileMessage(unsigned type, const char *cfg)
        : Message(type, cfg)
{
    load_data(messageFileData, &data, cfg);
    m_transfer = NULL;
}

FileMessage::~FileMessage()
{
    free_data(messageFileData, &data);
    if (m_transfer)
        delete m_transfer;
}

unsigned FileMessage::getSize()
{
    if (data.Size)
        return data.Size;
    Iterator it(*this);
    const char *name;
    while ((name = ++it) != NULL){
        QFile f(QFile::decodeName(name));
        if (!f.exists())
            continue;
        data.Size += f.size();
    }
    return data.Size;
}

void FileMessage::setSize(unsigned size)
{
    data.Size = size;
}

QString FileMessage::getDescription()
{
    if (data.Description && *data.Description)
        return QString::fromUtf8(data.Description);
    Iterator it(*this);
    if (it.count() <= 1){
        const char *name = ++it;
        if (name == NULL)
            return NULL;
        const char *short_name;
#ifdef WIN32
        short_name = strrchr(name, '\\');
#else
        short_name = strchr(name, '/');
#endif
        if (short_name){
            short_name++;
        }else{
            short_name = name;
        }
        return QFile::decodeName(short_name);
    }
    return QString("%1 files") .arg(it.count());
}

void FileMessage::setDescription(const QString &str)
{
    set_str(&data.Description, str.utf8());
}

string FileMessage::save()
{
    string s = Message::save();
    string s1 = save_data(messageFileData, &data);
    if (!s1.empty()){
        if (!s.empty())
            s += '\n';
        s += s1;
    }
    return s;
}

QString FileMessage::presentation()
{
    QString res = getFile();
    res += " ";
    unsigned size = getSize();
    if (size >= 1024 * 1024){
        res += i18n("%1 Mbytes") .arg(size / (1024 * 1024));
    }else if (size >= 1024){
        res += i18n("%1 kbytes") .arg(size / 1024);
    }else{
        res += i18n("%1 bytes") .arg(size);
    }
    QString descr = getDescription();
    if (descr.length()){
        res += "<br>";
        res += descr;
    }
    return res;
}

FileTransfer::FileTransfer(FileMessage *msg)
{
    m_msg		 = msg;
    m_notify	 = NULL;
    m_file		 = NO_FILE;
    m_files		 = 0;
    m_bytes		 = 0;
    m_totalBytes = 0;
    m_fileSize	 = 0;
    m_totalSize	 = 0;
    m_speed		 = 100;
    m_state		 = Unknown;
    m_transferBytes = 0;
    if (msg->m_transfer)
        delete msg->m_transfer;
    msg->m_transfer = this;
}

FileTransfer::~FileTransfer()
{
    setNotify(NULL);
    m_msg->m_transfer = NULL;
}

void FileTransfer::setSpeed(unsigned speed)
{
    m_speed = speed;
}

void FileTransfer::setNotify(FileTransferNotify *notify)
{
    if (m_notify)
        delete m_notify;
    m_notify = notify;
}

QString AuthMessage::presentation()
{
    return "";
}

static DataDef messageStatusData[] =
    {
        { "Status", DATA_ULONG, 1, STATUS_UNKNOWN },
        { NULL, 0, 0, 0 }
    };

StatusMessage::StatusMessage(const char *cfg)
        : Message(MessageStatus, cfg)
{
    load_data(messageStatusData, &data, cfg);
}

string StatusMessage::save()
{
    string s = Message::save();
    string s1 = save_data(messageStatusData, &data);
    if (!s1.empty()){
        if (!s.empty())
            s += '\n';
        s += s1;
    }
    return s;
}

QString StatusMessage::presentation()
{
    return "";
}

};

