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
#include "cfg.h"
#include "log.h"
#include "ui/filetransfer.h"

#include <qsocket.h>
#include <qsocketdevice.h>
#include <qsocketnotifier.h>
#include <qdns.h>
#include <qtimer.h>
#include <qstring.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qregexp.h>

#include <errno.h>

#include <string>

#undef HAVE_KEXTSOCK_H

#ifdef USE_KDE
#include <kglobal.h>
#include <kcharsets.h>
#endif

#ifdef HAVE_KEXTSOCK_H
#include <kextsock.h>
#include <ksockaddr.h>
#endif

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
#ifndef WIN32
        { I18N_NOOP("Cyrillic"), "KOI8-R" },
#endif
        { I18N_NOOP("Cyrillic"), "CP 1251" },
#ifndef WIN32
        { I18N_NOOP("Ukrainan"), "KOI8-U" },
#endif
        { I18N_NOOP("Ukrainan"), "CP 1251" },
        { I18N_NOOP("Arabic"), "ISO 8859-6-I" },
        { I18N_NOOP("Greek"), "ISO 8859-7" },
        { I18N_NOOP("Hebrew"), "ISO 8859-8-I" },
        { I18N_NOOP("Chinese Traditional"), "Big5" },
        { I18N_NOOP("Chinese Simplified"), "gbk" },
        { I18N_NOOP("Chinese Simplified"), "gbk2312" },
        { I18N_NOOP("Turkish"), "ISO 8859-9" },
        { I18N_NOOP("Turkish"), "CP 1254" },
        { I18N_NOOP("Korean"), "eucKR" },
        { I18N_NOOP("Japanese"), "eucJP" },
        { I18N_NOOP("Japanese"), "jis7" },
        { I18N_NOOP("Japanese"), "sjis7" },
        { I18N_NOOP("Tamil"), "tscii" },
        { I18N_NOOP("UTF8"), "utf8" },
        { I18N_NOOP("UTF16"), "utf16" }
    };

#endif

static void *createEMail()
{
    return new EMailInfo;
}

static void *createExtInfo()
{
    return new ExtInfo;
}

static void *createPhone()
{
    return new PhoneInfo;
}

cfgParam EMail_Params[] =
    {
        { "Email", offsetof(EMailInfo, Email), PARAM_STRING, 0 },
        { "Hide", offsetof(EMailInfo, Hide), PARAM_BOOL, 0 },
        { "MyInfo", offsetof(EMailInfo, MyInfo), PARAM_BOOL, 0 },
        { "", 0, 0, 0 }
    };

cfgParam ExtInfo_Params[] =
    {
        { "Category", offsetof(ExtInfo, Category), PARAM_USHORT, 0 },
        { "Specific", offsetof(ExtInfo, Specific), PARAM_STRING, 0 },
        { "", 0, 0, 0 }
    };

cfgParam PhoneInfo_Params[] =
    {
        { "Name", offsetof(PhoneInfo, Name), PARAM_STRING, 0 },
        { "Type", offsetof(PhoneInfo, Type), PARAM_USHORT, 0 },
        { "Active", offsetof(PhoneInfo, Active), PARAM_BOOL, 0 },
        { "Country", offsetof(PhoneInfo, Country), PARAM_STRING, 0 },
        { "AreaCode", offsetof(PhoneInfo, AreaCode), PARAM_STRING, 0 },
        { "Number", offsetof(PhoneInfo, Number), PARAM_STRING, 0 },
        { "Extension", offsetof(PhoneInfo, Extension), PARAM_STRING, 0 },
        { "Provider", offsetof(PhoneInfo, Provider), PARAM_STRING, 0 },
        { "Gateway", offsetof(PhoneInfo, Gateway), PARAM_STRING, 0 },
        { "Publish", offsetof(PhoneInfo, Publish), PARAM_BOOL, 0 },
        { "", 0, 0, 0 }
    };

cfgParam ICQContactList_Params[] =
    {
        { "Length", offsetof(ICQContactList, Len), PARAM_USHORT, 0 },
        { "Invsible", offsetof(ICQContactList, Invisible), PARAM_USHORT, 0 },
        { "Time", offsetof(ICQContactList, Time), PARAM_ULONG, 0 },
        { "Expand", offsetof(ICQContactList, Expand), PARAM_BOOL, 0 },
        { "", 0, 0, 0 }
    };

cfgParam ICQGroup_Params[] =
    {
        { "Name", offsetof(ICQGroup, Name), PARAM_STRING, 0 },
        { "Id", offsetof(ICQGroup, Id), PARAM_USHORT, 0 },
        { "Expand", offsetof(ICQGroup, Expand), PARAM_BOOL, 0 },
        { "", 0, 0, 0 }
    };

cfgParam ICQUser_Params[] =
    {
        { "Type", offsetof(ICQUser, Type), PARAM_USHORT, 0 },
        { "Alias", offsetof(ICQUser, Alias), PARAM_STRING, 0 },
        { "Id", offsetof(ICQUser, Id), PARAM_USHORT, 0 },
        { "GrpId", offsetof(ICQUser, GrpId), PARAM_USHORT, 0 },
        { "UIN", offsetof(ICQUser, Uin), PARAM_ULONG, 0 },
        { "Ignore", offsetof(ICQUser, inIgnore), PARAM_BOOL, 0 },
        { "Visible", offsetof(ICQUser, inVisible), PARAM_BOOL, 0 },
        { "Invisible", offsetof(ICQUser, inInvisible), PARAM_BOOL, 0 },
        { "WaitAuth", offsetof(ICQUser, WaitAuth), PARAM_BOOL, 0 },
        { "AutoResponseAway", offsetof(ICQUser, AutoResponseAway), PARAM_STRING, 0 },
        { "AutoResponseNA", offsetof(ICQUser, AutoResponseNA), PARAM_STRING, 0 },
        { "AutoResponseDND", offsetof(ICQUser, AutoResponseDND), PARAM_STRING, 0 },
        { "AutoResponseOccupied", offsetof(ICQUser, AutoResponseOccupied), PARAM_STRING, 0 },
        { "AutoResponseFFC", offsetof(ICQUser, AutoResponseFFC), PARAM_STRING, 0 },
        { "UnreadMessages", offsetof(ICQUser, unreadMsgs), PARAM_ULONGS, 0 },
        { "LastActive", offsetof(ICQUser, LastActive), PARAM_ULONG, 0 },
        { "OnlineTime", offsetof(ICQUser, OnlineTime), PARAM_ULONG, 0 },
        { "StatusTime", offsetof(ICQUser, StatusTime), PARAM_ULONG, 0 },
        { "IP", offsetof(ICQUser, IP), PARAM_ULONG, 0 },
        { "Port", offsetof(ICQUser, Port), PARAM_USHORT, 0 },
        { "RealIP", offsetof(ICQUser, RealIP), PARAM_ULONG, 0 },
        { "HostName", offsetof(ICQUser, HostName), PARAM_STRING, 0 },
        { "RealHostName", offsetof(ICQUser, RealHostName), PARAM_STRING, 0 },
        { "Version", offsetof(ICQUser, Version), PARAM_USHORT, 0 },
        { "Mode", offsetof(ICQUser, Mode), PARAM_USHORT, 0 },
        { "Nick", offsetof(ICQUser, Nick), PARAM_STRING, 0 },
        { "FirstName", offsetof(ICQUser, FirstName), PARAM_STRING, 0 },
        { "LastName", offsetof(ICQUser, LastName), PARAM_STRING, 0 },
        { "City", offsetof(ICQUser, City), PARAM_STRING, 0 },
        { "State", offsetof(ICQUser, State), PARAM_STRING, 0 },
        { "Address", offsetof(ICQUser, Address), PARAM_STRING, 0 },
        { "Zip", offsetof(ICQUser, Zip), PARAM_STRING, 0 },
        { "Country", offsetof(ICQUser, Country), PARAM_USHORT, 0 },
        { "TimeZone", offsetof(ICQUser, TimeZone), PARAM_CHAR, 0 },
        { "HomePhone", offsetof(ICQUser, HomePhone), PARAM_STRING, 0 },
        { "HomeFax", offsetof(ICQUser, HomeFax), PARAM_STRING, 0 },
        { "PrivateCellular", offsetof(ICQUser, PrivateCellular), PARAM_STRING, 0 },
        { "EMailInfo", offsetof(ICQUser, EMail), PARAM_STRING, 0 },
        { "HiddenEMail", offsetof(ICQUser, HiddenEMail), PARAM_BOOL, 0 },
        { "Notes", offsetof(ICQUser, Notes), PARAM_STRING, 0 },
        { "EMail", offsetof(ICQUser, EMails), (unsigned)createEMail, (unsigned)EMail_Params },
        { "Age", offsetof(ICQUser, Age), PARAM_CHAR, 0 },
        { "Gender", offsetof(ICQUser, Gender), PARAM_CHAR, 0 },
        { "Homepage", offsetof(ICQUser, Homepage), PARAM_STRING, 0 },
        { "BirthYear", offsetof(ICQUser, BirthYear), PARAM_USHORT, 0 },
        { "BirthMonth", offsetof(ICQUser, BirthMonth), PARAM_CHAR, 0 },
        { "BirthDay", offsetof(ICQUser, BirthDay), PARAM_CHAR, 0 },
        { "Language1", offsetof(ICQUser, Language1), PARAM_CHAR, 0 },
        { "Language2", offsetof(ICQUser, Language2), PARAM_CHAR, 0 },
        { "Language3", offsetof(ICQUser, Language3), PARAM_CHAR, 0 },
        { "WorkCity", offsetof(ICQUser, WorkCity), PARAM_STRING, 0 },
        { "WorkState", offsetof(ICQUser, WorkState), PARAM_STRING, 0 },
        { "WorkZip", offsetof(ICQUser, WorkZip), PARAM_STRING, 0 },
        { "WorkAddress", offsetof(ICQUser, WorkAddress), PARAM_STRING, 0 },
        { "WorkName", offsetof(ICQUser, WorkName), PARAM_STRING, 0 },
        { "WorkDepartment", offsetof(ICQUser, WorkDepartment), PARAM_STRING, 0 },
        { "WorkPosition", offsetof(ICQUser, WorkPosition), PARAM_STRING, 0 },
        { "WorkCountry", offsetof(ICQUser, WorkCountry), PARAM_USHORT, 0 },
        { "Occupation", offsetof(ICQUser, Occupation), PARAM_USHORT, 0 },
        { "WorkHomepage", offsetof(ICQUser, WorkHomepage), PARAM_STRING, 0 },
        { "WorkPhone", offsetof(ICQUser, WorkPhone), PARAM_STRING, 0 },
        { "WorkFax", offsetof(ICQUser, WorkFax), PARAM_STRING, 0 },
        { "About", offsetof(ICQUser, About), PARAM_STRING, 0 },
        { "Background", offsetof(ICQUser, Backgrounds), (unsigned)createExtInfo, (unsigned)ExtInfo_Params },
        { "Affilation", offsetof(ICQUser, Affilations), (unsigned)createExtInfo, (unsigned)ExtInfo_Params },
        { "Interest", offsetof(ICQUser, Interests), (unsigned)createExtInfo, (unsigned)ExtInfo_Params },
        { "Phone", offsetof(ICQUser, Phones), (unsigned)createPhone, (unsigned)PhoneInfo_Params },
        { "PhoneState", offsetof(ICQUser, PhoneState), PARAM_CHAR, 0 },
        { "PhoneBookTime", offsetof(ICQUser, PhoneBookTime), PARAM_ULONG, 0 },
        { "PhoneStatusTime", offsetof(ICQUser, PhoneStatusTime), PARAM_ULONG, 0 },
        { "InfoUpdateTime", offsetof(ICQUser, InfoUpdateTime), PARAM_ULONG, 0 },
        { "AlertOverride", offsetof(ICQUser, AlertOverride), PARAM_BOOL, 0 },
        { "AlertAway", offsetof(ICQUser, AlertAway), PARAM_BOOL, 1 },
        { "AlertBlink", offsetof(ICQUser, AlertBlink), PARAM_BOOL, 1 },
        { "AlertSound", offsetof(ICQUser, AlertSound), PARAM_BOOL, 1 },
        { "AlertOnScreen", offsetof(ICQUser, AlertOnScreen), PARAM_BOOL, 1 },
        { "AlertPopup", offsetof(ICQUser, AlertPopup), PARAM_BOOL, 0 },
        { "AlertWindow", offsetof(ICQUser, AlertWindow), PARAM_BOOL, 0 },
        { "AcceptMsgWindow", offsetof(ICQUser, AcceptMsgWindow), PARAM_BOOL, 0 },
        { "AcceptFileMode", offsetof(ICQUser, AcceptFileMode), PARAM_USHORT, 0 },
        { "AcceptFileOverride", offsetof(ICQUser, AcceptFileOverride), PARAM_BOOL, 0 },
        { "AcceptFileOverwrite", offsetof(ICQUser, AcceptFileOverwrite), PARAM_BOOL, 0 },
        { "AcceptFilePath", offsetof(ICQUser, AcceptFilePath), PARAM_STRING, 0 },
        { "DeclineFileMessage", offsetof(ICQUser, DeclineFileMessage), PARAM_STRING, 0 },
        { "ClientType", offsetof(ICQUser, ClientType), PARAM_ULONG, 0 },
        { "SoundOverride", offsetof(ICQUser, SoundOverride), PARAM_BOOL, 0 },
        { "IncomingMessage", offsetof(ICQUser, IncomingMessage), PARAM_STRING, (unsigned)"message.wav" },
        { "IncomingURL", offsetof(ICQUser, IncomingURL), PARAM_STRING, (unsigned)"url.wav" },
        { "IncomingSMS", offsetof(ICQUser, IncomingSMS), PARAM_STRING, (unsigned)"sms.wav" },
        { "IncomingAuth", offsetof(ICQUser, IncomingAuth), PARAM_STRING, (unsigned)"auth.wav" },
        { "IncomingFile", offsetof(ICQUser, IncomingFile), PARAM_STRING, (unsigned)"file.wav" },
        { "IncomingChat", offsetof(ICQUser, IncomingChat), PARAM_STRING, (unsigned)"chat.wav" },
        { "OnlineAlert", offsetof(ICQUser, OnlineAlert), PARAM_STRING, (unsigned)"alert.wav" },
        { "Encoding", offsetof(ICQUser, Encoding), PARAM_STRING, 0 },
        { "", 0, 0, 0 }
    };

cfgParam ClientOwner_Params[] =
    {
        { "AutoResponseAway" ,offsetof(ICQUser, AutoResponseAway), PARAM_I18N, (unsigned)I18N_NOOP(
              "I am currently away from ICQ.\n"
              "Please leave your message and I will get back to you as soon as I return!\n"
          ) },
        { "AutoResponseNA" ,offsetof(ICQUser, AutoResponseNA), PARAM_I18N, (unsigned)I18N_NOOP(
              "I am out'a here.\n"
              "See you tomorrow!\n"
          ) },
        { "AutoResponseDND" ,offsetof(ICQUser, AutoResponseDND), PARAM_I18N, (unsigned)I18N_NOOP(
              "Please do not disturb me now. Disturb me later.\n"
          ) },
        { "AutoResponseOccupied" ,offsetof(ICQUser, AutoResponseOccupied), PARAM_I18N, (unsigned)I18N_NOOP(
              "Please do not disturb me now.\n"
              "Disturb me later.\n"
              "Only urgent messages, please!\n") },
        { "AutoResponseFFC" ,offsetof(ICQUser, AutoResponseFFC), PARAM_I18N, (unsigned)I18N_NOOP(
              "We'd love to hear what you have to say. Join our chat.\n"
          ) },
        { "", 0, 0, (unsigned)ICQUser_Params }
    };

cfgParam Client_Params[] =
    {
        { "MinTCPPort", offsetof(ICQClient, MinTCPPort), PARAM_USHORT, 1024 },
        { "MaxTCPPort", offsetof(ICQClient, MaxTCPPort), PARAM_USHORT, 0xFFFF },
        { "ServerHost", offsetof(ICQClient, ServerHost), PARAM_STRING, (unsigned)"login.icq.com" },
        { "ServerPort", offsetof(ICQClient, ServerPort), PARAM_USHORT, 5190 },
        { "Password", offsetof(ICQClient, DecryptedPassword), PARAM_STRING, 0 },
        { "EncryptPassword", offsetof(ICQClient, EncryptedPassword), PARAM_STRING, 0 },
        { "WebAware", offsetof(ICQClient, WebAware), PARAM_BOOL, 0 },
        { "Authorize", offsetof(ICQClient, Authorize), PARAM_BOOL, 0 },
        { "HideIp", offsetof(ICQClient, HideIp), PARAM_BOOL, 0 },
        { "RejectMessage", offsetof(ICQClient, RejectMessage), PARAM_BOOL, 0 },
        { "RejectURL", offsetof(ICQClient, RejectURL), PARAM_BOOL, 0 },
        { "RejectWeb", offsetof(ICQClient, RejectWeb), PARAM_BOOL, 0 },
        { "RejectEmail", offsetof(ICQClient, RejectEmail), PARAM_BOOL, 0 },
        { "RejectOther", offsetof(ICQClient, RejectOther), PARAM_BOOL, 0 },
        { "RejectFilter", offsetof(ICQClient, RejectFilter), PARAM_STRING, 0 },
        { "DirectMode", offsetof(ICQClient, DirectMode), PARAM_USHORT, 0 },
        { "BirthdayReminder", offsetof(ICQClient, BirthdayReminder), PARAM_STRING, (unsigned)"birthday.wav" },
        { "FileDone", offsetof(ICQClient, FileDone), PARAM_STRING, (unsigned)"filedone.wav" },
        { "BypassAuth", offsetof(ICQClient, BypassAuth), PARAM_BOOL, 0 },
        { "ProxyType", offsetof(ICQClient, ProxyType), PARAM_USHORT, 0 },
        { "ProxyHost", offsetof(ICQClient, ProxyHost), PARAM_STRING, (unsigned)"proxy" },
        { "ProxyPort", offsetof(ICQClient, ProxyPort), PARAM_USHORT, 1080 },
        { "ProxyAuth", offsetof(ICQClient, ProxyAuth), PARAM_BOOL, 0 },
        { "ProxyUser", offsetof(ICQClient, ProxyUser), PARAM_STRING, 0 },
        { "ProxyPasswd", offsetof(ICQClient, ProxyPasswd), PARAM_STRING, 0 },
        { "ShareDir", offsetof(ICQClient, ShareDir), PARAM_STRING, (unsigned)"Shared" },
        { "ShareOn", offsetof(ICQClient, ShareOn), PARAM_BOOL, 0 },
        { "", offsetof(ICQClient, owner), 0, (unsigned)ClientOwner_Params }
    };

void Client::save(ostream &s)
{
    ::save(static_cast<ICQClient*>(this), Client_Params, s);
    s << "[ContactList]\n";
    ::save(&contacts, ICQContactList_Params, s);
    for (vector<ICQGroup*>::iterator it_grp = contacts.groups.begin(); it_grp != contacts.groups.end(); it_grp++){
        s << "[Group]\n";
        ::save(*it_grp, ICQGroup_Params, s);
    }
    for (list<ICQUser*>::iterator it = contacts.users.begin(); it != contacts.users.end(); it++){
        if ((*it)->bIsTemp) continue;
        s << "[User]\n";
        ::save(*it, ICQUser_Params, s);
    }
}

bool Client::load(istream &s, string &nextPart)
{
    if (!::load(static_cast<ICQClient*>(this), Client_Params, s, nextPart))
        return false;
    for (;;){
        if (!strcmp(nextPart.c_str(), "[ContactList]")){
            if (!::load(&contacts, ICQContactList_Params, s, nextPart)) break;
            continue;
        }
        if (!strcmp(nextPart.c_str(), "[Group]")){
            ICQGroup *grp = new ICQGroup;
            if (!::load(grp, ICQGroup_Params, s, nextPart)){
                delete grp;
                break;
            }
            contacts.groups.push_back(grp);
            continue;
        }
        if (!strcmp(nextPart.c_str(), "[User]")){
            ICQUser *u = new ICQUser;
            if (!::load(u, ICQUser_Params, s, nextPart)){
                delete u;
                break;
            }
            for (;;){
                bool ok = true;
                list<unsigned long>::iterator it;
                for (it = u->unreadMsgs.begin(); it != u->unreadMsgs.end(); ++it){
                    if (*it >= MSG_PROCESS_ID){
                        u->unreadMsgs.remove(*it);
                        ok = false;
                        break;
                    }
                }
                if (ok) break;
            }
            u->adjustPhones();
            u->adjustEMails();
            contacts.users.push_back(u);
            continue;
        }
        break;
    }
    for (;;){
        bool ok = true;
        list<unsigned long>::iterator it;
        for (it = owner->unreadMsgs.begin(); it != owner->unreadMsgs.end(); ++it){
            if (*it >= MSG_PROCESS_ID){
                owner->unreadMsgs.remove(*it);
                ok = false;
                break;
            }
        }
        if (ok) break;
    }
    owner->adjustPhones();
    owner->adjustEMails(NULL, true);
    return true;
}

Client::Client(QObject *parent, const char *name)
        : QObject(parent, name)
{
    QTimer *timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(timer()));
    timer->start(1000);
    resolver = new QDns;
    resolver->setRecordType(QDns::Ptr);
    QObject::connect(resolver, SIGNAL(resultsReady()), this, SLOT(resolve_ready()));
    encodings = new QStringList;
#ifdef USE_KDE
    *encodings = KGlobal::charsets()->descriptiveEncodingNames();
#else
    for (uint i=0; i< (sizeof(encodingTbl) / sizeof(encoding)); i++) {
        (*encodings).append(i18n(encodingTbl[i].language) + " ( " + encodingTbl[i].codec + " )");
    }
#endif
    ::init(static_cast<ICQClient*>(this), Client_Params);
}

Client::~Client()
{
    close();
    delete encodings;
    delete resolver;
}

void Client::timer()
{
    idle();
}

Socket *Client::createSocket()
{
    return new ICQClientSocket;
}

ServerSocket *Client::createServerSocket()
{
    return new ICQServerSocket(MinTCPPort, MaxTCPPort);
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
                for (list<unsigned long>::iterator it = msg->Uin.begin(); it != msg->Uin.end(); ++it){
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
            if (u && u->inIgnore){
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
            if (uin == owner->Uin) uin = 0;
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
                            if (!uFile->AcceptFileOverride) uFile = pClient->owner;
                            if (uFile->AcceptFileMode== 1){
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
                            if (uFile->AcceptFileMode == 2){
                                pClient->declineMessage(f, uFile->DeclineFileMessage.c_str());
                                return;
                            }
                            switch (owner->uStatus & 0xFF){
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
                        switch (owner->uStatus & 0xFF){
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
                if (u->IP && (u->HostName.size() == 0)){
                    resolveAddr a;
                    a.uin   = e->Uin();
                    a.bReal = false;
                    resolveQueue.push_back(a);
                }
                if (u->RealIP && (u->RealHostName.size() == 0)){
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

void Client::resolve_ready()
{
    if (resolveQueue.empty()) return;
    resolveAddr &a = *resolveQueue.begin();
    ICQUser *u = getUser(a.uin);
    if (u && resolver->hostNames().count()){
        if (a.bReal){
            u->RealHostName = resolver->hostNames().first().latin1();
        }else{
            u->HostName = resolver->hostNames().first().latin1();
        }
        ICQEvent e(EVENT_STATUS_CHANGED, a.uin);
        emit event(&e);
    }
    resolveQueue.erase(resolveQueue.begin());
    start_resolve();
}

void Client::start_resolve()
{
    if (resolver->isWorking()) return;
    for (; !resolveQueue.empty();){
        resolveAddr &a = *resolveQueue.begin();
        ICQUser *u = getUser(a.uin);
        if (u == NULL){
            resolveQueue.erase(resolveQueue.begin());
            continue;
        }
        unsigned long ip;
        if (a.bReal){
            ip = u->RealIP;
        }else{
            ip = u->IP;
        }
        if (ip == 0){
            resolveQueue.erase(resolveQueue.begin());
            continue;
        }
        resolver->setLabel(QHostAddress(htonl(ip)));
        return;
    }
}

QString Client::getName(bool bUseUin)
{
    return QString::fromLocal8Bit(owner->name(bUseUin).c_str());
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
    switch (u->Type){
    case USER_TYPE_EXT:
        return "nonim";
    }
    return getStatusIcon(u->uStatus);
}

const char *Client::getStatusIcon(unsigned long status)
{
    if ((status & 0xFFFF) == ICQ_STATUS_OFFLINE) return "offline";
    if (status & ICQ_STATUS_DND) return "dnd";
    if (status & ICQ_STATUS_OCCUPIED) return "occupied";
    if (status & ICQ_STATUS_NA) return "na";
    if (status & ICQ_STATUS_FREEFORCHAT) return "ffc";
    if (status & ICQ_STATUS_AWAY) return "away";
    return "online";
}

QString Client::getStatusText(unsigned long status)
{
    if ((status & 0xFFFF) == ICQ_STATUS_OFFLINE) return i18n("Offline");
    if (status & ICQ_STATUS_DND) return i18n("Do not disturb");;
    if (status & ICQ_STATUS_OCCUPIED) return i18n("Occupied");
    if (status & ICQ_STATUS_NA) return i18n("N/A");
    if (status & ICQ_STATUS_FREEFORCHAT) return i18n("Free for chat");
    if (status & ICQ_STATUS_AWAY) return i18n("Away");
    return i18n("Online");
}

const char *Client::getStatusIcon()
{
    if (((owner->uStatus && 0xFF) == ICQ_STATUS_ONLINE) && owner->inInvisible)
        return "invisible";
    return getStatusIcon(owner->uStatus);
}

QString Client::getStatusText()
{
    if (((owner->uStatus && 0xFF) == ICQ_STATUS_ONLINE) && owner->inInvisible)
        return i18n("Invisible");
    return getStatusText(owner->uStatus);
}

unsigned long Client::getFileSize(QString name, QString base, vector<fileName> &files)
{
#ifdef WIN32
    QString fName = base + "\\" + name;
#else
    QString fName = base + "/" + name;
#endif
    QFileInfo fInfo(fName);
    if (!fInfo.exists()) return 0;
    if (!fInfo.isDir()){
        if (fInfo.isReadable()){
            fileName f;
            QString n = name;
            n = n.replace(QRegExp("/"), "\\");
            f.name = n.local8Bit();
            f.localName = fName.local8Bit();
            f.size = fInfo.size();
            files.push_back(f);
        }
        return fInfo.size();
    }
    QDir d(fName);
    const QFileInfoList *f = d.entryInfoList();
    if (f == NULL)
        return 0;
    QFileInfoList ff = *f;
    unsigned long res = 0;
    for (const QFileInfo *fi = ff.first(); fi != NULL; fi = ff.next()){
        if ((fi->baseName() == ".") || (fi->baseName() == "..") || fi->isSymLink()) continue;
        QString fName = name;
        if (fName.length()){
#ifdef WIN32
            if (fName[(int)(fName.length()-1)] != '\\') name += "\\";
#else
            if (fName[(int)(fName.length()-1)] != '/') name += "/";
#endif
        }
        fName += fi->baseName();
        res += getFileSize(fName, base, files);
    }
    return res;
}

unsigned long Client::getFileSize(QString fName, vector<fileName> &files)
{
    if (fName.isEmpty()) return 0;
    for (int i = fName.length() - 1; i >= 0; i--){
#ifdef WIN32
        if (fName[i] != '\\') continue;
#else
        if (fName[i] != '/') continue;
#endif
        return getFileSize(fName.mid(i+1), fName.left(i), files);
    }
    return 0;
}

unsigned long Client::getFileSize(const char *name, int *nSrcFiles, vector<fileName> &files)
{
    *nSrcFiles = 0;
    unsigned long res = 0;
    bool bInQuote = false;
    int start = 0;
    QString file = QString::fromLocal8Bit(name);
    for (int i = 0; i < (int)file.length(); i++){
        if (file[i] == '\"'){
            QString fname = file.mid(start, i - start);
            if (!fname.isEmpty()){
                (*nSrcFiles)++;
                res += getFileSize(fname, files);
            }
            bInQuote = !bInQuote;
            start = i + 1;
            continue;
        }
        if ((file[i].isSpace() || (file[i] == ',')) && !bInQuote){
            QString fname = file.mid(start, i - start);
            if (!fname.isEmpty()){
                (*nSrcFiles)++;
                res += getFileSize(fname, files);
            }
            start = i + 1;
            continue;
        }
    }
    QString fname = file.mid(start);
    if (!fname.isEmpty()){
        (*nSrcFiles)++;
        res += getFileSize(fname, files);
    }
    return res;
}

bool makedir(char *p);

bool Client::createFile(ICQFile *f, int mode)
{
    bool bTruncate = false;
    int size = 0;
    QString name = QString::fromLocal8Bit(f->localName.c_str());
#ifdef WIN32
    if (name.isEmpty() || (name[(int)(name.length() - 1)] != '\\'))
        name += "\\";
#else
    if (name.isEmpty() || (name[(int)(name.length() - 1)] != '/'))
        name += "/";
#endif
    QString shortName = QString::fromLocal8Bit(f->ft->curName.c_str());
    int s = shortName.findRev(':');
    if (s >= 0) shortName = shortName.mid(s + 1);
    shortName = shortName.replace(QRegExp("\\"), "/");
    if (shortName[0] == '/'){
        int s = shortName.findRev('/');
        if (s >= 0) shortName = shortName.mid(s + 1);
    }
    s = shortName.findRev("/../");
    if (s >= 0) shortName = shortName.mid(s + 4);
    if (shortName.left(3) == "../") shortName = shortName.mid(3);
    shortName = shortName.replace(QRegExp("//"), "/");
    shortName = shortName.replace(QRegExp("/./"), "/");
#ifdef WIN32
    shortName = shortName.replace(QRegExp("/"), "\\");
    name += shortName;
#else
    name += shortName;
#endif
    f->ft->curName = shortName.local8Bit();
    QFile *file = new QFile(name);
    QFileInfo info(*file);
    if (info.exists()){
        if (info.isDir()){
            if (f->autoAccept)
                return false;
            emit fileNoCreate(f, name);
            return false;
        }
        size = info.size();
        if ((mode == FT_REPLACE) || (mode == FT_RESUME)){
            if ((mode == FT_REPLACE) || (info.size() > f->Size)){
                bTruncate = true;
                size = 0;
            }
        }else if (f->autoAccept){
            ICQUser *u = getUser(f->getUin());
            if ((u == NULL) || !u->AcceptFileOverride) u = owner;
            if (u->AcceptFileOverwrite || (info.size() > f->Size)){
                bTruncate = true;
                size = 0;
            }
        }else{
            emit fileExist(f, name, info.size() < f->ft->curSize());
            return false;
        }
    }
    QString path = name;
#ifdef WIN32
    int p = path.findRev('\\');
#else
    int p = path.findRev('/');
#endif
    if (p >= 0){
        path = path.left(p);
        if (!path.isEmpty()){
            string p;
            p = path.local8Bit();
            makedir((char*)p.c_str());
        }
    }
    if (!file->open(IO_WriteOnly | (bTruncate ? IO_Truncate : 0))){
        if (f->autoAccept)
            return false;
        emit fileNoCreate(f, name);
        return false;
    }
    file->at(size);
    f->ft->setPos(size);
    f->p = (unsigned long)file;
    return true;
}

bool Client::openFile(ICQFile *f)
{
    QFile *file = new QFile(QString::fromLocal8Bit(f->files[f->ft->curFile()].localName.c_str()));
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
    if (f->p == 0) return false;
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
            if (res){
                return res;
            }
        }
    }
    if (owner->Encoding.c_str()){
        QTextCodec *res = QTextCodec::codecForName(owner->Encoding.c_str());
        if (res){
            return res;
        }
    }
    return QTextCodec::codecForLocale();
}

string Client::to8Bit(unsigned long uin, const QString &str)
{
    return to8Bit(codecForUser(uin), str);
}

QString Client::from8Bit(unsigned long uin, const string &str, const char *strCharset)
{
    return from8Bit(codecForUser(uin), str, strCharset);
}

string Client::to8Bit(QTextCodec *codec, const QString &str)
{
    int lenOut = str.length();
    string res;
    if (lenOut == 0) return res;
    res = (const char*)(codec->makeEncoder()->fromUnicode(str, lenOut));
    return res;
}

QString Client::from8Bit(QTextCodec *codec, const string &str, const char *strCharset)
{
    if (!strcmp(codec->name(), serverCharset(codec->name())))
        return codec->makeDecoder()->toUnicode(str.c_str(), str.size());
    string s = str;
    if (strCharset && strcasecmp(strCharset, codec->name())){
        toServer(s, strCharset);
        fromServer(s, codec->name());
    }
    return codec->makeDecoder()->toUnicode(s.c_str(), s.size());
}

void Client::setUserEncoding(unsigned long uin, int i)
{
    if (userEncoding(uin) == i) return;
    ICQUser *u = NULL;
    if ((uin == 0) || (uin == pClient->owner->Uin)){
        u = pClient->owner;
    }else{
        u = getUser(uin);
    }
    if (u == NULL) return;
    QString name;
    if (i > 0){
        name = (*encodings)[i-1];
        int left = name.find(" ( ");
        if (left >= 0) name = name.mid(left + 3);
        int right = name.find(" )");
        if (right >= 0) name = name.left(right);
        u->Encoding = name.latin1();
    }
    emit encodingChanged(uin);
    ICQEvent e(EVENT_INFO_CHANGED, uin);
    process_event(&e);
    if (uin == pClient->owner->Uin)
        emit encodingChanged(0);
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

#ifdef HAVE_KEXTSOCK_H
ICQClientSocket::ICQClientSocket(KExtendedSocket *s)
#else
ICQClientSocket::ICQClientSocket(QSocket *s)
#endif
{
    sock = s;
    if (sock == NULL)
#ifdef HAVE_KEXTSOCK_H
        sock = new KExtendedSocket;
    sock->setSocketFlags(KExtendedSocket::outputBufferedSocket );
#else
        sock = new QSocket(this);
#endif
#ifdef HAVE_KEXTSOCK_H
    QObject::connect(sock, SIGNAL(connectionSuccess()), this, SLOT(slotConnected()));
    QObject::connect(sock, SIGNAL(lookupFinished(int)), this, SLOT(slotLookupFinished(int)));
    QObject::connect(sock, SIGNAL(connectionFailed(int)), this, SLOT(slotError(int)));
    QObject::connect(sock, SIGNAL(closed(int)), this, SLOT(slotError(int)));
#else
    QObject::connect(sock, SIGNAL(connected()), this, SLOT(slotConnected()));
    QObject::connect(sock, SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()));
    QObject::connect(sock, SIGNAL(error(int)), this, SLOT(slotError(int)));
#endif
    QObject::connect(sock, SIGNAL(readyRead()), this, SLOT(slotReadReady()));
    QObject::connect(sock, SIGNAL(bytesWritten(int)), this, SLOT(slotBytesWritten(int)));
    bInWrite = false;
#ifdef HAVE_KEXTSOCK_H
    if (s) sock->enableRead(true);
#endif
}

ICQClientSocket::~ICQClientSocket()
{
    close();
    delete sock;
}

void ICQClientSocket::close()
{
#ifdef HAVE_KEXTSOCK_H
    sock->closeNow();
#else
    sock->close();
#endif
}

void ICQClientSocket::slotLookupFinished(int state)
{
    log(L_DEBUG, "Lookup finished %u", state);
}

int ICQClientSocket::read(char *buf, unsigned int size)
{
    int res = sock->readBlock(buf, size);
    if (res < 0){
#ifdef HAVE_KEXTSOCK_H
        if ((errno == EWOULDBLOCK) || (errno == 0))
            return 0;
#endif
        log(L_DEBUG, "QClientSocket::read error %u", errno);
        if (notify) notify->error_state(ErrorRead);
        return -1;
    }
    return res;
}

void ICQClientSocket::write(const char *buf, unsigned int size)
{
    bInWrite = true;
    int res = sock->writeBlock(buf, size);
    bInWrite = false;
    if (res != (int)size){
        if (notify) notify->error_state(ErrorWrite);
        return;
    }
    if (sock->bytesToWrite() == 0)
        QTimer::singleShot(0, this, SLOT(slotBytesWritten()));
}

void ICQClientSocket::connect(const char *host, int port)
{
    log(L_DEBUG, "Connect to %s:%u", host, port);
#ifdef HAVE_KEXTSOCK_H
    sock->setAddress(host, port);
    if (sock->lookup() < 0){
        log(L_WARN, "Can't lookup");
        if (notify) notify->error_state(ErrorConnect);
    }
    if (sock->startAsyncConnect() < 0){
        log(L_WARN, "Can't connect");
        if (notify) notify->error_state(ErrorConnect);
    }
#else
    sock->connectToHost(host, port);
#endif
}

void ICQClientSocket::slotConnected()
{
    log(L_DEBUG, "Connected");
    if (notify) notify->connect_ready();
#ifdef HAVE_KEXTSOCK_H
    sock->setBlockingMode(false);
    sock->enableRead(true);
#endif
}

void ICQClientSocket::slotConnectionClosed()
{
    log(L_WARN, "Connection closed");
    if (notify) notify->error_state(ErrorConnectionClosed);
}

void ICQClientSocket::slotReadReady()
{
    if (notify) notify->read_ready();
}

void ICQClientSocket::slotBytesWritten(int)
{
    slotBytesWritten();
}

void ICQClientSocket::slotBytesWritten()
{
    if (bInWrite) return;
    if (sock->bytesToWrite() == 0) notify->write_ready();
}

unsigned long ICQClientSocket::localHost()
{
#ifdef HAVE_KEXTSOCK_H
    unsigned long res = 0;
    const KSocketAddress *addr = sock->localAddress();
    if (addr && addr->inherits("KInetSocketAddress")){
        const KInetSocketAddress *addr_in = static_cast<const KInetSocketAddress*>(addr);
        const sockaddr_in *a = addr_in->addressV4();
        if (a) res = htonl(a->sin_addr.s_addr);
    }
    return res;
#else
    return sock->address().ip4Addr();
#endif
}

void ICQClientSocket::slotError(int err)
{
#ifdef HAVE_KEXTSOCK_H
    if (!(err & KBufferedIO::involuntary)) return;
    log(L_DEBUG, "Connection closed by peer");
#else
    log(L_DEBUG, "Error %u", err);
#endif
    if (notify) notify->error_state(ErrorSocket);
}

void ICQClientSocket::pause(unsigned t)
{
    QTimer::singleShot(t * 1000, this, SLOT(slotBytesWritten()));
}

ICQServerSocket::ICQServerSocket(unsigned short minPort, unsigned short maxPort)
{
#ifdef HAVE_KEXTSOCK_H
    sock = new KExtendedSocket;
    connect(sock, SIGNAL(readyAccept()), this, SLOT(activated()));
    for (m_nPort = minPort; m_nPort <= maxPort; m_nPort++){
        sock->reset();
        sock->setBlockingMode(false);
        sock->setSocketFlags(KExtendedSocket::passiveSocket);
        sock->setPort(m_nPort);
        if (sock->listen() == 0)
            break;
    }
    if (m_nPort > maxPort){
        delete sock;
        sock = NULL;
        return;
    }
#else
    sn = NULL;
    sock = new QSocketDevice;
    for (m_nPort = minPort; m_nPort <= maxPort; m_nPort++){
        if (sock->bind(QHostAddress(), m_nPort))
            break;
    }
    if ((m_nPort > maxPort) || !sock->listen(50)){
        delete sock;
        sock = NULL;
        return;
    }
    sn = new QSocketNotifier(sock->socket(), QSocketNotifier::Read, this);
    connect(sn, SIGNAL(activated(int)), this, SLOT(activated(int)));
#endif
}

ICQServerSocket::~ICQServerSocket()
{
    if (sn) delete sn;
    if (sock) delete sock;
}

void ICQServerSocket::activated(int)
{
#ifndef HAVE_KEXTSOCK_H
    if (sock == NULL) return;
    int fd = sock->accept();
    if (fd >= 0){
        if (notify){
            QSocket *s = new QSocket;
            s->setSocket(fd);
            notify->accept(new ICQClientSocket(s));
        }else{
#ifdef WIN32
            ::closesocket(fd);
#else
            ::close(fd);
#endif
        }
    }
#endif
}

void ICQServerSocket::activated()
{
#ifdef HAVE_KEXTSOCK_H
    log(L_DEBUG, "accept ready");
    KExtendedSocket *s = NULL;
    sock->accept(s);
    log(L_DEBUG, "Accept: %u", s);
    if (s == NULL) return;
    if (notify){
        notify->accept(new ICQClientSocket(s));
    }else{
        delete s;
    }
#endif
}

#ifndef _WINDOWS
#include "client.moc"
#endif
