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
#include "sockets.h"
#include "cuser.h"
#include "cfg.h"
#include "log.h"
#include "tmpl.h"
#include "exec.h"
#include "ui/filetransfer.h"

#include <qdns.h>
#include <qtimer.h>
#include <qstring.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qregexp.h>
#include <qpopupmenu.h>

#ifdef USE_KDE
#include <kglobal.h>
#include <kcharsets.h>
#endif

typedef struct encoding
{
    const char *language;
    const char *codec;
    int			mib;
    bool		bMain;
} encoding;

encoding encodingTbl[] =
    {
        { I18N_NOOP("Unicode"), "UTF-8", 106, true },

        { I18N_NOOP("Arabic"), "ISO 8859-6", 82, false },
        { I18N_NOOP("Arabic"), "CP 1256", 2256, true },

        { I18N_NOOP("Baltic"), "ISO 8859-13", 109, false },
        { I18N_NOOP("Baltic"), "CP 1257", 2257, true },

        { I18N_NOOP("Central European"), "ISO 8859-2", 5, false },
        { I18N_NOOP("Central European"), "CP 1250", 2250, true },

        { I18N_NOOP("Chinese "), "GBK", 2025, false },
        { I18N_NOOP("Chinese Simplified"), "gbk2312", 2312, false },
        { I18N_NOOP("Chinese Traditional"), "Big5", 2026, true },

        { I18N_NOOP("Cyrillic"), "ISO 8859-5", 8, false },
        { I18N_NOOP("Cyrillic"), "KOI8-R", 2084, false },
        { I18N_NOOP("Cyrillic"), "CP 1251", 2251, true },

        { I18N_NOOP("Esperanto"), "ISO 8859-3", 6, false },

        { I18N_NOOP("Greek"), "ISO 8859-7", 10, false },
        { I18N_NOOP("Greek"), "CP 1253", 2253, true },

        { I18N_NOOP("Hebrew"), "ISO 8859-8-I", 85, false },
        { I18N_NOOP("Hebrew"), "CP 1255", 2255, true },

        { I18N_NOOP("Japanese"), "Shift-JIS", 17, true },
        { I18N_NOOP("Japanese"), "JIS7", 16, false },
        { I18N_NOOP("Japanese"), "eucJP", 18, false },

        { I18N_NOOP("Korean"), "eucKR", 38, true },

        { I18N_NOOP("Western European"), "ISO 8859-1", 4, false },
        { I18N_NOOP("Western European"), "ISO 8859-15", 111, false },
        { I18N_NOOP("Western European"), "CP 1252", 2252, true },

        { I18N_NOOP("Tamil"), "TSCII", 2028, true },

        { I18N_NOOP("Thai"), "TIS-620", 2259, true },

        { I18N_NOOP("Turkish"), "ISO 8859-9", 12, false },
        { I18N_NOOP("Turkish"), "CP 1254", 2254, true },

        { I18N_NOOP("Ukrainian"), "KOI8-U", 2088, false }
    };

class MsgFilter
{
public:
    MsgFilter(SIMClient *p, ICQMsg *msg, const char *prog);
    Tmpl tmpl;
    Exec exec;
    ICQMsg *msg;
    bool bDelete;
};

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
        { "Email", OFFSET_OF(EMailInfo, Email), PARAM_STRING, 0 },
        { "Hide", OFFSET_OF(EMailInfo, Hide), PARAM_BOOL, 0 },
        { "MyInfo", OFFSET_OF(EMailInfo, MyInfo), PARAM_BOOL, 0 },
        { "", 0, 0, 0 }
    };

cfgParam ExtInfo_Params[] =
    {
        { "Category", OFFSET_OF(ExtInfo, Category), PARAM_USHORT, 0 },
        { "Specific", OFFSET_OF(ExtInfo, Specific), PARAM_STRING, 0 },
        { "", 0, 0, 0 }
    };

cfgParam PhoneInfo_Params[] =
    {
        { "Name", OFFSET_OF(PhoneInfo, Name), PARAM_STRING, 0 },
        { "Type", OFFSET_OF(PhoneInfo, Type), PARAM_USHORT, 0 },
        { "Active", OFFSET_OF(PhoneInfo, Active), PARAM_BOOL, 0 },
        { "Country", OFFSET_OF(PhoneInfo, Country), PARAM_STRING, 0 },
        { "AreaCode", OFFSET_OF(PhoneInfo, AreaCode), PARAM_STRING, 0 },
        { "Number", OFFSET_OF(PhoneInfo, Number), PARAM_STRING, 0 },
        { "Extension", OFFSET_OF(PhoneInfo, Extension), PARAM_STRING, 0 },
        { "Provider", OFFSET_OF(PhoneInfo, Provider), PARAM_STRING, 0 },
        { "Gateway", OFFSET_OF(PhoneInfo, Gateway), PARAM_STRING, 0 },
        { "Publish", OFFSET_OF(PhoneInfo, Publish), PARAM_BOOL, 0 },
        { "", 0, 0, 0 }
    };

cfgParam ICQContactList_Params[] =
    {
        { "Length", OFFSET_OF(ICQContactList, Len), PARAM_USHORT, 0 },
        { "Invsible", OFFSET_OF(ICQContactList, Invisible), PARAM_USHORT, 0 },
        { "Time", OFFSET_OF(ICQContactList, Time), PARAM_ULONG, 0 },
        { "Expand", OFFSET_OF(ICQContactList, Expand), PARAM_BOOL, 1 },
        { "", 0, 0, 0 }
    };

cfgParam ICQGroup_Params[] =
    {
        { "Name", OFFSET_OF(ICQGroup, Name), PARAM_STRING, 0 },
        { "Id", OFFSET_OF(ICQGroup, Id), PARAM_USHORT, 0 },
        { "", 0, 0, 0 }
    };

cfgParam SIMGroup_Params[] =
    {
        { "Expand", OFFSET_OF(SIMGroup, Expand), PARAM_BOOL, 1 },
        { "", 0, PARAM_OFFS, (unsigned)ICQGroup_Params },
        { "", 0, 0, 0 }
    };

cfgParam ICQUser_Params[] =
    {
        { "Type", OFFSET_OF(ICQUser, Type), PARAM_USHORT, 0 },
        { "Alias", OFFSET_OF(ICQUser, Alias), PARAM_STRING, 0 },
        { "Id", OFFSET_OF(ICQUser, Id), PARAM_USHORT, 0 },
        { "GrpId", OFFSET_OF(ICQUser, GrpId), PARAM_USHORT, 0 },
        { "UIN", OFFSET_OF(ICQUser, Uin), PARAM_ULONG, 0 },
        { "Ignore", OFFSET_OF(ICQUser, inIgnore), PARAM_BOOL, 0 },
        { "Visible", OFFSET_OF(ICQUser, inVisible), PARAM_BOOL, 0 },
        { "Invisible", OFFSET_OF(ICQUser, inInvisible), PARAM_BOOL, 0 },
        { "WaitAuth", OFFSET_OF(ICQUser, WaitAuth), PARAM_BOOL, 0 },
        { "AutoResponseAway", OFFSET_OF(ICQUser, AutoResponseAway), PARAM_STRING, 0 },
        { "AutoResponseNA", OFFSET_OF(ICQUser, AutoResponseNA), PARAM_STRING, 0 },
        { "AutoResponseDND", OFFSET_OF(ICQUser, AutoResponseDND), PARAM_STRING, 0 },
        { "AutoResponseOccupied", OFFSET_OF(ICQUser, AutoResponseOccupied), PARAM_STRING, 0 },
        { "AutoResponseFFC", OFFSET_OF(ICQUser, AutoResponseFFC), PARAM_STRING, 0 },
        { "UnreadMessages", OFFSET_OF(ICQUser, unreadMsgs), PARAM_ULONGS, 0 },
        { "LastActive", OFFSET_OF(ICQUser, LastActive), PARAM_ULONG, 0 },
        { "OnlineTime", OFFSET_OF(ICQUser, OnlineTime), PARAM_ULONG, 0 },
        { "StatusTime", OFFSET_OF(ICQUser, StatusTime), PARAM_ULONG, 0 },
        { "IP", OFFSET_OF(ICQUser, IP), PARAM_ULONG, 0 },
        { "Port", OFFSET_OF(ICQUser, Port), PARAM_USHORT, 0 },
        { "RealIP", OFFSET_OF(ICQUser, RealIP), PARAM_ULONG, 0 },
        { "HostName", OFFSET_OF(ICQUser, HostName), PARAM_STRING, 0 },
        { "RealHostName", OFFSET_OF(ICQUser, RealHostName), PARAM_STRING, 0 },
        { "Version", OFFSET_OF(ICQUser, Version), PARAM_USHORT, 0 },
        { "Mode", OFFSET_OF(ICQUser, Mode), PARAM_USHORT, 0 },
        { "Nick", OFFSET_OF(ICQUser, Nick), PARAM_STRING, 0 },
        { "FirstName", OFFSET_OF(ICQUser, FirstName), PARAM_STRING, 0 },
        { "LastName", OFFSET_OF(ICQUser, LastName), PARAM_STRING, 0 },
        { "City", OFFSET_OF(ICQUser, City), PARAM_STRING, 0 },
        { "State", OFFSET_OF(ICQUser, State), PARAM_STRING, 0 },
        { "Address", OFFSET_OF(ICQUser, Address), PARAM_STRING, 0 },
        { "Zip", OFFSET_OF(ICQUser, Zip), PARAM_STRING, 0 },
        { "Country", OFFSET_OF(ICQUser, Country), PARAM_USHORT, 0 },
        { "TimeZone", OFFSET_OF(ICQUser, TimeZone), PARAM_CHAR, 0 },
        { "HomePhone", OFFSET_OF(ICQUser, HomePhone), PARAM_STRING, 0 },
        { "HomeFax", OFFSET_OF(ICQUser, HomeFax), PARAM_STRING, 0 },
        { "PrivateCellular", OFFSET_OF(ICQUser, PrivateCellular), PARAM_STRING, 0 },
        { "EMailInfo", OFFSET_OF(ICQUser, EMail), PARAM_STRING, 0 },
        { "HiddenEMail", OFFSET_OF(ICQUser, HiddenEMail), PARAM_BOOL, 0 },
        { "EMail", OFFSET_OF(ICQUser, EMails), (unsigned)createEMail, (unsigned)EMail_Params },
        { "Age", OFFSET_OF(ICQUser, Age), PARAM_CHAR, 0 },
        { "Gender", OFFSET_OF(ICQUser, Gender), PARAM_CHAR, 0 },
        { "Homepage", OFFSET_OF(ICQUser, Homepage), PARAM_STRING, 0 },
        { "BirthYear", OFFSET_OF(ICQUser, BirthYear), PARAM_USHORT, 0 },
        { "BirthMonth", OFFSET_OF(ICQUser, BirthMonth), PARAM_CHAR, 0 },
        { "BirthDay", OFFSET_OF(ICQUser, BirthDay), PARAM_CHAR, 0 },
        { "Language1", OFFSET_OF(ICQUser, Language1), PARAM_CHAR, 0 },
        { "Language2", OFFSET_OF(ICQUser, Language2), PARAM_CHAR, 0 },
        { "Language3", OFFSET_OF(ICQUser, Language3), PARAM_CHAR, 0 },
        { "WorkCity", OFFSET_OF(ICQUser, WorkCity), PARAM_STRING, 0 },
        { "WorkState", OFFSET_OF(ICQUser, WorkState), PARAM_STRING, 0 },
        { "WorkZip", OFFSET_OF(ICQUser, WorkZip), PARAM_STRING, 0 },
        { "WorkAddress", OFFSET_OF(ICQUser, WorkAddress), PARAM_STRING, 0 },
        { "WorkName", OFFSET_OF(ICQUser, WorkName), PARAM_STRING, 0 },
        { "WorkDepartment", OFFSET_OF(ICQUser, WorkDepartment), PARAM_STRING, 0 },
        { "WorkPosition", OFFSET_OF(ICQUser, WorkPosition), PARAM_STRING, 0 },
        { "WorkCountry", OFFSET_OF(ICQUser, WorkCountry), PARAM_USHORT, 0 },
        { "Occupation", OFFSET_OF(ICQUser, Occupation), PARAM_USHORT, 0 },
        { "WorkHomepage", OFFSET_OF(ICQUser, WorkHomepage), PARAM_STRING, 0 },
        { "WorkPhone", OFFSET_OF(ICQUser, WorkPhone), PARAM_STRING, 0 },
        { "WorkFax", OFFSET_OF(ICQUser, WorkFax), PARAM_STRING, 0 },
        { "About", OFFSET_OF(ICQUser, About), PARAM_STRING, 0 },
        { "Background", OFFSET_OF(ICQUser, Backgrounds), (unsigned)createExtInfo, (unsigned)ExtInfo_Params },
        { "Affilation", OFFSET_OF(ICQUser, Affilations), (unsigned)createExtInfo, (unsigned)ExtInfo_Params },
        { "Interest", OFFSET_OF(ICQUser, Interests), (unsigned)createExtInfo, (unsigned)ExtInfo_Params },
        { "Phone", OFFSET_OF(ICQUser, Phones), (unsigned)createPhone, (unsigned)PhoneInfo_Params },
        { "PhoneState", OFFSET_OF(ICQUser, PhoneState), PARAM_CHAR, 0 },
        { "PhoneBookTime", OFFSET_OF(ICQUser, PhoneBookTime), PARAM_ULONG, 0 },
        { "PhoneStatusTime", OFFSET_OF(ICQUser, PhoneStatusTime), PARAM_ULONG, 0 },
        { "InfoUpdateTime", OFFSET_OF(ICQUser, InfoUpdateTime), PARAM_ULONG, 0 },
        { "Caps", OFFSET_OF(ICQUser, Caps), PARAM_ULONG, 0 },
        { "Build", OFFSET_OF(ICQUser, Build), PARAM_ULONG, 0 },
        { "Encoding", OFFSET_OF(ICQUser, Encoding), PARAM_STRING, 0 },
        { "", 0, 0, 0 }
    };

cfgParam SIMUser_Params[] =
    {
        { "AlertOverride", OFFSET_OF(SIMUser, AlertOverride), PARAM_BOOL, 0 },
        { "AlertAway", OFFSET_OF(SIMUser, AlertAway), PARAM_BOOL, 1 },
        { "AlertBlink", OFFSET_OF(SIMUser, AlertBlink), PARAM_BOOL, 1 },
        { "AlertSound", OFFSET_OF(SIMUser, AlertSound), PARAM_BOOL, 1 },
        { "AlertOnScreen", OFFSET_OF(SIMUser, AlertOnScreen), PARAM_BOOL, 1 },
        { "AlertPopup", OFFSET_OF(SIMUser, AlertPopup), PARAM_BOOL, 0 },
        { "AlertWindow", OFFSET_OF(SIMUser, AlertWindow), PARAM_BOOL, 0 },
        { "LogStatus", OFFSET_OF(SIMUser, LogStatus), PARAM_BOOL, 0 },
        { "AcceptMsgWindow", OFFSET_OF(SIMUser, AcceptMsgWindow), PARAM_BOOL, 0 },
        { "AcceptFileMode", OFFSET_OF(SIMUser, AcceptFileMode), PARAM_USHORT, 0 },
        { "AcceptFileOverride", OFFSET_OF(SIMUser, AcceptFileOverride), PARAM_BOOL, 0 },
        { "AcceptFileOverwrite", OFFSET_OF(SIMUser, AcceptFileOverwrite), PARAM_BOOL, 0 },
        { "AcceptFilePath", OFFSET_OF(SIMUser, AcceptFilePath), PARAM_STRING, 0 },
        { "DeclineFileMessage", OFFSET_OF(SIMUser, DeclineFileMessage), PARAM_STRING, 0 },
        { "SoundOverride", OFFSET_OF(SIMUser, SoundOverride), PARAM_BOOL, 0 },
        { "IncomingMessage", OFFSET_OF(SIMUser, IncomingMessage), PARAM_STRING, (unsigned)"message.wav" },
        { "IncomingURL", OFFSET_OF(SIMUser, IncomingURL), PARAM_STRING, (unsigned)"url.wav" },
        { "IncomingSMS", OFFSET_OF(SIMUser, IncomingSMS), PARAM_STRING, (unsigned)"sms.wav" },
        { "IncomingAuth", OFFSET_OF(SIMUser, IncomingAuth), PARAM_STRING, (unsigned)"auth.wav" },
        { "IncomingFile", OFFSET_OF(SIMUser, IncomingFile), PARAM_STRING, (unsigned)"file.wav" },
        { "IncomingChat", OFFSET_OF(SIMUser, IncomingChat), PARAM_STRING, (unsigned)"chat.wav" },
        { "OnlineAlert", OFFSET_OF(SIMUser, OnlineAlert), PARAM_STRING, (unsigned)"alert.wav" },
        { "ProgOverride", OFFSET_OF(SIMUser, ProgOverride), PARAM_BOOL, 0 },
        { "ProgMessageOn", OFFSET_OF(SIMUser, ProgMessageOn), PARAM_BOOL, 0 },
        { "ProgMessage", OFFSET_OF(SIMUser, ProgMessage), PARAM_STRING, 0 },
        { "KabUid", OFFSET_OF(SIMUser, strKabUid), PARAM_STRING, 0 },
        { "Notes", OFFSET_OF(SIMUser, Notes), PARAM_STRING, 0 },
        { "", 0, PARAM_OFFS, (unsigned)ICQUser_Params },
        { "", 0, 0, 0 }
    };

cfgParam ClientOwner_Params[] =
    {
        { "AutoResponseAway" ,OFFSET_OF(ICQUser, AutoResponseAway), PARAM_I18N, (unsigned)I18N_NOOP(
              "I am currently away from ICQ.\n"
              "Please leave your message and I will get back to you as soon as I return!\n"
          ) },
        { "AutoResponseNA" ,OFFSET_OF(ICQUser, AutoResponseNA), PARAM_I18N, (unsigned)I18N_NOOP(
              "I am out'a here.\n"
              "See you tomorrow!\n"
          ) },
        { "AutoResponseDND" ,OFFSET_OF(ICQUser, AutoResponseDND), PARAM_I18N, (unsigned)I18N_NOOP(
              "Please do not disturb me now. Disturb me later.\n"
          ) },
        { "AutoResponseOccupied" ,OFFSET_OF(ICQUser, AutoResponseOccupied), PARAM_I18N, (unsigned)I18N_NOOP(
              "Please do not disturb me now.\n"
              "Disturb me later.\n"
              "Only urgent messages, please!\n") },
        { "AutoResponseFFC" ,OFFSET_OF(ICQUser, AutoResponseFFC), PARAM_I18N, (unsigned)I18N_NOOP(
              "We'd love to hear what you have to say. Join our chat.\n"
          ) },
        { "", 0, PARAM_OFFS, (unsigned)SIMUser_Params },
        { "", 0, 0, 0 }
    };

cfgParam Sockets_Params[] =
    {
        { "MinTCPPort", OFFSET_OF(SocketFactory, MinTCPPort), PARAM_USHORT, 1024 },
        { "MaxTCPPort", OFFSET_OF(SocketFactory, MaxTCPPort), PARAM_USHORT, 0xFFFF },
        { "ProxyType", OFFSET_OF(SocketFactory, ProxyType), PARAM_USHORT, 0 },
        { "ProxyHost", OFFSET_OF(SocketFactory, ProxyHost), PARAM_STRING, (unsigned)"proxy" },
        { "ProxyPort", OFFSET_OF(SocketFactory, ProxyPort), PARAM_USHORT, 1080 },
        { "ProxyAuth", OFFSET_OF(SocketFactory, ProxyAuth), PARAM_BOOL, 0 },
        { "ProxyUser", OFFSET_OF(SocketFactory, ProxyUser), PARAM_STRING, 0 },
        { "ProxyPasswd", OFFSET_OF(SocketFactory, ProxyPasswd), PARAM_STRING, 0 },
        { "", 0, 0, 0 }
    };

static void *owner(void *p)
{
    return ((ICQClient*)p)->owner;
}

cfgParam ICQClient_Params[] =
    {
        { "ServerHost", OFFSET_OF(ICQClient, ServerHost), PARAM_STRING, (unsigned)"login.icq.com" },
        { "ServerPort", OFFSET_OF(ICQClient, ServerPort), PARAM_USHORT, 5190 },
        { "Password", OFFSET_OF(ICQClient, DecryptedPassword), PARAM_STRING, 0 },
        { "EncryptPassword", OFFSET_OF(ICQClient, EncryptedPassword), PARAM_STRING, 0 },
        { "WebAware", OFFSET_OF(ICQClient, WebAware), PARAM_BOOL, 0 },
        { "Authorize", OFFSET_OF(ICQClient, Authorize), PARAM_BOOL, 0 },
        { "HideIp", OFFSET_OF(ICQClient, HideIp), PARAM_BOOL, 0 },
        { "RejectMessage", OFFSET_OF(ICQClient, RejectMessage), PARAM_BOOL, 0 },
        { "RejectURL", OFFSET_OF(ICQClient, RejectURL), PARAM_BOOL, 0 },
        { "RejectWeb", OFFSET_OF(ICQClient, RejectWeb), PARAM_BOOL, 0 },
        { "RejectEmail", OFFSET_OF(ICQClient, RejectEmail), PARAM_BOOL, 0 },
        { "RejectOther", OFFSET_OF(ICQClient, RejectOther), PARAM_BOOL, 0 },
        { "RejectFilter", OFFSET_OF(ICQClient, RejectFilter), PARAM_STRING, 0 },
        { "DirectMode", OFFSET_OF(ICQClient, DirectMode), PARAM_USHORT, 0 },
        { "BypassAuth", OFFSET_OF(ICQClient, BypassAuth), PARAM_BOOL, 0 },
        { "ShareDir", OFFSET_OF(ICQClient, ShareDir), PARAM_STRING, (unsigned)"Shared" },
        { "ShareOn", OFFSET_OF(ICQClient, ShareOn), PARAM_BOOL, 0 },
        { "owner", (int)owner, PARAM_PROC, (unsigned)ClientOwner_Params },
        { "", 0, 0, 0 },
    };

static void *factory(void *p)
{
    return ((SIMClient*)p)->factory();
}

static void *icqClient(void *p)
{
    return static_cast<ICQClient*>((SIMClient*)p);
}

cfgParam Client_Params[] =
    {
        { "_factory", (int)factory, PARAM_PROC, (unsigned)Sockets_Params },
        { "_icqclient", (int)icqClient, PARAM_PROC, (unsigned)ICQClient_Params },
        { "BirthdayReminder", OFFSET_OF(SIMClient, BirthdayReminder), PARAM_STRING, (unsigned)"birthday.wav" },
        { "FileDone", OFFSET_OF(SIMClient, FileDone), PARAM_STRING, (unsigned)"filedone.wav" },
        { "", 0, 0, 0 }
    };

ICQUser *SIMClient::createUser()
{
    ICQUser *u = new SIMUser;
    ::init(u, SIMUser_Params);
    return u;
}

ICQGroup *SIMClient::createGroup()
{
    ICQGroup *g = new SIMGroup;
    ::init(g, SIMGroup_Params);
    return g;
}

void SIMClient::save(QFile &s)
{
    ::save(this, Client_Params, s);
    writeStr(s, "[ContactList]\n");
    ::save(&contacts, ICQContactList_Params, s);
    for (vector<ICQGroup*>::iterator it_grp = contacts.groups.begin(); it_grp != contacts.groups.end(); it_grp++){
        writeStr(s, "[Group]\n");
        ::save(*it_grp, SIMGroup_Params, s);
    }
    for (list<ICQUser*>::iterator it = contacts.users.begin(); it != contacts.users.end(); it++){
        if ((*it)->bIsTemp) continue;
        writeStr(s, "[User]\n");
        ::save(*it, SIMUser_Params, s);
    }
}

bool SIMClient::load(QFile &s, string &nextPart)
{
    if (!::load(this, Client_Params, s, nextPart))
        return false;
    for (;;){
        if (!strcmp(nextPart.c_str(), "[ContactList]")){
            if (!::load(&contacts, ICQContactList_Params, s, nextPart)) break;
            continue;
        }
        if (!strcmp(nextPart.c_str(), "[Group]")){
            ICQGroup *grp = createGroup();
            if (!::load(grp, SIMGroup_Params, s, nextPart)){
                delete grp;
                break;
            }
            contacts.groups.push_back(grp);
            continue;
        }
        if (!strcmp(nextPart.c_str(), "[User]")){
            ICQUser *u = createUser();
            if (!::load(u, SIMUser_Params, s, nextPart)){
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
                    History h(u->Uin);
                    ICQMessage *msg = h.getMessage(*it);
                    if (msg == NULL){
                        u->unreadMsgs.remove(*it);
                        ok = false;
                        break;
                    }
                    delete msg;
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

SIMClient::SIMClient(QObject *parent, const char *name)
        : QObject(parent, name), ICQClient(getFactory())
{
    QTimer *timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(timer()));
    timer->start(1000);
    resolver = new QDns;
    resolver->setRecordType(QDns::Ptr);
    QObject::connect(resolver, SIGNAL(resultsReady()), this, SLOT(resolve_ready()));
    init();
    ::init(this, Client_Params);
}

SIMClient::~SIMClient()
{
    for (list<SMSmessage*>::iterator it = smsQueue.begin(); it != smsQueue.end(); ++it){
        delete (*it);
    }
    close();
    delete resolver;
}

class Encoding
{
public:
    Encoding(const QString &_name, int _mib) : name(_name), mib(_mib) {}
    QString name;
    int		mib;
};

bool operator < (const Encoding &a, const Encoding &b)
{
    return a.name < b.name;
}

static void fillEncodings(list<Encoding> &encodings, bool bMain)
{
    const encoding *e;
#ifdef USE_KDE
    QStringList sysenc = KGlobal::charsets()->descriptiveEncodingNames();
    for (QStringList::Iterator it = sysenc.begin(); it != sysenc.end(); ++it){
        QString enc = *it;
        int n = enc.find("(");
        if (n == -1) continue;
        enc = enc.mid(n + 1);
        n = enc.find(")");
        if (n == -1) continue;
        enc = enc.left(n);
        QTextCodec *codec = QTextCodec::codecForName(enc);
        if (codec == NULL) continue;
        e = encodingTbl;
        unsigned i;
        for (i = 0; i < (sizeof(encodingTbl) / sizeof(encoding)); i++, e++) {
            if (e->mib != codec->mibEnum()) continue;
            break;
        }
        if (i < (sizeof(encodingTbl) / sizeof(encoding))){
            if (e->bMain == bMain)
                encodings.push_back(Encoding(*it, e->mib));
        }else{
            if (bMain)
                encodings.push_back(Encoding(*it, codec->mibEnum()));
        }
    }
#else
    e = encodingTbl;
    for (unsigned i = 0; i < (sizeof(encodingTbl) / sizeof(encoding)); i++, e++) {
        if (e->bMain != bMain) continue;
        encodings.push_back(Encoding(i18n(e->language) + " ( " + e->codec + " )", e->mib));
    }
#endif
    encodings.sort();
}

void SIMClient::fillEncodings(QPopupMenu *menu, bool bMain)
{
    list<Encoding> enc;
    ::fillEncodings(enc, bMain);
    for (list<Encoding>::iterator it = enc.begin(); it != enc.end(); ++it){
        menu->insertItem((*it).name, (*it).mib);
    }
}

QStringList SIMClient::getEncodings(bool bMain)
{
    list<Encoding> enc;
    ::fillEncodings(enc, bMain);
    QStringList res;
    for (list<Encoding>::iterator it = enc.begin(); it != enc.end(); ++it){
        res.append((*it).name);
    }
    return res;
}

void SIMClient::timer()
{
    idle();
}

bool SIMClient::markAsRead(ICQMessage *msg)
{
    ICQUser *u = getUser(msg->getUin());
    if (u == NULL) return false;
    for (list<unsigned long>::iterator it = u->unreadMsgs.begin(); it != u->unreadMsgs.end(); it++){
        if (*it == msg->Id){
            u->unreadMsgs.remove(*it);
            emit messageRead(msg);
            return true;
        }
    }
    return false;
}

void SIMClient::process_event(ICQEvent *e)
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
                    if (msg->Id != MSG_NEW) msg->Id = id;
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
    case EVENT_MESSAGE_SEND:
        if (e->message()){
            for (list<SMSmessage*>::iterator it = smsQueue.begin(); it != smsQueue.end(); ++it){
                if ((*it)->msg == e->message()){
                    SMSmessage *sms = *it;
                    if (e->state == ICQEvent::Success){
                        sendSMS(sms);
                    }else{
                        smsQueue.remove(sms);
                        delete sms;
                    }
                    return;
                }
            }
        }
        break;
    case EVENT_MESSAGE_RECEIVED:{
            ICQMessage *msg = e->message();
            if (msg == NULL){
                log(L_WARN, "Message event without message");
                return;
            }
            unsigned long uin = msg->getUin();
            switch (msg->Type()){
            case ICQ_READxAWAYxMSG:
            case ICQ_READxOCCUPIEDxMSG:
            case ICQ_READxNAxMSG:
            case ICQ_READxDNDxMSG:
            case ICQ_READxFFCxMSG:
                getAutoResponse(msg->getUin(), msg);
                return;
            }
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
                if (msg->Id == MSG_NEW){
                    if (msg->Type() == ICQ_MSGxSMS){
                        ICQSMS *sms = static_cast<ICQSMS*>(msg);
                        if (*pMain->ForwardPhone.c_str() && PhoneInfo::isEqual(sms->Phone.c_str(), pMain->ForwardPhone.c_str())){
                            unsigned long uin = atol(sms->Message.c_str());
                            if (uin > 10000){
                                QString msgText = pClient->from8Bit(owner->Uin, sms->Message, msg->Charset.c_str());
                                int pos = msgText.find(':');
                                if (pos >= 0){
                                    msgText = msgText.mid(pos+1);
                                    ICQMsg *n = new ICQMsg;
                                    n->Uin.push_back(uin);
                                    n->Message = msgText.utf8();
                                    n->Charset = "utf-8";
                                    sendMessage(n);
                                    return;
                                }
                            }
                        }
                    }
                    if (msg->Type() == ICQ_MSGxMSG){
                        ICQMsg *m = static_cast<ICQMsg*>(msg);
                        if (((owner->uStatus & ICQ_STATUS_NA) || (owner->uStatus & ICQ_STATUS_AWAY)) &&
                                *pMain->ForwardPhone.c_str()){
                            QString str = pClient->from8Bit(msg->getUin(), m->Message, msg->Charset.c_str());
                            if (!str.isEmpty()){
                                string text(str.utf8());
                                text = pClient->clearHTML(text);
                                str = QString::fromUtf8(text.c_str());
                                QString uin = QString::number(msg->getUin());
                                CUser u(msg->getUin());
                                QString name = QString::fromLocal8Bit(u->name(false).c_str());
                                if (name != uin){
                                    uin += " ";
                                    uin += name;
                                }
                                str = uin + ":\n" + str;
                                SMSmessage *sms = new SMSmessage;
                                sms->str = str;
                                sms->msg = NULL;
                                smsQueue.push_back(sms);
                                sendSMS(sms);
                            }
                        }
                        SIMUser *msgUser = static_cast<SIMUser*>(u);
                        if (!msgUser->ProgOverride) msgUser = static_cast<SIMUser*>(owner);
                        if (msgUser->ProgMessageOn && *msgUser->ProgMessage.c_str()){
                            new MsgFilter(this, m, msgUser->ProgMessage.c_str());
                            return;
                        }
                    }
                    History h(uin);
                    msg->Id = h.addMessage(msg);
                }else{
                    switch (msg->Type()){
                    case ICQ_MSGxFILE:{
                            ICQFile *f = static_cast<ICQFile*>(msg);
                            SIMUser *uFile = static_cast<SIMUser*>(u);
                            if (!uFile->AcceptFileOverride)
                                uFile = static_cast<SIMUser*>(pClient->owner);
                            if (uFile->AcceptFileMode== 1){
                                string name = uFile->AcceptFilePath.c_str();
                                if (*name.c_str() == 0)
                                    pMain->buildFileName(name, "IncomingFiles/");
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
                                getAutoResponse(uin, msg);
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
                            getAutoResponse(uin, msg);
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
                SIMUser *_u = static_cast<SIMUser*>(u);
                if (!_u->AlertOverride) _u = static_cast<SIMUser*>(owner);
                if ((_u->LogStatus) && (u->uStatus != u->prevStatus)){
                    ICQStatus m;
                    m.Uin.push_back(e->Uin());
                    m.status = u->uStatus;
                    History h(e->Uin());
                    h.addMessage(&m);
                    emit messageReceived(&m);
                }
            }
            break;
        }
    }
    emit event(e);
}

void SIMClient::sendSMS(SMSmessage *sms)
{
    if (sms->msg){
        delete sms->msg;
        sms->msg = NULL;
    }
    ICQSMS *m = new ICQSMS;
    m->Uin.push_back(owner->Uin);
    m->Message = sms->smsChunk();
    if (m->Message.length()){
        m->Phone = pClient->to8Bit(owner->Uin, QString::fromLocal8Bit(pMain->ForwardPhone.c_str()));
        m->Charset = pClient->userEncoding(owner->Uin);
        sms->msg = m;
        pClient->sendMessage(sms->msg);
    }else{
        smsQueue.remove(sms);
        delete m;
        delete sms;
    }
}

QString SMSmessage::chunk(const QString &s, int len)
{
    if ((int)s.length() < len) return s;
    QString res = s.left(len+1);
    int n = res.length() - 1;
    for (n = res.length() - 1; n >= 0; n--)
        if (res[n].isSpace()) break;
    for (; n >= 0; n--)
        if (!res[n].isSpace()) break;
    if (n < 0){
        res = s.left(len);
        return res;
    }
    res = s.left(n + 1);
    return res;
}

bool SMSmessage::isLatin1(const QString &s)
{
    for (int n = 0; n < (int)s.length(); n++)
        if (!s[n].latin1()) return false;
    return true;
}

#define MAX_SMS_LEN_LATIN1	160
#define MAX_SMS_LEN_UNICODE	70

string SMSmessage::smsChunk()
{
    string res;
    if (str.isEmpty()) return res;
    QString part = chunk(str, MAX_SMS_LEN_LATIN1);
    if (!isLatin1(part))
        part = chunk(str, MAX_SMS_LEN_UNICODE);
    str = trim(str.mid(part.length()));
    part = part.replace(QRegExp("&"), "&amp;");
    part = part.replace(QRegExp("\""), "&quot;");
    part = part.replace(QRegExp("<"), "&lt;");
    part = part.replace(QRegExp(">"), "&gt;");
    res = pClient->to8Bit(pClient->owner->Uin, part);
    return res;
}

QString SMSmessage::trim(const QString &s)
{
    QString res = s;
    int n;
    for (n = 0; n < (int)res.length(); n++)
        if (!res[n].isSpace()) break;
    if (n) res = res.mid(n);
    if (res.isEmpty()) return res;
    for (n = (int)res.length() - 1; n >= 0; n--)
        if (!res[n].isSpace()) break;
    if (n < (int)res.length() - 1) res = res.left(n + 1);
    return res;
}

void SIMClient::resolve_ready()
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

void SIMClient::start_resolve()
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

QString SIMClient::getName(bool bUseUin)
{
    return QString::fromLocal8Bit(owner->name(bUseUin).c_str());
}

const char *SIMClient::getMessageIcon(int type)
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

QString SIMClient::getMessageText(int type, int n)
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
        return i18n("Request secure channel", "%n requests secure channel", n);
    case ICQ_MSGxSECURExCLOSE:
        return i18n("Close secure channel", "%n times close secure channel", n);
    default:
        log(L_WARN, "Unknown message type %u", type);
    }
    return i18n("Unknown message", "%n unknown messages", n);
}

QString SIMClient::getMessageAccel(int type)
{
    switch (type){
    case ICQ_MSGxMSG:
        return i18n("Ctrl+N", "new");
    case ICQ_MSGxCHAT:
        return i18n("Ctrl+B", "chat msg");
    case ICQ_MSGxFILE:
        return i18n("Ctrl+F", "File");
    case ICQ_MSGxURL:
        return i18n("Ctrl+L", "URL");
    case ICQ_MSGxAUTHxREQUEST:
        return i18n("Ctrl+R", "AuthRequest");
    case ICQ_MSGxCONTACTxLIST:
        return i18n("Ctrl+T", "Contacts");
    case ICQ_MSGxSMS:
        return i18n("Ctrl+S", "SMS");
    case ICQ_MSGxMAIL:
        return i18n("Ctrl+E", "EMail");
    default:
        break;
    }
    return "";
}

const char *SIMClient::getUserIcon(ICQUser *u)
{
    switch (u->Type){
    case USER_TYPE_EXT:
        return "nonim";
    }
    return getStatusIcon(u->uStatus);
}

const char *SIMClient::getStatusIcon(unsigned long status)
{
    if ((status & 0xFFFF) == ICQ_STATUS_OFFLINE) return "offline";
    if (status & ICQ_STATUS_DND) return "dnd";
    if (status & ICQ_STATUS_OCCUPIED) return "occupied";
    if (status & ICQ_STATUS_NA) return "na";
    if (status & ICQ_STATUS_FREEFORCHAT) return "ffc";
    if (status & ICQ_STATUS_AWAY) return "away";
    return "online";
}

QString SIMClient::getStatusText(unsigned long status)
{
    if ((status & 0xFFFF) == ICQ_STATUS_OFFLINE) return i18n("Offline");
    if (status & ICQ_STATUS_DND) return i18n("Do not disturb");;
    if (status & ICQ_STATUS_OCCUPIED) return i18n("Occupied");
    if (status & ICQ_STATUS_NA) return i18n("N/A");
    if (status & ICQ_STATUS_FREEFORCHAT) return i18n("Free for chat");
    if (status & ICQ_STATUS_AWAY) return i18n("Away");
    return i18n("Online");
}

const char *SIMClient::getStatusIcon()
{
    if (((owner->uStatus && 0xFF) == ICQ_STATUS_ONLINE) && owner->inInvisible)
        return "invisible";
    return getStatusIcon(owner->uStatus);
}

QString SIMClient::getStatusText()
{
    if (((owner->uStatus && 0xFF) == ICQ_STATUS_ONLINE) && owner->inInvisible)
        return i18n("Invisible");
    return getStatusText(owner->uStatus);
}

unsigned long SIMClient::getFileSize(QString name, QString base, vector<fileName> &files)
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
        QString baseName = fi->fileName();
        if ((baseName == "") || (baseName == ".") || (baseName == "..") || fi->isSymLink()) continue;
        QString fName = name;
        if (fName.length()){
#ifdef WIN32
            if (fName[(int)(fName.length()-1)] != '\\') fName += "\\";
#else
            if (fName[(int)(fName.length()-1)] != '/') fName += "/";
#endif
        }
        fName += baseName;
        res += getFileSize(fName, base, files);
    }
    return res;
}

unsigned long SIMClient::getFileSize(QString fName, vector<fileName> &files)
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

unsigned long SIMClient::getFileSize(const char *name, int *nSrcFiles, vector<fileName> &files)
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

bool SIMClient::createFile(ICQFile *f, int mode)
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
    QString shortName = QString::fromLocal8Bit(f->curName().c_str());
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
    f->setCurName(shortName.local8Bit());
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
            if (u == NULL) u = owner;
            SIMUser *_u = static_cast<SIMUser*>(u);
            if (!_u->AcceptFileOverride)
                _u = static_cast<SIMUser*>(owner);
            if (_u->AcceptFileOverwrite || (info.size() > f->Size)){
                bTruncate = true;
                size = 0;
            }
        }else{
            emit fileExist(f, name, info.size() < f->curSize());
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
    f->setPos(size);
    f->p = (unsigned long)file;
    return true;
}

bool SIMClient::openFile(ICQFile *f)
{
    QFile *file = new QFile(QString::fromLocal8Bit(f->curName().c_str()));
    if (!file->open(IO_ReadOnly)){
        delete file;
        return false;
    }
    f->p = (unsigned long)file;
    return true;
}

bool SIMClient::seekFile(ICQFile *f, unsigned long pos)
{
    emit fileProcess(f);
    return ((QFile*)(f->p))->at(pos);
}

bool SIMClient::readFile(ICQFile *f, Buffer &b, unsigned short size)
{
    b.allocate(b.writePos() + size, 1024);
    char *p = b.Data(b.writePos());
    int readn = ((QFile*)(f->p))->readBlock(p, size);
    if (readn < 0) return false;
    b.setWritePos(b.writePos() + readn);
    emit fileProcess(f);
    return true;
}

bool SIMClient::writeFile(ICQFile *f, Buffer &b)
{
    if (f->p == 0) return false;
    unsigned size = b.size() - b.readPos();
    char *p = b.Data(b.readPos());
    bool res = (((QFile*)(f->p))->writeBlock(p, size) >= 0);
    emit fileProcess(f);
    return res;
}

void SIMClient::closeFile(ICQFile *f)
{
    if (f->p) delete (QFile*)(f->p);
    f->p = 0;
}

QTextCodec *SIMClient::codecForUser(unsigned long uin)
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

string SIMClient::to8Bit(unsigned long uin, const QString &str)
{
    return to8Bit(codecForUser(uin), str);
}

QString SIMClient::from8Bit(unsigned long uin, const string &str, const char *strCharset)
{
    return from8Bit(codecForUser(uin), str, strCharset);
}

string SIMClient::to8Bit(QTextCodec *codec, const QString &str)
{
    int lenOut = str.length();
    string res;
    if (lenOut == 0) return res;
    res = (const char*)(codec->makeEncoder()->fromUnicode(str, lenOut));
    return res;
}

QString SIMClient::from8Bit(QTextCodec *codec, const string &str, const char *strCharset)
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

void SIMClient::setUserEncoding(unsigned long uin, int i)
{
    if (userEncoding(uin) == i) return;
    ICQUser *u = NULL;
    if ((uin == 0) || (uin == pClient->owner->Uin)){
        u = pClient->owner;
    }else{
        u = getUser(uin);
    }
    if (u == NULL) return;
    QTextCodec *codec = NULL;
    if (i) codec = QTextCodec::codecForMib(i);
    u->Encoding = "";
    if (codec) u->Encoding = codec->name();
    emit encodingChanged(uin);
    ICQEvent e(EVENT_INFO_CHANGED, uin);
    process_event(&e);
    if (uin == pClient->owner->Uin)
        emit encodingChanged(0);
}

int SIMClient::userEncoding(unsigned long uin)
{
    QTextCodec *codec = codecForUser(uin);
    return codec ? codec->mibEnum() : 0;
}

int SIMClient::encodingMib(const QString &s)
{
    QString name = s;
    int p = name.find('(');
    if (p >= 0) name = name.mid(p + 1);
    p = name.find(')');
    if (p > 0) name = name.left(p - 1);
    QTextCodec *codec = QTextCodec::codecForName(name);
    return codec ? codec->mibEnum() : 0;
}

QString SIMClient::encodingName(int mib)
{
    if (mib){
        list<Encoding> enc;
        ::fillEncodings(enc, false);
        ::fillEncodings(enc, true);
        for (list<Encoding>::iterator it = enc.begin(); it != enc.end(); ++it)
            if ((*it).mib == mib) return (*it).name;
    }
    return "";
}

void SIMClient::createGroup(const char *name)
{
	ICQClient::createGroup(name);
}

SIMClient *pClient = NULL;

void SIMClient::getAutoResponse(unsigned long uin, ICQMessage *msg)
{
    string res;
    ICQUser *u = getUser(uin);
    unsigned long status = owner->uStatus;
    if (status & ICQ_STATUS_DND){
        if (u)
            res = u->AutoResponseDND;
        if (*res.c_str() == 0)
            res = owner->AutoResponseDND;
    }else if (status & ICQ_STATUS_OCCUPIED){
        if (u)
            res = u->AutoResponseOccupied;
        if (*res.c_str() == 0)
            res = owner->AutoResponseOccupied;
    }else if (status & ICQ_STATUS_NA){
        if (u)
            res = u->AutoResponseNA;
        if (*res.c_str() == 0)
            res = owner->AutoResponseNA;
    }else if (status & ICQ_STATUS_FREEFORCHAT){
        if (u)
            res = u->AutoResponseFFC;
        if (*res.c_str() == 0)
            res = owner->AutoResponseFFC;
    }else{
        if (u)
            res = u->AutoResponseAway;
        if (*res.c_str() == 0)
            res = owner->AutoResponseAway;
    }
    autoResponse ar;
    ar.msg = msg;
    ar.bDelete = false;
    ar.tmpl = new Tmpl(this);
    connect(ar.tmpl, SIGNAL(ready(Tmpl*, const QString&)), this, SLOT(tmpl_ready(Tmpl*, const QString&)));
    responses.push_back(ar);
    QString str = QString::fromLocal8Bit(res.c_str());
    ar.tmpl->expand(str, uin);
}

void SIMClient::tmpl_ready(Tmpl *t, const QString &res)
{
    for (list<autoResponse>::iterator it = responses.begin(); it != responses.end(); ++it){
        if ((*it).tmpl == t){
            if ((*it).msg){
                QString str = res;
                str.replace(QRegExp("\\r?\\n"), "\r\n");
                pClient->declineMessage((*it).msg, str.local8Bit());
            }
            (*it).bDelete = true;
            QTimer::singleShot(0, this, SLOT(tmpl_clear()));
            break;
        }
    }
}

void SIMClient::tmpl_clear()
{
    for (list<autoResponse>::iterator it = responses.begin(); it != responses.end();){
        if (!(*it).bDelete){
            ++it;
            continue;
        }
        delete (*it).tmpl;
        responses.remove(*it);
        it = responses.begin();
    }
}

void SIMClient::fltrTmplReady(Tmpl *t, const QString &res)
{
    for (list<MsgFilter*>::iterator it = filters.begin(); it != filters.end(); ++it){
        MsgFilter *f = *it;
        if (&f->tmpl != t) continue;
        f->exec.execute(res.local8Bit(), f->msg->Message.c_str());
        return;
    }
    log(L_WARN, "Can't find filter template");
}

void SIMClient::fltrExecReady(Exec *e, int res, const char *out)
{
    for (list<MsgFilter*>::iterator it = filters.begin(); it != filters.end(); ++it){
        MsgFilter *f = *it;
        if (&f->exec != e) continue;
        if (*out == 0){
            if (res == 0){
                delete f->msg;
                f->msg = NULL;
            }
        }else{
            f->msg->Message = out;
        }
        if (f->msg){
            unsigned long uin = f->msg->getUin();
            ICQUser *u = getUser(uin, true);
            History h(uin);
            f->msg->Id = h.addMessage(f->msg);
            u->unreadMsgs.push_back(f->msg->Id);
            f->msg->bDelete = true;
            emit messageReceived(f->msg);
            if (f->msg->bDelete) delete f->msg;
        }
        QTimer::singleShot(50, this, SLOT(fltrDone()));
        return;
    }
    log(L_WARN, "Can't find filter exec");
}

void SIMClient::fltrDone()
{
    for (list<MsgFilter*>::iterator it = filters.begin(); it != filters.end(); ){
        if (!(*it)->bDelete){
            ++it;
            continue;
        }
        MsgFilter *f = *it;
        filters.remove(f);
        delete f;
        it = filters.begin();
    }
}

MsgFilter::MsgFilter(SIMClient *p, ICQMsg *_msg, const char *prog)
        : tmpl(p), exec(p)
{
    bDelete = false;
    msg = _msg;
    msg->bDelete = false;
    p->filters.push_back(this);
    QObject::connect(&tmpl, SIGNAL(ready(Tmpl*, const QString&)), p, SLOT(fltrTmplReady(Tmpl*, const QString&)));
    QObject::connect(&exec, SIGNAL(ready(Exec*, int, const char*)), p, SLOT(fltrExecReady(Exec*, int, const char*)));
    tmpl.expand(QString::fromLocal8Bit(prog), msg->getUin());
}

bool SIMUser::notEnoughInfo(void)
{
    return (Nick.empty()&&FirstName.empty()&&LastName.empty());
}

#ifndef _WINDOWS
#include "client.moc"
#endif
