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
#include "country.h"
#include "log.h"

#include <time.h>
#include <stdio.h>

ICQContactList::ICQContactList(ICQClient *_client)
        : Len(this, "Length"),
        Invisible(this, "Invsible"),
        Time(this, "Time"),
        Expand(this, "Expand", true)
{
    client = _client;
}

ICQContactList::~ICQContactList()
{
    list<ICQUser*>::iterator it_user;
    for (it_user = users.begin(); it_user != users.end(); it_user++)
        delete *it_user;
    vector<ICQGroup*>::iterator it_group;
    for (it_group = groups.begin(); it_group != groups.end(); it_group++)
        delete *it_group;
}

unsigned short ICQContactList::getUserId(ICQUser *u)
{
    if (u->Id()) return u->Id();
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
    if (g->Id()) return g->Id();
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
        if ((*it_group)->Id() == id) return *it_group;
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
            if ((*it_usr)->Uin() == uin) return *it_usr;
        }
    }
    if (uin == client->Uin()) return client;
    if (!create) return NULL;
    ICQUser *usr = new ICQUser();
    if (uin == 0){
        uin = UIN_SPECIAL;
        for (list<ICQUser*>::iterator it = users.begin(); it != users.end(); ++it){
            if ((*it)->Uin() < uin) continue;
            uin = (*it)->Uin() + 1;
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
    if (!u->inIgnore())
        addToContacts(id);
    addInfoRequest(id, true);
    return u;
}

ICQUser::ICQUser()
        : Type(this, "Type"),
        Alias(this, "Alias"),
        Id(this, "Id"),
        GrpId(this, "GrpId"),
        Uin(this, "UIN"),
        inIgnore(this, "Ignore"),
        inVisible(this, "Visible"),
        inInvisible(this, "Invisible"),
        WaitAuth(this, "WaitAuth"),
        AutoResponseAway(this, "AutoResponseAway"),
        AutoResponseNA(this, "AutoResponseNA"),
        AutoResponseDND(this, "AutoResponseDND"),
        AutoResponseOccupied(this, "AutoResponseOccupied"),
        AutoResponseFFC(this, "AutoResponseFFC"),
        uStatus(ICQ_STATUS_OFFLINE),
        prevStatus(ICQ_STATUS_OFFLINE),
        unreadMsgs(this, "UnreadMessages"),
        LastActive(this, "LastActive"),
        OnlineTime(this, "OnlineTime"),
        StatusTime(this, "StatusTime"),
        IP(this, "IP"),
        Port(this, "Port"),
        RealIP(this, "RealIP"),
        HostName(this, "HostName"),
        RealHostName(this, "RealHostName"),
        Version(this, "Version"),
        Mode(this, "Mode"),
        Nick(this, "Nick"),
        FirstName(this, "FirstName"),
        LastName(this, "LastName"),
        City(this, "City"),
        State(this, "State"),
        Address(this, "Address"),
        Zip(this, "Zip"),
        Country(this, "Country"),
        TimeZone(this, "TimeZone"),
        HomePhone(this, "HomePhone"),
        HomeFax(this, "HomeFax"),
        PrivateCellular(this, "PrivateCellular"),
        EMail(this, "EMailInfo"),
        HiddenEMail(this, "HiddenEMail"),
        Notes(this, "Notes"),
        EMails(this, "EMail"),
        Age(this, "Age"),
        Gender(this, "Gender"),
        Homepage(this, "Homepage"),
        BirthYear(this, "BirthYear"),
        BirthMonth(this, "BirthMonth"),
        BirthDay(this, "BirthDay"),
        Language1(this, "Language1"),
        Language2(this, "Language2"),
        Language3(this, "Language3"),
        WorkCity(this, "WorkCity"),
        WorkState(this, "WorkState"),
        WorkZip(this, "WorkZip"),
        WorkAddress(this, "WorkAddress"),
        WorkName(this, "WorkName"),
        WorkDepartment(this, "WorkDepartment"),
        WorkPosition(this, "WorkPosition"),
        WorkCountry(this, "WorkCountry"),
        Occupation(this, "Occupation"),
        WorkHomepage(this, "WorkHomepage"),
        WorkPhone(this, "WorkPhone"),
        WorkFax(this, "WorkFax"),
        About(this, "About"),
        Backgrounds(this, "Background"),
        Affilations(this, "Affilation"),
        Interests(this, "Interest"),
        Phones(this, "Phone"),
        PhoneState(this, "PhoneState"),
        PhoneBookTime(this, "PhoneBookTime"),
        PhoneStatusTime(this, "PhoneStatusTime"),
        TimeStamp(this, "TimeStamp"),
        AlertOverride(this, "AlertOverride"),
        AlertAway(this, "AlertAway", true),
        AlertBlink(this, "AlertBlink", true),
        AlertSound(this, "AlertSound", true),
        AlertOnScreen(this, "AlertOnScreen", true),
        AlertPopup(this, "AlertPopup"),
        AlertWindow(this, "AlertWindow"),
        AcceptMsgWindow(this, "AcceptMsgWindow"),
        AcceptFileMode(this, "AcceptFileMode"),
        AcceptFileOverride(this, "AcceptFileOverride"),
        AcceptFileOverwrite(this, "AcceptFileOverwrite"),
        AcceptFilePath(this, "AcceptFilePath"),
        DeclineFileMessage(this, "DeclineFileMessage"),
        ClientType(this, "ClientType"),
        SoundOverride(this, "SoundOverride"),
        IncomingMessage(this, "IncomingMessage", "message.wav"),
        IncomingURL(this, "IncomingURL", "url.wav"),
        IncomingSMS(this, "IncomingSMS", "sms.wav"),
        IncomingAuth(this, "IncomingAuth", "auth.wav"),
        IncomingFile(this, "IncomingFile", "file.wav"),
        IncomingChat(this, "IncomingChat", "chat.wav"),
        OnlineAlert(this, "OnlineAlert", "alert.wav"),
        Encoding(this, "Encoding")
{
    direct = 0;
    GetRTF = false;
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

void ICQUser::setOffline()
{
    if (direct){
        delete direct;
        direct = NULL;
    }
    uStatus = ICQ_STATUS_OFFLINE;
    AutoReply = "";
    CanPlugin = false;
    CanResponse = false;
    time_t now;
    time(&now);
    DCcookie = 0;
    StatusTime = (unsigned long)now;
}

bool ICQUser::load(std::istream &s, string &nextPart)
{
    if (!ConfigArray::load(s, nextPart)) return false;
    for (;;){
        bool ok = true;
        list<unsigned long>::iterator it;
        for (it = unreadMsgs.begin(); it != unreadMsgs.end(); ++it){
            if (*it >= MSG_PROCESS_ID){
                unreadMsgs.remove(*it);
                ok = false;
                break;
            }
        }
        if (ok) break;
    }
    adjustPhones();
    adjustEMails(EMails);
    return true;
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
    if (Type() == USER_TYPE_EXT){
        for (PhoneBook::iterator itPhone = Phones.begin(); itPhone != Phones.end(); ++itPhone){
            PhoneInfo *info = static_cast<PhoneInfo*>(*itPhone);
            return info->getNumber();
        }
        for (EMailList::iterator itMail = EMails.begin(); itMail != EMails.end(); ++itMail){
            EMailInfo *info = static_cast<EMailInfo*>(*itMail);
            return info->Email;
        }
    }
    if (Uin() && UseUin){
        char b[32];
        snprintf(b, sizeof(b), "%lu", Uin());
        s = b;
        return s;
    }
    return s;
}

ICQGroup::ICQGroup()
        : Name(this, "Name"),  Id(this, "Id"), Expand(this, "Expand", true)
{
}

EMailInfo::EMailInfo()
        : Email(this, "Email"),
        Hide(this, "Hide"),
        MyInfo(this, "MyInfo")
{
}

ExtInfo::ExtInfo()
        : Category(this, "Category"),
        Specific(this, "Specific")
{
}

PhoneInfo::PhoneInfo()
        : Name(this, "Name"),
        Type(this, "Type"),
        Active(this, "Active"),
        Country(this, "Country"),
        AreaCode(this, "AreaCode"),
        Number(this, "Number"),
        Extension(this, "Extension"),
        Provider(this, "Provider"),
        Gateway(this, "Gateway"),
        Publish(this, "Publish"),
        FromInfo(this, "FromInfo"),
        MyInfo(this, "MyInfo")
{
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
    if (Type() == PAGER){
        res = Number;
        if (Gateway){
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
    if (!*c->szName) return res;
    res += " (";
    res += AreaCode.c_str();
    res += ") ";
    res += Number.c_str();
    if (*Extension.c_str()){
        res += " - ";
        res += Extension.c_str();
    }
    return res;
}

bool PhoneInfo::isEqual(const char *number)
{
    string my_number = getNumber();
    int p1 = strlen(number) - 1;
    int p2 = strlen(my_number.c_str()) - 1;
    int nMatch = 0;
    for (; (p1 >= 0) && (p2 >= 0); p1--, p2--){
        char c1 = 0;
        char c2 = 0;
        for (; p1 >= 0; p1--){
            c1 = number[p1];
            if ((c1 >= '0') && (c1 <= '9')) break;
        }
        if (p1 < 0) break;
        for (; p2 >= 0; p2--){
            c2 = my_number.c_str()[p2];
            if ((c2 >= '0') && (c1 <= '9')) break;
        }
        if (p2 < 0) break;
        if (c1 != c2){
            nMatch = 0;
            break;
        }
        nMatch++;
        if (nMatch >= 7) return true;
    }
    return (nMatch >= 7);
}

void ICQUser::adjustPhones()
{
    PhoneBook::iterator it;
    for (it = Phones.begin(); it != Phones.end(); ){
        PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
        if (phone->FromInfo()){
            Phones.remove(*it);
            delete phone;
            it = Phones.begin();
            continue;
        }
        phone->Publish = false;
        it++;
    }
    Phones.add(HomePhone.c_str(), "Home phone", PHONE, bMyInfo);
    Phones.add(HomeFax.c_str(), "Home fax", FAX, bMyInfo);
    Phones.add(PrivateCellular.c_str(), "Private cellular", SMS, bMyInfo);
    Phones.add(WorkPhone.c_str(), "Work phone", PHONE, bMyInfo);
    Phones.add(WorkFax.c_str(), "Work fax", FAX, bMyInfo);
}

static void addEMail(ConfigPtrList &mails, EMailInfo *mail)
{
    if (*mail->Email.c_str() == 0) return;
    ConfigPtrList::iterator it;
    EMailInfo *info = NULL;
    for (it = mails.begin(); it != mails.end(); ++it){
        info = static_cast<EMailInfo*>(*it);
        if (info->Email == mail->Email) break;
    }
    if (it != mails.end()){
        info->MyInfo = info->MyInfo() && mail->MyInfo();
        info->Hide = info->Hide() || mail->Hide();
        return;
    }
    info = new EMailInfo;
    info->Email = mail->Email;
    info->Hide = mail->Hide;
    info->MyInfo = mail->MyInfo;
    mails.push_back(info);
}

static void addMyEMails(ConfigPtrList &mails, const ConfigPtrList &add)
{
    for (ConfigPtrList::const_iterator it = add.begin(); it != add.end(); ++it){
        EMailInfo *info = static_cast<EMailInfo*>(*it);
        if (!info->MyInfo) continue;
        addEMail(mails, info);
    }
}

void ICQUser::adjustEMails(const ConfigPtrList &addMails)
{
    EMailPtrList mails;
    bool myInfoFirst = false;
    if (EMails.size()){
        EMailInfo *info = static_cast<EMailInfo*>(*EMails.begin());
        if (info->MyInfo) myInfoFirst = true;
    }
    if (myInfoFirst) addMyEMails(mails, EMails);
    if (*EMail.c_str()){
        EMailInfo info;
        info.Email = EMail.c_str();
        info.Hide = HiddenEMail;
        addEMail(mails, &info);
    }
    for (ConfigPtrList::const_iterator it = addMails.begin(); it != addMails.end(); ++it){
        EMailInfo *info = static_cast<EMailInfo*>(*it);
        addEMail(mails, info);
    }
    if (!myInfoFirst) addMyEMails(mails, EMails);
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

void PhoneBook::add(const char *number, const char *name, unsigned long type, bool ownInfo)
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
        if (phone->Type() != type) continue;
        if (phone->isEqual(number)){
            phone->Publish = true;
            break;
        }
    }
    if (it != end()) return;
    PhoneInfo *phone = new PhoneInfo;
    phone->Type = type;
    phone->Name = name;
    phone->Number = number;
    phone->FromInfo = true;
    phone->Publish = true;
    phone->MyInfo = ownInfo;
    push_back(phone);
}

void PhoneBook::add(const PhonePtrList &l)
{
    for (PhonePtrList::const_iterator itList = l.begin(); itList != l.end(); ++itList){
        PhoneInfo *infoAdd = static_cast<PhoneInfo*>(*itList);
        iterator it;
        for (it = begin(); it != end(); ++it){
            PhoneInfo *info = static_cast<PhoneInfo*>(*it);
            if (info->Type() != infoAdd->Type()) continue;
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
    if (it != users.end()) return (*it)->Uin();
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
    if (it != users.end()) return (*it)->Uin();
    ICQUser *u = getUser(0, true);
    u->Nick = name;
    if (!*u->Nick.c_str()) u->Nick = email;
    u->EMail = email;
    u->Type = USER_TYPE_EXT;
    return u->Uin();
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
    if (it != users.end()) return (*it)->Uin();
    ICQUser *u = getUser(0, true);
    u->Nick = phone;
    PhoneInfo *phoneInfo = new PhoneInfo;
    phoneInfo->setNumber(phone, SMS);
    u->Phones.push_back(phoneInfo);
    u->Type = USER_TYPE_EXT;
    return u->Uin();
}

bool ICQUser::isOnline()
{
    switch (Type()){
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
