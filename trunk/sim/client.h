/***************************************************************************
                          client.h  -  description
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

#ifndef _CLIENT_H
#define _CLIENT_H

#include "defs.h"
#include "cfg.h"

#include "icqclient.h"

class QDns;
class QSocket;
class QSocketDevice;
class QSocketNotifier;

class QStringList;
class QTextCodec;
class QPopupMenu;

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

class SMSmessage
{
public:
    QString		str;
    ICQSMS		*msg;
    QString chunk(const QString &s, int len);
    bool isLatin1(const QString &s);
    string smsChunk();
    QString trim(const QString &s);
};

class Tmpl;

typedef struct autoResponse
{
    Tmpl		*tmpl;
    ICQMessage	*msg;
    bool		bDelete;
} autoResponse;

inline bool operator == (const autoResponse &a, const autoResponse &b)
{
    return (a.tmpl == b.tmpl) && (a.msg == b.msg);
}

class SIMGroup : public ICQGroup
{
public:
    bool				Expand;
};

class SIMUser : public ICQUser
{
public:
    // Alert mode
    bool				AlertOverride;
    bool				AlertAway;
    bool				AlertBlink;
    bool				AlertSound;
    bool				AlertOnScreen;
    bool				AlertPopup;
    bool				AlertWindow;
    bool				LogStatus;

    // Accept mode
    bool				AcceptMsgWindow;
    unsigned short		AcceptFileMode;
    bool				AcceptFileOverride;
    bool				AcceptFileOverwrite;
    string				AcceptFilePath;
    string				DeclineFileMessage;

    // Sounds
    bool				SoundOverride;
    string				IncomingMessage;
    string				IncomingURL;
    string				IncomingSMS;
    string				IncomingAuth;
    string				IncomingFile;
    string				IncomingChat;
    string				OnlineAlert;

    // Programms
    bool				ProgOverride;
    bool				ProgMessageOn;
    string				ProgMessage;

    // corresponding KAddressBook identifier
    string				strKabUid;

    // User notes
    string				Notes;
public:
    bool notEnoughInfo(void);
};

class Tmpl;
class Exec;
class MsgFilter;

class SIMClient : public QObject, public ICQClient
{
    Q_OBJECT
public:
    SIMClient(QObject *parent = NULL, const char *name = NULL);
    ~SIMClient();
    string			BirthdayReminder;
    string			FileDone;
    ICQUser			*createUser();
    ICQGroup		*createGroup();
    void init();
    void save();
    bool load(unsigned long ownerUin);
    QString getName(bool bUserUIN=true);
    const char *getStatusIcon();
    QString getStatusText();
    static const char *getUserIcon(ICQUser*);
    static QString getUserText(ICQUser*);
    static const char *getStatusIcon(unsigned long);
    static QString getStatusText(unsigned long);
    static const char *getMessageIcon(int type);
    static QString getMessageText(int type, int n);
    static QString getMessageAccel(int type);
    bool markAsRead(ICQMessage *msg);
    virtual void process_event(ICQEvent *e);
    void setUserEncoding(unsigned long uin, int i);
    int userEncoding(unsigned long uin);
    QString encodingName(int mib);
    int encodingMib(const QString&);
    QTextCodec *codecForUser(unsigned long uin);
    string to8Bit(unsigned long uin, const QString&);
    QString from8Bit(unsigned long uin, const string&, const char *srcCharset=NULL);
    static string to8Bit(QTextCodec*, const QString&);
    static QString from8Bit(QTextCodec*, const string&, const char *srcCharset=NULL);
    void fillEncodings(QPopupMenu *menu, bool bMain);
    QStringList getEncodings(bool bMain);
    void createGroup(const char *name);
protected:
    void sendSMS(SMSmessage *sms);
    list<SMSmessage*> smsQueue;
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
    void tmpl_ready(Tmpl *tmpl, const QString &res);
    void tmpl_clear();
    void resolve_ready();
    void timer();
    void fltrTmplReady(Tmpl*, const QString &res);
    void fltrExecReady(Exec*, int res, const char *out);
    void fltrDone();
protected:
    void getAutoResponse(unsigned long uin, ICQMessage *msg);
    list<autoResponse> responses;
    list<MsgFilter*> filters;
    void start_resolve();
    virtual bool createFile(ICQFile *f, int mode);
    virtual bool openFile(ICQFile *f);
    virtual bool seekFile(ICQFile *f, unsigned long pos);
    virtual bool readFile(ICQFile *f, Buffer &b, unsigned short size);
    virtual bool writeFile(ICQFile *f, Buffer &b);
    virtual void closeFile(ICQFile *f);

    virtual unsigned long getFileSize(const char *name, vector<fileName> &files);
    unsigned long getFileSize(QString name, QString base, vector<fileName> &file);
    unsigned long getFileSize(QString name, vector<fileName> &file);

    friend class MsgFilter;
};

extern SIMClient *pClient;

#endif

