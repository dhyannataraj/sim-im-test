/***************************************************************************
                          contcatlist.cpp  -  description
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
    init();
}

ICQContactList::ICQContactList(ICQClient *_client)
{
    client = _client;
    init();
}

void ICQContactList::init()
{
    Len= 0;
    Invisible = false;
    Time = 0;
    Expand = true;
    for (list<ICQUser*>::iterator itUser = users.begin(); itUser != users.end(); ++itUser)
        delete *itUser;
    users.clear();
    for (vector<ICQGroup*>::iterator itGrp = groups.begin(); itGrp != groups.end(); ++itGrp)
        delete *itGrp;
    groups.clear();
}

unsigned short ICQContactList::getUserId(ICQUser *u, unsigned short grpId, bool bNoStore)
{
    switch (grpId){
    case ICQ_VISIBLE_LIST:
        if (u->VisibleId) return u->VisibleId;
        break;
    case ICQ_INVISIBLE_LIST:
        if (u->InvisibleId) return u->InvisibleId;
        break;
    case ICQ_IGNORE_LIST:
        if (u->IgnoreId) return u->IgnoreId;
        break;
    default:
        if (u->Id) return u->Id;
    }
    time_t now;
    time(&now);
    for (unsigned short id = now & 0x7FFF;;id++){
        id &= 0x7FFF;
        if (id == 0) continue;
        list<ICQUser*>::iterator it;
        for (it = users.begin(); it != users.end(); it++){
            if ((*it)->Id == id) break;
            if ((*it)->VisibleId == id) break;
            if ((*it)->InvisibleId == id) break;
            if ((*it)->IgnoreId == id) break;
        }
        if (it == users.end()){
            if (!bNoStore){
                switch (grpId){
                case ICQ_VISIBLE_LIST:
                    u->VisibleId = id;
                    break;
                case ICQ_INVISIBLE_LIST:
                    u->InvisibleId = id;
                    break;
                case ICQ_IGNORE_LIST:
                    u->IgnoreId = id;
                    break;
                }
            }
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
    ICQGroup *grp = client->createGroup();
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
    ICQUser *usr = client->createUser();
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
    ICQEvent e(EVENT_INFO_CHANGED, u->Uin);
    process_event(&e);
    if (id == 0) return u;
    if (u->IgnoreId == 0)
        p->addToContacts(id);
    addInfoRequest(id, true);
    return u;
}

ICQUser::ICQUser()
{
    Type = 0;
    Id = 0;
    VisibleId = 0;
    InvisibleId = 0;
    IgnoreId = 0;
    GrpId = 0;
    Uin = 0;
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
    Caps = 0;
    Build = 0;
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
    if (FirstName.c_str()[0])
        s = FirstName;
    if (LastName.c_str()[0]){
        if (!s.empty())
            s += " ";
        s += LastName;
    }
    if (!s.empty())
        return s;
    if (Type == USER_TYPE_EXT)
        return s;
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

bool ICQUser::canUTF()
{
    return (Version >= 8) && (uStatus != ICQ_STATUS_OFFLINE) && hasCap(CAP_UTF);
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
    u->adjustEMails(NULL, true);
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

static string verString(unsigned ver)
{
    string res;
    if (ver == 0) return res;
    unsigned char v[4];
    v[0] = (ver >> 24) & 0xFF;
    v[1] = (ver >> 16) & 0xFF;
    v[2] = (ver >> 8) & 0xFF;
    v[3] = ver & 0xFF;
    if ((v[0] & 0x80) || (v[1] & 0x80) || (v[2] & 0x80) || (v[3] & 0x80))
        return res;
    char b[32];
    snprintf(b, sizeof(b), " %u.%u", v[0], v[1]);
    res = b;
    if (v[2] || v[3]){
        snprintf(b, sizeof(b), ".%u", v[2]);
        res += b;
    }
    if (v[3]){
        snprintf(b, sizeof(b), ".%u", v[3]);
        res += b;
    }
    return res;
}

string ICQUser::client()
{
    string res;
    char b[32];
    if (Version)
    {
        snprintf(b, sizeof(b), "v%u ", Version);
        res = b;
    }

    if (hasCap(CAP_SIMOLD))
    {
        if (Build == 0){
            res += "Kopete";
            return res;
        }
        snprintf(b, sizeof(b), "SIM %lu.%lu", (Build >> 6) - 1, Build & 0x1F);
        res += b;
        return res;
    }

    if (hasCap(CAP_SIM))
    {
        unsigned ver1 = (Build >> 24) & 0xFF;
        unsigned ver2 = (Build >> 16) & 0xFF;
        unsigned ver3 = (Build >> 8) & 0xFF;
        if (ver3){
            snprintf(b, sizeof(b), "SIM %u.%u.%u", ver1, ver2, ver3);
        }else{
            snprintf(b, sizeof(b), "SIM %u.%u", ver1, ver2);
        }
        res += b;
        if (Build & 0x80)
            res += "/win32";
        return res;
    }

    if (hasCap(CAP_LICQ))
    {
        unsigned ver1 = (Build >> 24) & 0xFF;
        unsigned ver2 = (Build >> 16) & 0xFF;
        unsigned ver3 = (Build >> 8) & 0xFF;
        if (ver3){
            snprintf(b, sizeof(b), "Licq %u.%u.%u", ver1, ver2, ver3);
        }else{
            snprintf(b, sizeof(b), "Licq %u.%u", ver1, ver2);
        }
        res += b;
        if (Build & 0xFF)
            res += "/SSL";
        return res;
    }

    if (hasCap(CAP_IS_WEB))
    {
        res += (Version == 9) ? "ICQ Lite" : "ICQ2go";
        return res;
    }

    if (hasCap(CAP_TRIL_CRYPT) || hasCap(CAP_TRILLIAN))
    {
        res += "Trillian";
        return res;
    }

    if (hasCap(CAP_MACICQ)){
        res += "ICQ for Mac";
        return res;
    }

    if (hasCap(CAP_AIM_CHAT)){
        res += "AIM";
        return res;
    }
    if ((InfoUpdateTime & 0xFF7F0000L) == 0x7D000000L){
        unsigned ver = InfoUpdateTime & 0xFFFF;
        if (ver % 10){
            snprintf(b, sizeof(b), "%u.%u.%u", ver / 1000, (ver / 10) % 100, ver % 10);
        }else{
            snprintf(b, sizeof(b), "%u.%u", ver / 1000, (ver / 10) % 100);
        }
        res += b;
        if (InfoUpdateTime & 0x00800000L)
            res += "/SSL";
        return res;
    }
    switch (InfoUpdateTime){
    case 0xFFFFFFFFL:
        if ((PhoneStatusTime == 0xFFFFFFFFL) && (PhoneBookTime == 0xFFFFFFFFL)){
            res += "GAIM";
            return res;
        }
        res += "MIRANDA";
        res += verString(PhoneStatusTime & 0xFFFFFF);
        if (PhoneStatusTime & 0x80000000)
            res += " alpha";
        return res;
    case 0xFFFFFF8FL:
        res += "StrICQ";
        res += verString(PhoneStatusTime & 0xFFFFFF);
        return res;
    case 0xFFFFFF42L:
        res += "mICQ";
        return res;
    case 0xFFFFFFBEL:
        res += "alicq";
        res += verString(PhoneStatusTime & 0xFFFFFF);
        return res;
    case 0xFFFFFF7FL:
        res += "&RQ";
        res += verString(PhoneStatusTime & 0xFFFFFF);
        return res;
    case 0xFFFFFFABL:
        res += "YSM";
        res += verString(PhoneStatusTime & 0xFFFFFF);
        return res;
    case 0x04031980L:
        snprintf(b, sizeof(b), "vICQ 0.43.%lu.%lu", PhoneStatusTime & 0xffff, PhoneStatusTime & (0x7fff0000) >> 16);
        res += b;
        return res;
    case 0x3AA773EEL:
        if ((PhoneStatusTime == 0x3AA66380L) && (PhoneBookTime == 0x3A877A42L))
        {
            res += "libicq2000";
            return res;
        }
        break;
    }

    if (InfoUpdateTime && (InfoUpdateTime == PhoneStatusTime) && (PhoneStatusTime == PhoneBookTime) && (Caps == 0)){
        res += "vICQ";
        return res;
    }
    if (hasCap(CAP_STR_2002) && hasCap(CAP_UTF)){
        res += "ICQ 2002";
        return res;
    }
    if ((hasCap(CAP_STR_2001) || hasCap(CAP_STR_2002)) && hasCap(CAP_IS_2001)){
        res += "ICQ 2001";
        return res;
    }
    if ((Version == 7) && hasCap(CAP_RTF)){
        res += "GnomeICU";
        return res;
    }
    return res;
}
