/***************************************************************************
                          client.cpp  -  description
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

#include "client.h"
#include "history.h"
#include "mainwin.h"
#include "chatwnd.h"
#include "log.h"
#include "ui/filetransfer.h"

#include <qsocketnotifier.h>
#include <qdns.h>
#include <qtimer.h>
#include <qstring.h>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <qtextcodec.h>

#include <string>

#ifdef USE_KDE
#include <kglobal.h>
#include <kcharsets.h>
#endif

class SocketNotifier
{
public:
    SocketNotifier(int fd, Client *client);
    ~SocketNotifier();
    void setHaveData(bool bHaveData);
protected:
    QSocketNotifier *readNotifier;
    QSocketNotifier *writeNotifier;
    QSocketNotifier *errorNotifier;
};

SocketNotifier::SocketNotifier(int fd, Client *client)
{
    readNotifier = new QSocketNotifier(fd, QSocketNotifier::Read, client);
    writeNotifier = new QSocketNotifier(fd, QSocketNotifier::Write, client);
    errorNotifier = new QSocketNotifier(fd, QSocketNotifier::Exception, client);
    writeNotifier->setEnabled(false);
    QObject::connect(readNotifier, SIGNAL(activated(int)), client, SLOT(data_ready(int)));
    QObject::connect(writeNotifier, SIGNAL(activated(int)), client, SLOT(data_ready(int)));
    QObject::connect(errorNotifier, SIGNAL(activated(int)), client, SLOT(data_ready(int)));
}

SocketNotifier::~SocketNotifier()
{
    delete readNotifier;
    delete writeNotifier;
    delete errorNotifier;
}

void SocketNotifier::setHaveData(bool bHaveData)
{
    writeNotifier->setEnabled(bHaveData);
}

void Client::createSocket(Socket *s)
{
    s->intData = new SocketNotifier(s->m_fd, this);
    Sockets::createSocket(s);
}

void Client::closeSocket(Socket *s)
{
    if (s->intData){
        delete (SocketNotifier*)(s->intData);
        s->intData = NULL;
    }
    Sockets::closeSocket(s);
}

void Client::setHaveData(Socket *s)
{
    if (s->intData)
        ((SocketNotifier*)(s->intData))->setHaveData(s->have_data());
}

#ifndef USE_KDE

typedef struct encoding
{
    const char *language;
    const char *codec;
} encoding;

encoding encodingTbl[] =
    {
        { I18N_NOOP("Unicode"), "UTF-8" },
        { I18N_NOOP("Western European"), "ISO 8859-1" },
        { I18N_NOOP("Western European"), "ISO 8859-15" },
        { I18N_NOOP("Western European"), "CP 1252" },
        { I18N_NOOP("Central European"), "CP 1250" },
        { I18N_NOOP("Central European"), "ISO 8859-2" },
        { I18N_NOOP("Esperanto"), "ISO 8859-3" },
        { I18N_NOOP("Baltic"), "ISO 8859-13" },
        { I18N_NOOP("Baltic"), "CP 1257" },
#ifdef WIN32
        { I18N_NOOP("Cyrillic"), "CP 1251" },
        { I18N_NOOP("Ukrainan"), "CP 1251" },
#else
        { I18N_NOOP("Cyrillic"), "KOI8-R" },
        { I18N_NOOP("Ukrainan"), "KOI8-U" },
#endif
        { I18N_NOOP("Arabic"), "ISO 8859-6-I" },
        { I18N_NOOP("Greek"), "ISO 8859-7" },
        { I18N_NOOP("Hebrew"), "ISO 8859-8-I" },
        { I18N_NOOP("Chinese Traditional"), "Big5" },
        { I18N_NOOP("Chinese Simplified"), "gbk" },
        { I18N_NOOP("Chinese Simplified"), "gbk2312" },
        { I18N_NOOP("Turkish"), "ISO 8859-9" },
        { I18N_NOOP("Turkish"), "CP 1254" },
    };

#endif

Client::Client(QObject *parent, const char *name)
        : QObject(parent, name)
{
    QTimer *timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(data_ready()));
    timer->start(1000);
    resolver.setRecordType(QDns::A);
    ptrResolver.setRecordType(QDns::Ptr);
    QObject::connect(&resolver, SIGNAL(resultsReady()), this, SLOT(resolve_ready()));
    QObject::connect(&ptrResolver, SIGNAL(resultsReady()), this, SLOT(ptr_resolve_ready()));
    encodings = new QStringList;
#ifdef USE_KDE
    *encodings = KGlobal::charsets()->descriptiveEncodingNames();
#else
    for (uint i=0; i< (sizeof(encodingTbl) / sizeof(encoding)); i++) {
        (*encodings).append(i18n(encodingTbl[i].language) + " ( " + encodingTbl[i].codec + " )");
    }
#endif
}

Client::~Client()
{
    close();
    delete encodings;
}

void Client::markAsRead(ICQMessage *msg)
{
    ICQUser *u = getUser(msg->getUin());
    if (u == NULL) return;
    for (list<unsigned long>::iterator it = u->unreadMsgs.begin(); it != u->unreadMsgs.end(); it++){
        if (*it == msg->Id){
            u->unreadMsgs.remove(*it);
            emit messageRead(msg);
            return;
        }
    }
}

void Client::process_event(ICQEvent *e)
{
    switch (e->type()){
    case EVENT_DONE:{
            ICQMessage *msg = e->message();
            if (msg){
                for (ConfigULongs::iterator it = msg->Uin.begin(); it != msg->Uin.end(); ++it){
                    ICQUser *u = getUser(*it);
                    if (u == NULL) continue;
                    History h(*it);
                    unsigned long id = h.addMessage(msg);
                    if (msg->Id == 0) msg->Id = id;
                }
            }
            break;
        }
    case EVENT_ACKED:
        if (e->message() == NULL){
            log(L_WARN, "Ack without message");
            break;
        }
        switch (e->message()->Type()){
        case ICQ_MSGxFILE:{
                FileTransferDlg *dlg = new FileTransferDlg(NULL, static_cast<ICQFile*>(e->message()));
                dlg->show();
                break;
            }
        case ICQ_MSGxCHAT:{
                ChatWindow *chatWnd = new ChatWindow(static_cast<ICQChat*>(e->message()));
                chatWnd->show();
                break;
            }
        default:
            log(L_WARN, "Unknown message type acked");
        }
        break;
    case EVENT_INFO_CHANGED:{
            ICQUser *u = getUser(e->Uin());
            if (u && u->inIgnore()){
                History h(e->Uin());
                h.remove();
            }
            break;
        }
    case EVENT_USER_DELETED:{
            History h(e->Uin());
            h.remove();
            break;
        }
    case EVENT_MESSAGE_RECEIVED:{
            ICQMessage *msg = e->message();
            if (msg == NULL){
                log(L_WARN, "Message event without message");
                return;
            }
            unsigned long uin = msg->getUin();
            if (uin == Uin) uin = 0;
            if (e->state == ICQEvent::Fail){
                ICQUser *u = getUser(e->Uin());
                if (u){
                    list<unsigned long>::iterator it;
                    for (it = u->unreadMsgs.begin(); it != u->unreadMsgs.end(); ++it)
                        if ((*it) == msg->Id) break;
                    if (it != u->unreadMsgs.end()){
                        u->unreadMsgs.remove(msg->Id);
                        emit messageRead(msg);
                    }
                }
            }else{
                ICQUser *u = getUser(e->Uin(), true);
                if (msg->Id == 0){
                    History h(uin);
                    msg->Id = h.addMessage(msg);
                }else{
                    switch (msg->Type()){
                    case ICQ_MSGxFILE:{
                            ICQFile *f = static_cast<ICQFile*>(msg);
                            ICQUser *uFile = u;
                            if (!uFile->AcceptFileOverride()) uFile = pClient;
                            if (uFile->AcceptFileMode() == 1){
                                string name = uFile->AcceptFilePath.c_str();
                                if (*name.c_str() == 0)
                                    pMain->buildFileName(name, "IncomingFiles/");
#ifdef WIN32
                                if ((name.length() == 0) || (name[(int)(name.length() - 1)] != '\\'))
                                    name += "\\";
#else
                                if ((name.length() == 0) || (name[(int)(name.length() - 1)] != '/'))
                                    name += "/";
#endif
                                name += f->shortName();
                                f->localName = name;
                                f->autoAccept = true;
                                pClient->acceptMessage(f);
                                return;
                            }
                            if (uFile->AcceptFileMode() == 2){
                                pClient->declineMessage(f, uFile->DeclineFileMessage.c_str());
                                return;
                            }
                            switch (uStatus & 0xFF){
                            case ICQ_STATUS_ONLINE:
                            case ICQ_STATUS_FREEFORCHAT:
                                break;
                            default:
                                string response;
                                getAutoResponse(uin, response);
                                pClient->declineMessage(msg, response.c_str());
                                return;
                            }
                            break;
                        }
                    case ICQ_MSGxCHAT:
                        switch (uStatus & 0xFF){
                        case ICQ_STATUS_ONLINE:
                        case ICQ_STATUS_FREEFORCHAT:
                            break;
                        default:
                            string response;
                            getAutoResponse(uin, response);
                            pClient->declineMessage(msg, response.c_str());
                            return;
                        }
                        break;
                    default:
                        break;
                    }
                }
                u->unreadMsgs.push_back(msg->Id);
                emit messageReceived(msg);
            }
            break;
        }
    case EVENT_STATUS_CHANGED:{
            ICQUser *u = getUser(e->Uin());
            if (u){
                if (u->IP() && (u->HostName.size() == 0)){
                    resolveAddr a;
                    a.uin   = e->Uin();
                    a.bReal = false;
                    resolveQueue.push_back(a);
                }
                if (u->RealIP() && (u->RealHostName.size() == 0)){
                    resolveAddr a;
                    a.uin   = e->Uin();
                    a.bReal = true;
                    resolveQueue.push_back(a);
                }
                start_resolve();
            }
            break;
        }
    }
    emit event(e);
}

void Client::resolve()
{
    resolver.setLabel(m_szResolveHost);
}

void Client::ptr_resolve_ready()
{
    if (resolveQueue.empty()) return;
    resolveAddr &a = *resolveQueue.begin();
    ICQUser *u = getUser(a.uin);
    if (u && ptrResolver.hostNames().count()){
        if (a.bReal){
            u->RealHostName = ptrResolver.hostNames().first().latin1();
        }else{
            u->HostName = ptrResolver.hostNames().first().latin1();
        }
        ICQEvent e(EVENT_STATUS_CHANGED, a.uin);
        emit event(&e);
    }
    resolveQueue.erase(resolveQueue.begin());
    start_resolve();
}

void Client::start_resolve()
{
    if (ptrResolver.isWorking()) return;
    for (; !resolveQueue.empty();){
        resolveAddr &a = *resolveQueue.begin();
        ICQUser *u = getUser(a.uin);
        if (u == NULL){
            resolveQueue.erase(resolveQueue.begin());
            continue;
        }
        unsigned long ip;
        if (a.bReal){
            ip = u->RealIP();
        }else{
            ip = u->IP();
        }
        if (ip == 0){
            resolveQueue.erase(resolveQueue.begin());
            continue;
        }
        ptrResolver.setLabel(QHostAddress(htonl(ip)));
        return;
    }
}

void Client::resolve_ready()
{
    if (m_resolving != Resolving) return;
    free(m_szResolveHost);
    m_szResolveHost = NULL;
    m_remoteAddr = (unsigned long)(-1);
    m_resolving = Done;
    const QValueList<QHostAddress> &hosts = resolver.addresses();
    if (hosts.isEmpty()) return;
    m_remoteAddr = htonl(hosts.first().ip4Addr());
}

void Client::data_ready()
{
    process(0);
}

void Client::data_ready(int)
{
    data_ready();
}

QString Client::getName(bool bUseUin)
{
    return QString::fromLocal8Bit(ICQUser::name(bUseUin).c_str());
}

const char *Client::getMessageIcon(int type)
{
    switch (type){
    case ICQ_MSGxMSG:
        return "message";
    case ICQ_MSGxCHAT:
        return "chat";
    case ICQ_MSGxFILE:
        return "file";
    case ICQ_MSGxURL:
        return "url";
    case ICQ_MSGxAUTHxREQUEST:
    case ICQ_MSGxAUTHxREFUSED:
    case ICQ_MSGxAUTHxGRANTED:
        return "auth";
    case ICQ_MSGxADDEDxTOxLIST:
        return "added";
    case ICQ_MSGxWEBxPANEL:
        return "web";
    case ICQ_MSGxEMAILxPAGER:
        return "pager";
    case ICQ_MSGxCONTACTxLIST:
    case ICQ_MSGxCONTACTxREQUEST:
        return "contacts";
    case ICQ_MSGxEXT:
        return "message";
    case ICQ_MSGxMAIL:
        return "mail_generic";
    case ICQ_MSGxSMS:
    case ICQ_MSGxSMSxRECEIPT:
        return "sms";
    case ICQ_MSGxSECURExOPEN:
    case ICQ_MSGxSECURExCLOSE:
        return "encrypted";
    default:
        log(L_WARN, "Unknown message type %u", type);
    }
    return "message";
}

QString Client::getMessageText(int type, int n)
{
    switch (type){
    case ICQ_MSGxMSG:
        return i18n("Message", "%n messages", n);
    case ICQ_MSGxCHAT:
        return i18n("Chat request", "%n chat requests", n);
    case ICQ_MSGxFILE:
        return i18n("File", "%n files", n);
    case ICQ_MSGxURL:
        return i18n("URL", "%n URL", n);
    case ICQ_MSGxAUTHxREQUEST:
        return i18n("Authorization request", "%n authorization requests", n);
    case ICQ_MSGxAUTHxREFUSED:
        return i18n("Authorization refused", "%n authorizations refused", n);
    case ICQ_MSGxAUTHxGRANTED:
        return i18n("Authorization granted", "%n authorizations granted", n);
    case ICQ_MSGxADDEDxTOxLIST:
        return i18n("Added to contact list", "%n times added to contact list", n);
    case ICQ_MSGxWEBxPANEL:
        return i18n("WWW-panel message", "%n WWW-panel messages", n);
    case ICQ_MSGxEMAILxPAGER:
        return i18n("Email pager message", "%n Email pager messages", n);
    case ICQ_MSGxCONTACTxLIST:
        return i18n("Contact list", "%n contact lists", n);
    case ICQ_MSGxCONTACTxREQUEST:
        return i18n("Contacts request", "%n contacts requests", n);
    case ICQ_MSGxEXT:
        return i18n("External unknown message", "%n unknown messages", n);
    case ICQ_MSGxSMS:
        return i18n("SMS", "%n SMS", n);
    case ICQ_MSGxSMSxRECEIPT:
        return i18n("SMS receipt", "%n SMS receipts", n);
    case ICQ_MSGxMAIL:
        return i18n("Email", "%n emails", n);
    case ICQ_MSGxSECURExOPEN:
        return i18n("Request secure channel", "%n requests sequre channel", n);
    case ICQ_MSGxSECURExCLOSE:
        return i18n("Close secure channel", "%n times close sequre channel", n);
    default:
        log(L_WARN, "Unknown message type %u", type);
    }
    return i18n("Unknown message", "%n unknown messages", n);
}

const char *Client::getUserIcon(ICQUser *u)
{
    switch (u->Type()){
    case USER_TYPE_EXT:
        return "nonim";
    }
    return getStatusIcon(u->uStatus);
}

const char *Client::getStatusIcon(unsigned long status)
{
    if ((status & 0xFFFF) == ICQ_STATUS_OFFLINE) return "offline";
    switch (status & 0xFF){
    case ICQ_STATUS_ONLINE:
        return "online";
    case ICQ_STATUS_AWAY:
        return "away";
    case ICQ_STATUS_DND:
        return "dnd";
    case ICQ_STATUS_OCCUPIED:
        return "occupied";
    case ICQ_STATUS_FREEFORCHAT:
        return "ffc";
    default:
        return "na";
    }
    return "online";
}

QString Client::getStatusText(unsigned long status)
{
    if ((status & 0xFFFF) == ICQ_STATUS_OFFLINE) return i18n("Offline");
    switch (status & 0xFF){
    case ICQ_STATUS_ONLINE:
        return i18n("Online");
    case ICQ_STATUS_AWAY:
        return i18n("Away");
    case ICQ_STATUS_DND:
        return i18n("Do not disturb");
    case ICQ_STATUS_OCCUPIED:
        return i18n("Occupied");
    case ICQ_STATUS_FREEFORCHAT:
        return i18n("Free for chat");
    default:
        return i18n("N/A");
    }
    return i18n("Online");
}

const char *Client::getStatusIcon()
{
    if (((uStatus && 0xFF) == ICQ_STATUS_ONLINE) && inInvisible())
        return "invisible";
    return getStatusIcon(uStatus);
}

QString Client::getStatusText()
{
    if (((uStatus && 0xFF) == ICQ_STATUS_ONLINE) && inInvisible())
        return i18n("Invisible");
    return getStatusText(uStatus);
}

unsigned long Client::getFileSize(const char *name)
{
    QFileInfo fInfo(name);
    return fInfo.size();
}

bool Client::createFile(ICQFile *f, int mode)
{
    bool bTruncate = false;
    int size = 0;
    QFile *file = new QFile(QString::fromLocal8Bit(f->localName.c_str()));
    QFileInfo info(*file);
    if (info.exists()){
        size = info.size();
        if ((mode == FT_REPLACE) || (mode == FT_RESUME)){
            if ((mode == FT_REPLACE) || (info.size() > f->Size())){
                bTruncate = true;
                size = 0;
            }
        }else if (f->autoAccept){
            ICQUser *u = getUser(f->getUin());
            if ((u == NULL) || !u->AcceptFileOverride()) u = this;
            if (u->AcceptFileOverwrite() || (info.size() > f->Size())){
                bTruncate = true;
                size = 0;
            }
        }else{
            emit fileExist(f, info.size() < f->Size());
            f->wait = true;
            return false;
        }
    }
    if (!file->open(IO_WriteOnly | (bTruncate ? IO_Truncate : 0))){
        delete file;
        return false;
    }
    file->at(size);
    f->state = size;
    f->p = (unsigned long)file;
    return true;
}

bool Client::openFile(ICQFile *f)
{
    QFile *file = new QFile(QString::fromLocal8Bit(f->Name.c_str()));
    if (!file->open(IO_ReadOnly)){
        delete file;
        return false;
    }
    f->p = (unsigned long)file;
    return true;
}

bool Client::seekFile(ICQFile *f, unsigned long pos)
{
    emit fileProcess(f);
    return ((QFile*)(f->p))->at(pos);
}

bool Client::readFile(ICQFile *f, Buffer &b, unsigned short size)
{
    b.allocate(b.writePos() + size, 1024);
    char *p = b.Data(b.writePos());
    int readn = ((QFile*)(f->p))->readBlock(p, size);
    if (readn < 0) return false;
    b.setWritePos(b.writePos() + readn);
    emit fileProcess(f);
    return true;
}

bool Client::writeFile(ICQFile *f, Buffer &b)
{
    unsigned size = b.size() - b.readPos();
    char *p = b.Data(b.readPos());
    bool res = (((QFile*)(f->p))->writeBlock(p, size) >= 0);
    emit fileProcess(f);
    return res;
}

void Client::closeFile(ICQFile *f)
{
    if (f->p) delete (QFile*)(f->p);
    f->p = 0;
}

QTextCodec *Client::codecForUser(unsigned long uin)
{
    ICQUser *u = getUser(uin);
    if (u){
        if (u->Encoding.c_str()){
            QTextCodec *res = QTextCodec::codecForName(u->Encoding.c_str());
            if (res) return res;
        }
    }
    return QTextCodec::codecForLocale();
}

string Client::to8Bit(unsigned long uin, const QString &str)
{
    return to8Bit(codecForUser(uin), str);
}

QString Client::from8Bit(unsigned long uin, const string &str)
{
    return from8Bit(codecForUser(uin), str);
}

string Client::to8Bit(QTextCodec *codec, const QString &str)
{
    int lenOut = str.length();
	string res;
	if (lenOut == 0) return res;
    res = (const char*)(codec->makeEncoder()->fromUnicode(str, lenOut));
    toServer(res, codec->name());
    return res;
}

QString Client::from8Bit(QTextCodec *codec, const string &str)
{
    if (!strcmp(codec->name(), serverCharset(codec->name())))
        return codec->makeDecoder()->toUnicode(str.c_str(), str.size());
    string s = str;
    fromServer(s);
    return codec->makeDecoder()->toUnicode(s.c_str(), s.size());
}

void Client::setUserEncoding(unsigned long uin, int i)
{
    if (userEncoding(uin) == i) return;
    ICQUser *u = getUser(uin);
    if (u == NULL) return;
    QString name;
    if (i > 0){
        name = (*encodings)[i-1];
        int left = name.find(" ( ");
        if (left >= 0) name = name.mid(left + 3);
        int right = name.find(" )");
        if (right >= 0) name = name.left(right);
	log(L_DEBUG, "Set encoding for %lu - [%s]", uin, (const char*)name.latin1());
        u->Encoding = name.latin1();
    }
    emit encodingChanged(uin);
    ICQEvent e(EVENT_INFO_CHANGED, uin);
    process_event(&e);
}

int Client::userEncoding(unsigned long uin)
{
    QTextCodec *codec = codecForUser(uin);
    int n = 1;
    for (QStringList::Iterator it = encodings->begin(); it != encodings->end(); ++it, n++){
        QString name = *it;
        int left = name.find(" ( ");
        if (left >= 0) name = name.mid(left + 3);
        int right = name.find(" )");
        if (right >= 0) name = name.left(right);
        if (!strcasecmp(name.latin1(), codec->name()))
            return n;
    }
    return 0;
}

Client *pClient = NULL;

#ifndef _WINDOWS
#include "client.moc"
#endif
