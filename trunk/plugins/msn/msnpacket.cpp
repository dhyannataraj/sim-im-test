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
#include "msn.h"

#ifdef WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include <qtimer.h>
#include <openssl/md5.h>

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
        { 217, "User not on-line" },
        { 218, "Already in the mode" },
        { 219, "Already in opposite list" },
        { 223, "Too many groups" },
        { 224, "Invalid group" },
        { 225, "User not in group" },
        { 229, "Group name too long" },
        { 230, "Cannot remove group 0" },
        { 231, "Invalid group" },
        { 280, "Switchboard failed" },
        { 281, "Notify XFR failed" },
        { 300, "required fields missing" },
        { 302, "Not logged in" },
        { 500, "Internal server error" },
        { 501, "DB server error" },
        { 502, "Command disabled" },
        { 510, "File operation error" },
        { 520, "Memory allocation error" },
        { 540, "Challenge response failed" },
        { 600, "Server busy" },
        { 601, "Server unavailable" },
        { 602, "Peer NS down" },
        { 603, "DB connect error" },
        { 604, "Server going down" },
        { 605, "Server unavailable" },
        { 707, "Create connection error" },
        { 710, "Bad CVR parameters sent" },
        { 711, "Blocking write" },
        { 712, "Session overload" },
        { 713, "User too active" },
        { 714, "Too many sessions" },
        { 715, "Not expected (PRP)" },
        { 717, "Bad friend file" },
        { 731, "Not expected (CVR)" },
        { 800, "Changing too rapidly" },
        { 910, "Server too busy" },
        { 911, "Authentication failed" },
        { 912, "Server too busy" },
        { 913, "Not allowed when offline" },
        { 914, "Server unavailable" },
        { 915, "Server unavailable" },
        { 916, "Server unavailable" },
        { 917, "Authentication failed" },
        { 918, "Server too busy" },
        { 919, "Server too busy" },
        { 920, "Not accepting new users" },
        { 921, "Server too busy" },
        { 922, "Server too busy" },
        { 923, "Kids Passport without parental consent" },
        { 924, "Email address not verified" },
        { 928, "Bad ticket" },
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

void VerPacket::answer(vector<string>&)
{
    MSNPacket *packet = new CvrPacket(m_client);
    packet->send();
}

CvrPacket::CvrPacket(MSNClient *client)
        : MSNPacket(client, "CVR")
{
    addArg("0x0409 winnt 5.1 i386 MSNMSGR");
    addArg(client->getVersion());
    addArg("MSMSGS");
    addArg(m_client->getLogin());
}

void CvrPacket::answer(vector<string> &arg)
{
    m_client->setVersion(arg[0].c_str());
    MSNPacket *packet = new UsrPacket(m_client);
    packet->send();
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

void UsrPacket::answer(vector<string> &args)
{
    if (args[0] == "OK"){
        QTimer::singleShot(0, m_client, SLOT(authOk()));
        return;
    }
    if (args[1] == "S"){
        m_client->m_authChallenge = args[2].c_str();
        m_client->requestLoginHost("https://nexus.passport.com/rdr/pprdr.asp");
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
    client->m_bJoin = false;
    addArg("0");
}

void SynPacket::answer(vector<string> &args)
{
    unsigned m_ver = 0;
    if (!args[0].empty())
        m_ver = atol(args[0].c_str());
    m_client->m_nBuddies = 0;
    m_client->m_nGroups  = 0;
    if ((args.size() > 1) && !args[1].empty())
        m_client->m_nBuddies = atol(args[1].c_str());
    if ((args.size() > 2) && !args[2].empty())
        m_client->m_nGroups  = atol(args[2].c_str());
    m_client->setListVer(m_ver);
    ContactList::GroupIterator itg;
    Group *grp;
    while ((grp = ++itg) != NULL){
        MSNUserData *data;
        ClientDataIterator it(grp->clientData, m_client);
        while ((data = (MSNUserData*)(++it)) != NULL){
            data->sFlags.value = data->Flags.value;
            if (args.size() > 1)
                data->Flags.value  = 0;
        }
    }
    ContactList::ContactIterator itc;
    Contact *contact;
    while ((contact = ++itc) != NULL){
        MSNUserData *data;
        ClientDataIterator it(contact->clientData, m_client);
        while ((data = (MSNUserData*)(++it)) != NULL){
            data->sFlags.value = data->Flags.value;
            if (args.size() > 1)
                data->Flags.value  = 0;
        }
    }
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

void AdgPacket::answer(vector<string> &args)
{
    Group *grp = getContacts()->group(m_id);
    if (grp == NULL)
        return;
    MSNUserData *data;
    ClientDataIterator it(grp->clientData, m_client);
    data = (MSNUserData*)(++it);
    if (data == NULL)
        data = (MSNUserData*)(grp->clientData.createData(m_client));
    data->Group.value = atol(args[2].c_str());
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

void AddPacket::error(unsigned)
{
    Contact *contact;
    MSNUserData *data = m_client->findContact(m_mail.c_str(), contact);
    if (data){
        contact->clientData.freeData(data);
        if (contact->clientData.size() == 0)
            delete contact;
    }
    Event e(static_cast<MSNPlugin*>(m_client->protocol()->plugin())->EventAddFail, (void*)(m_mail.c_str()));
    e.process();
}

void AddPacket::answer(vector<string>&)
{
    Event e(static_cast<MSNPlugin*>(m_client->protocol()->plugin())->EventAddOk, (void*)(m_mail.c_str()));
    e.process();
}

RemPacket::RemPacket(MSNClient *client, const char *listType, const char *mail, unsigned group)
        : MSNPacket(client, "REM")
{
    addArg(listType);
    addArg(mail);
    if (!strcmp(listType, "FL") && (group != NO_GROUP))
        addArg(number(group).c_str());
}

ReaPacket::ReaPacket(MSNClient *client, const char *mail, const char *name)
        : MSNPacket(client, "REA")
{
    addArg(mail);
    addArg(name);
}

void ReaPacket::error(unsigned code)
{
    if (code == 216)
        return;
    MSNPacket::error(code);
}

BlpPacket::BlpPacket(MSNClient *client)
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

void XfrPacket::answer(vector<string> &args)
{
    if (m_socket)
        m_socket->connect(args[1].c_str(), "", args[3].c_str(), true);
}

MSNServerMessage::MSNServerMessage(MSNClient *client, unsigned size)
{
    m_client = client;
    m_size   = size;
}

typedef map<QString, QString> KEY_MAP;

MSNServerMessage::~MSNServerMessage()
{
    KEY_MAP values;
    QString msg = QString::fromUtf8(m_msg.c_str());
    for (;!msg.isEmpty();){
        QString line;
        int n = msg.find("\r\n");
        if (n >= 0){
            line = msg.left(n);
            msg  = msg.mid(n + 2);
        }else{
            line = msg;
            msg  = "";
        }
        n = line.find(":");
        if (n < 0)
            continue;
        values.insert(KEY_MAP::value_type(line.left(n), trim(line.mid(n + 1))));
    }
    KEY_MAP::iterator it = values.find("ClientIP");
    if (it != values.end())
        set_ip(&m_client->data.owner.IP, inet_addr((*it).second.latin1()));
    it = values.find("Content-Type");
    if (it != values.end()){
        QString content_type = (*it).second;
        content_type = getToken(content_type, ';');
        if (content_type == "text/x-msmsgsinitialemailnotification"){
            m_client->m_init_mail = "";
            it = values.find("Post-URL");
            if (it != values.end())
                m_client->m_init_mail = (*it).second.latin1();
            it = values.find("Inbox-URL");
            if (it != values.end())
                m_client->m_init_mail += (*it).second.latin1();
            it = values.find("Inbox-Unread");
            if (it == values.end())
                return;
            unsigned nUnread = (*it).second.toUInt();
            if (nUnread){
                clientErrorData data;
                data.client		= m_client;
                data.err_str	= "%1";
                data.options	= NULL;
                data.args		= strdup(i18n("You have %n unread message", "You have %n unread messages", nUnread).utf8());
                data.code		= 0;
                data.flags		= ERR_INFO;
                data.id			= static_cast<MSNPlugin*>(m_client->protocol()->plugin())->MSNInitMail;
                Event e(EventShowError, &data);
                e.process();
            }
        }
        if (content_type == "text/x-msmsgsemailnotification"){
            m_client->m_new_mail = "";
            it = values.find("Post-URL");
            if (it != values.end())
                m_client->m_new_mail = (*it).second.latin1();
            it = values.find("Message-URL");
            if (it != values.end())
                m_client->m_new_mail += (*it).second.latin1();
            QString from;
            it = values.find("From-Addr");
            if (it != values.end())
                from = (*it).second;
            QString msg = i18n("You have new mail");
            if (!from.isEmpty())
                msg = i18n("%1 from %2") .arg(msg) .arg(from);
            clientErrorData data;
            data.client		= m_client;
            data.err_str	= "%1";
            data.options	= NULL;
            data.args		= strdup(msg.utf8());
            data.code		= 0;
            data.flags		= ERR_INFO;
            data.id			= static_cast<MSNPlugin*>(m_client->protocol()->plugin())->MSNNewMail;
            Event e(EventShowError, &data);
            e.process();
        }
    }
}

bool MSNServerMessage::packet()
{
    Buffer &b = m_client->m_socket->readBuffer;
    unsigned size = b.writePos() - b.readPos();
    if (size > m_size)
        size = m_size;
    if (size > 0){
        m_msg.append(b.data(b.readPos()), size);
        b.incReadPos(size);
        m_size -= size;
    }
    return (m_size == 0);
}

