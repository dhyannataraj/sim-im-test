/***************************************************************************
                          msnpacket.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
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

#include "msnpacket.h"
#include "msnclient.h"

#ifdef USE_OPENSSL
#include <openssl/md5.h>
#else
#include "md5.h"
#endif

#include <qtimer.h>

MSNPacket::MSNPacket(MSNClient *client, const char *cmd)
{
    m_cmd = cmd;
    m_client = client;
    m_id   = ++m_client->m_packetId;
    m_line = cmd;
    m_line += ' ';
    m_line += number(m_id);
}

void MSNPacket::addArg(const char *str)
{
    m_line += ' ';
    m_line += str;
}

void MSNPacket::send()
{
    m_client->sendLine(m_line.c_str());
    m_line = "";
    m_client->m_packets.push_back(this);
}

void MSNPacket::error(unsigned code)
{
    switch (code){
    case 911:
        m_client->authFailed();
        return;
    }
    log(L_WARN, "MSN protocol error %u", code);
    m_client->m_socket->error_state(I18N_NOOP("MSN protocol error"));
}

VerPacket::VerPacket(MSNClient *client)
        : MSNPacket(client, "VER")
{
    addArg("MSNP7 MSNP6 MSNP5 MSNP4 CVR0");
}

void VerPacket::answer(const char*, vector<string>&)
{
    MSNPacket *packet = new InfPacket(m_client);
    packet->send();
}

InfPacket::InfPacket(MSNClient *client)
        : MSNPacket(client, "INF")
{
}

void InfPacket::answer(const char*, vector<string>&)
{
    MSNPacket *packet = new UsrPacket(m_client);
    packet->send();
}

UsrPacket::UsrPacket(MSNClient *client, const char *digest)
        : MSNPacket(client, "USR")
{
    addArg("MD5");
    if (digest){
        addArg("S");
        addArg(digest);
    }else{
        addArg("I");
        addArg(m_client->getLogin());
    }
}

void UsrPacket::answer(const char *cmd, vector<string> &args)
{
    if (strcmp(cmd, "USR")){
        QTimer::singleShot(0, m_client, SLOT(authFailed()));
        return;
    }
    if (args[0] == "OK"){
        QTimer::singleShot(0, m_client, SLOT(authOk()));
        return;
    }
    if (args[1] == "S"){
        MD5_CTX c;
        unsigned char md[MD5_DIGEST_LENGTH];
        MD5_Init(&c);
        string user;
        user = m_client->getLogin().utf8();
        MD5_Update(&c, args[2].c_str(),(unsigned long)args[2].length());
        string pswd;
        pswd = m_client->getPassword().utf8();
        MD5_Update(&c, pswd.c_str(), pswd.length());
        MD5_Final(md, &c);
        string digest = "";
        for (unsigned i = 0; i < MD5_DIGEST_LENGTH; i++){
            char b[3];
            sprintf(b, "%02x",md[i]);
            digest += b;
        }
        MSNPacket *packet = new UsrPacket(m_client, digest.c_str());
        packet->send();
    }
}

OutPacket::OutPacket(MSNClient *client)
        : MSNPacket(client, "OUT")
{
}

ChgPacket::ChgPacket(MSNClient *client)
        : MSNPacket(client, "CHG")
{
    const char *status = "NLN";
    if (m_client->getInvisible()){
        status = "HDN";
    }else{
        switch (m_client->getStatus()){
        case STATUS_NA:
            status = "IDL";
            break;
        case STATUS_AWAY:
            status = "AWY";
            break;
        case STATUS_DND:
            status = "BSY";
            break;
        case STATUS_BRB:
            status = "BRB";
            break;
        case STATUS_PHONE:
            status = "PHN";
            break;
        case STATUS_LUNCH:
            status = "LUN";
            break;
        }
    }
    addArg(status);
}

SynPacket::SynPacket(MSNClient *client)
        : MSNPacket(client, "SYN")
{
    addArg(number(client->data.ListVer).c_str());
}

SynPacket::~SynPacket()
{
    if (m_client->getListVer() != m_ver){
        m_client->setListVer(m_ver);
        ContactList::GroupIterator itg;
        Group *grp;
        list<Group*>	grpRemove;
        list<Contact*>	contactRemove;
        while ((grp = ++itg) != NULL){
            ClientDataIterator it(grp->clientData, m_client);
            MSNUserData *data = (MSNUserData*)(++it);
            if (grp->id() && (data == NULL)){
                MSNListRequest lr;
                lr.Type = LR_GROUPxCHANGED;
                lr.Name = number(grp->id());
                m_client->m_requests.push_back(lr);
                continue;
            }
            if (data == NULL)
                continue;
            if ((data->sFlags & MSN_CHECKED) == 0)
                grpRemove.push_back(grp);
        }
        ContactList::ContactIterator itc;
        Contact *contact;
        while ((contact = ++itc) != NULL){
            MSNUserData *data;
            ClientDataIterator it(contact->clientData, m_client);
            list<void*> forRemove;
            while ((data = (MSNUserData*)(++it)) != NULL){
                if (data->sFlags & MSN_CHECKED){
                    if ((data->sFlags & MSN_REVERSE) && ((data->Flags & MSN_REVERSE) == 0))
                        m_client->auth_message(contact, MessageRemoved, data);
                }else{
                    forRemove.push_back(data);
                }
            }
            if (forRemove.empty())
                continue;
            for (list<void*>::iterator itr = forRemove.begin(); itr != forRemove.end(); ++itr)
                contact->clientData.freeData(*itr);
            if (contact->clientData.size() == 0)
                contactRemove.push_back(contact);
        }
        for (list<Contact*>::iterator rc = contactRemove.begin(); rc != contactRemove.end(); ++rc)
            delete *rc;
        for (list<Group*>::iterator rg = grpRemove.begin(); rg != grpRemove.end(); ++rg)
            delete *rg;
    }
    m_client->processRequests();
}

void SynPacket::answer(const char *_cmd, vector<string> &args)
{
    string cmd = _cmd;
    if (cmd == "SYN"){
        m_ver = atol(args[0].c_str());
        if (m_ver != m_client->getListVer()){
            ContactList::GroupIterator itg;
            Group *grp;
            while ((grp = ++itg) != NULL){
                MSNUserData *data;
                ClientDataIterator it(grp->clientData, m_client);
                while ((data = (MSNUserData*)(++it)) != NULL){
                    data->sFlags = data->Flags;
                    data->Flags  = 0;
                }
            }
            ContactList::ContactIterator itc;
            Contact *contact;
            while ((contact = ++itc) != NULL){
                MSNUserData *data;
                ClientDataIterator it(contact->clientData, m_client);
                while ((data = (MSNUserData*)(++it)) != NULL){
                    data->sFlags = data->Flags;
                    data->Flags = 0;
                }
            }
        }
    }
    if (cmd == "LSG"){
        if (args.size() < 5){
            log(L_WARN, "Bad LSG size");
            return;
        }
        unsigned id = atol(args[3].c_str());
        if (id == 0)
            return;
        Group *grp;
        string grp_name;
        grp_name = m_client->unquote(QString::fromUtf8(args[4].c_str())).utf8();
        MSNListRequest *lr = m_client->findRequest(id, LR_GROUPxREMOVED);
        if (lr)
            return;
        MSNUserData *data = m_client->findGroup(id, NULL, grp);
        if (data){
            lr = m_client->findRequest(grp->id(), LR_GROUPxCHANGED);
            if (lr){
                data->sFlags |= MSN_CHECKED;
                return;
            }
        }
        data = m_client->findGroup(id, grp_name.c_str(), grp);
        data->sFlags |= MSN_CHECKED;
    }
    if (cmd == "LST"){
        if (args.size() < 6){
            log(L_WARN, "Bad size for LST");
            return;
        }
        string mail;
        mail = m_client->unquote(QString::fromUtf8(args[4].c_str())).utf8();
        string name;
        name = m_client->unquote(QString::fromUtf8(args[5].c_str())).utf8();
        Contact *contact;
        MSNListRequest *lr = m_client->findRequest(mail.c_str(), LR_CONTACTxREMOVED);
        if (lr)
            return;
        bool bNew = false;
        MSNUserData *data = m_client->findContact(mail.c_str(), contact);
        if (data == NULL){
            data = m_client->findContact(mail.c_str(), name.c_str(), contact);
            bNew = true;
        }else{
            set_str(&data->EMail, mail.c_str());
            set_str(&data->ScreenName, name.c_str());
        }
        data->sFlags |= MSN_CHECKED;
        lr = m_client->findRequest(mail.c_str(), LR_CONTACTxCHANGED);
        if (args[0] == "FL"){
            unsigned grp = atol(args[6].c_str());
            data->Group = grp;
            data->Flags |= MSN_FORWARD;
            Group *group = NULL;
            m_client->findGroup(grp, NULL, group);
            if ((lr == NULL) && group && (group->id() != contact->getGroup())){
                unsigned grp = group->id();
                if (grp == 0){
                    void *d;
                    ClientDataIterator it_d(contact->clientData);
                    while ((d = ++it_d) != NULL){
                        if (d != data)
                            break;
                    }
                    if (d){
                        grp = contact->getGroup();
                        m_client->findRequest(data->EMail, LR_CONTACTxCHANGED, true);
                        MSNListRequest lr;
                        lr.Type = LR_CONTACTxCHANGED;
                        lr.Name = data->EMail;
                        m_client->m_requests.push_back(lr);
                        m_client->processRequests();
                    }
                }
                contact->setGroup(grp);
                Event e(EventContactChanged, contact);
                e.process();
            }
            return;
        }
        if (args[0] == "RL"){
            if (((data->sFlags & MSN_REVERSE) == 0) && m_client->getListVer())
                m_client->auth_message(contact, MessageAdded, data);
            data->Flags |= MSN_REVERSE;
            return;
        }
        if (args[0] == "AL"){
            data->Flags |= MSN_ACCEPT;
            return;
        }
        if (args[0] == "BL"){
            data->Flags |= MSN_BLOCKED;
            if ((lr == NULL) && !contact->getIgnore()){
                contact->setIgnore(true);
                Event e(EventContactChanged, contact);
                e.process();
            }
            return;
        }
    }
}

QryPacket::QryPacket(MSNClient *client, const char *qry)
        : MSNPacket(client, "QRY")
{
    addArg("msmsgs@msnmsgr.com");
    addArg("32");
    m_line += "\r\n";
    char qry_add[] = "Q1P7W2E4J9R8U3S5";
    MD5_CTX c;
    unsigned char md[MD5_DIGEST_LENGTH];
    MD5_Init(&c);
    MD5_Update(&c, qry, (unsigned long)strlen(qry));
    MD5_Update(&c, qry_add, (unsigned long)strlen(qry_add));
    MD5_Final(md, &c);
    for (unsigned i = 0; i < MD5_DIGEST_LENGTH; i++){
        char b[3];
        sprintf(b, "%02x",md[i]);
        m_line += b;
    }
}

void QryPacket::send()
{
    m_client->sendLine(m_line.c_str(), false);
    m_line = "";
    m_client->m_packets.push_back(this);
}

AdgPacket::AdgPacket(MSNClient *client, unsigned grp_id, const char *name)
        : MSNPacket(client, "ADG")
{
    m_id = grp_id;
    addArg(name);
    addArg("0");
}

void AdgPacket::answer(const char*, vector<string> &args)
{
    Group *grp = getContacts()->group(m_id);
    if (grp == NULL)
        return;
    MSNUserData *data;
    ClientDataIterator it(grp->clientData, m_client);
    data = (MSNUserData*)(++it);
    if (data == NULL)
        data = (MSNUserData*)(grp->clientData.createData(m_client));
    data->Group = atol(args[2].c_str());
}

RegPacket::RegPacket(MSNClient *client, unsigned id, const char *name)
        : MSNPacket(client, "REG")
{
    addArg(number(id).c_str());
    addArg(name);
    addArg("0");
}

RmgPacket::RmgPacket(MSNClient *client, unsigned id)
        : MSNPacket(client, "RMG")
{
    addArg(number(id).c_str());
}

AddPacket::AddPacket(MSNClient *client, const char *listType, const char *mail, const char *name, unsigned grp)
        : MSNPacket(client, "ADD")
{
    m_mail = mail;
    addArg(listType);
    addArg(mail);
    addArg(name);
    if (!strcmp(listType, "FL"))
        addArg(number(grp).c_str());
}

RemPacket::RemPacket(MSNClient *client, const char *listType, const char *mail, unsigned grp)
        : MSNPacket(client, "REM")
{
    addArg(listType);
    addArg(mail);
    if (!strcmp(listType, "FL"))
        addArg(number(grp).c_str());
}

BlpPacket::BlpPacket(MSNClient *client, bool bInvisible)
        : MSNPacket(client, "BLP")
{
    addArg(bInvisible ? "BL" : "AL");
}

XfrPacket::XfrPacket(MSNClient *client, SBSocket *socket)
        : MSNPacket(client, "XFR")
{
    m_socket = socket;
    addArg("SB");
}

void XfrPacket::clear()
{
    m_socket = NULL;
}

void XfrPacket::answer(const char*, vector<string> &args)
{
    if (m_socket == NULL)
        return;
    m_socket->connect(args[1].c_str(), "", args[3].c_str(), true);
}

MSNServerMessage::MSNServerMessage(MSNClient *client, unsigned size)
{
    m_client = client;
    m_size   = size;
}

bool MSNServerMessage::packet()
{
    Buffer &b = m_client->m_socket->readBuffer;
    if (b.readPos() + m_size >= b.writePos()){
        b.incReadPos(m_size);
        return true;
    }
    return false;
}

