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

#include <qtimer.h>
#include <openssl/md5.h>

const unsigned NO_GROUP = (unsigned)(-1);

MSNPacket::MSNPacket(MSNClient *client, const char *cmd)
{
    m_bAnswer = false;
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

typedef struct err_str
{
    unsigned	code;
    const char	*str;
} err_str;

static err_str msn_errors[] =
    {
        { 200, "Syntax error" },
        { 201, "Invalid parameter" },
        { 205, "Invalid user" },
        { 206, "FQDN missing" },
        { 207, "Already login" },
        { 208, "Invalid username" },
        { 209, "Invalid friendly name" },
        { 210, "List full" },
        { 215, "Already there" },
        { 216, "Not on list" },
        { 218, "Already in the mode" },
        { 219, "Already in opposite list" },
        { 280, "Switchboard failed" },
        { 281, "Notify XFR failed" },
        { 300, "required fields missing" },
        { 302, "Not logged in" },
        { 500, "Internal server error" },
        { 501, "DB server error" },
        { 510, "File operation error" },
        { 520, "Memory allocation error" },
        { 600, "Server busy" },
        { 601, "Server unavailable" },
        { 602, "Peer NS down" },
        { 603, "DB connect error" },
        { 604, "Server going down" },
        { 707, "Create connection error" },
        { 711, "Blocking write" },
        { 712, "Session overload" },
        { 713, "User too active" },
        { 714, "Too many sessions" },
        { 715, "Not expected" },
        { 717, "Bad friend file" },
        { 911, "Authentication failed" },
        { 913, "Not allowed when offline" },
        { 920, "Not accepting new users" },
        { 924, "Email address not verified" },
        { 0, NULL }
    };

void MSNPacket::error(unsigned code)
{
    switch (code){
    case 911:
        m_client->authFailed();
        return;
    }
    const err_str *err;
    for (err = msn_errors; err->code; err++)
        if (err->code == code)
            break;
    if (err->code){
        m_client->m_socket->error_state(err->str);
        return;
    }
    log(L_WARN, "Unknown error code %u", code);
    m_client->m_socket->error_state("Protocol error");
}

VerPacket::VerPacket(MSNClient *client)
        : MSNPacket(client, "VER")
{
    addArg("MSNP8 CVR0");
}

bool VerPacket::answer(const char*, vector<string>&)
{
    MSNPacket *packet = new CvrPacket(m_client);
    packet->send();
    return false;
}

CvrPacket::CvrPacket(MSNClient *client)
        : MSNPacket(client, "CVR")
{
    addArg("0x0409 winnt 5.1 i386 MSNMSGR");
    addArg(client->getVersion());
    addArg("MSMSGS");
    addArg(m_client->getLogin());
}

bool CvrPacket::answer(const char*, vector<string> &arg)
{
    m_client->setVersion(arg[0].c_str());
    MSNPacket *packet = new UsrPacket(m_client);
    packet->send();
    return false;
}

UsrPacket::UsrPacket(MSNClient *client, const char *digest)
        : MSNPacket(client, "USR")
{
    addArg("TWN");
    if (digest){
        addArg("S");
        addArg(digest);
    }else{
        addArg("I");
        addArg(m_client->getLogin());
    }
}

bool UsrPacket::answer(const char*, vector<string> &args)
{
    if (args[0] == "OK"){
        QTimer::singleShot(0, m_client, SLOT(authOk()));
        return false;;
    }
    if (args[1] == "S"){
        m_client->m_authChallenge = args[2].c_str();
        m_client->requestLoginHost("https://nexus.passport.com/rdr/pprdr.asp");
    }
    return false;
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
    bDone = false;
    client->data.ListVer = 0;
    addArg(number(client->data.ListVer).c_str());
}

SynPacket::~SynPacket()
{
    if ((m_client->getListVer() != m_ver) && bDone){
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

bool SynPacket::answer(const char *_cmd, vector<string> &args)
{
    string cmd = _cmd;
    if (cmd == "SYN"){
        m_ver = atol(args[0].c_str());
        if (m_ver == m_client->getListVer())
            return false;
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
        return true;
    }
    if (cmd == "GTC")
        return true;
    if (cmd == "BLP")
        return true;
    if (cmd == "LSG"){
        if (args.size() < 3){
            log(L_WARN, "Bad LSG size");
            return true;
        }
        unsigned id = atol(args[0].c_str());
        if (id == 0)
            return true;
        Group *grp;
        string grp_name;
        grp_name = m_client->unquote(QString::fromUtf8(args[1].c_str())).utf8();
        MSNListRequest *lr = m_client->findRequest(id, LR_GROUPxREMOVED);
        if (lr)
            return true;
        MSNUserData *data = m_client->findGroup(id, NULL, grp);
        if (data){
            lr = m_client->findRequest(grp->id(), LR_GROUPxCHANGED);
            if (lr){
                data->sFlags |= MSN_CHECKED;
                return true;
            }
        }
        data = m_client->findGroup(id, grp_name.c_str(), grp);
        data->sFlags |= MSN_CHECKED;
        return true;
    }
    if (cmd == "LST"){
        if (args.size() < 3){
            log(L_WARN, "Bad size for LST");
            return true;
        }
        string mail;
        mail = m_client->unquote(QString::fromUtf8(args[0].c_str())).utf8();
        string name;
        name = m_client->unquote(QString::fromUtf8(args[1].c_str())).utf8();
        Contact *contact;
        MSNListRequest *lr = m_client->findRequest(mail.c_str(), LR_CONTACTxREMOVED);
        if (lr)
            return true;
        bool bNew = false;
        MSNUserData *data = m_client->findContact(mail.c_str(), contact);
        if (data == NULL){
            data = m_client->findContact(mail.c_str(), name.c_str(), contact);
            bNew = true;
        }else{
            set_str(&data->EMail, mail.c_str());
            set_str(&data->ScreenName, name.c_str());
            if (name != (const char*)(contact->getName().utf8())){
                contact->setName(QString::fromUtf8(name.c_str()));
                Event e(EventContactChanged, contact);
                e.process();
            }
        }
        data->sFlags |= MSN_CHECKED;
        lr = m_client->findRequest(mail.c_str(), LR_CONTACTxCHANGED);
        unsigned grp = NO_GROUP;
        if (args.size() > 3)
            grp = atol(args[3].c_str());
        data->Group = grp;
        data->Flags |= MSN_FORWARD;
        Group *group = NULL;
        if (grp == NO_GROUP){
            group = getContacts()->group(0);
        }else{
            m_client->findGroup(grp, NULL, group);
        }
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
        return true;
    }
    bDone = true;
    return false;
}

QryPacket::QryPacket(MSNClient *client, const char *qry)
        : MSNPacket(client, "QRY")
{
    addArg("PROD0038W!61ZTF9");
    addArg("32");
    m_line += "\r\n";
    char qry_add[] = "VT6PX?UQTM4WM%YR";
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

bool AdgPacket::answer(const char*, vector<string> &args)
{
    Group *grp = getContacts()->group(m_id);
    if (grp == NULL)
        return false;
    MSNUserData *data;
    ClientDataIterator it(grp->clientData, m_client);
    data = (MSNUserData*)(++it);
    if (data == NULL)
        data = (MSNUserData*)(grp->clientData.createData(m_client));
    data->Group = atol(args[2].c_str());
    return false;
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
}

ReaPacket::ReaPacket(MSNClient *client, const char *mail, const char *name)
        : MSNPacket(client, "REA")
{
    addArg(mail);
    addArg(name);
}

BlpPacket::BlpPacket(MSNClient *client, bool bInvisible)
        : MSNPacket(client, "BLP")
{
    addArg("BL");
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

bool XfrPacket::answer(const char*, vector<string> &args)
{
    if (m_socket)
        m_socket->connect(args[1].c_str(), "", args[3].c_str(), true);
    return false;
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

