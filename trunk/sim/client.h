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

#include <qdns.h>
#include "icqclient.h"

class QDns;
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

class Client : public QObject, public Sockets, public ICQClient
{
    Q_OBJECT
public:
    Client(QObject *parent = NULL, const char *name = NULL);
    ~Client();
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
    virtual void createSocket(Socket*);
    virtual void closeSocket(Socket*);
    virtual void setHaveData(Socket*);
    QStringList *encodings;
    void setUserEncoding(unsigned long uin, int i);
    int userEncoding(unsigned long uin);
    QTextCodec *codecForUser(unsigned long uin);
    string to8Bit(unsigned long uin, const QString&);
    QString from8Bit(unsigned long uin, const string&);
    static string to8Bit(QTextCodec*, const QString&);
    static QString from8Bit(QTextCodec*, const string&);
protected:
    QDns resolver;
    list<resolveAddr> resolveQueue;
    QDns ptrResolver;
    void resolve(unsigned long ip);
signals:
    void event(ICQEvent*);
    void messageReceived(ICQMessage *msg);
    void messageRead(ICQMessage *msg);
    void fileProcess(ICQFile*);
    void fileExist(ICQFile*, bool canResume);
    void encodingChanged(unsigned long uin);
protected slots:
    void ptr_resolve_ready();
    void resolve_ready();
    void data_ready();
    void data_ready(int);
protected:
    virtual bool createFile(ICQFile *f, int mode);
    virtual bool openFile(ICQFile *f);
    virtual bool seekFile(ICQFile *f, unsigned long pos);
    virtual bool readFile(ICQFile *f, Buffer &b, unsigned short size);
    virtual bool writeFile(ICQFile *f, Buffer &b);
    virtual void closeFile(ICQFile *f);

    void start_resolve();
    virtual void resolve();
    virtual unsigned long getFileSize(const char *name);
};

extern Client *pClient;

#endif

