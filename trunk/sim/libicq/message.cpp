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
#include "icqprivate.h"
#include "log.h"
#include "xml.h"

#ifndef WIN32
#include <stdio.h>
#endif

#include <memory>
#include <time.h>

#ifndef HAVE_STRCASECMP
int strcasecmp(const char *a, const char *b);
#endif

const unsigned short ICQ_SMS_MESSAGE = 0;
const unsigned short ICQ_SMS_SUCCESS = 2;
const unsigned short ICQ_SMS_FAIL    = 3;

ICQMessage::ICQMessage(unsigned short type)
{
    Received = false;
    Direct = false;
    m_nType = type;
    Id = MSG_NEW;
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
        : ICQMessage(ICQ_MSGxMSG)
{
    ForeColor = 0;
    BackColor = 0;
}

ICQUrl::ICQUrl()
        : ICQMessage(ICQ_MSGxURL)
{
}

ICQAuthRequest::ICQAuthRequest()
        : ICQMessage(ICQ_MSGxAUTHxREQUEST)
{
}

ICQAuthGranted::ICQAuthGranted()
        : ICQMessage(ICQ_MSGxAUTHxGRANTED)
{
}

ICQAuthRefused::ICQAuthRefused()
        : ICQMessage(ICQ_MSGxAUTHxREFUSED)
{
}

ICQAddedToList::ICQAddedToList()
        : ICQMessage(ICQ_MSGxADDEDxTOxLIST)
{
}

ICQSMS::ICQSMS()
        : ICQMessage(ICQ_MSGxSMS)
{
}

ICQSMSReceipt::ICQSMSReceipt()
        : ICQMessage(ICQ_MSGxSMSxRECEIPT)
{
}

ICQMsgExt::ICQMsgExt()
        : ICQMessage(ICQ_MSGxEXT)
{
}

ICQFile::ICQFile()
        : ICQMessage(ICQ_MSGxFILE)
{
    Size = 0;
    listener = NULL;
    ft = NULL;
    p = 0;
    client = NULL;
    autoAccept = false;
    wait = false;
}

ICQFile::~ICQFile()
{
    if (p && client) client->closeFile(this);
    if (listener) delete listener;
    if (ft) delete ft;
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

unsigned ICQFile::nFiles()
{
    if (ft == NULL) return 0;
    return ft->nFiles();
}

unsigned ICQFile::curFile()
{
    if (ft == NULL) return 0;
    return ft->curFile();
}

unsigned short ICQFile::speed()
{
    if (ft == NULL) return 100;
    return ft->speed();
}

unsigned long ICQFile::totalSize()
{
    if (ft == NULL) return 0;
    return ft->totalSize();
}

unsigned long ICQFile::sendSize()
{
    if (ft == NULL) return 0;
    return ft->sendSize();
}

unsigned long ICQFile::curSize()
{
    if (ft == NULL) return 0;
    return ft->curSize();
}

string ICQFile::curName()
{
    if (ft == NULL) return "";
    return ft->curName;
}

void ICQFile::setCurName(const char *name)
{
    if (ft == NULL) return;
    ft->curName = name;
}

void ICQFile::setPos(unsigned long pos)
{
    if (ft == NULL) return;
    ft->setPos(pos);
}

void ICQFile::setSpeed(unsigned short speed)
{
    if (ft == NULL) return;
    ft->setSpeed(speed);
}

ICQChat::ICQChat()
        : ICQMessage(ICQ_MSGxCHAT)
{
    listener = NULL;
    chat = NULL;
}

ICQChat::~ICQChat()
{
    if (listener) delete listener;
    if (chat) delete chat;
}

void ICQChat::sendLine(const char *line)
{
    if (chat == NULL) return;
    chat->sendLine(line);
}

bool ICQChat::isBold()
{
    if (chat == NULL) return false;
    return (chat->fontFace & FONT_BOLD) != 0;
}

bool ICQChat::isItalic()
{
    if (chat == NULL) return false;
    return (chat->fontFace & FONT_ITALIC) != 0;
}

bool ICQChat::isUnderline()
{
    if (chat == NULL) return false;
    return (chat->fontFace & FONT_UNDERLINE) != 0;
}

unsigned long ICQChat::bgColor()
{
    if (chat == NULL) return 0xFFFFFF;
    return chat->bgColor;
}

unsigned long ICQChat::fgColor()
{
    if (chat == NULL) return 0;
    return chat->fgColor;
}

ICQWebPanel::ICQWebPanel()
        : ICQMessage(ICQ_MSGxWEBxPANEL)
{
}

ICQEmailPager::ICQEmailPager()
        : ICQMessage(ICQ_MSGxEMAILxPAGER)
{
}

ICQContacts::ICQContacts()
        : ICQMessage(ICQ_MSGxCONTACTxLIST)
{
}

ICQContactRequest::ICQContactRequest()
        : ICQMessage(ICQ_MSGxCONTACTxREQUEST)
{
}

ICQSecureOn::ICQSecureOn()
        : ICQMessage(ICQ_MSGxSECURExOPEN)
{
}

ICQSecureOff::ICQSecureOff()
        : ICQMessage(ICQ_MSGxSECURExCLOSE)
{
}

ICQAutoResponse::ICQAutoResponse()
        : ICQMessage(ICQ_READxAWAYxMSG)
{
}

ICQStatus::ICQStatus()
        : ICQMessage(ICQ_MSGxSTATUS)
{
    status = ICQ_STATUS_OFFLINE;
}

ICQMessage *ICQClientPrivate::parseMessage(unsigned short type, unsigned long uin, string &p, Buffer &packet,
        unsigned short cookie1, unsigned short cookie2,
        unsigned long timestamp1, unsigned long timestamp2)
{
    log(L_DEBUG, "Parse message [type=%u]", type);
    ICQUser *u = client->getUser(uin);
    switch (type){
    case ICQ_MSGxMSG:{
            if (uin < 1000){
                log(L_WARN, "Strange UIN %lu in message", uin);
                return NULL;
            }
            if (*(p.c_str()) == 0) return NULL;
            ICQMsg *msg = new ICQMsg;
            msg->Uin.push_back(uin);
            if (parseMessageText(p.c_str(), msg->Message, u))
                msg->Charset = "utf-8";
            unsigned long forecolor, backcolor;
            packet >> forecolor >> backcolor;
            if (forecolor != backcolor){
                msg->ForeColor = forecolor >> 8;
                msg->BackColor = backcolor >> 8;
            }
            return msg;
        }
    case ICQ_MSGxURL:{
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
            parseMessageText(l[0].c_str(), msg->Message, u);
            msg->URL = l[1];
            return msg;
        }
    case ICQ_MSGxAUTHxREQUEST:{
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
            parseMessageText(l[4].c_str(), msg->Message, u);
            return msg;
        }
    case ICQ_MSGxAUTHxGRANTED:{
            if (uin < 1000){
                log(L_WARN, "Strange UIN %lu in auth granted message", uin);
                return NULL;
            }
            ICQAuthGranted *msg = new ICQAuthGranted;
            msg->Uin.push_back(uin);
            return msg;
        }
    case ICQ_MSGxAUTHxREFUSED:{
            if (uin < 1000){
                log(L_WARN, "Strange UIN %lu in auth refused message", uin);
                return NULL;
            }
            ICQAuthRefused *msg = new ICQAuthRefused;
            msg->Uin.push_back(uin);
            parseMessageText(p.c_str(), msg->Message, u);
            return msg;
        }
    case ICQ_MSGxADDEDxTOxLIST:{
            if (uin < 1000){
                log(L_WARN, "Strange UIN %lu in added to list message", uin);
                return NULL;
            }
            ICQAddedToList *msg = new ICQAddedToList;
            msg->Uin.push_back(uin);
            return msg;
        }
    case ICQ_MSGxCONTACTxLIST:{
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
                client->fromServer(c[i*2+1], u);
                contact->Alias = c[i*2+1];
                msg->Contacts.push_back(contact);
            }
            return msg;
        }
    case ICQ_MSGxCHAT:{
            unsigned short port;
            unsigned short junk;
            string name;
            packet
            >> name
            >> port >> junk
            >> junk >> junk;
            client->fromServer(p, u);
            ICQChat *msg = new ICQChat();
            msg->Uin.push_back(uin);
            msg->Reason = p;
            msg->id1 = port;
            msg->timestamp1 = timestamp1;
            return msg;
        }
    case ICQ_MSGxFILE:{
            unsigned short port;
            unsigned short junk;
            unsigned long fileSize;
            string fileName;
            packet
            >> port
            >> junk
            >> fileName;
            packet.unpack(fileSize);
            packet.incReadPos(4);
            client->fromServer(p, u);
            client->fromServer(fileName, u);
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
            msg->Size = fileSize;
            msg->id1 = port;
            msg->timestamp1 = timestamp1;
            return msg;
        }
    case ICQ_MSGxSECURExOPEN:{
            ICQSecureOn *msg = new ICQSecureOn;
            msg->Uin.push_back(uin);
            return msg;
        }
    case ICQ_MSGxSECURExCLOSE:{
            ICQSecureOff *msg = new ICQSecureOff;
            msg->Uin.push_back(uin);
            return msg;
        }
    case ICQ_READxAWAYxMSG:
    case ICQ_READxOCCUPIEDxMSG:
    case ICQ_READxNAxMSG:
    case ICQ_READxDNDxMSG:
    case ICQ_READxFFCxMSG:{
            ICQAutoResponse *msg = new ICQAutoResponse;
            msg->setType(type);
            return msg;
        }
    case ICQ_MSGxEXT:{
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
                b.unpack(fileSize);
                client->fromServer(fileDescr, u);
                client->fromServer(fileName, u);
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
                parseMessageText(l[0].c_str(), msg->Message, u);
                msg->URL = l[1];
                return msg;
            }
            if (!strcmp(msgType.c_str(), "Request For Contacts")){
                string info;
                b.unpackStr32(info);
                ICQContactRequest *msg = new ICQContactRequest();
                msg->Uin.push_back(uin);
                parseMessageText(info.c_str(), msg->Message, u);
                return msg;
            }
            if (!strcmp(msgType.c_str(), "Send / Start ICQ Chat")){
                string reason;
                b.unpackStr32(reason);
                ICQChat *msg = new ICQChat;
                parseMessageText(reason.c_str(), msg->Reason, u);
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
                    client->fromServer(c[i*2+1], u);
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
                        m->Charset = "utf-8";
                        m->Message = text->getValue();
                        XmlLeaf *sender = sms_message->getLeaf("sender");
                        if (sender != NULL) m->Phone = sender->getValue();
                        XmlLeaf *senders_network = sms_message->getLeaf("senders_network");
                        if (senders_network != NULL) m->Network = senders_network->getValue();

                        m->Uin.push_back(client->contacts.findByPhone(m->Phone));
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
    default:
        log(L_WARN, "Unknown message type %04X", type);
    }
    return NULL;
}

bool ICQClientPrivate::parseMessageText(const char *p, string &s, ICQUser *u)
{
    if ((strlen(p) >= 5) && !memcmp(p, "{\\rtf", 5)){
        string r(p);
        client->fromServer(r, u);
        s = parseRTF(r.c_str(), u);
        return true;
    }
    s = ICQClient::quoteText(p);
    client->fromServer(s, u);
    return false;
}

static string replace_all(const string& s, const string& r1, const string& r2) {
    string t(s.c_str());
    int curr = 0, next;
    while ( (next = t.find( r1, curr )) != -1) {
        t.replace( next, r1.size(), r2 );
        curr = next + r2.size();
    }
    return t;
}

string ICQClient::quoteText(const char *p)
{
    return
        replace_all(
            replace_all(
                replace_all(
                    replace_all(
                        replace_all(
                            replace_all(p,
                                        "&", "&amp;"),
                            "\"", "&quot;"),
                        "<", "&lt;"),
                    ">", "&gt;"),
                "\t", " "),
            "\n", "<br>");
}

string ICQClient::unquoteText(const char *p)
{
    return
        replace_all(
            replace_all(
                replace_all(
                    replace_all(
                        replace_all(p,
                                    "<br>", "\n"),
                        "&lt;", "<"),
                    "&gt;", ">"),
                "&quot;", "\""),
            "&amp;", "&");
}

bool ICQClientPrivate::parseFE(const char *str, vector<string> &l, unsigned n)
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

bool ICQEvent::processAnswer(ICQClientPrivate *client, Buffer &, unsigned short)
{
    state = Fail;
    client->client->process_event(this);
    return true;
}

void ICQEvent::failAnswer(ICQClientPrivate *client)
{
    state = Fail;
    client->client->process_event(this);
}

class SMSSendEvent : public ICQEvent
{
public:
    SMSSendEvent() : ICQEvent(EVENT_MESSAGE_SEND) {}
protected:
    bool processAnswer(ICQClientPrivate *client, Buffer &b, unsigned short nSubtype);
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

bool SMSSendEvent::processAnswer(ICQClientPrivate *client, Buffer &b, unsigned short)
{
    log(L_DEBUG, "Process SMS response");
    char c;
    b >> c;
    if (c){
        string err_msg;
        err_msg.assign("\x00", b.size() - b.readPos() + 2);
        ((char*)(err_msg.c_str()))[0] = c;
        b.unpack((char*)(err_msg.c_str()) + 1, b.size() - b.readPos());
        ICQSMS *sms = static_cast<ICQSMS*>(msg);
        sms->DeclineReason = err_msg.c_str();
        state = Fail;
    }else{
        b.incReadPos(5);
        string oper;
        b.unpackStr(oper);
        string xmlstring;
        b.unpackStr(xmlstring);
        state = parseResponse(xmlstring) ? Success : Fail;
    }
    client->client->process_event(this);
    return true;
}

ICQEvent *ICQClient::sendMessage(ICQMessage *msg)
{
    if (msg->Type() == ICQ_MSGxFILE){
        ICQFile *f = static_cast<ICQFile*>(msg);
        if (f->Size == 0){
            f->localName = f->Name;
            int nSrcFiles = 0;
            f->Size = getFileSize(f->Name.c_str(), &nSrcFiles, f->files);
            if (nSrcFiles == 0){
                f->DeclineReason = "No files for transfer";
                ICQEvent e(f->getUin(), EVENT_MESSAGE_SEND);
                e.setMessage(f);
                e.state = ICQEvent::Fail;
                process_event(&e);
                return NULL;
            }
            if (nSrcFiles > 1){
                char b[32];
                snprintf(b, sizeof(b), "%u Files", nSrcFiles);
                f->Name = b;
            }else{
                f->Name = f->shortName();
            }
        }
    }
    for (list<unsigned long>::iterator itUin = msg->Uin.begin(); itUin != msg->Uin.end(); ++itUin){
        ICQUser *u = getUser(*itUin);
        if (u){
            time_t now;
            time(&now);
            u->LastActive = (unsigned long)now;
            ICQEvent e(EVENT_STATUS_CHANGED, *itUin);
            process_event(&e);
        }
    }

    if (msg->Uin.size() == 1){
        ICQUser *u = getUser(msg->getUin(), false);
        if ((msg->Type() == ICQ_MSGxCHAT) || (msg->Type() == ICQ_MSGxFILE)){
            if (u && (u->uStatus != ICQ_STATUS_OFFLINE) &&
                    (u->direct || (u->Port && (u->IP || u->RealIP)))){
                ICQEvent *e = u->addMessage(msg, p);
                if (e){
                    for (list<ICQEvent*>::iterator it = u->msgQueue.begin(); it != u->msgQueue.end(); it++)
                        if ((*it) == e){
                            return e;
                        }
                }
                return NULL;
            }
        }
        if (u && u->direct && u->direct->isSecure() &&
                ((msg->Type() == ICQ_MSGxMSG) ||
                 (msg->Type() == ICQ_MSGxURL) ||
                 (msg->Type() == ICQ_MSGxCONTACTxLIST))){
            ICQEvent *e = u->addMessage(msg, p);
            if (e){
                for (list<ICQEvent*>::iterator it = u->msgQueue.begin(); it != u->msgQueue.end(); it++)
                    if ((*it) == e){
                        return e;
                    }
            }
            return NULL;
        }
    }

    list<ICQEvent*>::iterator it;
    for (it = p->msgQueue.begin(); it != p->msgQueue.end(); it++){
        if ((*it)->msg == msg) return NULL;
    }
    ICQEvent *e;
    if (msg->Type() == ICQ_MSGxSMS){
        e = new SMSSendEvent();
    }else{
        e = new ICQEvent(EVENT_MESSAGE_SEND);
    }
    e->msg = msg;
    p->msgQueue.push_back(e);
    p->processMsgQueue();
    for (it = p->msgQueue.begin(); it != p->msgQueue.end(); it++)
        if ((*it) == e){
            if (e->message()) e->message()->Id = p->m_nProcessId++;
            return e;
        }
    for (it = p->processQueue.begin(); it != p->processQueue.end(); it++)
        if ((*it) == e){
            if (e->message()) e->message()->Id = p->m_nProcessId++;
            return e;
        }
    for (it = p->varEvents.begin(); it != p->varEvents.end(); it++)
        if ((*it) == e){
            if (e->message()) e->message()->Id = p->m_nProcessId++;
            return e;
        }
    return NULL;
}

bool ICQClient::cancelMessage(ICQMessage *m, bool bSendCancel)
{
    if (m == NULL) return false;
    list<ICQEvent*>::iterator it;
    for (it = p->msgQueue.begin(); it != p->msgQueue.end(); it++){
        if ((*it)->message() == m){
            ICQEvent *e = *it;
            p->msgQueue.remove(e);
            e->state = ICQEvent::Fail;
            m->bDelete = true;
            process_event(e);
            if (m->bDelete) delete m;
            delete e;
            return true;
        }
    }
    for (it = p->processQueue.begin(); it != p->processQueue.end(); it++){
        if ((*it)->message() == m){
            ICQEvent *e = *it;
            if (bSendCancel && (m->Type() == ICQ_MSGxFILE))
                p->cancelSendFile(static_cast<ICQFile*>(m));
            p->processQueue.remove(e);
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

void ICQClientPrivate::messageReceived(ICQMessage *msg)
{
    bool bAddUser;
    ICQUser *u = NULL;
    switch (msg->Type()){
    case ICQ_READxAWAYxMSG:
    case ICQ_READxOCCUPIEDxMSG:
    case ICQ_READxNAxMSG:
    case ICQ_READxDNDxMSG:
    case ICQ_READxFFCxMSG:{
            ICQEvent *e = new ICQEvent(EVENT_MESSAGE_RECEIVED, msg->getUin());
            msg->Id = m_nProcessId++;
            e->msg = msg;
            processQueue.push_back(e);
            client->process_event(e);
            return;
        }
    case ICQ_MSGxMSG:
        bAddUser = !client->RejectMessage;
        break;
    case ICQ_MSGxURL:
        bAddUser = !client->RejectURL;
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
        bAddUser = !client->RejectWeb;
        break;
    case ICQ_MSGxEMAILxPAGER:
        bAddUser = !client->RejectEmail;
        break;
    default:
        bAddUser = !client->RejectOther;
    }
    if (u == NULL) u = client->getUser(msg->getUin(), bAddUser, true);
    if ((u== NULL) || u->IgnoreId){
        delete msg;
        return;
    }
    if (!bAddUser && (u->GrpId == 0)){
        delete msg;
        return;
    }
    if (bAddUser && (u->GrpId == 0)){
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
        if (match(text.c_str(), client->RejectFilter.c_str())){
            delete msg;
            return;
        }
        u = client->getUser(msg->getUin(), true);
    }
    if (msg->Charset.length() == 0){
        msg->Charset = client->localCharset(u);
    }else if (msg->Type() == ICQ_MSGxMSG){
        ICQMsg *m = static_cast<ICQMsg*>(msg);
        const char *encoding = client->localCharset(u);
        string msg_text = m->Message;
        client->fromUTF(msg_text, encoding);
        client->toUTF(msg_text, encoding);
        if (msg_text == m->Message){
            client->fromUTF(m->Message, encoding);
            m->Charset = encoding;
        }
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
            client->process_event(e);
            return;
        }
    default:
        break;
    }
    ICQEvent e(EVENT_MESSAGE_RECEIVED, msg->getUin());
    e.msg = msg;
    msg->bDelete = true;
    client->process_event(&e);
    if (msg->bDelete) delete msg;
}

void ICQClientPrivate::processMsgQueue()
{
    if (m_state != Logged) return;
    processMsgQueueThruServer();
    processMsgQueueSMS();
    processMsgQueueAuth();
}

Contact::Contact()
{
    Uin = 0;
}

string ICQClientPrivate::makeMessageText(ICQMsg *msg, ICQUser *u)
{
    const char *encoding = client->localCharset(u);
    string msg_text;
    if (strcasecmp(msg->Charset.c_str(), "utf-8")){
        log(L_WARN, "No UTF encoding for send message");
        msg_text = msg->Message.c_str();
        client->toUTF(msg_text, msg->Charset.c_str());
    }else{
        msg_text = msg->Message.c_str();
    }
    if (u && u->canRTF() &&
            ((client->owner->InvisibleId && u->VisibleId) ||
             ((client->owner->InvisibleId == 0) && (u->InvisibleId == 0))))
        return createRTF(msg_text, msg->ForeColor, encoding);
    string message = client->clearHTML(msg_text);
    client->fromUTF(message, encoding);
    client->toServer(message, encoding);
    string m;
    for (const char *p = message.c_str(); *p; p++){
        if (*p == '\n') m += '\r';
        m += *p;
    }
    return m;
}

