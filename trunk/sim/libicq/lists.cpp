/***************************************************************************
                          lists.cpp  -  description
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
#include "log.h"

#include <stdio.h>

const unsigned short ICQ_SNACxLISTS_REQxRIGHTS     = 0x0002;
const unsigned short ICQ_SNACxLISTS_RIGHTS         = 0x0003;
const unsigned short ICQ_SNACxLISTS_REQxROSTER     = 0x0005;
const unsigned short ICQ_SNACxLISTS_ROSTER         = 0x0006;
const unsigned short ICQ_SNACxLISTS_UNKNOWN	   = 0x0007;
const unsigned short ICQ_SNACxLISTS_CREATE         = 0x0008;
const unsigned short ICQ_SNACxLISTS_RENAME         = 0x0009;
const unsigned short ICQ_SNACxLISTS_DELETE         = 0x000A;
const unsigned short ICQ_SNACxLISTS_DONE           = 0x000E;
const unsigned short ICQ_SNACxLISTS_ROSTERxOK      = 0x000F;
const unsigned short ICQ_SNACxLISTS_EDIT           = 0x0011;
const unsigned short ICQ_SNACxLISTS_SAVE           = 0x0012;
const unsigned short ICQ_SNACxLISTS_CREATE_USER    = 0x0014;
const unsigned short ICQ_SNACxLISTS_REQUEST_AUTH   = 0x0018;
const unsigned short ICQ_SNACxLISTS_AUTHxREQUEST   = 0x0019;
const unsigned short ICQ_SNACxLISTS_AUTHxSEND	   = 0x001A;
const unsigned short ICQ_SNACxLISTS_AUTH           = 0x001B;
const unsigned short ICQ_SNACxLISTS_ADDED          = 0x001C;

void ICQClientPrivate::snac_lists(unsigned short type, unsigned short seq)
{
    bool bFull = false;
    switch (type){
    case ICQ_SNACxLISTS_RIGHTS:
        log(L_DEBUG, "List rights");
        break;
    case ICQ_SNACxLISTS_ROSTER:{
            char c;
            unsigned short list_len;
            log(L_DEBUG,"Rosters");
            sock->readBuffer >> c;
            if (c){
                log(L_WARN, "Bad first roster byte %02X", c);
                break;
            }
            bool bIgnoreTime = false;
            vector<ICQGroup*>::iterator it_grp;
            list<ICQUser*>::iterator it_usr;
            if (!m_bRosters){
                m_bRosters = true;
                for (it_grp = client->contacts.groups.begin(); it_grp != client->contacts.groups.end(); it_grp++)
                    (*it_grp)->bChecked = false;
                for (it_usr = client->contacts.users.begin(); it_usr != client->contacts.users.end(); it_usr++){
                    if ((*it_usr)->Type != USER_TYPE_ICQ) continue;
                    (*it_usr)->Id = 0;
                    (*it_usr)->GrpId = 0;
                    (*it_usr)->IgnoreId = 0;
                    (*it_usr)->VisibleId = 0;
                    (*it_usr)->InvisibleId = 0;
                }
            }
            sock->readBuffer >> list_len;
            for (unsigned i = 0; i < list_len; i++){
                string str;
                unsigned short id, grp_id, type, len;
                sock->readBuffer.unpackStr(str);
                sock->readBuffer >> grp_id >> id >> type >> len;
                TlvList *inf = NULL;
                if (len){
                    Buffer b(len);
                    b.pack(sock->readBuffer.Data(sock->readBuffer.readPos()), len);
                    sock->readBuffer.incReadPos(len);
                    inf = new TlvList(b);
                }
                switch (type){
                case 0x0000: /* User */{
                        unsigned long uin = atol(str.c_str());
                        if (uin){
                            Tlv *tlv_name = NULL;
                            if (inf) tlv_name = (*inf)(0x0131);
                            string alias = tlv_name ? (char*)(*tlv_name) : "";
                            client->fromUTF(alias, client->owner->Encoding.c_str());
                            bool needAuth = false;
                            if (inf && (*inf)(0x0066)) needAuth = true;
                            ICQUser *user = client->getUser(uin, true);
                            user->Id = id;
                            user->GrpId = grp_id;
                            user->Alias = alias;
                            user->WaitAuth = needAuth;
                            Tlv *tlv_phone = NULL;
                            if (inf) tlv_phone = (*inf)(0x13A);
                            if (tlv_phone){
                                user->Phones.add(*tlv_phone, "Private cellular", SMS, true, false);
                                user->adjustPhones();
                            }
                        }else{
                            bIgnoreTime = true;
                        }
                        break;
                    }
                case ICQ_GROUPS:{
                        if (str.size() == 0) break;
                        client->fromUTF(str, client->owner->Encoding.c_str());
                        ICQGroup *grp = client->getGroup(grp_id, true);
                        if (grp == NULL){
                            grp = client->createGroup();
                            client->contacts.groups.push_back(grp);
                        }
                        grp->Id = grp_id;
                        grp->Name = str;
                        grp->bChecked = true;
                        break;
                    }
                case ICQ_VISIBLE_LIST:{
                        unsigned long uin = atol(str.c_str());
                        if (uin)
                            client->getUser(atol(str.c_str()), true)->VisibleId = id;
                        break;
                    }
                case ICQ_INVISIBLE_LIST:{
                        unsigned long uin = atol(str.c_str());
                        if (uin)
                            client->getUser(atol(str.c_str()), true)->InvisibleId = id;
                        break;
                    }
                case ICQ_IGNORE_LIST:{
                        unsigned long uin = atol(str.c_str());
                        if (uin)
                            client->getUser(atol(str.c_str()), true)->IgnoreId = id;
                        break;
                    }
                case ICQ_INVISIBLE_STATE:
                    client->contacts.Invisible = id;
                    break;
                case 0x0009:
                case 0x0011:
                case 0x0013:
                    break;
                default:
                    log(L_WARN,"Unknown roster type %04X", type);
                }
                if (inf) delete inf;
            }
            unsigned long time;
            sock->readBuffer >> time;
            if ((time == 0) && !bIgnoreTime)
                break;

            client->contacts.Time = time;
            for (;;){
                bool ok = true;
                for (it_grp = client->contacts.groups.begin(); it_grp != client->contacts.groups.end(); it_grp++){
                    if (!(*it_grp)->bChecked){
                        client->contacts.groups.erase(it_grp);
                        ok = false;
                        break;
                    }
                }
                if (ok) break;
            }
            for (it_usr = client->contacts.users.begin(); it_usr != client->contacts.users.end(); it_usr++){
                unsigned short grpId = (*it_usr)->GrpId;
                bool ok = false;
                for (it_grp = client->contacts.groups.begin(); it_grp != client->contacts.groups.end(); it_grp++){
                    if ((*it_grp)->Id == grpId){
                        ok = true;
                        break;
                    }
                }
                if (!ok) (*it_usr)->GrpId = 0;
            }
            ICQEvent e(EVENT_GROUP_CHANGED);
            client->process_event(&e);
            m_state = Logged;
            bFull = true;
        }
    case ICQ_SNACxLISTS_ROSTERxOK:	// FALLTHROUGH
        {
            log(L_DEBUG, "Rosters OK");
            snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_UNKNOWN);
            sendPacket();
            sendCapability();
            sendICMB();
            sendLogonStatus();
            sendClientReady();
            sendMessageRequest();
            sendPhoneStatus();
            if (bFull || (client->owner->Nick.size() == 0)){
                client->addInfoRequest(client->owner->Uin);
                client->searchByUin(client->owner->Uin);
            }
            list<ICQUser*>::iterator it;
            for (it = client->contacts.users.begin(); it != client->contacts.users.end(); it++){
                if ((*it)->IgnoreId) continue;
                if (!bFull && (*it)->Nick.size()) continue;
                client->addInfoRequest((*it)->Uin);
            }
            if (client->contacts.groups.size() == 0){
                m_state = Logged;
                client->createGroup("General");
            }
            break;
        }
    case ICQ_SNACxLISTS_ADDED:{
            sock->readBuffer.incReadPos(8);
            unsigned long uin = sock->readBuffer.unpackUin();
            ICQAddedToList *m = new ICQAddedToList;
            m->Uin.push_back(uin);
            messageReceived(m);
            break;
        }
    case ICQ_SNACxLISTS_AUTHxREQUEST:{
            sock->readBuffer.incReadPos(8);
            unsigned long uin = sock->readBuffer.unpackUin();
            ICQUser *u = client->getUser(uin);
            string message;
            string charset;
            unsigned short have_charset;
            sock->readBuffer.unpackStr(message);
            sock->readBuffer >> have_charset;
            if (have_charset){
                sock->readBuffer.incReadPos(2);
                sock->readBuffer.unpackStr(charset);
            }
            if (charset.size()){
                client->translate(client->localCharset(u), charset.c_str(), message);
            }else{
                client->fromServer(message, u);
            }
            log(L_DEBUG, "Auth request %lu", uin);

            ICQAuthRequest *m = new ICQAuthRequest;
            m->Uin.push_back(uin);
            m->Message = message;
            messageReceived(m);
            break;
        }
    case ICQ_SNACxLISTS_AUTH:{
            sock->readBuffer.incReadPos(8);
            unsigned long uin = sock->readBuffer.unpackUin();
            char auth_ok;
            sock->readBuffer >> auth_ok;
            string message;
            string charset;
            unsigned short have_charset;
            sock->readBuffer.unpackStr(message);
            sock->readBuffer >> have_charset;
            if (have_charset){
                sock->readBuffer.incReadPos(2);
                sock->readBuffer.unpackStr(charset);
            }
            ICQUser *u = client->getUser(uin);
            if (charset.size()){
                client->translate(client->localCharset(u), charset.c_str(), message);
            }else{
                client->fromServer(message, u);
            }
            log(L_DEBUG, "Auth %u %lu", auth_ok, uin);
            if (auth_ok){
                ICQUser *user = client->getUser(uin);
                if (user){
                    user->WaitAuth = false;
                    ICQEvent e(EVENT_INFO_CHANGED, uin);
                    client->process_event(&e);
                }
                ICQAuthGranted *m = new ICQAuthGranted();
                m->Uin.push_back(uin);
                messageReceived(m);
            }else{
                ICQAuthRefused *m = new ICQAuthRefused();
                m->Uin.push_back(uin);
                m->Message = message;
                messageReceived(m);
            }
            break;
        }
    case ICQ_SNACxLISTS_DONE:{
            ICQEvent *e = findListEvent(seq);
            if (e == NULL) break;
            sock->readBuffer.incReadPos(8);
            unsigned short res;
            sock->readBuffer >> res;
            e->processAnswer(this, sock->readBuffer, res);
            delete e;
            break;
        }
    default:
        log(L_WARN, "Unknown lists family type %04X", type);
    }
}

void ICQClientPrivate::listsRequest()
{
    log(L_DEBUG, "lists request");
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_REQxRIGHTS);
    sendPacket();
    if (client->contacts.Invisible == 0){
        client->contacts.Time = 0;
        client->contacts.Len = 0;
    }
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_REQxROSTER);
    sock->writeBuffer << client->contacts.Time << client->contacts.Len;
    sendPacket();
}

class ICQListEvent : public ICQEvent
{
public:
    ICQListEvent(int type, unsigned long uin) : ICQEvent(type, uin) {}
    virtual bool process(ICQClientPrivate*, unsigned short result) = 0;
protected:
    virtual bool processAnswer(ICQClientPrivate*, Buffer&, unsigned short nSubtype);
};

bool ICQListEvent::processAnswer(ICQClientPrivate *client, Buffer&, unsigned short result)
{
    if (process(client, result))
        client->client->process_event(this);
    return true;
}

class ICQSetListEvent : public ICQListEvent
{
public:
    ICQSetListEvent(unsigned long uin, unsigned type, bool bSet, unsigned short id)
            : ICQListEvent(EVENT_INFO_CHANGED, uin),
    m_type(type), m_id(id), m_bSet(bSet) {}
    bool process(ICQClientPrivate*, unsigned short result);
protected:
    unsigned m_type;
    unsigned short m_id;
    bool m_bSet;
};

bool ICQSetListEvent::process(ICQClientPrivate *icq, unsigned short result)
{
    if (result != 0){
        log(L_DEBUG, "ICQSetListEvent failed %04X", result);
        return false;
    }
    ICQUser *u = icq->client->getUser(m_nUin);
    if (u){
        switch (m_type){
        case ICQ_VISIBLE_LIST:
            u->VisibleId = m_id;
            break;
        case ICQ_INVISIBLE_LIST:
            u->InvisibleId = m_id;
            break;
        case ICQ_IGNORE_LIST:
            u->IgnoreId = m_id;
            if ((u->IgnoreId == 0) && (u->GrpId == 0)){
                m_nType = EVENT_USER_DELETED;
                icq->client->contacts.users.remove(u);
                delete u;
            }
            break;
        default:
            log(L_WARN, "Unknown ICQSetListEvent type");
            return false;
        }
    }
    return true;
}

void ICQClientPrivate::setInList(ICQUser *u, bool bSet, unsigned short list_type)
{
    if ((sock == NULL) || (sock->isError())) return;
    unsigned short userId = client->contacts.getUserId(u, list_type, true);
    ICQSetListEvent *e = new ICQSetListEvent(u->Uin, list_type, bSet, bSet ? userId : 0);
    sendRoster(e, bSet ? ICQ_SNACxLISTS_CREATE : ICQ_SNACxLISTS_DELETE,
               u->Uin, 0, userId, list_type, "");
}

void ICQClient::setInVisible(ICQUser *u, bool bSet)
{
    if ((u->VisibleId != 0) == bSet) return;
    if (u->Uin >= UIN_SPECIAL) return;
    p->setInList(u, bSet, ICQ_VISIBLE_LIST);
}

void ICQClient::setInInvisible(ICQUser *u, bool bSet)
{
    if ((u->InvisibleId != 0) == bSet) return;
    if (u->Uin >= UIN_SPECIAL) return;
    p->setInList(u, bSet, ICQ_INVISIBLE_LIST);
}

void ICQClient::setInIgnore(ICQUser *u, bool bSet)
{
    if ((u->IgnoreId != 0) == bSet) return;
    if (u->Uin >= UIN_SPECIAL){
        ICQSetListEvent e(u->Uin, ICQ_IGNORE_LIST, bSet, bSet ? 0xFFFF : 0);
        e.process(p, 0);
        process_event(&e);
    }
    p->setInList(u, bSet, ICQ_IGNORE_LIST);
}

class MoveUserEvent : public ICQListEvent
{
public:
    MoveUserEvent(unsigned long uin, unsigned short _grp_id) : ICQListEvent(EVENT_USERGROUP_CHANGED, uin), grp_id(_grp_id) {}
    bool process(ICQClientPrivate *icq, unsigned short result);
protected:
    unsigned short grp_id;
};

class MoveUserDummyEvent : public ICQListEvent
{
public:
    MoveUserDummyEvent() : ICQListEvent(EVENT_INFO_CHANGED, 0) {}
    bool process(ICQClientPrivate *icq, unsigned short result);
};

bool MoveUserDummyEvent::process(ICQClientPrivate*, unsigned short result)
{
    if (result)
        log(L_WARN, "Move user fail %04X", result);
    return false;
}

bool MoveUserEvent::process(ICQClientPrivate *icq, unsigned short result)
{
    if (result == 0x0E){
        log(L_DEBUG, "Need auth");
        ICQUser *u = icq->client->getUser(m_nUin);
        ICQGroup *g = icq->client->getGroup(grp_id);
        if ((u == NULL) || (g == NULL)) return false;
        if (u->WaitAuth){
            log(L_WARN, "Move user failed (permission)");
            return false;
        }
        u->WaitAuth = true;
        icq->client->moveUser(u, g);
        ICQEvent e(EVENT_AUTH_REQUIRED, m_nUin);
        icq->client->process_event(&e);
        return false;
    }
    if (result != 0){
        log(L_WARN, "Move user failed %04X", result);
        return false;
    }
    ICQUser *u = icq->client->getUser(m_nUin);
    if (u == NULL) return false;
    ICQGroup *g = icq->client->getGroup(grp_id);
    if (g == NULL) return false;
    if (u->Uin < UIN_SPECIAL){
        icq->sendRosterGrp(g->Name.c_str(), g->Id, u->Id);
        icq->snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_SAVE);
        icq->sendPacket();
        if (u->GrpId){
            ICQGroup *g = icq->client->getGroup(u->GrpId);
            if (g){
                icq->snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_EDIT);
                icq->sendPacket();
                MoveUserDummyEvent *e = new MoveUserDummyEvent;
                icq->sendRoster(e, ICQ_SNACxLISTS_DELETE,
                                u->Uin, g->Id, u->Id, 0, u->Alias.c_str(), u->WaitAuth);
                icq->sendRosterGrp(g->Name.c_str(), g->Id, 0);
                icq->snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_SAVE);
                icq->sendPacket();
            }
        }
    }
    u->GrpId = grp_id;
    return true;
}

void ICQClientPrivate::sendRosterGrp(const char *name, unsigned short grpId, unsigned short usrId)
{
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_RENAME, true);
    string sName = name;
    client->toUTF(sName, client->owner->Encoding.c_str());
    sock->writeBuffer.pack(sName);
    sock->writeBuffer
    << grpId
    << (unsigned long) ICQ_GROUPS;
    if (usrId){
        sock->writeBuffer
        << (unsigned short) 6
        << (unsigned short) 0xC8
        << (unsigned short) 2
        << (unsigned short) usrId;
    }else{
        sock->writeBuffer
        << (unsigned short) 4
        << (unsigned short) 0xC8
        << (unsigned short) 0;
    }
    sendPacket();
    MoveUserDummyEvent *e = new MoveUserDummyEvent;
    e->m_nId = m_nMsgSequence - 1;
    listEvents.push_back(e);
}

void ICQClient::moveUser(ICQUser *u, ICQGroup *g)
{
    if (u->Uin >= UIN_SPECIAL){
        MoveUserEvent *e = new MoveUserEvent(u->Uin, g->Id);
        if (e->process(p, 0))
            process_event(e);
        delete e;
        return;
    }
    p->moveUser(u, g);
}

void ICQClientPrivate::moveUser(ICQUser *u, ICQGroup *g)
{
    if (m_state != Logged) return;
    unsigned short id = client->contacts.getUserId(u, 0, false);
    MoveUserEvent *e = new MoveUserEvent(u->Uin, g->Id);
    if (!u->GrpId){
        snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_CREATE_USER);
        sock->writeBuffer.packUin(u->Uin);
        sock->writeBuffer << 0x00000000L;
        sendPacket();
    }
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_EDIT);
    sendPacket();
    sendRoster(e, ICQ_SNACxLISTS_CREATE, u->Uin, g->Id, id, 0, u->Alias.c_str(), u->WaitAuth);
}

class RenameUserEvent : public ICQListEvent
{
public:
    RenameUserEvent(unsigned long uin, const char *_name) : ICQListEvent(EVENT_INFO_CHANGED, uin)
    { name = strdup(_name); }
    ~RenameUserEvent() { free(name); }
protected:
    bool process(ICQClientPrivate*, unsigned short result);
    char *name;
};

bool RenameUserEvent::process(ICQClientPrivate *icq, unsigned short result)
{
    if (result != 0){
        log(L_WARN, "Rename user fail %04X", result);
        return false;
    }
    ICQUser *u = icq->client->getUser(m_nUin);
    if (u == NULL) return false;
    u->Alias = name;
    return true;
}

void ICQClient::renameUser(ICQUser *u, const char *alias)
{
    if ((u->GrpId == 0) || (u->Uin >= UIN_SPECIAL)){
        u->Alias = alias;
        ICQEvent e(EVENT_INFO_CHANGED, u->Uin);
        process_event(&e);
        return;
    }
    p->renameUser(u, alias);
}

void ICQClientPrivate::renameUser(ICQUser *u, const char *alias)
{
    if (m_state != Logged) return;
    RenameUserEvent *e = new RenameUserEvent(u->Uin, alias);
    sendRoster(e, ICQ_SNACxLISTS_RENAME, u->Uin, u->GrpId, u->Id, 0, alias, u->WaitAuth);
}

class DeleteUserEvent : public ICQListEvent
{
public:
    DeleteUserEvent(unsigned long uin) : ICQListEvent(EVENT_USER_DELETED, uin) {}
protected:
    bool process(ICQClientPrivate*, unsigned short);
};

bool DeleteUserEvent::process(ICQClientPrivate *icq, unsigned short result)
{
    if (result != 0){
        log(L_WARN, "Delete user fail %04X", result);
        return false;
    }
    ICQUser *u = icq->client->getUser(m_nUin);
    if (u == NULL) return false;
    icq->client->contacts.users.remove(u);
    return true;
}

void ICQClient::deleteUser(ICQUser *u)
{
    if (u->IgnoreId) setInIgnore(u, false);
    if (u->InvisibleId) setInInvisible(u, false);
    if (u->VisibleId) setInVisible(u, false);
    if ((u->GrpId == 0) || (u->Uin >= UIN_SPECIAL)){
        contacts.users.remove(u);
        ICQEvent e(EVENT_USER_DELETED, u->Uin);
        process_event(&e);
        return;
    }
    p->deleteUser(u);
}

void ICQClientPrivate::deleteUser(ICQUser *u)
{
    if (m_state != Logged) return;
    DeleteUserEvent *e = new DeleteUserEvent(u->Uin);
    sendRoster(e, ICQ_SNACxLISTS_DELETE, u->Uin, u->GrpId, u->Id, 0, u->Alias.c_str(), u->WaitAuth);
}

class CreateGroupEvent : public ICQListEvent
{
public:
    CreateGroupEvent(ICQGroup *_grp) : ICQListEvent(EVENT_GROUP_CREATED, _grp->Id), grp(_grp) {}
    ~CreateGroupEvent() { if (grp) delete grp; }
protected:
    ICQGroup *grp;
    bool process(ICQClientPrivate*, unsigned short result);
};

bool CreateGroupEvent::process(ICQClientPrivate *icq, unsigned short result)
{
    if (result != 0){
        log(L_WARN, "Create group failed %04X", result);
        return false;
    }
    icq->client->contacts.groups.push_back(grp);
    grp = NULL;
    return true;
}

void ICQClient::createGroup(const char *name)
{
    p->createGroup(name);
}

void ICQClientPrivate::createGroup(const char *name)
{
    if (m_state != Logged) return;
    ICQGroup *grp = client->createGroup();
    unsigned short id = client->contacts.getGroupId(grp);
    CreateGroupEvent *e = new CreateGroupEvent(grp);
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_EDIT);
    sendPacket();
    sendRoster(e, ICQ_SNACxLISTS_CREATE, name, id, 0, ICQ_GROUPS);
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_SAVE);
    sendPacket();
}

class DeleteGroupEvent : public ICQListEvent
{
public:
    DeleteGroupEvent(unsigned short grpId) : ICQListEvent(EVENT_GROUP_CHANGED, grpId) {}
protected:
    bool process(ICQClientPrivate*, unsigned short result);
};

bool DeleteGroupEvent::process(ICQClientPrivate *icq, unsigned short result)
{
    if (result != 0){
        log(L_WARN, "Delete group failed %04X", result);
        return false;
    }
    ICQGroup *grp = icq->client->getGroup(m_nUin);
    if (grp == NULL) return false;
    vector<ICQGroup*>::iterator it;
    for (it = icq->client->contacts.groups.begin(); it != icq->client->contacts.groups.end(); it++){
        if (*it == grp) break;
    }
    if (it == icq->client->contacts.groups.end()) return false;
    icq->client->contacts.groups.erase(it);
    delete grp;
    return true;
}

void ICQClient::deleteGroup(ICQGroup *g)
{
    p->deleteGroup(g);
}

void ICQClientPrivate::deleteGroup(ICQGroup *g)
{
    if (m_state != Logged) return;
    unsigned short id = client->contacts.getGroupId(g);
    if (client->contacts.groups.size() <= 1) return;
    for (list<ICQUser*>::iterator u = client->contacts.users.begin(); u != client->contacts.users.end() ; u++){
        if ((*u)->GrpId != id) continue;
        moveUser(*u, (client->contacts.groups[0] == g) ? client->contacts.groups[1] : client->contacts.groups[0]);
    }
    vector<ICQGroup*>::iterator it;
    for (it = client->contacts.groups.begin(); it != client->contacts.groups.end(); it++)
        if (*it == g) break;
    if (it == client->contacts.groups.end()) return;
    DeleteGroupEvent *e = new DeleteGroupEvent(id);
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_EDIT);
    sendPacket();
    sendRoster(e, ICQ_SNACxLISTS_DELETE, g->Name.c_str(), id, 0, ICQ_GROUPS);
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_SAVE);
    sendPacket();
}

class RenameGroupEvent : public ICQListEvent
{
public:
    RenameGroupEvent(unsigned short grpId, const char *_name) :
    ICQListEvent(EVENT_GROUP_CHANGED, grpId) { name = strdup(_name); }
    ~RenameGroupEvent() { free(name); }
protected:
    bool process(ICQClientPrivate*, unsigned short result);
    char *name;
};

bool RenameGroupEvent::process(ICQClientPrivate *icq, unsigned short result)
{
    if (result != 0){
        log(L_WARN, "Rename group failed %04X", result);
        return false;
    }
    ICQGroup *grp = icq->client->getGroup(m_nUin);
    if (grp == NULL) return false;
    grp->Name = name;
    return true;
}

void ICQClient::renameGroup(ICQGroup *g, const char *name)
{
    p->renameGroup(g, name);
}

void ICQClientPrivate::renameGroup(ICQGroup *g, const char *name)
{
    if (m_state != Logged) return;
    RenameGroupEvent *e = new RenameGroupEvent(g->Id, name);
    sendRoster(e, ICQ_SNACxLISTS_RENAME, name, g->Id, 0, ICQ_GROUPS);
}

ICQEvent *ICQClientPrivate::findListEvent(unsigned short id)
{
    for (list<ICQEvent*>::iterator it = listEvents.begin(); it != listEvents.end(); it++){
        if ((*it)->m_nId == id){
            ICQEvent *res = *it;
            listEvents.erase(it);
            return res;
        }
    }
    log(L_WARN, "List event %04X not found", id);
    return NULL;
}

void ICQClientPrivate::sendRoster(ICQEvent *e,
                                  unsigned short cmd, unsigned long uin, unsigned short grp_id,
                                  unsigned short usr_id, unsigned short subCmd, const char *alias, bool waitAuth)
{
    char buff[13];
    snprintf(buff, sizeof(buff), "%lu", uin);
    sendRoster(e, cmd, buff, grp_id, usr_id, subCmd, alias, waitAuth);
}

void ICQClientPrivate::sendRoster(ICQEvent *e,
                                  unsigned short cmd, const char *name, unsigned short grp_id,
                                  unsigned short usr_id, unsigned short subCmd, const char *alias, bool waitAuth)
{
    snac(ICQ_SNACxFAM_LISTS, cmd, true);
    string sName;
    if (name) sName = name;
    client->toUTF(sName, client->owner->Encoding.c_str());
    sock->writeBuffer.pack(sName);
    sock->writeBuffer
    << grp_id
    << usr_id
    << subCmd;
    if (alias){
        string sAlias = alias;
        client->toUTF(sAlias, client->owner->Encoding.c_str());
        unsigned short size = sAlias.length() + 4 + (waitAuth ? 4 : 0);
        sock->writeBuffer
        << size
        << (unsigned short)0x0131;
        sock->writeBuffer.pack(sAlias);
        if (waitAuth){
            sock->writeBuffer
            << (unsigned short)0x66
            << (unsigned short)0;
        }
    }else{
        sock->writeBuffer << (unsigned short)0;
    }
    sendPacket();
    e->m_nId = m_nMsgSequence - 1;
    listEvents.push_back(e);
}

void ICQClientPrivate::processMsgQueueAuth()
{
    list<ICQEvent*>::iterator it;
    for (it = msgQueue.begin(); it != msgQueue.end();){
        ICQEvent *e = *it;
        if (e->message() == NULL){
            msgQueue.remove(e);
            e->state = ICQEvent::Fail;
            client->process_event(e);
            it = msgQueue.begin();
            continue;
        }
        switch (e->message()->Type()){
        case ICQ_MSGxAUTHxREQUEST:{
                ICQAuthRequest *msg = static_cast<ICQAuthRequest*>(e->message());
                snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_REQUEST_AUTH);
                ICQUser *u = client->getUser(msg->getUin());
                sock->writeBuffer.packUin(msg->getUin());
                string message = client->clearHTML(msg->Message.c_str());
                client->toServer(message, u);
                sock->writeBuffer << (unsigned short)(message.length());
                sock->writeBuffer << message.c_str();
                sock->writeBuffer << (unsigned short)0;
                sendPacket();
                (*it)->state = ICQEvent::Success;
                break;
            }
        case ICQ_MSGxAUTHxGRANTED:{
                ICQAuthRequest *msg = static_cast<ICQAuthRequest*>(e->message());
                snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_AUTHxSEND);
                sock->writeBuffer.packUin(msg->getUin());
                sock->writeBuffer
                << (char)0x01
                << (unsigned long)0;
                sendPacket();
                (*it)->state = ICQEvent::Success;
                break;
            }
        case ICQ_MSGxAUTHxREFUSED:{
                ICQAuthRequest *msg = static_cast<ICQAuthRequest*>(e->message());
                snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_REQUEST_AUTH);
                ICQUser *u = client->getUser(msg->getUin());
                sock->writeBuffer.packUin(msg->getUin());
                string message = client->clearHTML(msg->Message.c_str());
                string original = message;
                client->toServer(message, u);
                sock->writeBuffer
                << (char) 0
                << message
                << (unsigned long)0x00010001;
                if (message == original){
                    sock->writeBuffer << (unsigned char)0;
                }else{
                    string charset = "utf-8";
                    sock->writeBuffer << charset;
                }
                sock->writeBuffer << (unsigned short)0;
                sendPacket();
                (*it)->state = ICQEvent::Success;
                break;
            }
        }
        if (e->state != ICQEvent::Success){
            it++;
            continue;
        }
        msgQueue.remove(e);
        client->process_event(e);
        it = msgQueue.begin();
    }
}

void ICQClientPrivate::sendVisibleList()
{
    if (client->contacts.Invisible == 0) client->contacts.Invisible = rand() & 0x7FFF;
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_RENAME, true);
    sock->writeBuffer
    << 0x00000000L << client->contacts.Invisible
    << (unsigned short)0x0004
    << (unsigned short)0x0005
    << 0x00CA0001L
    << (char)3;
    sendPacket();
}

void ICQClientPrivate::sendInvisibleList()
{
    if (client->contacts.Invisible == 0) client->contacts.Invisible = rand() & 0x7FFF;
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_RENAME, true);
    sock->writeBuffer
    << 0x00000000L << client->contacts.Invisible
    << (unsigned short)0x0004
    << (unsigned short)0x0005
    << 0x00CA0001L
    << (char)4;
    sendPacket();
}
