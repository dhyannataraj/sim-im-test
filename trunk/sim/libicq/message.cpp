/***************************************************************************
                          message.cpp  -  description
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
#include "xml.h"

#include <memory>
#include <time.h>

const unsigned short ICQ_SMS_MESSAGE = 0;
const unsigned short ICQ_SMS_SUCCESS = 2;
const unsigned short ICQ_SMS_FAIL    = 3;

ICQMessage::ICQMessage(unsigned short type)
        : Time(this, "Time"),
        Received(this, "Direction"),
        Uin(this, "Uin"),
        Direct(this, "Direct"),
        m_nType(type)
{
    Id = 0;
    time_t now;
    time(&now);
    Time = (unsigned long)now;
    isExt = false;
    id1 = rand() & 0xFFFF;
    id2 = rand() & 0xFFFF;
    timestamp1 = 0;
    timestamp2 = 0;
    cookie1 = 0;
    cookie2 = 0;
    state = 0;
}

unsigned long ICQMessage::getUin()
{
    if (Uin.size() == 0) return 0;
    return *Uin.begin();
}

ICQMsg::ICQMsg()
        : ICQMessage(ICQ_MSGxMSG),
        Message(this, "Message"),
        ForeColor(this, "ForeColor"),
        BackColor(this, "BackColor")
{
}

ICQUrl::ICQUrl()
        : ICQMessage(ICQ_MSGxURL),
        URL(this, "URL"),
        Message(this, "Message")
{
}

ICQAuthRequest::ICQAuthRequest()
        : ICQMessage(ICQ_MSGxAUTHxREQUEST),
        Message(this, "Message")
{
}

ICQAuthGranted::ICQAuthGranted()
        : ICQMessage(ICQ_MSGxAUTHxGRANTED)
{
}

ICQAuthRefused::ICQAuthRefused()
        : ICQMessage(ICQ_MSGxAUTHxREFUSED),
        Message(this, "Message")
{
}

ICQAddedToList::ICQAddedToList()
        : ICQMessage(ICQ_MSGxADDEDxTOxLIST)
{
}

ICQSMS::ICQSMS()
        : ICQMessage(ICQ_MSGxSMS),
        Phone(this, "Phone"),
        Message(this, "Message"),
        Network(this, "Network")
{
}

ICQSMSReceipt::ICQSMSReceipt()
        : ICQMessage(ICQ_MSGxSMSxRECEIPT),
        MessageId(this, "MessageId"),
        Destination(this, "Destination"),
        Delivered(this, "Delivered"),
        Message(this, "Message")
{
}

ICQMsgExt::ICQMsgExt()
        : ICQMessage(ICQ_MSGxEXT),
        MessageType(this, "MessageType")
{
}

ICQFile::ICQFile()
        : ICQMessage(ICQ_MSGxFILE),
        Name(this, "Name"),
        Description(this, "Description"),
        Size(this, "Size")
{
    listener = NULL;
    ft = NULL;
    p = 0;
    autoAccept = false;
    wait = false;
}

ICQFile::~ICQFile()
{
    if (listener) listener->remove();
    if (ft) ft->remove();
}

void ICQFile::resume(int mode)
{
    if (ft) ft->resume(mode);
}

string ICQFile::shortName()
{
    string res = Name;
    const char *name = Name.c_str();
#ifdef WIN32
    char *p = strrchr(name, '\\');
#else
    char *p = strrchr(name, '/');
#endif
    if (p) res = p + 1;
    return res;
}

ICQChat::ICQChat()
        : ICQMessage(ICQ_MSGxCHAT),
        Reason(this, "Reason"),
        Clients(this, "Clients")
{
    listener = NULL;
    chat = NULL;
}

ICQChat::~ICQChat()
{
    if (listener) listener->remove();
    if (chat) chat->remove();
}

ICQWebPanel::ICQWebPanel()
        : ICQMessage(ICQ_MSGxWEBxPANEL),
        Name(this, "Name"),
        Email(this, "Email"),
        Message(this, "Message")
{
}

ICQEmailPager::ICQEmailPager()
        : ICQMessage(ICQ_MSGxEMAILxPAGER),
        Name(this, "Name"),
        Email(this, "Email"),
        Message(this, "Message")
{
}

ICQContacts::ICQContacts()
        : ICQMessage(ICQ_MSGxCONTACTxLIST),
        Contacts(this, "Contact")
{
}

ICQContactRequest::ICQContactRequest()
        : ICQMessage(ICQ_MSGxCONTACTxREQUEST),
        Message(this, "Message")
{
}

ICQMessage *ICQClient::parseMessage(unsigned short type, unsigned long uin, string &p, Buffer &packet,
                                    unsigned short cookie1, unsigned short cookie2,
                                    unsigned long timestamp1, unsigned long timestamp2)
{
    log(L_DEBUG, "Parse message [type=%u]", type);
    if (type == ICQ_MSGxMSG){
        if (uin < 1000){
            log(L_WARN, "Strange UIN %lu in message", uin);
            return NULL;
        }
        if (*(p.c_str()) == 0) return NULL;
        ICQMsg *msg = new ICQMsg;
        msg->Uin.push_back(uin);
        parseMessageText(p.c_str(), msg->Message);
        unsigned long forecolor, backcolor;
        packet >> forecolor >> backcolor;
        if (forecolor != backcolor){
            msg->ForeColor = forecolor >> 8;
            msg->BackColor = backcolor >> 8;
        }
        return msg;
    }
    if (type == ICQ_MSGxURL){
        if (uin < 1000){
            log(L_WARN, "Strange UIN %lu in URL message", uin);
            return NULL;
        }
        vector<string> l;
        if (!parseFE(p.c_str(), l, 2)){
            log(L_WARN, "Parse error URL message");
            return NULL;
        }
        ICQUrl *msg = new ICQUrl;
        msg->Uin.push_back(uin);
        parseMessageText(l[0].c_str(), msg->Message);
        msg->URL = l[1];
        return msg;
    }
    if (type == ICQ_MSGxAUTHxREQUEST){
        if (uin < 1000){
            log(L_WARN, "Strange UIN %lu in auth request message", uin);
            return NULL;
        }
        vector<string> l;
        if (!parseFE(p.c_str(), l, 6)){
            log(L_WARN, "Parse error auth request message");
            return NULL;
        }
        ICQAuthRequest *msg = new ICQAuthRequest;
        msg->Uin.push_back(uin);
        parseMessageText(l[4].c_str(), msg->Message);
        return msg;
    }
    if (type == ICQ_MSGxAUTHxGRANTED){
        if (uin < 1000){
            log(L_WARN, "Strange UIN %lu in auth granted message", uin);
            return NULL;
        }
        ICQAuthGranted *msg = new ICQAuthGranted;
        msg->Uin.push_back(uin);
        return msg;
    }
    if (type == ICQ_MSGxAUTHxREFUSED){
        if (uin < 1000){
            log(L_WARN, "Strange UIN %lu in auth refused message", uin);
            return NULL;
        }
        ICQAuthRefused *msg = new ICQAuthRefused;
        msg->Uin.push_back(uin);
        parseMessageText(p.c_str(), msg->Message);
        return msg;
    }
    if (type == ICQ_MSGxADDEDxTOxLIST){
        if (uin < 1000){
            log(L_WARN, "Strange UIN %lu in added to list message", uin);
            return NULL;
        }
        ICQAddedToList *msg = new ICQAddedToList;
        msg->Uin.push_back(uin);
        return msg;
    }
    if (type == ICQ_MSGxCONTACTxLIST){
        if (uin < 1000){
            log(L_WARN, "Strange UIN %lu in contact message", uin);
            return NULL;
        }
        vector<string> l;
        if (!parseFE(p.c_str(), l, 2)){
            log(L_WARN, "Parse error contacts message");
            return NULL;
        }
        unsigned nContacts = atol(l[0].c_str());
        if (nContacts == 0){
            log(L_WARN, "No contacts found");
            return NULL;
        }
        vector<string> c;
        if (!parseFE(l[1].c_str(), c, nContacts*2+1)){
            log(L_WARN, "Parse error contacts message");
            return NULL;
        }
        ICQContacts *msg = new ICQContacts;
        msg->Uin.push_back(uin);
        for (unsigned i = 0; i < nContacts; i++){
            Contact *contact = new Contact;
            contact->Uin = atol(c[i*2].c_str());
            fromServer(c[i*2+1]);
            contact->Alias = c[i*2+1];
            msg->Contacts.push_back(contact);
        }
        return msg;
    }
    if (type == ICQ_MSGxCHAT){
        unsigned short port;
        unsigned short junk;
        string name;
        packet
        >> name
        >> port >> junk
        >> junk >> junk;
        fromServer(p);
        ICQChat *msg = new ICQChat();
        msg->Uin.push_back(uin);
        msg->Reason = p;
        msg->id1 = port;
        msg->timestamp1 = timestamp1;
        return msg;
    }
    if (type == ICQ_MSGxFILE){
        unsigned short port;
        unsigned short junk;
        unsigned long fileSize;
        string fileName;
        packet
        >> port
        >> junk
        >> fileName
        >> fileSize
        >> junk >> junk;
        fromServer(p);
        fromServer(fileName);
        const char *shortName = strrchr(fileName.c_str(), '\\');
        if (shortName){
            shortName++;
        }else{
            shortName = fileName.c_str();
        }
        ICQFile *msg = new ICQFile();
        msg->Uin.push_back(uin);
        msg->Name = shortName;
        msg->Description = p;
        msg->Size = htonl(fileSize);
        msg->id1 = port;
        msg->timestamp1 = timestamp1;
        return msg;
    }
    if (type == ICQ_MSGxEXT){
        string header;
        packet >> header;
        Buffer h(header.size());
        h.pack(header.c_str(), header.size());
        h.incReadPos(16);
        unsigned short msg_type;
        h >> msg_type;
        string msgType;
        h.unpackStr32(msgType);
        string info;
        packet.unpackStr32(info);
        Buffer b(info.size());
        b.pack(info.c_str(), info.size());
        log(L_DEBUG, "Extended message %s [%04X] %u", msgType.c_str(), msg_type, info.size());
        if (strstr(msgType.c_str(), "File")){
            string fileDescr;
            b.unpackStr32(fileDescr);
            unsigned short id1, id2;
            b >> id1 >> id2;
            string fileName;
            b >> fileName;
            unsigned long fileSize;
            b >> fileSize;
            fileSize = htonl(fileSize);
            fromServer(fileDescr);
            fromServer(fileName);
            ICQFile *msg = new ICQFile();
            msg->isExt = true;
            msg->Uin.push_back(uin);
            msg->Name = fileName;
            msg->Description = fileDescr;
            msg->Size = fileSize;
            msg->id1 = id1;
            msg->id2 = id2;
            msg->cookie1 = cookie1;
            msg->cookie2 = cookie2;
            msg->timestamp1 = timestamp1;
            msg->timestamp2 = timestamp2;
            return msg;
        }
        if (strstr(msgType.c_str(), "Web Page Address (URL)")){
            string info;
            b.unpackStr32(info);
            vector<string> l;
            if (!parseFE(info.c_str(), l, 2)){
                log(L_DEBUG, "Parse error extended URL message");
                return NULL;
            }
            ICQUrl *msg = new ICQUrl();
            msg->Uin.push_back(uin);
            parseMessageText(l[0].c_str(), msg->Message);
            msg->URL = l[1];
            return msg;
        }
        if (!strcmp(msgType.c_str(), "Request For Contacts")){
            string info;
            b.unpackStr32(info);
            ICQContactRequest *msg = new ICQContactRequest();
            msg->Uin.push_back(uin);
            parseMessageText(info.c_str(), msg->Message);
            return msg;
        }
        if (!strcmp(msgType.c_str(), "Send / Start ICQ Chat")){
            string reason;
            b.unpackStr32(reason);
            ICQChat *msg = new ICQChat;
            parseMessageText(reason.c_str(), msg->Reason);
            char flag;
            b >> flag;
            if (flag){
                b.incReadPos(2);
                b >> msg->id1 >> msg->id2;
            }
            msg->Uin.push_back(uin);
            msg->cookie1 = cookie1;
            msg->cookie2 = cookie2;
            msg->timestamp1 = timestamp1;
            msg->timestamp2 = timestamp2;
            return msg;
        }
        if (!strcmp(msgType.c_str(), "Contacts")){
            string p;
            b.unpackStr32(p);
            vector<string> l;
            if (!parseFE(p.c_str(), l, 2)){
                log(L_WARN, "Parse error contacts message");
                return NULL;
            }
            unsigned nContacts = atol(l[0].c_str());
            if (nContacts == 0){
                log(L_WARN, "No contacts found");
                return NULL;
            }
            vector<string> c;
            if (!parseFE(l[1].c_str(), c, nContacts*2+1)){
                log(L_WARN, "Parse error contacts message");
                return NULL;
            }
            ICQContacts *msg = new ICQContacts;
            msg->Uin.push_back(uin);
            for (unsigned i = 0; i < nContacts; i++){
                Contact *contact = new Contact;
                contact->Uin = atol(c[i*2].c_str());
                fromServer(c[i*2+1]);
                contact->Alias = c[i*2+1];
                msg->Contacts.push_back(contact);
            }
            return msg;
        }
        if (!strcmp(msgType.c_str(), "ICQSMS")){
            string xmlstring;
            b.unpackStr32(xmlstring);
            string::iterator s = xmlstring.begin();
            auto_ptr<XmlNode> top(XmlNode::parse(s, xmlstring.end()));
            if (top.get() == NULL){
                log(L_WARN, "Parse SMS XML error");
                return NULL;
            }
            switch (msg_type){
            case ICQ_SMS_MESSAGE:{
                    if (top->getTag() != "sms_message"){
                        log(L_WARN, "No sms_message tag in SMS message");
                        return NULL;
                    }
                    XmlNode *n = top.get();
                    if ((n == NULL) || !n->isBranch()){
                        log(L_WARN, "Parse no branch");
                        return NULL;
                    }
                    XmlBranch *sms_message = static_cast<XmlBranch*>(n);
                    XmlLeaf *text = sms_message->getLeaf("text");
                    if (text == NULL){
                        log(L_WARN, "No <text> in SMS message");
                        return NULL;
                    }
                    ICQSMS *m = new ICQSMS;
                    m->Message = text->getValue();
                    fromUTF(m->Message);

                    XmlLeaf *sender = sms_message->getLeaf("sender");
                    if (sender != NULL) m->Phone = sender->getValue();
                    fromUTF(m->Phone);

                    XmlLeaf *senders_network = sms_message->getLeaf("senders_network");
                    if (senders_network != NULL) m->Network = senders_network->getValue();

                    m->Uin.push_back(contacts.findByPhone(m->Phone));
                    return m;
                }
            case ICQ_SMS_SUCCESS:
            case ICQ_SMS_FAIL:{
                    if (top->getTag() != "sms_delivery_receipt"){
                        log(L_WARN, "No sms_delivery_receipt tag in SMS message");
                        return NULL;
                    }
                    XmlNode *n = top.get();
                    if ((n == NULL) || !n->isBranch()){
                        log(L_WARN, "Parse no branch");
                        return NULL;
                    }
                    XmlBranch *sms_message = static_cast<XmlBranch*>(n);
                    ICQSMSReceipt *m = new ICQSMSReceipt;

                    XmlLeaf *message_id = sms_message->getLeaf("message_id");
                    if (message_id != NULL) m->MessageId = message_id->getValue();

                    XmlLeaf *destination = sms_message->getLeaf("destination");
                    if (destination != NULL) m->Destination = destination->getValue();

                    XmlLeaf *delivered = sms_message->getLeaf("delivered");
                    if (delivered != NULL) m->Delivered = delivered->getValue();

                    XmlLeaf *text = sms_message->getLeaf("text");
                    if (text != NULL) m->Message = text->getValue();

                    return m;
                }
            default:
                log(L_WARN, "Unknown delivery status for SMS %04X", msg_type);
                return NULL;
            }
        }
        ICQMsgExt *m = new ICQMsgExt;
        m->Uin.push_back(uin);
        m->MessageType = msgType;
        return m;
    }
    log(L_WARN, "Unknown message type %04X", type);
    return NULL;
}

void ICQClient::parseMessageText(const char *p, string &s)
{
    if ((strlen(p) >= 5) && !memcmp(p, "{\\rtf", 5)){
        string r(p);
        fromServer(r);
        s = parseRTF(r.c_str());
        return;
    }
    quoteText(p, s);
    fromServer(s);
}

void ICQClient::quoteText(const char *p, string &s)
{
    for (; *p; p++){
        switch (*p){
        case '\n':
            s += "<br>";
            break;
        case '\t':
            s += ' ';
            break;
        case '<':
            s += "&lt;";
            break;
        case '>':
            s += "&gt;";
            break;
        case '"':
            s += "&quot;";
            break;
        case '&':
            s += "&amp;";
            break;
        default:
            if ((unsigned char)(*p) >= ' ')
                s += *p;
        }
    }
}

bool ICQClient::parseFE(const char *str, vector<string> &l, unsigned n)
{
    const char *p = str;
    for (unsigned i = 0; i < n - 1; i++){
        for (; *str; str++)
            if (*str == (char)0xFE) break;
        if (*str == 0) return false;
        l.push_back(string(p, str - p));
        str++;
        p = str;
    }
    l.push_back(string(p));
    return true;
}

// _______________________________________________________________________________________________

ICQEvent::ICQEvent(int type, unsigned long uin, unsigned long subtype, ICQMessage *_msg)
{
    msg = _msg;
    state = Unknown;
    m_nType = type;
    m_nSubType = subtype;
    m_nId = 0;
    m_nUin = uin;
}

bool ICQEvent::processAnswer(ICQClient *client, Buffer &, unsigned short)
{
    state = Fail;
    client->process_event(this);
    return true;
}

void ICQEvent::failAnswer(ICQClient *client)
{
    state = Fail;
    client->process_event(this);
}

class SMSSendEvent : public ICQEvent
{
public:
    SMSSendEvent() : ICQEvent(EVENT_MESSAGE_SEND) {}
protected:
    bool processAnswer(ICQClient *client, Buffer &b, unsigned short nSubtype);
    bool parseResponse(string &xml);
};

bool SMSSendEvent::parseResponse(string &xmlstring)
{
    ICQSMS *sms = static_cast<ICQSMS*>(msg);
    bool res = false;
    string::iterator s = xmlstring.begin();
    auto_ptr<XmlNode> top(XmlNode::parse(s, xmlstring.end()));
    if (top.get() == NULL){
        log(L_WARN, "Parse SMS XML error");
        return false;
    }
    if (top->getTag() != "sms_response"){
        log(L_WARN, "No sms_message tag in SMS message");
        return false;
    }
    XmlNode *n = top.get();
    if ((n == NULL) || !n->isBranch()){
        log(L_WARN, "Parse no branch");
        return false;
    }
    XmlBranch *sms_response = static_cast<XmlBranch*>(n);
    XmlLeaf *deliverable = sms_response->getLeaf("deliverable");
    if (deliverable && (deliverable->getValue() == "Yes"))
        res = true;
    XmlLeaf *network = sms_response->getLeaf("network");
    if (network)
        sms->Network = network->getValue();
    XmlBranch *error = sms_response->getBranch("error");
    if (error){
        XmlBranch *params = error->getBranch("params");
        if (params){
            XmlLeaf *param = params->getLeaf("param");
            if (param)
                sms->DeclineReason = param->getValue();
        }
    }
    return res;
}

bool SMSSendEvent::processAnswer(ICQClient *client, Buffer &b, unsigned short)
{
    log(L_DEBUG, "Process SMS response");
    b.incReadPos(6);
    string oper;
    b.unpackStr(oper);
    string xmlstring;
    b.unpackStr(xmlstring);
    state = parseResponse(xmlstring) ? Success : Fail;
    client->process_event(this);
    return true;
}

ICQEvent *ICQClient::sendMessage(ICQMessage *msg)
{
    if (msg->Type() == ICQ_MSGxFILE){
        ICQFile *f = static_cast<ICQFile*>(msg);
        if (f->Size() == 0)
            f->Size = getFileSize(f->Name.c_str());
    }
    for (ConfigULongs::iterator itUin = msg->Uin.begin(); itUin != msg->Uin.end(); ++itUin){
        ICQUser *u = getUser(*itUin);
        if (u){
            time_t now;
            time(&now);
            u->LastActive = (unsigned long)now;
            ICQEvent e(EVENT_STATUS_CHANGED, *itUin);
            process_event(&e);
        }
    }

    if ((msg->Type() == ICQ_MSGxCHAT) || (msg->Type() == ICQ_MSGxFILE)){
        ICQUser *u = getUser(msg->getUin(), false);
        if (u && (u->uStatus != ICQ_STATUS_OFFLINE) &&
                (u->direct || (Port() && (IP() || RealIP()))))
            return u->addMessage(msg, this);
    }


    list<ICQEvent*>::iterator it;
    for (it = msgQueue.begin(); it != msgQueue.end(); it++){
        if ((*it)->msg == msg) return NULL;
    }
    ICQEvent *e;
    if (msg->Type() == ICQ_MSGxSMS){
        e = new SMSSendEvent();
    }else{
        e = new ICQEvent(EVENT_MESSAGE_SEND);
    }
    e->msg = msg;
    msgQueue.push_back(e);
    processMsgQueue();
    for (it = msgQueue.begin(); it != msgQueue.end(); it++)
        if ((*it) == e){
            if (e->message()) e->message()->Id = m_nProcessId++;
            return e;
        }
    for (it = processQueue.begin(); it != processQueue.end(); it++)
        if ((*it) == e){
            if (e->message()) e->message()->Id = m_nProcessId++;
            return e;
        }
    return NULL;
}

bool ICQClient::cancelMessage(ICQMessage *m, bool bSendCancel)
{
    if (m == NULL) return false;
    list<ICQEvent*>::iterator it;
    for (it = msgQueue.begin(); it != msgQueue.end(); it++){
        if ((*it)->message() == m){
            ICQEvent *e = *it;
            msgQueue.remove(e);
            e->state = ICQEvent::Fail;
            m->bDelete = true;
            process_event(e);
            if (m->bDelete) delete m;
            delete e;
            return true;
        }
    }
    for (it = processQueue.begin(); it != processQueue.end(); it++){
        if ((*it)->message() == m){
            ICQEvent *e = *it;
            if (bSendCancel && (m->Type() == ICQ_MSGxFILE))
                cancelSendFile(static_cast<ICQFile*>(m));
            processQueue.remove(e);
            e->state = ICQEvent::Fail;
            m->bDelete = true;
            process_event(e);
            if (m->bDelete) delete m;
            delete e;
            return true;
        }
    }
    ICQUser *u = getUser(m->getUin());
    if (u == NULL) return false;
    for (it = u->msgQueue.begin(); it != u->msgQueue.end(); it++){
        if ((*it)->message() == m){
            ICQEvent *e = *it;
            u->msgQueue.remove(e);
            e->state = ICQEvent::Fail;
            m->bDelete = true;
            process_event(e);
            if (m->bDelete) delete m;
            delete e;
            return true;
        }
    }
    return false;
}

void ICQClient::messageReceived(ICQMessage *msg)
{
    bool bAddUser;
    switch (msg->Type()){
    case ICQ_MSGxMSG:
        bAddUser = !RejectMessage();
        break;
    case ICQ_MSGxURL:
        bAddUser = !RejectURL();
        break;
    case ICQ_MSGxAUTHxREQUEST:
    case ICQ_MSGxAUTHxREFUSED:
    case ICQ_MSGxAUTHxGRANTED:
    case ICQ_MSGxADDEDxTOxLIST:
    case ICQ_MSGxSMS:
    case ICQ_MSGxSMSxRECEIPT:
        bAddUser = true;
        break;
    case ICQ_MSGxWEBxPANEL:
        bAddUser = !RejectWeb();
        break;
    case ICQ_MSGxEMAILxPAGER:
        bAddUser = !RejectEmail();
        break;
    default:
        bAddUser = !RejectOther();
    }
    ICQUser *u = getUser(msg->getUin(), bAddUser, true);
    if ((u== NULL) || u->inIgnore()){
        delete msg;
        return;
    }
    if (!bAddUser && (u->GrpId() == 0)){
        delete msg;
        return;
    }
    if (bAddUser){
        string text;
        switch (msg->Type()){
        case ICQ_MSGxMSG:{
                ICQMsg *m = static_cast<ICQMsg*>(msg);
                text = m->Message;
                break;
            }
        case ICQ_MSGxURL:{
                ICQUrl *m = static_cast<ICQUrl*>(msg);
                text = m->Message;
                break;
            }
        }
        if (match(text.c_str(), RejectFilter.c_str())){
            delete msg;
            return;
        }
        u = getUser(msg->getUin(), true);
    }
    time_t now;
    time(&now);
    u->LastActive = (unsigned long)now;
    msg->Received = true;
    switch (msg->Type()){
    case ICQ_MSGxCHAT:
    case ICQ_MSGxFILE:{
            ICQEvent *e = new ICQEvent(EVENT_MESSAGE_RECEIVED, msg->getUin());
            msg->Id = m_nProcessId++;
            e->msg = msg;
            processQueue.push_back(e);
            process_event(e);
            return;
        }
    default:
        break;
    }
    ICQEvent e(EVENT_MESSAGE_RECEIVED, msg->getUin());
    e.msg = msg;
    process_event(&e);
    delete msg;
}

void ICQClient::processMsgQueue()
{
    if (m_state != Logged) return;
    processMsgQueueThruServer();
    processMsgQueueSMS();
    processMsgQueueAuth();
}

Contact::Contact()
        : Uin(this, "Uin"),
        Alias(this, "Alias")
{
}


