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

const unsigned short ICQ_GROUPS					= 0x0001;
const unsigned short ICQ_VISIBLE_LIST			= 0x0002;
const unsigned short ICQ_INVISIBLE_LIST			= 0x0003;
const unsigned short ICQ_INVISIBLE_STATE		= 0x0004;
const unsigned short ICQ_IGNORE_LIST			= 0x000E;

void ICQClient::snac_lists(unsigned short type, unsigned short seq)
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
            if (m_bRosters){
                log(L_DEBUG, "Rosters part 2");
                break;
            }
            readBuffer >> c;
            if (c){
                log(L_WARN, "Bad first roster byte %02X", c);
                break;
            }
            vector<ICQGroup*>::iterator it_grp;
            list<ICQUser*>::iterator it_usr;
            for (it_grp = contacts.groups.begin(); it_grp != contacts.groups.end(); it_grp++)
                (*it_grp)->bChecked = false;
            for (it_usr = contacts.users.begin(); it_usr != contacts.users.end(); it_usr++){
                if ((*it_usr)->Type != USER_TYPE_ICQ) continue;
                (*it_usr)->Id = 0;
                (*it_usr)->GrpId = 0;
                (*it_usr)->inIgnore = false;
                (*it_usr)->inVisible = false;
                (*it_usr)->inInvisible = false;
            }
            readBuffer >> list_len;
            for (unsigned i = 0; i < list_len; i++){
                string str;
                unsigned short id, grp_id, type, len;
                readBuffer.unpackStr(str);
                readBuffer >> grp_id >> id >> type >> len;
                TlvList *inf = NULL;
                if (len){
                    Buffer b(len);
                    b.pack(readBuffer.Data(readBuffer.readPos()), len);
                    readBuffer.incReadPos(len);
                    inf = new TlvList(b);
                }
                switch (type){
                case 0x0000: /* User */{
                        unsigned long uin = atol(str.c_str());
                        if (uin == 0){
                            log(L_WARN, "Bad uin record %s\n", str.c_str());
                            break;
                        }
                        Tlv *tlv_name = NULL;
                        if (inf) tlv_name = (*inf)(0x0131);
                        string alias = tlv_name ? (char*)(*tlv_name) : "";
                        fromUTF(alias);
                        bool needAuth = false;
                        if (inf && (*inf)(0x0066)) needAuth = true;
                        ICQUser *user = getUser(uin, true);
                        user->Id = id;
                        user->GrpId = grp_id;
                        user->Alias = alias;
                        user->WaitAuth = needAuth;
                        break;
                    }
                case ICQ_GROUPS:{
                        if (str.size() == 0) break;
                        fromUTF(str);
                        ICQGroup *grp = getGroup(grp_id, true);
                        if (grp == NULL){
                            grp = new ICQGroup();
                            contacts.groups.push_back(grp);
                        }
                        grp->Id = grp_id;
                        grp->Name = str;
                        grp->bChecked = true;
                        break;
                    }
                case ICQ_VISIBLE_LIST:{
                        unsigned long uin = atol(str.c_str());
                        if (uin)
                            getUser(atol(str.c_str()), true)->inVisible = true;
                        break;
                    }
                case ICQ_INVISIBLE_LIST:{
                        unsigned long uin = atol(str.c_str());
                        if (uin)
                            getUser(atol(str.c_str()), true)->inInvisible = true;
                        break;
                    }
                case ICQ_IGNORE_LIST:{
                        unsigned long uin = atol(str.c_str());
                        if (uin)
                            getUser(atol(str.c_str()), true)->inIgnore = true;
                        break;
                    }
                case ICQ_INVISIBLE_STATE:
                    contacts.Invisible = id;
                    break;
                case 0x0009:
                    break;
                default:
                    log(L_WARN,"Unknown roster type %04X", type);
                }
                if (inf) delete inf;
            }
            unsigned long time;
            readBuffer >> time;
            contacts.Time = time;
            for (;;){
                bool ok = true;
                for (it_grp = contacts.groups.begin(); it_grp != contacts.groups.end(); it_grp++){
                    if (!(*it_grp)->bChecked){
                        contacts.groups.erase(it_grp);
                        ok = false;
                        break;
                    }
                }
                if (ok) break;
            }
            for (it_usr = contacts.users.begin(); it_usr != contacts.users.end(); it_usr++){
                unsigned short grpId = (*it_usr)->GrpId();
                bool ok = false;
                for (it_grp = contacts.groups.begin(); it_grp != contacts.groups.end(); it_grp++){
                    if ((*it_grp)->Id() == grpId){
                        ok = true;
                        break;
                    }
                }
                if (!ok) (*it_usr)->GrpId = 0;
            }
            ICQEvent e(EVENT_GROUP_CHANGED);
            process_event(&e);
            m_state = Logged;
            bFull = true;
        }
    case ICQ_SNACxLISTS_ROSTERxOK:	// FALLTHROUGH
        {
            m_bRosters = true;
            log(L_DEBUG, "Rosters OK");
            snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_UNKNOWN);
            sendPacket();
            sendCapability();
            sendICMB();
            sendLogonStatus();
            sendClientReady();
            sendMessageRequest();
            sendPhoneInit();
            sendPhoneStatus();
            if (bFull || (Nick.size() == 0)){
                addInfoRequest(Uin);
                searchByUin(Uin);
            }
            list<ICQUser*>::iterator it;
            for (it = contacts.users.begin(); it != contacts.users.end(); it++){
                if ((*it)->inIgnore()) continue;
                if (!bFull && (*it)->Nick.size()) continue;
                addInfoRequest((*it)->Uin);
            }
            if (contacts.groups.size() == 0){
                m_state = Logged;
                createGroup("General");
            }
            break;
        }
    case ICQ_SNACxLISTS_ADDED:{
            readBuffer.incReadPos(8);
            unsigned long uin = readBuffer.unpackUin();
            ICQAddedToList *m = new ICQAddedToList;
            m->Uin.push_back(uin);
            messageReceived(m);
            break;
        }
    case ICQ_SNACxLISTS_AUTHxREQUEST:{
            readBuffer.incReadPos(8);
            unsigned long uin = readBuffer.unpackUin();
            ICQUser *u = getUser(uin);
            string message;
            string charset;
            unsigned short have_charset;
            readBuffer.unpackStr(message);
            readBuffer >> have_charset;
            if (have_charset){
                readBuffer.incReadPos(2);
                readBuffer.unpackStr(charset);
            }
            if (charset.size()){
                translate(localCharset(u), charset.c_str(), message);
            }else{
                fromServer(message, u);
            }
            log(L_DEBUG, "Auth request %lu", uin);

            ICQAuthRequest *m = new ICQAuthRequest;
            m->Uin.push_back(uin);
            m->Message = message;
            messageReceived(m);
            break;
        }
    case ICQ_SNACxLISTS_AUTH:{
            readBuffer.incReadPos(8);
            unsigned long uin = readBuffer.unpackUin();
            char auth_ok; readBuffer >> auth_ok;
            string message;
            string charset;
            unsigned short have_charset;
            readBuffer.unpackStr(message);
            readBuffer >> have_charset;
            if (have_charset){
                readBuffer.incReadPos(2);
                readBuffer.unpackStr(charset);
            }
            ICQUser *u = getUser(uin);
            if (charset.size()){
                translate(localCharset(u), charset.c_str(), message);
            }else{
                fromServer(message, u);
            }
            log(L_DEBUG, "Auth %u %lu", auth_ok, uin);
            if (auth_ok){
                ICQUser *user = getUser(uin);
                if (user){
                    user->WaitAuth = false;
                    ICQEvent e(EVENT_INFO_CHANGED, uin);
                    process_event(&e);
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
            readBuffer.incReadPos(8);
            unsigned short res;
            readBuffer >> res;
            e->processAnswer(this, readBuffer, res);
            delete e;
            break;
        }
    default:
        log(L_WARN, "Unknown lists family type %04X", type);
    }
}

void ICQClient::listsRequest()
{
    log(L_DEBUG, "lists request");
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_REQxRIGHTS);
    sendPacket();
    if (contacts.Invisible() == 0){
        contacts.Time = 0;
        contacts.Len = 0;
    }
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_REQxROSTER);
    writeBuffer << contacts.Time() << contacts.Len();
    sendPacket();
}

class ICQListEvent : public ICQEvent
{
public:
    ICQListEvent(int type, unsigned long uin) : ICQEvent(type, uin) {}
    virtual bool process(ICQClient*, unsigned short result) = 0;
protected:
    virtual bool processAnswer(ICQClient*, Buffer&, unsigned short nSubtype);
};

bool ICQListEvent::processAnswer(ICQClient *client, Buffer&, unsigned short result)
{
    if (process(client, result))
        client->process_event(this);
    return true;
}

class ICQSetListEvent : public ICQListEvent
{
public:
    ICQSetListEvent(unsigned long uin, unsigned type, bool bSet)
            : ICQListEvent(EVENT_INFO_CHANGED, uin),
    m_type(type), m_bSet(bSet) {}
    bool process(ICQClient*, unsigned short result);
protected:
    unsigned m_type;
    bool m_bSet;
};

bool operator == (const list_req &r1, const list_req &r2)
{
    return (r1.uin == r2.uin) && (r1.list_type == r2.list_type) && (r1.bSet == r2.bSet);
}

bool ICQSetListEvent::process(ICQClient *icq, unsigned short result)
{
    if (result != 0){
        log(L_DEBUG, "ICQSetListEvent failed %04X", result);
        icq->listQueue.remove(*icq->listQueue.begin());
        icq->processListQueue();
        return false;
    }
    ICQUser *u = icq->getUser(m_nUin);
    switch (m_type){
    case ICQ_VISIBLE_LIST:
        if (m_bSet){
            icq->addToVisibleList(m_nUin);
        }else{
            icq->removeFromVisibleList(m_nUin);
        }
        u->inVisible = m_bSet;
        break;
    case ICQ_INVISIBLE_LIST:
        if (m_bSet){
            icq->addToInvisibleList(m_nUin);
        }else{
            icq->removeFromInvisibleList(m_nUin);
        }
        u->inInvisible = m_bSet;
        break;
    case ICQ_IGNORE_LIST:
        u->inIgnore = m_bSet;
        break;
    default:
        log(L_WARN, "Unknown ICQSetListEvent type");
        return false;
    }
    if (m_nUin >= UIN_SPECIAL) return true;
    icq->listQueue.remove(*icq->listQueue.begin());
    icq->processListQueue();
    return true;
}

void ICQClient::processListQueue()
{
    for (;;){
        if (listQueue.size() == 0) return;
        list_req lr = *listQueue.begin();
        ICQUser *u = getUser(lr.uin);
        if (u == NULL){
            listQueue.remove(lr);
            continue;
        }
        unsigned short userId = contacts.getUserId(u);
        ICQSetListEvent *e = new ICQSetListEvent(u->Uin(), lr.list_type, lr.bSet);
        sendRoster(e,
                   lr.bSet ? ICQ_SNACxLISTS_CREATE : ICQ_SNACxLISTS_DELETE,
                   u->Uin, 0, userId, lr.list_type);
        return;
    }
}

void ICQClient::setInVisible(ICQUser *u, bool bSet)
{
    if (u->inVisible() == bSet) return;
    if (u->Uin() >= UIN_SPECIAL){
        ICQSetListEvent *e = new ICQSetListEvent(u->Uin(), ICQ_VISIBLE_LIST, bSet);
        if (e->process(this, 0)) process_event(e);
        delete e;
        return;
    }
    list_req lr;
    lr.uin = u->Uin();
    lr.list_type = ICQ_VISIBLE_LIST;
    lr.bSet = bSet;
    listQueue.push_back(lr);
    if (listQueue.size() <= 1)
        processListQueue();
}

void ICQClient::setInInvisible(ICQUser *u, bool bSet)
{
    if (u->inInvisible() == bSet) return;
    if (u->Uin() >= UIN_SPECIAL){
        ICQSetListEvent *e = new ICQSetListEvent(u->Uin(), ICQ_INVISIBLE_LIST, bSet);
        if (e->process(this, 0)) process_event(e);
        delete e;
        return;
    }
    list_req lr;
    lr.uin = u->Uin();
    lr.list_type = ICQ_INVISIBLE_LIST;
    lr.bSet = bSet;
    listQueue.push_back(lr);
    if (listQueue.size() <= 1)
        processListQueue();
}

void ICQClient::setInIgnore(ICQUser *u, bool bSet)
{
    if (u->inIgnore() == bSet) return;
    if (u->Uin() >= UIN_SPECIAL){
        ICQSetListEvent *e = new ICQSetListEvent(u->Uin(), ICQ_IGNORE_LIST, bSet);
        if (e->process(this, 0)) process_event(e);
        delete e;
        return;
    }
    list_req lr;
    lr.uin = u->Uin();
    lr.list_type = ICQ_IGNORE_LIST;
    lr.bSet = bSet;
    listQueue.push_back(lr);
    if (listQueue.size() <= 1)
        processListQueue();
}

class MoveUserEvent : public ICQListEvent
{
public:
    MoveUserEvent(unsigned long uin, unsigned short _grp_id) : ICQListEvent(EVENT_USERGROUP_CHANGED, uin), grp_id(_grp_id) {}
    bool process(ICQClient *icq, unsigned short result);
protected:
    unsigned short grp_id;
};

class MoveUserDummyEvent : public ICQListEvent
{
public:
    MoveUserDummyEvent() : ICQListEvent(EVENT_INFO_CHANGED, 0) {}
    bool process(ICQClient *icq, unsigned short result);
};

bool MoveUserDummyEvent::process(ICQClient*, unsigned short result)
{
    if (result)
        log(L_WARN, "Move user fail %04X", result);
    return false;
}

bool MoveUserEvent::process(ICQClient *icq, unsigned short result)
{
    if (result == 0x0E){
        log(L_DEBUG, "Need auth");
        ICQUser *u = icq->getUser(m_nUin);
        ICQGroup *g = icq->getGroup(grp_id);
        if ((u == NULL) || (g == NULL)) return false;
        if (u->WaitAuth()){
            log(L_WARN, "Move user failed (permission)");
            return false;
        }
        u->WaitAuth = true;
        icq->moveUser(u, g);
        ICQEvent e(EVENT_AUTH_REQUIRED, m_nUin);
        icq->process_event(&e);
        return false;
    }
    if (result != 0){
        log(L_WARN, "Move user failed %04X", result);
        return false;
    }
    ICQUser *u = icq->getUser(m_nUin);
    if (u == NULL) return false;
    ICQGroup *g = icq->getGroup(grp_id);
    if (g == NULL) return false;
    if (u->Uin() < UIN_SPECIAL){
        icq->sendRosterGrp(g->Name, g->Id, u->Id);
        icq->snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_SAVE);
        icq->sendPacket();
        if (u->GrpId){
            ICQGroup *g = icq->getGroup(u->GrpId);
            if (g){
                icq->snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_EDIT);
                icq->sendPacket();
                MoveUserDummyEvent *e = new MoveUserDummyEvent;
                icq->sendRoster(e, ICQ_SNACxLISTS_DELETE, u->Uin, g->Id(), u->Id(), 0, u->Alias.c_str(), u->WaitAuth());
                icq->sendRosterGrp(g->Name.c_str(), g->Id(), 0);
                icq->snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_SAVE);
                icq->sendPacket();
            }
        }
    }
    u->GrpId = grp_id;
    return true;
}

void ICQClient::sendVisibleList()
{
    if (contacts.Invisible() == 0) contacts.Invisible = rand() & 0x7FFF;
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_RENAME, true);
    writeBuffer
    << 0x00000000L << contacts.Invisible()
    << (unsigned short)0x0004
    << (unsigned short)0x0005
    << 0x00CA0001L
    << (char)3;
    sendPacket();
}

void ICQClient::sendInvisibleList()
{
    if (contacts.Invisible() == 0) contacts.Invisible = rand() & 0x7FFF;
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_RENAME, true);
    writeBuffer
    << 0x00000000L << contacts.Invisible()
    << (unsigned short)0x0004
    << (unsigned short)0x0005
    << 0x00CA0001L
    << (char)4;
    sendPacket();
}

void ICQClient::sendRosterGrp(const char *name, unsigned short grpId, unsigned short usrId)
{
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_RENAME, true);
    string sName = name;
    toUTF(sName);
    writeBuffer.pack(sName);
    writeBuffer
    << grpId
    << (unsigned long) ICQ_GROUPS;
    if (usrId){
        writeBuffer
        << (unsigned short) 6
        << (unsigned short) 0xC8
        << (unsigned short) 2
        << (unsigned short) usrId;
    }else{
        writeBuffer
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
    unsigned short id = contacts.getUserId(u);
    MoveUserEvent *e = new MoveUserEvent(u->Uin, g->Id());
    if (u->Uin() >= UIN_SPECIAL){
        if (e->process(this, 0))
            process_event(e);
        delete e;
        return;
    }
    if (m_state != Logged) return;
    if (!u->GrpId()){
        snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_CREATE_USER);
        writeBuffer.packUin(u->Uin);
        writeBuffer << 0x00000000L;
        sendPacket();
    }
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_EDIT);
    sendPacket();
    sendRoster(e, ICQ_SNACxLISTS_CREATE, u->Uin, g->Id(), id, 0, u->Alias.c_str(), u->WaitAuth());
}

class RenameUserEvent : public ICQListEvent
{
public:
    RenameUserEvent(unsigned long uin, const char *_name) : ICQListEvent(EVENT_INFO_CHANGED, uin)
    { name = strdup(_name); }
    ~RenameUserEvent() { free(name); }
protected:
    bool process(ICQClient*, unsigned short result);
    char *name;
};

bool RenameUserEvent::process(ICQClient *icq, unsigned short result)
{
    if (result != 0){
        log(L_WARN, "Rename user fail %04X", result);
        return false;
    }
    ICQUser *u = icq->getUser(m_nUin);
    if (u == NULL) return false;
    u->Alias = name;
    return true;
}

void ICQClient::renameUser(ICQUser *u, const char *alias)
{
    if ((u->GrpId() == 0) || (u->Uin() >= UIN_SPECIAL)){
        u->Alias = alias;
        ICQEvent e(EVENT_INFO_CHANGED, u->Uin);
        process_event(&e);
        return;
    }
    if (m_state != Logged) return;
    RenameUserEvent *e = new RenameUserEvent(u->Uin(), alias);
    sendRoster(e, ICQ_SNACxLISTS_RENAME, u->Uin, u->GrpId(), u->Id(), 0, alias, u->WaitAuth());
}

class DeleteUserEvent : public ICQListEvent
{
public:
    DeleteUserEvent(unsigned long uin) : ICQListEvent(EVENT_USER_DELETED, uin) {}
protected:
    bool process(ICQClient*, unsigned short);
};

bool DeleteUserEvent::process(ICQClient *icq, unsigned short result)
{
    if (result != 0){
        log(L_WARN, "Delete user fail %04X", result);
        return false;
    }
    ICQUser *u = icq->getUser(m_nUin);
    if (u == NULL) return false;
    icq->contacts.users.remove(u);
    return true;
}

void ICQClient::deleteUser(ICQUser *u)
{
    if (u->inIgnore()) setInIgnore(u, false);
    if (u->inInvisible()) setInInvisible(u, false);
    if (u->inVisible()) setInVisible(u, false);
    if ((u->GrpId() == 0) || (u->Uin() >= UIN_SPECIAL)){
        contacts.users.remove(u);
        ICQEvent e(EVENT_USER_DELETED, u->Uin);
        process_event(&e);
        return;
    }
    if (m_state != Logged) return;
    DeleteUserEvent *e = new DeleteUserEvent(u->Uin());
    sendRoster(e, ICQ_SNACxLISTS_DELETE, u->Uin, u->GrpId(), u->Id(), 0, u->Alias.c_str(), u->WaitAuth());
}

class CreateGroupEvent : public ICQListEvent
{
public:
    CreateGroupEvent(ICQGroup *_grp) : ICQListEvent(EVENT_GROUP_CREATED, _grp->Id), grp(_grp) {}
    ~CreateGroupEvent() { if (grp) delete grp; }
protected:
    ICQGroup *grp;
    bool process(ICQClient*, unsigned short result);
};

bool CreateGroupEvent::process(ICQClient *icq, unsigned short result)
{
    if (result != 0){
        log(L_WARN, "Create group failed %04X", result);
        return false;
    }
    icq->contacts.groups.push_back(grp);
    grp = NULL;
    return true;
}

void ICQClient::createGroup(const char *name)
{
    ICQGroup *grp = new ICQGroup;
    unsigned short id = contacts.getGroupId(grp);
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
    bool process(ICQClient*, unsigned short result);
};

bool DeleteGroupEvent::process(ICQClient *icq, unsigned short result)
{
    if (result != 0){
        log(L_WARN, "Delete group failed %04X", result);
        return false;
    }
    ICQGroup *grp = icq->getGroup(m_nUin);
    if (grp == NULL) return false;
    vector<ICQGroup*>::iterator it;
    for (it = icq->contacts.groups.begin(); it != icq->contacts.groups.end(); it++){
        if (*it == grp) break;
    }
    if (it == icq->contacts.groups.end()) return false;
    icq->contacts.groups.erase(it);
    delete grp;
    return true;
}

void ICQClient::deleteGroup(ICQGroup *g)
{
    if (m_state != Logged) return;
    unsigned short id = contacts.getGroupId(g);
    if (contacts.groups.size() <= 1) return;
    for (list<ICQUser*>::iterator u = contacts.users.begin(); u != contacts.users.end() ; u++){
        if ((*u)->GrpId() != id) continue;
        moveUser(*u, (contacts.groups[0] == g) ? contacts.groups[1] : contacts.groups[0]);
    }
    vector<ICQGroup*>::iterator it;
    for (it = contacts.groups.begin(); it != contacts.groups.end(); it++)
        if (*it == g) break;
    if (it == contacts.groups.end()) return;
    DeleteGroupEvent *e = new DeleteGroupEvent(id);
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_EDIT);
    sendPacket();
    sendRoster(e, ICQ_SNACxLISTS_DELETE, g->Name, id, 0, ICQ_GROUPS);
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
    bool process(ICQClient*, unsigned short result);
    char *name;
};

bool RenameGroupEvent::process(ICQClient *icq, unsigned short result)
{
    if (result != 0){
        log(L_WARN, "Rename group failed %04X", result);
        return false;
    }
    ICQGroup *grp = icq->getGroup(m_nUin);
    if (grp == NULL) return false;
    grp->Name = name;
    return true;
}

void ICQClient::renameGroup(ICQGroup *g, const char *name)
{
    if (m_state != Logged) return;
    RenameGroupEvent *e = new RenameGroupEvent(g->Id(), name);
    sendRoster(e, ICQ_SNACxLISTS_RENAME, name, g->Id(), 0, ICQ_GROUPS);
}

ICQEvent *ICQClient::findListEvent(unsigned short id)
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

void ICQClient::sendRoster(ICQEvent *e,
                           unsigned short cmd, unsigned long uin, unsigned short grp_id,
                           unsigned short usr_id, unsigned short subCmd, const char *alias, bool waitAuth)
{
    char buff[13];
    snprintf(buff, sizeof(buff), "%lu", uin);
    sendRoster(e, cmd, buff, grp_id, usr_id, subCmd, alias, waitAuth);
}

void ICQClient::sendRoster(ICQEvent *e,
                           unsigned short cmd, const char *name, unsigned short grp_id,
                           unsigned short usr_id, unsigned short subCmd, const char *alias, bool waitAuth)
{
    snac(ICQ_SNACxFAM_LISTS, cmd, true);
    string sName;
    if (name) sName = name;
    toUTF(sName);
    writeBuffer.pack(sName);
    writeBuffer
    << grp_id
    << usr_id
    << subCmd;
    if (alias){
        string sAlias = alias;
        toUTF(sAlias);
        unsigned short size = sAlias.length() + 4 + (waitAuth ? 4 : 0);
        writeBuffer
        << size
        << (unsigned short)0x0131;
        writeBuffer.pack(sAlias);
        if (waitAuth){
            writeBuffer
            << (unsigned short)0x66
            << (unsigned short)0;
        }
    }else{
        writeBuffer << (unsigned short)0;
    }
    sendPacket();
    e->m_nId = m_nMsgSequence - 1;
    listEvents.push_back(e);
}

void ICQClient::processMsgQueueAuth()
{
    list<ICQEvent*>::iterator it;
    for (it = msgQueue.begin(); it != msgQueue.end();){
        ICQEvent *e = *it;
        if (e->message() == NULL){
            msgQueue.remove(e);
            e->state = ICQEvent::Fail;
            process_event(e);
            it = msgQueue.begin();
            continue;
        }
        switch (e->message()->Type()){
        case ICQ_MSGxAUTHxREQUEST:{
                ICQAuthRequest *msg = static_cast<ICQAuthRequest*>(e->message());
                snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_REQUEST_AUTH);
                ICQUser *u = getUser(msg->getUin());
                writeBuffer.packUin(msg->getUin());
                string message = clearHTML(msg->Message.c_str());
                toServer(message, u);
                writeBuffer << (unsigned short)(message.length());
                writeBuffer << message.c_str();
                writeBuffer << (unsigned short)0;
                sendPacket();
                (*it)->state = ICQEvent::Success;
                break;
            }
        case ICQ_MSGxAUTHxGRANTED:{
                ICQAuthRequest *msg = static_cast<ICQAuthRequest*>(e->message());
                snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_AUTHxSEND);
                writeBuffer.packUin(msg->getUin());
                writeBuffer
                << (char)0x01
                << (unsigned long)0;
                sendPacket();
                (*it)->state = ICQEvent::Success;
                break;
            }
        case ICQ_MSGxAUTHxREFUSED:{
                ICQAuthRequest *msg = static_cast<ICQAuthRequest*>(e->message());
                snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_REQUEST_AUTH);
                ICQUser *u = getUser(msg->getUin());
                writeBuffer.packUin(msg->getUin());
                string message = clearHTML(msg->Message.c_str());
                string original = message;
                toServer(message, u);
                writeBuffer
                << (char) 0
                << message
                << (unsigned long)0x00010001;
                if (message == original){
                    writeBuffer << (unsigned char)0;
                }else{
                    string charset = "utf-8";
                    writeBuffer << charset;
                }
                writeBuffer << (unsigned short)0;
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
        process_event(e);
        it = msgQueue.begin();
    }
}

