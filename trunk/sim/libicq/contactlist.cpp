/***************************************************************************
                          contcatlist.cpp  -  description
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

#include "icqclient.h"
#include "icqprivate.h"
#include "country.h"
#include "log.h"

#include <time.h>
#include <stdio.h>

EMailList &EMailList::operator = (const EMailList &l)
{
    const_iterator it;
    for (it = begin(); it != end(); ++it)
        delete *it;
    clear();
    for (it = l.begin(); it != l.end(); ++it)
        push_back(new EMailInfo(*(*it)));
    return *this;
}

EMailList::~EMailList()
{
    for (iterator it = begin(); it != end(); ++it)
        delete *it;
    clear();
}

ExtInfoList &ExtInfoList::operator = (const ExtInfoList &l)
{
    const_iterator it;
    for (it = begin(); it != end(); ++it)
        delete *it;
    clear();
    for (it = l.begin(); it != l.end(); ++it)
        push_back(new ExtInfo(*(*it)));
    return *this;
}

ExtInfoList::~ExtInfoList()
{
    for (iterator it = begin(); it != end(); ++it)
        delete *it;
    clear();
}

PhoneBook &PhoneBook::operator = (const PhoneBook &l)
{
    const_iterator it;
    for (it = begin(); it != end(); ++it)
        delete *it;
    clear();
    for (it = l.begin(); it != l.end(); ++it)
        push_back(new PhoneInfo(*(*it)));
    return *this;
}

PhoneBook::~PhoneBook()
{
    for (iterator it = begin(); it != end(); ++it)
        delete *it;
    clear();
}

ContactList::~ContactList()
{
    for (iterator it = begin(); it != end(); ++it)
        delete *it;
    clear();
}

ICQContactList::~ICQContactList()
{
    for (list<ICQUser*>::iterator itUser = users.begin(); itUser != users.end(); ++itUser)
        delete *itUser;
    users.clear();
    for (vector<ICQGroup*>::iterator itGrp = groups.begin(); itGrp != groups.end(); ++itGrp)
        delete *itGrp;
    groups.clear();
}

ICQContactList::ICQContactList(ICQClient *_client)
{
    Len= 0;
    Invisible = false;
    Time = 0;
    Expand = true;
    client = _client;
}

unsigned short ICQContactList::getUserId(ICQUser *u)
{
    if (u->Id) return u->Id;
    time_t now;
    time(&now);
    for (unsigned short id = now & 0x7FFF;;id++){
        id &= 0x7FFF;
        if (id == 0) continue;
        list<ICQUser*>::iterator it;
        for (it = users.begin(); it != users.end(); it++)
            if ((*it)->Id == id) break;
        if (it == users.end()){
            u->Id = id;
            return id;
        }
    }
}

unsigned short ICQContactList::getGroupId(ICQGroup *g)
{
    if (g->Id) return g->Id;
    time_t now;
    time(&now);
    for (unsigned short id = now & 0x7FFF;;id++){
        id &= 0x7FFF;
        if (id == 0) continue;
        vector<ICQGroup*>::iterator it;
        for (it = groups.begin(); it != groups.end(); it++)
            if ((*it)->Id == id) break;
        if (it == groups.end()){
            g->Id = id;
            return id;
        }
    }
}

ICQGroup *ICQContactList::getGroup(unsigned short id, bool create)
{
    vector<ICQGroup*>::iterator it_group;
    for (it_group = groups.begin(); it_group != groups.end(); it_group++)
        if ((*it_group)->Id == id) return *it_group;
    if (!create) return NULL;
    ICQGroup *grp = new ICQGroup();
    groups.push_back(grp);
    grp->Id = id;
    return grp;
}

ICQGroup *ICQClient::getGroup(unsigned short id, bool create)
{
    ICQGroup *grp = contacts.getGroup(id, false);
    if (grp || !create) return grp;
    grp = contacts.getGroup(id, true);
    ICQEvent e(EVENT_GROUP_CHANGED, id);
    process_event(&e);
    return grp;
}

ICQUser *ICQContactList::getUser(unsigned long uin, bool create)
{
    list<ICQUser*>::iterator it_usr;
    if (uin){
        for (it_usr = users.begin(); it_usr != users.end(); it_usr++){
            if ((*it_usr)->Uin == uin) return *it_usr;
        }
    }
    if (uin == client->owner->Uin) return client->owner;
    if (!create) return NULL;
    ICQUser *usr = new ICQUser();
    if (uin == 0){
        uin = UIN_SPECIAL;
        for (list<ICQUser*>::iterator it = users.begin(); it != users.end(); ++it){
            if ((*it)->Uin < uin) continue;
            uin = (*it)->Uin + 1;
        }
        usr->Type = USER_TYPE_EXT;
    }
    usr->Uin = uin;
    users.push_back(usr);
    return usr;
}

ICQUser *ICQClient::getUser(unsigned long id, bool create, bool bIsTemp)
{
    ICQUser *u = contacts.getUser(id, false);
    if (!create) return u;
    if (u){
        if (!bIsTemp && u->bIsTemp){
            u->bIsTemp = false;
            ICQEvent e(EVENT_INFO_CHANGED, id);
            process_event(&e);
        }
        return u;
    }
    u = contacts.getUser(id, true);
    u->bIsTemp = bIsTemp;
    ICQEvent e(EVENT_INFO_CHANGED, id);
    process_event(&e);
    if (!u->inIgnore)
        p->addToContacts(id);
    addInfoRequest(id, true);
    return u;
}

ICQUser::ICQUser()
{
    Type = 0;
    Id = 0;
    GrpId = 0;
    Uin = 0;
    inIgnore = false;
    inVisible = false;
    inInvisible = false;
    WaitAuth = false;
    uStatus = ICQ_STATUS_OFFLINE;
    prevStatus = ICQ_STATUS_OFFLINE;
    LastActive = 0;
    OnlineTime = 0;
    StatusTime = 0;
    IP = 0;
    Port = 0;
    RealIP = 0;
    Version = 0;
    Mode = 0;
    TimeZone = 0;
    Age = 0;
    Gender = 0;
    BirthYear = 0;
    BirthMonth = 0;
    BirthDay = 0;
    Language1 = 0;
    Language2 = 0;
    Language3 = 0;
    PhoneState = 0;
    PhoneBookTime = 0;
    PhoneStatusTime = 0;
    InfoUpdateTime = 0;
    AlertOverride = false;
    AlertAway = true;
    AlertBlink = true;
    AlertSound = true;
    AlertOnScreen = true;
    AlertPopup = false;
    AlertWindow = false;
    AcceptMsgWindow = false;
    AcceptFileMode = false;
    AcceptFileOverride = false;
    AcceptFileOverwrite = false;
    Caps = 0;
    Build = 0;
    SoundOverride = false;
    IncomingMessage = "message.wav";
    IncomingURL = "url.wav";
    IncomingSMS ="sms.wav";
    IncomingAuth = "auth.wav";
    IncomingFile = "file.wav";
    IncomingChat = "chat.wav";
    OnlineAlert = "alert.wav";
    direct = 0;
    CanPlugin = false;
    NewMessages = 0;
    LastMessageType = 0;
    bMyInfo = false;
    bPhoneChanged = false;
    DCcookie = 0;
    bIsTemp = false;
}

ICQUser::~ICQUser()
{
    if (uStatus != ICQ_STATUS_OFFLINE)
        setOffline();
}

bool ICQUser::hasCap(unsigned n)
{
    return (Caps & (1 << n)) != 0;
}

void ICQUser::setOffline()
{
    if (direct){
        delete direct;
        direct = NULL;
    }
    uStatus = ICQ_STATUS_OFFLINE;
    AutoReply = "";
    CanPlugin = false;
    time_t now;
    time(&now);
    DCcookie = 0;
    StatusTime = (unsigned long)now;
}

string ICQUser::name(bool UseUin)
{
    string s;
    if (Alias.c_str()[0]){
        s = Alias;
        return s;
    }
    if (Nick.c_str()[0]){
        s = Nick;
        return s;
    }
    if (Type == USER_TYPE_EXT){
        for (PhoneBook::iterator itPhone = Phones.begin(); itPhone != Phones.end(); ++itPhone){
            PhoneInfo *info = static_cast<PhoneInfo*>(*itPhone);
            return info->getNumber();
        }
        for (EMailList::iterator itMail = EMails.begin(); itMail != EMails.end(); ++itMail){
            EMailInfo *info = static_cast<EMailInfo*>(*itMail);
            return info->Email;
        }
    }
    if (Uin && UseUin){
        char b[32];
        snprintf(b, sizeof(b), "%lu", Uin);
        s = b;
        return s;
    }
    return s;
}

ICQGroup::ICQGroup()
{
    Id = 0;
    Expand = true;
}

EMailInfo::EMailInfo()
{
    Hide = false;
    MyInfo = false;
}

ExtInfo::ExtInfo()
{
    Category = 0;
}

PhoneInfo::PhoneInfo()
{
    Type = 0;
    Active = false;
    Publish = false;
    FromInfo = false;
    MyInfo = false;
}

void PhoneInfo::setNumber(const string &number, unsigned long type)
{
    Type = type;
    char *numberBuf = new char[number.length() + 1];
    char *f = (char*)number.c_str();
    char *t = numberBuf;
    for (; *f; f++){
        if ((*f < '0') || (*f > '9')) continue;
        *(t++) = *f;
    }
    *t = 0;
    f = numberBuf;
    const ext_info *c;
    for (c = countries; *c->szName; c++){
        char b[16];
        snprintf(b, sizeof(b), "%u", c->nCode);
        if (strlen(f) < strlen(b)) continue;
        if (!memcmp(f, b, strlen(b))){
            Country = c->szName;
            f += strlen(b);
            break;
        }
    }
    if (strlen(f) > 7){
        t = f + strlen(f) - 7;
        char c = *t;
        *t = 0;
        AreaCode = f;
        *t = c;
        f = t;
    }
    Number = f;
    delete[] numberBuf;
}

string PhoneInfo::getNumber()
{
    string res;
    if (FromInfo) return Number;
    if (Type == PAGER){
        res = Number;
        if (Gateway.size()){
            res += "@";
            res += Gateway;
            return res;
        }
        for (const pager_provider *p = providers; *(p->szName); p++){
            if (Provider == p->szName){
                res += "@";
                res += p->szGate;
                return res;
            }
        }
        return res;
    }
    const ext_info *c;
    for (c = countries; *c->szName; c++){
        if (!strcmp(c->szName, Country.c_str())){
            char b[16];
            snprintf(b, sizeof(b), "+%u", c->nCode);
            res = b;
            break;
        }
    }
    if (*AreaCode.c_str()){
        res += " (";
        res += AreaCode.c_str();
        res += ") ";
    }
    res += Number.c_str();
    if (*Extension.c_str()){
        res += " - ";
        res += Extension.c_str();
    }
    return res;
}

static string strip_number(const char *s)
{
    string res;
    for (; *s; s++)
        if ((*s >= '0') && (*s <= '9')) res += *s;
    return res;
}

bool PhoneInfo::isEqual(const char *number)
{
    string my_number = getNumber();
    return isEqual(my_number.c_str(), number);
}

bool PhoneInfo::isEqual(const char *number1, const char *number)
{
    string n1 = strip_number(number1);
    string n2 = strip_number(number);
    if (!strcmp(n1.c_str(), n2.c_str())) return true;
    int s1 = n1.length();
    int s2 = n2.length();
    int s = s1;
    if (s2 < s1) s = s2;
    int nMatch;
    for (nMatch = 0; nMatch < s; nMatch++)
        if (n1[s1-1-nMatch] != n2[s2-1-nMatch]) break;
    return (nMatch >= 7);
}

void ICQUser::adjustPhones()
{
    PhoneBook::iterator it;
    for (it = Phones.begin(); it != Phones.end(); ){
        PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
        if ((phone->FromInfo) || (*phone->Name.c_str() == 0)){
            Phones.remove(*it);
            delete phone;
            it = Phones.begin();
            continue;
        }
        PhoneBook::iterator it1;
        string number = phone->getNumber();
        for (it1 = Phones.begin(); it1 != Phones.end(); ++it1){
            if ((*it1) == phone) break;
            if ((*it1)->isEqual(number.c_str())) break;
        }
        if ((*it1) != phone){
            Phones.remove(*it);
            delete phone;
            it = Phones.begin();
            continue;
        }
        phone->Publish = false;
        it++;
    }
    Phones.add(HomePhone.c_str(), "Home phone", PHONE, bMyInfo, true);
    Phones.add(HomeFax.c_str(), "Home fax", FAX, bMyInfo, true);
    Phones.add(PrivateCellular.c_str(), "Private cellular", SMS, bMyInfo, true);
    Phones.add(WorkPhone.c_str(), "Work phone", PHONE, bMyInfo, true);
    Phones.add(WorkFax.c_str(), "Work fax", FAX, bMyInfo, true);
}

bool ICQUser::canRTF()
{
    return (Version >= 8) && (uStatus != ICQ_STATUS_OFFLINE) && hasCap(CAP_RTF);
}

bool ICQUser::isSecure()
{
    return (direct != NULL) && direct->isSecure();
}

static void addEMail(EMailList &mails, EMailInfo *mail)
{
    if (*mail->Email.c_str() == 0) return;
    EMailList::iterator it;
    EMailInfo *info = NULL;
    for (it = mails.begin(); it != mails.end(); ++it){
        info = static_cast<EMailInfo*>(*it);
        if (info->Email == mail->Email) break;
    }
    if (it != mails.end()){
        info->MyInfo = info->MyInfo && mail->MyInfo;
        info->Hide = info->Hide || mail->Hide;
        return;
    }
    info = new EMailInfo;
    info->Email = mail->Email;
    info->Hide = mail->Hide;
    info->MyInfo = mail->MyInfo;
    mails.push_back(info);
}

static void addMyEMails(EMailList &mails, const EMailList &add, bool bOwn)
{
    for (EMailList::const_iterator it = add.begin(); it != add.end(); ++it){
        EMailInfo *info = static_cast<EMailInfo*>(*it);
        if (!bOwn && !info->MyInfo) continue;
        if (bOwn) info->MyInfo = true;
        addEMail(mails, info);
    }
}

void ICQUser::adjustEMails(EMailList *addMails, bool bOwn)
{
    EMailList mails;
    bool myInfoFirst = false;
    if (EMails.size()){
        EMailInfo *info = static_cast<EMailInfo*>(*EMails.begin());
        if (info->MyInfo) myInfoFirst = true;
    }
    if (myInfoFirst) addMyEMails(mails, EMails, bOwn);
    if (*EMail.c_str()){
        EMailInfo info;
        info.Email = EMail.c_str();
        info.Hide = HiddenEMail;
        addEMail(mails, &info);
    }
    if (addMails){
        for (EMailList::const_iterator it = addMails->begin(); it != addMails->end(); ++it){
            EMailInfo *info = static_cast<EMailInfo*>(*it);
            addEMail(mails, info);
        }
    }
    if (!myInfoFirst) addMyEMails(mails, EMails, bOwn);
    EMails = mails;
}

#ifndef HAVE_STRCASECMP

int strcasecmp(const char *a, const char *b)
{
    for (; *a && *b; a++, b++){
        if (tolower(*a) < tolower(*b)) return -1;
        if (tolower(*a) > tolower(*b)) return 1;
    }
    if (*a) return 1;
    if (*b) return -1;
    return 0;
}

#endif

void PhoneBook::add(const char *number, const char *name, unsigned long type, bool ownInfo, bool publish)
{
    if (*number == 0) return;
    char *p = (char*)number;
    for (; *p ; p++)
        if ((*p >= '0') && (*p <= '9')) break;
    if (*p == 0) return;
    PhoneBook::iterator it;
    for (it = begin(); it != end(); it++){
        PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
        if (phone->FromInfo) continue;
        if (phone->Type != type) continue;
        if (phone->isEqual(number)){
            if (publish && !phone->Publish)
                phone->Publish = true;
            break;
        }
    }
    if (it != end()) return;
    PhoneInfo *phone = new PhoneInfo;
    phone->setNumber(number, type);
    phone->Name = name;
    phone->FromInfo = true;
    phone->Publish = publish;
    phone->MyInfo = ownInfo;
    push_back(phone);
}

void PhoneBook::add(const PhoneBook &l)
{
    for (PhoneBook::const_iterator itList = l.begin(); itList != l.end(); ++itList){
        PhoneInfo *infoAdd = static_cast<PhoneInfo*>(*itList);
        iterator it;
        for (it = begin(); it != end(); ++it){
            PhoneInfo *info = static_cast<PhoneInfo*>(*it);
            if (info->Type != infoAdd->Type) continue;
            if (info->isEqual(infoAdd->getNumber().c_str())) break;
        }
        if (it != end()) continue;
        PhoneInfo *addInfo = new PhoneInfo;
        *addInfo = *infoAdd;
        push_back(addInfo);
    }
}

unsigned long ICQContactList::findByEmail(const string &name, const string &email)
{
    list<ICQUser*>::iterator it;
    for (it = users.begin(); it != users.end(); ++it){
        ICQUser *u = *it;
        if (strcasecmp(u->Nick.c_str(), name.c_str())) continue;
        EMailList::iterator itMail;
        for (itMail = u->EMails.begin(); itMail != u->EMails.end(); ++itMail){
            EMailInfo *mail = static_cast<EMailInfo*>(*itMail);
            if (!strcasecmp(mail->Email.c_str(), email.c_str()))
                break;
        }
        if (itMail != u->EMails.end()) break;
    }
    if (it != users.end()) return (*it)->Uin;
    for (it = users.begin(); it != users.end(); ++it){
        ICQUser *u = *it;
        EMailList::iterator itMail;
        for (itMail = u->EMails.begin(); itMail != u->EMails.end(); ++itMail){
            EMailInfo *mail = static_cast<EMailInfo*>(*itMail);
            if (!strcasecmp(mail->Email.c_str(), email.c_str()))
                break;
        }
        if (itMail != u->EMails.end()) break;
    }
    if (it != users.end()) return (*it)->Uin;
    ICQUser *u = getUser(0, true);
    u->Nick = name;
    if (!*u->Nick.c_str()) u->Nick = email;
    u->EMail = email;
    u->Type = USER_TYPE_EXT;
    return u->Uin;
}

unsigned long ICQContactList::findByPhone(const string &phone)
{
    list<ICQUser*>::iterator it;
    for (it = users.begin(); it != users.end(); ++it){
        ICQUser *u = *it;
        PhoneBook::iterator itPhone;
        for (itPhone = u->Phones.begin(); itPhone != u->Phones.end(); ++itPhone){
            PhoneInfo *phoneInfo = static_cast<PhoneInfo*>(*itPhone);
            if (phoneInfo->isEqual(phone.c_str())) break;
        }
        if (itPhone != u->Phones.end()) break;
    }
    if (it != users.end()) return (*it)->Uin;
    ICQUser *u = getUser(0, true);
    u->Nick = phone;
    PhoneInfo *phoneInfo = new PhoneInfo;
    phoneInfo->setNumber(phone, SMS);
    u->Phones.push_back(phoneInfo);
    u->Type = USER_TYPE_EXT;
    return u->Uin;
}

bool ICQUser::isOnline()
{
    switch (Type){
    case USER_TYPE_ICQ:
        switch (uStatus & 0xFF){
        case ICQ_STATUS_ONLINE:
        case ICQ_STATUS_FREEFORCHAT:
            return true;
        default:
            return false;
        }
    default:
        break;
    }
    return false;
}

string ICQUser::client()
{
    string res;
    string add;
    unsigned long id = InfoUpdateTime;
    unsigned long id2 = PhoneStatusTime;
    unsigned long id3 = PhoneBookTime;
    unsigned char v1 = 0;
    unsigned char v2 = 0;
    unsigned char v3 = 0;
    unsigned char v4 = 0;
    unsigned int ver = id & 0xffff;
    if (((id & 0xff7f0000) == 0x7d000000L) && (ver > 1000))
    {
        res = "Licq";
        v1 = ver/1000;
        v2 = (ver / 10) % 100;
        v3 = ver % 10;
        v4 = 0;
        if (id & 0x00800000L)
            add += "/SSL";
    }
    else if ((id == 0xffffff42L) || ((id & 0xff7f0000) == 0x7d000000L))
    {
        res = "mICQ";
        v1 = ver / 10000;
        v2 = (ver / 100) % 100;
        v3 = (ver / 10) % 10;
        v4 = ver % 10;
    }
    else if ((id & 0xffff0000L) == 0xffff0000L)
    {
        v1 = (id2 >> 24) & 0x7F;
        v2 = (id2 >> 16) & 0xFF;
        v3 = (id2 >> 8) & 0xFF;
        v4 = id2 & 0xFF;
        switch (id){
        case 0xffffffffL:
            res = "Miranda";
            if (id2 & 0x80000000L) add += " alpha";
            break;
        case 0xffffff8fL:
            res += "StrICQ";
            break;
        case 0xffffffabL:
            res = "YSM";
            if ((v1 | v2 | v3 | v4) & 0x80)
                v1 = v2 = v3 = v4 = 0;
            break;
        case 0xffffff7fL:
            res = "&RQ";
            break;
        }
    }
    else if (id == 0x04031980L)
    {
        v1 = 0;
        v2 = 43;
        v3 = id2 & 0xffff;
        v4 = id2 & (0x7fff0000) >> 16;
        res = "vICQ";
    }
    else if ((id == 0x3b75ac09) && (id2 == 0x3bae70b6) && (id3 == 0x3b744adb))
    {
        res = "Trillian";
    }
    else if ((id == id2) && (id2 == id3) && (id == 0xffffffff))
        res = "vICQ/GAIM(?)";
    else if (id && (id == id2) && (id2 == id3) && (Caps == 0))
        res = "vICQ";
    else if ((Version == 7) && hasCap(CAP_IS_WEB))
        res = "ICQ2go";
    else if ((Version == 9) && hasCap(CAP_IS_WEB))
        res = "ICQ Lite";
    else if (hasCap(CAP_TRIL_CRYPT) || hasCap(CAP_TRILLIAN))
        res = "Trillian";
    else if (hasCap(CAP_LICQ))
        res = "Licq";
    else if (hasCap(CAP_SIM))
    {
        res = "SIM";
        v1 = (Build >> 6) - 1;
        v2 = Build & 0x1F;
        if (Build == 0){
            res = "Kopete";
            v1 = v2 = 0;
        }
    }
    else if (hasCap(CAP_STR_2002) && hasCap(CAP_IS_2002))
        res = "ICQ 2002";
    else if ((hasCap(CAP_STR_2001) || hasCap(CAP_STR_2002)) && hasCap(CAP_IS_2001))
        res = "ICQ 2001";
    else if (hasCap(CAP_MACICQ))
        res = "ICQ for Mac";
    else if ((Version == 8) && hasCap(CAP_IS_2002))
        res = "ICQ 2002 (?)";
    else if ((Version == 8) && hasCap(CAP_IS_2001))
        res = "ICQ 2001 (?)";
    else if (hasCap(CAP_AIM_CHAT))
        res = "AIM(?)";
    else if ((Version == 7) && !hasCap(CAP_RTF))
        res = "ICQ 2000 (?)";
    else if (Version == 7)
        res = "GnomeICU";
    if (Version){
        char b[32];
        snprintf(b, sizeof(b), "v%u ", Version);
        string s = b;
        res = s + res;
    }
    if (v1 || v2){
        char b[32];
        snprintf(b, sizeof(b), " %u.%u", v1, v2);
        res += b;
    }
    if (v3){
        char b[32];
        snprintf(b, sizeof(b), ".%u", v3);
        res += b;
    }
    if (v4){
        char b[32];
        snprintf(b, sizeof(b), ".%u", v4);
        res += b;
    }
    if (*add.c_str())
        res += add;
    return res;
}
