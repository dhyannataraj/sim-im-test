/***************************************************************************
                          client.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CLIENT_H
#define _CLIENT_H

#include "defs.h"

#include "icqclient.h"

class QDns;
class QSocket;
class QSocketDevice;
class QSocketNotifier;

class QStringList;
class QTextCodec;

typedef struct resolveAddr
{
    unsigned long uin;
    bool          bReal;
} resolveAddr;

typedef struct searchResult
{
    unsigned short id;
    unsigned short result;
    unsigned long uin;
    const char *nick;
    const char *first;
    const char *last;
    const char *email;
    bool auth;
} searchResult;

#ifdef USE_KDE
class KExtendedSocket;
#endif

class QClientSocket : public QObject, public Socket
{
    Q_OBJECT
public:
#ifdef HAVE_KEXTSOCK_H
    QClientSocket(KExtendedSocket *s=NULL);
#else
    QClientSocket(QSocket *s=NULL);
#endif
    virtual ~QClientSocket();
    virtual int read(char *buf, unsigned int size);
    virtual void write(const char *buf, unsigned int size);
    virtual void connect(const char *host, int port);
    virtual unsigned long localHost();
    virtual void pause(unsigned);
    virtual void close();
protected slots:
    void slotConnected();
    void slotConnectionClosed();
    void slotReadReady();
    void slotBytesWritten(int);
    void slotBytesWritten();
    void slotError(int);
protected:
#ifdef HAVE_KEXTSOCK_H
    KExtendedSocket *sock;
#else
    QSocket *sock;
#endif
    bool bInWrite;
};

class QServerSocket : public QObject, public ServerSocket
{
    Q_OBJECT
public:
    QServerSocket(unsigned short minPort, unsigned short maxPort);
    ~QServerSocket();
    virtual unsigned short port() { return m_nPort; }
    bool created() { return (sock != NULL); }
protected slots:
    void activated(int);
    void activated();
protected:
#ifdef HAVE_KEXTSOCK_H
    KExtendedSocket *sock;
#else
    QSocketDevice   *sock;
    QSocketNotifier *sn;
#endif
    unsigned short m_nPort;
};

class Client : public QObject, public ICQClient
{
    Q_OBJECT
public:
    Client(QObject *parent = NULL, const char *name = NULL);
    ~Client();

    ConfigUShort	MinTCPPort;
    ConfigUShort	MaxTCPPort;

    QString getName(bool bUserUIN=true);
    const char *getStatusIcon();
    QString getStatusText();
    static const char *getUserIcon(ICQUser*);
    static QString getUserText(ICQUser*);
    static const char *getStatusIcon(unsigned long);
    static QString getStatusText(unsigned long);
    static const char *getMessageIcon(int type);
    static QString getMessageText(int type, int n);
    void markAsRead(ICQMessage *msg);
    virtual void process_event(ICQEvent *e);
    QStringList *encodings;
    void setUserEncoding(unsigned long uin, int i);
    int userEncoding(unsigned long uin);
    QTextCodec *codecForUser(unsigned long uin);
    string to8Bit(unsigned long uin, const QString&);
    QString from8Bit(unsigned long uin, const string&, const char *srcCharset=NULL);
    static string to8Bit(QTextCodec*, const QString&);
    static QString from8Bit(QTextCodec*, const string&, const char *srcCharset=NULL);
protected:
    list<resolveAddr> resolveQueue;
    QDns *resolver;
signals:
    void event(ICQEvent*);
    void messageReceived(ICQMessage *msg);
    void messageRead(ICQMessage *msg);
    void fileProcess(ICQFile*);
    void fileExist(ICQFile*, const QString&, bool canResume);
    void fileNoCreate(ICQFile*, const QString&);
    void encodingChanged(unsigned long uin);
protected slots:
    void resolve_ready();
    void timer();
protected:
    virtual Socket *createSocket();
    virtual ServerSocket *createServerSocket();

    void start_resolve();

    virtual bool createFile(ICQFile *f, int mode);
    virtual bool openFile(ICQFile *f);
    virtual bool seekFile(ICQFile *f, unsigned long pos);
    virtual bool readFile(ICQFile *f, Buffer &b, unsigned short size);
    virtual bool writeFile(ICQFile *f, Buffer &b);
    virtual void closeFile(ICQFile *f);

    virtual unsigned long getFileSize(const char *name, int *nSrcFiles, vector<fileName> &files);
    unsigned long getFileSize(QString name, QString base, vector<fileName> &file);
    unsigned long getFileSize(QString name, vector<fileName> &file);
};

extern Client *pClient;

#endif

