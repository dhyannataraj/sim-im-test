/***************************************************************************
                          icqmessage.cpp  -  description
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

#include "icqmessage.h"
#include "icqclient.h"
#include "icq.h"
#include "toolbtn.h"
#include "core.h"
#include "searchresult.h"
#include "xml.h"

#include <qtimer.h>
#include <qlayout.h>
#include <qtextcodec.h>
#include <qfile.h>
#include <qimage.h>
#include <qregexp.h>

static DataDef icqMessageData[] =
    {
        { "ServerText", DATA_STRING, 1, 0 },
        { NULL, 0, 0, 0 }
    };

ICQMessage::ICQMessage(unsigned type, const char *cfg)
        : Message(type, cfg)
{
    load_data(icqMessageData, &data, cfg);
}

ICQMessage::~ICQMessage()
{
    free_data(icqMessageData, &data);
}

bool ICQMessage::setText(const char *r)
{
    return setServerText(r);
}

string ICQMessage::save()
{
    string s = Message::save();
    string s1 = save_data(icqMessageData, &data);
    if (!s1.empty()){
        if (!s.empty())
            s += '\n';
        s += s1;
    }
    return s;
}

QString ICQMessage::getText()
{
    const char *serverText = getServerText();
    if ((serverText == NULL) || (*serverText == 0))
        return Message::getText();
    return ICQClient::toUnicode(serverText, client(), contact());
}

static DataDef aimFileMessageData[] =
    {
        { "", DATA_ULONG, 1, 0 },				// Port
        { "", DATA_ULONG, 1, 0 },				// ID_L
        { "", DATA_ULONG, 1, 0 },				// ID_H
        { NULL, 0, 0, 0 }
    };

AIMFileMessage::AIMFileMessage(const char *cfg)
        : FileMessage(MessageFile, cfg)
{
    load_data(aimFileMessageData, &data, cfg);
}

AIMFileMessage::~AIMFileMessage()
{
    free_data(aimFileMessageData, &data);
}

static DataDef icqFileMessageData[] =
    {
        { "ServerText", DATA_STRING, 1, 0 },
        { "ServerDescr", DATA_STRING, 1, 0 },
        { "", DATA_ULONG, 1, 0 },				// IP
        { "", DATA_ULONG, 1, 0 },				// Port
        { "", DATA_ULONG, 1, 0 },				// ID_L
        { "", DATA_ULONG, 1, 0 },				// ID_H
        { "", DATA_ULONG, 1, 0 },				// Cookie
        { "", DATA_ULONG, 1, 0 },				// Extended
        { NULL, 0, 0, 0 }
    };

ICQFileMessage::ICQFileMessage(const char *cfg)
        : FileMessage(MessageICQFile, cfg)
{
    load_data(icqFileMessageData, &data, cfg);
}

ICQFileMessage::~ICQFileMessage()
{
    free_data(icqFileMessageData, &data);
}

QString ICQFileMessage::getDescription()
{
    const char *serverText = getServerDescr();
    if ((serverText == NULL) || (*serverText == 0))
        return FileMessage::getDescription();
    return ICQClient::toUnicode(serverText, client(), contact());
}

QString ICQFileMessage::getText()
{
    const char *serverText = getServerText();
    if ((serverText == NULL) || (*serverText == 0))
        return FileMessage::getText();
    return ICQClient::toUnicode(serverText, client(), contact());
}

string ICQFileMessage::save()
{
    string s = FileMessage::save();
    string s1 = save_data(icqFileMessageData, &data);
    if (!s1.empty()){
        if (!s.empty())
            s += '\n';
        s += s1;
    }
    return s;
}

static DataDef icqUrlMessageData[] =
    {
        { "ServerUrl", DATA_STRING, 1, 0 },
        { "ServerText", DATA_STRING, 1, 0 },
        { NULL, 0, 0, 0 }
    };

IcqUrlMessage::IcqUrlMessage(const char *cfg)
        : UrlMessage(MessageICQUrl, cfg)
{
    load_data(icqUrlMessageData, &data, cfg);
}

IcqUrlMessage::~IcqUrlMessage()
{
    free_data(icqUrlMessageData, &data);
}

string IcqUrlMessage::save()
{
    string s = Message::save();
    string s1 = save_data(icqUrlMessageData, &data);
    if (!s1.empty()){
        if (!s.empty())
            s += '\n';
        s += s1;
    }
    return s;
}

QString IcqUrlMessage::getText()
{
    const char *serverText = getServerText();
    if ((serverText == NULL) || (*serverText == 0))
        return Message::getText();
    return ICQClient::toUnicode(serverText, client(), contact());
}

QString IcqUrlMessage::getUrl()
{
    const char *serverText = data.ServerUrl;
    if (serverText && *serverText)
        return ICQClient::toUnicode(serverText, client(), contact());
    return UrlMessage::getUrl();
}

static DataDef icqContactsMessageData[] =
    {
        { "ServerText", DATA_STRING, 1, 0 },
        { NULL, 0, 0, 0 }
    };

IcqContactsMessage::IcqContactsMessage(const char *cfg)
        : ContactsMessage(MessageICQContacts, cfg)
{
    load_data(icqContactsMessageData, &data, cfg);
}

IcqContactsMessage::~IcqContactsMessage()
{
    free_data(icqContactsMessageData, &data);
}

QString IcqContactsMessage::getContacts()
{
    const char *serverText = getServerText();
    if ((serverText == NULL) || (*serverText == 0))
        return ContactsMessage::getContacts();
    return ICQClient::toUnicode(serverText, client(), contact());
}

string IcqContactsMessage::save()
{
    string s = Message::save();
    string s1 = save_data(icqContactsMessageData, &data);
    if (!s1.empty()){
        if (!s.empty())
            s += '\n';
        s += s1;
    }
    return s;
}

static DataDef icqAuthMessageData[] =
    {
        { "ServerText", DATA_STRING, 1, 0 },
        { "Charset", DATA_STRING, 1, 0 },
        { NULL, 0, 0, 0 }
    };

ICQAuthMessage::ICQAuthMessage(unsigned type, unsigned baseType, const char *cfg)
        : AuthMessage(type, cfg)
{
    load_data(icqAuthMessageData, &data, cfg);
    m_baseType = baseType;
}

ICQAuthMessage::~ICQAuthMessage()
{
    free_data(icqAuthMessageData, &data);
}

QString ICQAuthMessage::getText()
{
    const char *serverText = getServerText();
    if ((serverText == NULL) || (*serverText == 0))
        return Message::getText();
    const char *charset = getCharset();
    if (charset && *charset){
        QTextCodec *codec = QTextCodec::codecForName(charset);
        if (codec)
            return codec->toUnicode(serverText);
    }
    return ICQClient::toUnicode(serverText, client(), contact());
}

string ICQAuthMessage::save()
{
    string s = Message::save();
    string s1 = save_data(icqAuthMessageData, &data);
    if (!s1.empty()){
        if (!s.empty())
            s += '\n';
        s += s1;
    }
    return s;
}

static bool h2b(const char *&p, char &r)
{
    char c = *(p++);
    if ((c >= '0') && (c <= '9')){
        r = (char)(c - '0');
        return true;
    }
    if ((c >= 'A') && (c <= 'F')){
        r = (char)(c - 'A' + 10);
        return true;
    }
    return false;
}

static bool h2b(const char *&p, string &cap)
{
    char r1, r2;
    if (h2b(p, r1) && h2b(p, r2)){
        cap += (char)((r1 << 4) + r2);
        return true;
    }
    return false;
}

static bool parseFE(const char *str, vector<string> &l, unsigned n)
{
    const char *p = str;
    for (unsigned i = 0; i < n - 1; i++){
        for (; *str; str++)
            if (*str == '\xFE') break;
        if (*str == 0) return false;
        l.push_back(string(p, str - p));
        str++;
        p = str;
    }
    l.push_back(string(p));
    return true;
}

static Message *parseTextMessage(const char *str, const char *pp, const char *encoding)
{
    if (*str == 0)
        return NULL;
    log(L_DEBUG, "Text message: %s %s", str, pp);
    if (strlen(pp) == 38){
        string cap;
        if ((*(pp++) == '{') &&
                h2b(pp, cap) && h2b(pp, cap) && h2b(pp, cap) && h2b(pp, cap) &&
                (*(pp++) == '-') &&
                h2b(pp, cap) && h2b(pp, cap) &&
                (*(pp++) == '-') &&
                h2b(pp, cap) && h2b(pp, cap) &&
                (*(pp++) == '-') &&
                h2b(pp, cap) && h2b(pp, cap) &&
                (*(pp++) == '-') &&
                h2b(pp, cap) && h2b(pp, cap) && h2b(pp, cap) && h2b(pp, cap) &&
                h2b(pp, cap) && h2b(pp, cap) &&
                (*(pp++) == '}')){
            const char *unpack_cap = cap.c_str();
            if (!memcmp(unpack_cap, ICQClient::capabilities[CAP_RTF], sizeof(capability))){
                Message *msg = new Message(MessageGeneric);
                QString text;
                if (ICQClient::parseRTF(str, encoding, text))
                    msg->setFlags(MESSAGE_RICHTEXT);
                log(L_DEBUG, "Msg: %s", str);
                msg->setText(text);
                return msg;
            }
            if (!memcmp(unpack_cap, ICQClient::capabilities[CAP_UTF], sizeof(capability))){
                Message *msg = new Message(MessageGeneric);
                msg->setText(QString::fromUtf8(str));
                return msg;
            }
        }
    }
    ICQMessage *m = new ICQMessage;
    m->setServerText(str);
    return m;
}

static Message *parseURLMessage(const char *str)
{
    vector<string> l;
    if (!parseFE(str, l, 2)){
        log(L_WARN, "Parse error URL message");
        return NULL;
    }
    IcqUrlMessage *m = new IcqUrlMessage;
    m->setServerText(l[0].c_str());
    m->setServerUrl(l[1].c_str());
    return m;
}

static Message *parseContactMessage(const char *str)
{
    vector<string> l;
    if (!parseFE(str, l, 2)){
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
    string serverText;
    for (unsigned i = 0; i < nContacts; i++){
        string screen = c[i*2];
        string alias  = c[i*2+1];
        if (!serverText.empty())
            serverText += ";";
        if (atol(screen.c_str())){
            serverText += "icq:";
            serverText += screen;
            serverText += "/";
            serverText += alias;
            serverText += ",";
            if (screen == alias){
                serverText += "ICQ ";
                serverText += screen;
            }else{
                serverText += alias;
                serverText += " (ICQ ";
                serverText += screen;
                serverText += ")";
            }
        }else{
            serverText += "aim:";
            serverText += screen;
            serverText += "/";
            serverText += alias;
            serverText += ",";
            if (screen == alias){
                serverText += "AIM ";
                serverText += screen;
            }else{
                serverText += alias;
                serverText += " (AIM ";
                serverText += screen;
                serverText += ")";
            }
        }
    }
    IcqContactsMessage *m = new IcqContactsMessage;
    m->setServerText(serverText.c_str());
    return m;
}

static Message *parseAuthRequest(const char *str)
{
    vector<string> l;
    if (!parseFE(str, l, 6)){
        log(L_WARN, "Parse error auth request message");
        return NULL;
    }
    ICQAuthMessage *m = new ICQAuthMessage(MessageICQAuthRequest, MessageAuthRequest);
    m->setServerText(l[4].c_str());
    return m;
}

Message *ICQClient::parseExtendedMessage(const char *screen, Buffer &packet, MessageId &id, unsigned cookie)
{
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
    int n = msgType.find("URL");
    if (n >= 0){
        string info;
        b.unpackStr32(info);
        return parseURLMessage(info.c_str());
    }
    if (msgType == "Request For Contacts"){
        string info;
        b.unpackStr32(info);
        ICQAuthMessage *m = new ICQAuthMessage(MessageContactRequest, MessageContactRequest);
        m->setServerText(info.c_str());
        return m;
    }
    if (msgType == "Contacts"){
        string p;
        b.unpackStr32(p);
        return parseContactMessage(p.c_str());
    }
    if (msgType == "Message"){
        string p;
        b.unpackStr32(p);
        unsigned long forecolor, backcolor;
        b >> forecolor >> backcolor;
        string cap_str;
        b.unpackStr32(cap_str);
        Contact *contact;
        ICQUserData *data = findContact(screen, NULL, true, contact);
        if (data == NULL)
            return NULL;
        Message *msg = parseTextMessage(p.c_str(), cap_str.c_str(), data->Encoding ? data->Encoding : this->data.owner.Encoding);
        if (msg){
            if (forecolor != backcolor){
                msg->setForeground(forecolor >> 8);
                msg->setBackground(backcolor >> 8);
            }
        }
        return msg;
    }
    n = msgType.find("File");
    if (n >= 0){
        string fileDescr;
        b.unpackStr32(fileDescr);
        unsigned short port;
        b >> port;
        b.incReadPos(2);
        string fileName;
        b >> fileName;
        unsigned long fileSize;
        b.unpack(fileSize);
        ICQFileMessage *m = new ICQFileMessage;
        m->setServerDescr(fileName.c_str());
        m->setServerText(fileDescr.c_str());
        m->setSize(fileSize);
        m->setPort(port);
        m->setFlags(MESSAGE_TEMP);
        m->setID_L(id.id_l);
        m->setID_H(id.id_h);
        m->setCookie(cookie);
        m->setExtended(true);
        return m;
    }
    if (msgType == "ICQSMS"){
        string p;
        b.unpackStr32(p);
        string::iterator s = p.begin();
        auto_ptr<XmlNode> top(XmlNode::parse(s, p.end()));
        if (top.get() == NULL){
            log(L_WARN, "Parse SMS XML error");
            return NULL;
        }
        if (msg_type == 0){
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
            SMSMessage *m = new SMSMessage;
            XmlLeaf *sender = sms_message->getLeaf("sender");
            if (sender != NULL){
                m->setPhone(QString::fromUtf8(sender->getValue().c_str()));
                Contact *contact = getContacts()->contactByPhone(sender->getValue().c_str());
                m->setContact(contact->id());
            }
            XmlLeaf *senders_network = sms_message->getLeaf("senders_network");
            if (senders_network != NULL)
                m->setNetwork(QString::fromUtf8(senders_network->getValue().c_str()));
            m->setText(QString::fromUtf8(text->getValue().c_str()));
            return m;
        }
    }
    if (msgType == "StatusMsgExt"){
        StatusMessage *m = new StatusMessage;
        return m;
    }
    log(L_WARN, "Unknown extended message type %s", msgType.c_str());
    return NULL;
}

Message *ICQClient::parseMessage(unsigned short type, const char *screen, string &p, Buffer &packet, MessageId &id, unsigned cookie)
{
    if (atol(screen) == 0x0A){
        vector<string> l;
        if (!parseFE(p.c_str(), l, 6)){
            log(L_WARN, "Parse error web panel message");
            return NULL;
        }
        char SENDER_IP[] = "Sender IP:";
        string head = l[5].substr(0, strlen(SENDER_IP));
        ICQMessage *msg = new ICQMessage((head == SENDER_IP) ? MessageWebPanel : MessageEmailPager);
        QString name = toUnicode(l[0].c_str(), NULL);
        QString mail = toUnicode(l[3].c_str(), NULL);
        msg->setServerText(l[5].c_str());
        Contact *contact = getContacts()->contactByMail(mail, name);
        if (contact == NULL){
            delete msg;
            return NULL;
        }
        msg->setContact(contact->id());
        return msg;
    }
    log(L_DEBUG, "Parse message [type=%u]", type);
    Message *msg = NULL;
    switch (type){
    case ICQ_MSGxMSG:{
            unsigned long forecolor, backcolor;
            packet >> forecolor >> backcolor;
            string cap_str;
            packet.unpackStr32(cap_str);
            Contact *contact;
            ICQUserData *data = findContact(screen, NULL, true, contact);
            if (data == NULL)
                return NULL;
            msg = parseTextMessage(p.c_str(), cap_str.c_str(), data->Encoding ? data->Encoding : this->data.owner.Encoding);
            if (msg == NULL)
                break;
            if (forecolor != backcolor){
                msg->setForeground(forecolor >> 8);
                msg->setBackground(backcolor >> 8);
            }
            break;
        }
    case ICQ_MSGxURL:
        msg = parseURLMessage(p.c_str());
        break;
    case ICQ_MSGxAUTHxREQUEST:
        msg = parseAuthRequest(p.c_str());
        break;
    case ICQ_MSGxAUTHxGRANTED:
        msg = new AuthMessage(MessageAuthGranted);
        break;
    case ICQ_MSGxAUTHxREFUSED:
        msg = new AuthMessage(MessageAuthRefused);
        break;
    case ICQ_MSGxADDEDxTOxLIST:
        msg = new AuthMessage(MessageAdded);
        break;
    case ICQ_MSGxCONTACTxLIST:
        msg = parseContactMessage(p.c_str());
        break;
    case ICQ_MSGxFILE:{
            ICQFileMessage *m = new ICQFileMessage;
            m->setServerText(p.c_str());
            unsigned short port;
            unsigned long  fileSize;
            string fileName;
            packet >> port;
            packet.incReadPos(2);
            packet >> fileName;
            packet.unpack(fileSize);
            m->setPort(port);
            m->setSize(fileSize);
            m->setServerDescr(fileName.c_str());
            msg = m;
            break;
        }
    case ICQ_MSGxEXT:
        msg = parseExtendedMessage(screen, packet, id, cookie);
        break;
    default:
        log(L_WARN, "Unknown message type %04X", type);
    }
    return msg;
}

static Message *createIcq(const char *cfg)
{
    return new ICQMessage(MessageICQ, cfg);
}

static MessageDef defIcq =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT,
        NULL,
        NULL,
        createIcq,
        NULL,
        NULL
    };

static Message *createIcqFile(const char *cfg)
{
    return new ICQFileMessage(cfg);
}

static MessageDef defIcqFile =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT,
        NULL,
        NULL,
        createIcqFile,
        NULL,
        NULL
    };

#if 0
i18n("WWW-panel message", "%n WWW-panel messages", 1);
#endif

static Message *createWebPanel(const char *cfg)
{
    return new ICQMessage(MessageWebPanel, cfg);
}

static MessageDef defWebPanel =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT,
        "WWW-panel message",
        "%n WWW-panel messages",
        createWebPanel,
        NULL,
        NULL
    };

#if 0
i18n("Email pager message", "%n Email pager messages", 1);
#endif

static Message *createEmailPager(const char *cfg)
{
    return new ICQMessage(MessageEmailPager, cfg);
}

static MessageDef defEmailPager =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT,
        "Email pager message",
        "%n Email pager messages",
        createEmailPager,
        NULL,
        NULL
    };

#if 0
i18n("Request secure channel", "%n requests secure channel", 1);
#endif

static Message *createOpenSecure(const char *cfg)
{
    return new Message(MessageOpenSecure, cfg);
}

static MessageDef defOpenSecure =
    {
        NULL,
        NULL,
        MESSAGE_SENDONLY,
        "Request secure channel",
        "%n requests secure channel",
        createOpenSecure,
        NULL,
        NULL
    };

#if 0
i18n("Close secure channel", "%n times close secure channel", 1);
#endif

static Message *createCloseSecure(const char *cfg)
{
    return new Message(MessageCloseSecure, cfg);
}

static MessageDef defCloseSecure =
    {
        NULL,
        NULL,
        MESSAGE_SILENT | MESSAGE_SENDONLY,
        "Close secure channel",
        "%n times close secure channel",
        createCloseSecure,
        NULL,
        NULL
    };

#if 0
i18n("Check invisible", "%n times check invisible", 1);
#endif

static Message *createCheckInvisible(const char *cfg)
{
    return new Message(MessageCheckInvisible, cfg);
}

static MessageDef defCheckInvisible =
    {
        NULL,
        NULL,
        MESSAGE_SILENT | MESSAGE_SENDONLY,
        "Check invisible",
        "%n times checkInvisible",
        createCheckInvisible,
        NULL,
        NULL
    };

#if 0
i18n("Warning", "%n warnings", 1);
#endif

static DataDef warningMessageData[] =
    {
        { "Anonymous", DATA_BOOL, 1, 0 },
        { "OldLevel", DATA_ULONG, 1, 0 },
        { "NewLevel", DATA_ULONG, 1, 0 },
        { NULL, 0, 0, 0 }
    };

WarningMessage::WarningMessage(const char *cfg)
        : AuthMessage(MessageWarning, cfg)
{
    load_data(warningMessageData, &data, cfg);
}

string WarningMessage::save()
{
    string res = AuthMessage::save();
    if (!res.empty())
        res += "\n";
    return res + save_data(warningMessageData, &data);
}

QString WarningMessage::presentation()
{
    return QString("Increase warning level from %1% to %2%")
           .arg(ICQClient::warnLevel(getOldLevel()))
           .arg(ICQClient::warnLevel(getNewLevel()));
}

static Message *createWarning(const char *cfg)
{
    return new WarningMessage(cfg);
}

static MessageDef defWarning =
    {
        NULL,
        NULL,
        MESSAGE_SENDONLY,
        "Warning",
        "%n warnings",
        createWarning,
        NULL,
        NULL
    };

static Message *createIcqAuthRequest(const char *cfg)
{
    return new ICQAuthMessage(MessageICQAuthRequest, MessageAuthRequest, cfg);
}

static MessageDef defIcqAuthRequest =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT,
        NULL,
        NULL,
        createIcqAuthRequest,
        NULL,
        NULL
    };

static Message *createIcqAuthGranted(const char *cfg)
{
    return new ICQAuthMessage(MessageICQAuthGranted, MessageAuthGranted, cfg);
}

static MessageDef defIcqAuthGranted =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT,
        NULL,
        NULL,
        createIcqAuthGranted,
        NULL,
        NULL
    };

static Message *createIcqAuthRefused(const char *cfg)
{
    return new ICQAuthMessage(MessageICQAuthRefused, MessageAuthRefused, cfg);
}

static MessageDef defIcqAuthRefused =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT,
        NULL,
        NULL,
        createIcqAuthRefused,
        NULL,
        NULL
    };

static Message *createContactRequest(const char *cfg)
{
    return new ICQAuthMessage(MessageContactRequest, MessageContactRequest, cfg);
}

#if 0
i18n("Contact request", "%n contact requests", 1);
#endif

static MessageDef defContactRequest =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT | MESSAGE_SYSTEM,
        "Contact request",
        "%n contact requests",
        createContactRequest,
        NULL,
        NULL
    };

static Message *createIcqUrl(const char *cfg)
{
    return new IcqUrlMessage(cfg);
}

static MessageDef defIcqUrl =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT,
        NULL,
        NULL,
        createIcqUrl,
        NULL,
        NULL
    };

static Message *createIcqContacts(const char *cfg)
{
    return new IcqContactsMessage(cfg);
}

static MessageDef defIcqContacts =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT,
        NULL,
        NULL,
        createIcqContacts,
        NULL,
        NULL
    };

void ICQPlugin::registerMessages()
{
    Command cmd;

    cmd->id			 = MessageICQUrl;
    cmd->text		 = "ICQUrl";
    cmd->icon		 = "url";
    cmd->param		 = &defIcqUrl;
    Event eMsg(EventCreateMessageType, cmd);
    eMsg.process();

    cmd->id			 = MessageICQContacts;
    cmd->text		 = "ICQContacts";
    cmd->icon		 = "contacts";
    cmd->param		 = &defIcqContacts;
    eMsg.process();

    cmd->id			= MessageICQ;
    cmd->text		= "ICQMessage";
    cmd->icon		= "message";
    cmd->param		= &defIcq;
    eMsg.process();

    cmd->id			= MessageICQFile;
    cmd->text		= "ICQFile";
    cmd->icon		= "file";
    cmd->param		= &defIcqFile;
    eMsg.process();

    cmd->id			= MessageContactRequest;
    cmd->text		= I18N_NOOP("Contact Request");
    cmd->icon		= "contacts";
    cmd->param		= &defContactRequest;
    eMsg.process();

    cmd->id         = MessageICQAuthRequest;
    cmd->text       = "ICQAuthRequest";
    cmd->icon       = "auth";
    cmd->param      = &defIcqAuthRequest;
    eMsg.process();


    cmd->id			= MessageICQAuthGranted;
    cmd->text		= "ICQAuthGranted";
    cmd->icon		= "auth";
    cmd->param		= &defIcqAuthGranted;
    eMsg.process();

    cmd->id			= MessageICQAuthRefused;
    cmd->text		= "ICQAuthRefused";
    cmd->icon		= "auth";
    cmd->param		= &defIcqAuthRefused;
    eMsg.process();

    cmd->id			= MessageWebPanel;
    cmd->text		= I18N_NOOP("Web panel");
    cmd->icon		= "web";
    cmd->param		= &defWebPanel;
    eMsg.process();

    cmd->id			= MessageEmailPager;
    cmd->text		= I18N_NOOP("Email pager");
    cmd->icon		= "mailpager";
    cmd->param		= &defEmailPager;
    eMsg.process();

    cmd->id			= MessageOpenSecure;
    cmd->text		= I18N_NOOP("Request secure channel");
    cmd->icon		= "encrypted";
    cmd->menu_grp	= 0x30F0;
    cmd->param		= &defOpenSecure;
    eMsg.process();

    cmd->id			= MessageCloseSecure;
    cmd->text		= I18N_NOOP("Close secure channel");
    cmd->icon		= "encrypted";
    cmd->menu_grp	= 0x30F0;
    cmd->param		= &defCloseSecure;
    eMsg.process();

    cmd->id			= MessageCheckInvisible;
    cmd->text		= I18N_NOOP("Check invisible");
    cmd->icon		= "ICQ_invisible";
    cmd->menu_grp	= 0x30F1;
    cmd->param		= &defCheckInvisible;
    eMsg.process();

    cmd->id			= MessageWarning;
    cmd->text		= I18N_NOOP("Warning");
    cmd->icon		= "error";
    cmd->menu_grp	= 0x30F2;
    cmd->param		= &defWarning;
    eMsg.process();

    cmd->id			= CmdUrlInput;
    cmd->text		= I18N_NOOP("&URL");
    cmd->icon		= "empty";
    cmd->icon_on	= NULL;
    cmd->bar_id		= ToolBarMsgEdit;
    cmd->bar_grp	= 0x1030;
    cmd->menu_id	= 0;
    cmd->menu_grp	= 0;
    cmd->flags		= BTN_EDIT | BTN_NO_BUTTON | COMMAND_CHECK_STATE;

    Event eCmd(EventCommandCreate, cmd);
    eCmd.process();

}

void ICQPlugin::unregisterMessages()
{
    Event eUrl(EventRemoveMessageType, (void*)MessageICQUrl);
    eUrl.process();

    Event eContact(EventRemoveMessageType, (void*)MessageICQContacts);
    eContact.process();

    Event eIcq(EventRemoveMessageType, (void*)MessageICQ);
    eIcq.process();

    Event eIcqFile(EventRemoveMessageType, (void*)MessageICQFile);
    eIcqFile.process();

    Event eAuthRequest(EventRemoveMessageType, (void*)MessageICQAuthRequest);
    eAuthRequest.process();

    Event eAuthGranted(EventRemoveMessageType, (void*)MessageICQAuthGranted);
    eAuthGranted.process();

    Event eAuthRefused(EventRemoveMessageType, (void*)MessageICQAuthRefused);
    eAuthRefused.process();

    Event eRequest(EventRemoveMessageType, (void*)MessageContactRequest);
    eRequest.process();

    Event eWeb(EventRemoveMessageType, (void*)MessageWebPanel);
    eWeb.process();

    Event ePager(EventRemoveMessageType, (void*)MessageEmailPager);
    ePager.process();

    Event eOpenSecure(EventRemoveMessageType, (void*)MessageOpenSecure);
    eOpenSecure.process();

    Event eCloseSecure(EventRemoveMessageType, (void*)MessageCloseSecure);
    eCloseSecure.process();

    Event eWarning(EventRemoveMessageType, (void*)MessageWarning);
    eWarning.process();

    Event eCheckInvisible(EventRemoveMessageType, (void*)MessageCheckInvisible);
    eCheckInvisible.process();

    Event eURL(EventCommandRemove, (void*)CmdUrlInput);
    eURL.process();
}

void ICQClient::packExtendedMessage(Message *msg, Buffer &buf, Buffer &msgBuf, ICQUserData *data)
{
    unsigned short port = 0;
    switch (msg->type()){
    case MessageICQFile:
        port = static_cast<ICQFileMessage*>(msg)->getPort();
    case MessageFile:
        buf.pack((char*)plugins[PLUGIN_FILE], sizeof(plugin));
        buf.packStr32("File");
        buf << 0x00000100L << 0x00010000L << 0x00000000L << (unsigned short)0 << (char)0;
        msgBuf.packStr32(fromUnicode(msg->getPlainText(), data).c_str());
        msgBuf << port << (unsigned short)0;
        msgBuf << fromUnicode(static_cast<FileMessage*>(msg)->getDescription(), data);
        msgBuf.pack((unsigned long)(static_cast<FileMessage*>(msg)->getSize()));
        msgBuf << 0x00000000L;
        break;
    }
}

bool my_string::operator < (const my_string &a) const
{
    return strcmp(c_str(), a.c_str()) < 0;
}

QString ICQClient::packContacts(ContactsMessage *msg, ICQUserData *data, CONTACTS_MAP &c)
{
    QString contacts = msg->getContacts();
    QString newContacts;
    while (!contacts.isEmpty()){
        QString contact = getToken(contacts, ';');
        QString url = getToken(contact, ',');
        QString proto = getToken(url, ':');
        if (proto == "sim"){
            Contact *contact = getContacts()->contact(atol(url.latin1()));
            if (contact){
                ClientDataIterator it(contact->clientData);
                clientData *cdata;
                while ((cdata = ++it) != NULL){
                    Contact *cc = contact;
                    if (!isMyData(cdata, cc))
                        continue;
                    ICQUserData *d = (ICQUserData*)cdata;
                    string screen = this->screen(d);
                    CONTACTS_MAP::iterator it = c.find(screen.c_str());
                    if (it == c.end()){
                        alias_group ci;
                        ci.alias = fromUnicode(contact->getName(), data);
                        ci.grp   = cc ? cc->getGroup() : 0;
                        c.insert(CONTACTS_MAP::value_type(screen.c_str(), ci));
                        if (!newContacts.isEmpty())
                            newContacts += ";";
                        if (atol(screen.c_str())){
                            newContacts += "icq:";
                            newContacts += screen.c_str();
                            newContacts += "/";
                            newContacts += contact->getName();
                            newContacts += ",";
                            if (contact->getName() == screen.c_str()){
                                newContacts += "ICQ ";
                                newContacts += screen.c_str();
                            }else{
                                newContacts += contact->getName();
                                newContacts += " (ICQ ";
                                newContacts += screen.c_str();
                                newContacts += ")";
                            }
                        }else{
                            newContacts += "aim:";
                            newContacts += screen.c_str();
                            newContacts += "/";
                            newContacts += contact->getName();
                            newContacts += ",";
                            if (contact->getName() == screen.c_str()){
                                newContacts += "AIM ";
                                newContacts += screen.c_str();
                            }else{
                                newContacts += contact->getName();
                                newContacts += " (AIM ";
                                newContacts += screen.c_str();
                                newContacts += ")";
                            }
                        }
                    }
                }
            }
        }
        if ((proto == "icq") || (proto == "aim")){
            QString screen = getToken(url, '/');
            if (url.isEmpty())
                url = screen;
            CONTACTS_MAP::iterator it = c.find(screen.latin1());
            if (it == c.end()){
                alias_group ci;
                ci.alias = fromUnicode(url, data);
                ci.grp   = 0;
                c.insert(CONTACTS_MAP::value_type(screen.latin1(), ci));
            }
        }
    }
    return newContacts;
}

void ICQClient::packMessage(Buffer &b, Message *msg, ICQUserData *data, unsigned short &type, unsigned short flags)
{
    Buffer msgBuf;
    Buffer buf;
    string res;
    switch (msg->type()){
    case MessageUrl:
        res = fromUnicode(msg->getPlainText(), data);
        res += '\xFE';
        res += fromUnicode(static_cast<UrlMessage*>(msg)->getUrl(), data);
        type = ICQ_MSGxURL;
        break;
    case MessageContacts:{
            CONTACTS_MAP c;
            QString nc = packContacts(static_cast<ContactsMessage*>(msg), data, c);
            if (c.empty()){
                msg->setError(I18N_NOOP("No contacts for send"));
                return;
            }
            static_cast<ContactsMessage*>(msg)->setContacts(nc);
            res = number(c.size());
            for (CONTACTS_MAP::iterator it = c.begin(); it != c.end(); ++it){
                res += '\xFE';
                res += (*it).first.c_str();
                res += '\xFE';
                res += (*it).second.alias.c_str();
            }
            res += '\xFE';
            type = ICQ_MSGxCONTACTxLIST;
            break;
        }
    case MessageICQFile:
        if (!static_cast<ICQFileMessage*>(msg)->getExtended()){
            res = fromUnicode(msg->getPlainText(), data);
            type = ICQ_MSGxFILE;
            break;
        }
    case MessageFile:	// FALLTHROW
        type = ICQ_MSGxEXT;
        packExtendedMessage(msg, buf, msgBuf, data);
        break;
    case MessageOpenSecure:
        type = ICQ_MSGxSECURExOPEN;
        break;
    case MessageCloseSecure:
        type = ICQ_MSGxSECURExCLOSE;
        break;
    }
    if (flags == ICQ_TCPxMSG_NORMAL){
        if (msg->getFlags() & MESSAGE_URGENT)
            flags = ICQ_TCPxMSG_URGENT;
        if (msg->getFlags() & MESSAGE_LIST)
            flags = ICQ_TCPxMSG_LIST;
    }
    if (type == ICQ_MSGxEXT){
        b.pack(type);
        b.pack(msgStatus());
        b.pack(flags);
    }else{
        b.pack(this->data.owner.Uin);
        b << (char)type << (char)0;
    }
    b << res;
    if (buf.size()){
        b.pack((unsigned short)buf.size());
        b.pack(buf.data(0), buf.size());
        b.pack32(msgBuf);
    }
}

void ICQClient::parsePluginPacket(Buffer &b, unsigned plugin_type, ICQUserData *data, unsigned uin, bool bDirect)
{
    b.incReadPos(1);
    unsigned short type;
    b >> type;
    b.incReadPos(bDirect ? 1 : 4);
    vector<string> phonebook;
    vector<string> numbers;
    vector<string> phonedescr;
    string phones;
    Contact *contact = NULL;
    unsigned long state, time, size, nEntries;
    unsigned i;
    unsigned nActive;
    switch (type){
    case 0:
    case 1:
        b.unpack(time);
        b.unpack(size);
        b.incReadPos(4);
        b.unpack(nEntries);
        if (data)
            log(L_DEBUG, "Plugin info reply %u %u (%u %u) %u %u (%u)", data->Uin, time, data->PluginInfoTime, data->PluginStatusTime, size, nEntries, plugin_type);
        switch (plugin_type){
        case PLUGIN_RANDOMxCHAT:{
                b.incReadPos(-12);
                string name;
                b.unpack(name);
                string topic;
                b.unpack(topic);
                unsigned short age;
                char gender;
                unsigned short country;
                unsigned short language;
                b.unpack(age);
                b.unpack(gender);
                b.unpack(country);
                b.unpack(language);
                string homepage;
                b.unpack(homepage);
                ICQUserData data;
                load_data(static_cast<ICQProtocol*>(protocol())->icqUserData, &data, NULL);
                data.Uin = uin;
                set_str(&data.Alias, name.c_str());
                set_str(&data.About, topic.c_str());
                data.Age = age;
                data.Gender = gender;
                data.Country = country;
                data.Language = language;
                set_str(&data.Homepage, homepage.c_str());
                Event e(EventRandomChatInfo, &data);
                e.process();
                free_data(static_cast<ICQProtocol*>(protocol())->icqUserData, &data);
                break;
            }
        case PLUGIN_QUERYxSTATUS:
            if (data == NULL)
                break;
            if (!bDirect){
                b.incReadPos(5);
                b.unpack(nEntries);
            }
            log(L_DEBUG, "Status info answer %u", nEntries);
        case PLUGIN_QUERYxINFO:
            if (data == NULL)
                break;
            if (nEntries > 0x80){
                log(L_DEBUG, "Bad entries value %X", nEntries);
                break;
            }
            for (i = 0; i < nEntries; i++){
                plugin p;
                b.unpack((char*)p, sizeof(p));
                b.incReadPos(4);
                string name, descr;
                b.unpackStr32(name);
                b.unpackStr32(descr);
                b.incReadPos(4);
                unsigned plugin_index;
                for (plugin_index = 0; plugin_index < PLUGIN_NULL; plugin_index++)
                    if (memcmp(p, plugins[plugin_index], sizeof(p)) == 0)
                        break;
                if (plugin_index >= PLUGIN_NULL){
                    log(L_DEBUG, "Unknown plugin sign %s %s", name.c_str(), descr.c_str());
                    continue;
                }
                log(L_DEBUG, "Plugin %u %s %s", plugin_index, name.c_str(), descr.c_str());
                switch (plugin_index){
                case PLUGIN_PHONEBOOK:
                case PLUGIN_FOLLOWME:
                    if (plugin_type == PLUGIN_QUERYxINFO){
                        addPluginInfoRequest(uin, PLUGIN_PHONEBOOK);
                    }else{
                        addPluginInfoRequest(uin, PLUGIN_FOLLOWME);
                    }
                    break;
                case PLUGIN_PICTURE:
                    if (plugin_type == PLUGIN_QUERYxINFO)
                        addPluginInfoRequest(uin, plugin_index);
                    break;
                case PLUGIN_FILESERVER:
                case PLUGIN_ICQPHONE:
                    if (plugin_type == PLUGIN_QUERYxSTATUS)
                        addPluginInfoRequest(uin, plugin_index);
                    break;
                }
            }
            if (plugin_type == PLUGIN_QUERYxINFO){
                data->PluginInfoFetchTime = data->PluginInfoTime;
            }else{
                data->PluginStatusFetchTime = data->PluginStatusTime;
            }
            break;
        case PLUGIN_PICTURE:
            if (data){
                b.incReadPos(-4);
                string pict;
                b.unpackStr32(pict);
                b.unpackStr32(pict);
                QImage img;
                QString fName = pictureFile(data);
                QFile f(fName);
                if (f.open(IO_WriteOnly | IO_Truncate)){
                    f.writeBlock(pict.c_str(), pict.size());
                    f.close();
                    img.load(fName);
                }else{
                    log(L_ERROR, "Can't create %s", (const char*)fName.local8Bit());
                }
                data->PictureWidth  = img.width();
                data->PictureHeight = img.height();
            }
            break;
        case PLUGIN_PHONEBOOK:
            if (data){
                nActive = (unsigned)(-1);
                if (nEntries > 0x80){
                    log(L_DEBUG, "Bad entries value %X", nEntries);
                    break;
                }
                for (i = 0; i < nEntries; i++){
                    string descr, area, phone, ext, country;
                    unsigned long active;
                    b.unpackStr32(descr);
                    b.unpackStr32(area);
                    b.unpackStr32(phone);
                    b.unpackStr32(ext);
                    b.unpackStr32(country);
                    numbers.push_back(phone);
                    string value;
                    for (const ext_info *e = getCountries(); e->szName; e++){
                        if (country == e->szName){
                            value = "+";
                            value += number(e->nCode);
                            break;
                        }
                    }
                    if (!area.empty()){
                        if (!value.empty())
                            value += " ";
                        value += "(";
                        value += area;
                        value += ")";
                    }
                    if (!value.empty())
                        value += " ";
                    value += phone;
                    if (!ext.empty()){
                        value += " - ";
                        value += ext;
                    }
                    b.unpack(active);
                    if (active)
                        nActive = i;
                    phonebook.push_back(value);
                    phonedescr.push_back(descr);
                }
                for (i = 0; i < nEntries; i++){
                    unsigned long type;
                    string phone = phonebook[i];
                    string gateway;
                    b.incReadPos(4);
                    b.unpack(type);
                    b.unpackStr32(gateway);
                    b.incReadPos(16);
                    switch (type){
                    case 1:
                    case 2:
                        type = CELLULAR;
                        break;
                    case 3:
                        type = FAX;
                        break;
                    case 4:{
                            type = PAGER;
                            phone = numbers[i];
                            const pager_provider *p;
                            for (p = getProviders(); *p->szName; p++){
                                if (gateway == p->szName){
                                    phone += "@";
                                    phone += p->szGate;
                                    phone += "[";
                                    phone += p->szName;
                                    phone += "]";
                                    break;
                                }
                            }
                            if (*p->szName == 0){
                                phone += "@";
                                phone += gateway;
                            }
                            break;
                        }
                    default:
                        type = PHONE;
                    }
                    phone += ",";
                    phone += phonedescr[i];
                    phone += ",";
                    phone += number(type);
                    if (i == nActive)
                        phone += ",1";
                    if (!phones.empty())
                        phones += ";";
                    phones += phone;
                }
                set_str(&data->PhoneBook, phones.c_str());
                Contact *contact = NULL;
                findContact(number(data->Uin).c_str(), NULL, false, contact);
                if (contact){
                    setupContact(contact, data);
                    Event e(EventContactChanged, contact);
                    e.process();
                }
            }
        }
        break;
    case 2:
        if (data){
            if (bDirect)
                b.incReadPos(3);
            b.unpack(state);
            b.unpack(time);
            log(L_DEBUG, "Plugin status reply %u %u %u (%u)", uin, state, time, plugin_type);
            findContact(number(uin).c_str(), NULL, false, contact);
            if (contact == NULL)
                break;
            switch (plugin_type){
            case PLUGIN_FILESERVER:
                if ((state != 0) != (data->SharedFiles != 0)){
                    data->SharedFiles = state;
                    Event e(EventContactChanged, contact);
                    e.process();
                }
                break;
            case PLUGIN_FOLLOWME:
                if (state != data->FollowMe){
                    data->FollowMe = state;
                    Event e(EventContactChanged, contact);
                    e.process();
                }
                break;
            case PLUGIN_ICQPHONE:
                if (state != data->ICQPhone){
                    data->ICQPhone = state;
                    Event e(EventContactChanged, contact);
                    e.process();
                }
                break;
            }
        }
        break;
    default:
        log(L_DEBUG, "Unknown plugin type answer %u %u (%u)", uin, type, plugin_type);
    }
}

static const char* plugin_name[] =
    {
        "Phone Book",				// PLUGIN_PHONEBOOK
        "Picture",					// PLUGIN_PICTURE
        "Shared Files Directory",	// PLUGIN_FILESERVER
        "Phone \"Follow Me\"",		// PLUGIN_FOLLOWME
        "ICQphone Status"			// PLUGIN_ICQPHONE
    };

static const char* plugin_descr[] =
    {
        "Phone Book / Phone \"Follow Me\"",		// PLUGIN_PHONEBOOK
        "Picture",								// PLUGIN_PICTURE
        "Shared Files Directory",				// PLUGIN_FILESERVER
        "Phone Book / Phone \"Follow Me\"",		// PLUGIN_FOLLOWME
        "ICQphone Status"						// PLUGIN_ICQPHONE
    };

void ICQClient::pluginAnswer(unsigned plugin_type, unsigned long uin, Buffer &info)
{
    Contact *contact;
    ICQUserData *data = findContact(number(uin).c_str(), NULL, false, contact);
    log(L_DEBUG, "Request about %u", plugin_type);
    Buffer answer;
    unsigned long typeAnswer = 0;
    unsigned long nEntries = 0;
    unsigned long time = 0;
    switch (plugin_type){
    case PLUGIN_PHONEBOOK:{
            if (data && data->GrpId && !contact->getIgnore()){
                Buffer answer1;
                time = this->data.owner.PluginInfoTime;
                QString phones = getContacts()->owner()->getPhones();
                while (!phones.isEmpty()){
                    QString item = getToken(phones, ';', false);
                    unsigned long publish = 0;
                    QString phoneItem = getToken(item, '/', false);
                    if (item != "-")
                        publish = 1;
                    QString number = getToken(phoneItem, ',');
                    QString descr = getToken(phoneItem, ',');
                    unsigned long type = getToken(phoneItem, ',').toUInt();
                    unsigned long active = 0;
                    if (!phoneItem.isEmpty())
                        active = 1;
                    QString area;
                    QString phone;
                    QString ext;
                    QString country;
                    QString gateway;
                    if (type == PAGER){
                        phone = getToken(number, '@');
                        int n = number.find('[');
                        if (n >= 0){
                            getToken(number, '[');
                            gateway = getToken(number, ']');
                        }else{
                            gateway = number;
                        }
                    }else{
                        int n = number.find('(');
                        if (n >= 0){
                            country = getToken(number, '(');
                            area    = getToken(number, ')');
                            if (country[0] == '+')
                                country = country.mid(1);
                            unsigned code = atol(country.latin1());
                            country = "";
                            for (const ext_info *e = getCountries(); e->nCode; e++){
                                if (e->nCode == code){
                                    country = e->szName;
                                    break;
                                }
                            }
                        }
                        n = number.find(" - ");
                        if (n >= 0){
                            ext = number.mid(n + 3);
                            number = number.left(n);
                        }
                        phone = number;
                    }
                    answer.packStr32(descr.local8Bit());
                    answer.packStr32(area.local8Bit());
                    answer.packStr32(phone.local8Bit());
                    answer.packStr32(ext.local8Bit());
                    answer.packStr32(country.local8Bit());
                    answer.pack(active);

                    unsigned long len = gateway.length() + 24;
                    unsigned long sms_available = 0;
                    switch (type){
                    case PHONE:
                        type = 0;
                        break;
                    case FAX:
                        type = 3;
                        break;
                    case CELLULAR:
                        type = 2;
                        sms_available = 1;
                        break;
                    case PAGER:
                        type = 4;
                        break;
                    }
                    answer1.pack(len);
                    answer1.pack(type);
                    answer1.packStr32(gateway.local8Bit());
                    answer1.pack((unsigned long)0);
                    answer1.pack(sms_available);
                    answer1.pack((unsigned long)0);
                    answer1.pack(publish);
                    nEntries++;
                }
                answer.pack(answer1.data(0), answer1.size());
                typeAnswer = 0x00000003;
                break;
            }
        }
    case PLUGIN_PICTURE:{
            time = this->data.owner.PluginInfoTime;
            typeAnswer = 0x00000001;
            QString pictFile = getPicture();
            if (!pictFile.isEmpty()){
#ifdef WIN32
                pictFile = pictFile.replace(QRegExp("/"), "\\");
#endif
                QFile f(pictFile);
                if (f.open(IO_ReadOnly)){
#ifdef WIN32
                    int n = pictFile.findRev("\\");
#else
int n = pictFile.findRev("/");
#endif
                    if (n >= 0)
                        pictFile = pictFile.mid(n + 1);
                    nEntries = pictFile.length();
                    answer.pack(pictFile.local8Bit(), pictFile.length());
                    unsigned long size = f.size();
                    answer.pack(size);
                    while (size > 0){
                        char buf[2048];
                        unsigned tail = sizeof(buf);
                        if (tail > size)
                            tail = size;
                        f.readBlock(buf, tail);
                        answer.pack(buf, tail);
                        size -= tail;
                    }
                }
            }
            break;
        }
    case PLUGIN_FOLLOWME:
        time = this->data.owner.PluginStatusTime;
        break;
    case PLUGIN_QUERYxINFO:
        time = this->data.owner.PluginInfoTime;
        typeAnswer = 0x00010002;
        if (!getPicture().isEmpty()){
            nEntries++;
            answer.pack((char*)plugins[PLUGIN_PICTURE], sizeof(plugin));
            answer.pack((unsigned short)0);
            answer.pack((unsigned short)1);
            answer.packStr32(plugin_name[PLUGIN_PICTURE]);
            answer.packStr32(plugin_descr[PLUGIN_PICTURE]);
            answer.pack((unsigned long)0);
        }
        if (!getContacts()->owner()->getPhones().isEmpty()){
            nEntries++;
            answer.pack((char*)plugins[PLUGIN_PHONEBOOK], sizeof(plugin));
            answer.pack((unsigned short)0);
            answer.pack((unsigned short)1);
            answer.packStr32(plugin_name[PLUGIN_PHONEBOOK]);
            answer.packStr32(plugin_descr[PLUGIN_PHONEBOOK]);
            answer.pack((unsigned long)0);
        }
        break;
    case PLUGIN_QUERYxSTATUS:
        time = this->data.owner.PluginStatusTime;
        typeAnswer = 0x00010000;
        nEntries++;
        answer.pack((char*)plugins[PLUGIN_FOLLOWME], sizeof(plugin));
        answer.pack((unsigned short)0);
        answer.pack((unsigned short)1);
        answer.packStr32(plugin_name[PLUGIN_FOLLOWME]);
        answer.packStr32(plugin_descr[PLUGIN_FOLLOWME]);
        answer.pack((unsigned long)0);
        if (this->data.owner.SharedFiles){
            nEntries++;
            answer.pack((char*)plugins[PLUGIN_FILESERVER], sizeof(plugin));
            answer.pack((unsigned short)0);
            answer.pack((unsigned short)1);
            answer.packStr32(plugin_name[PLUGIN_FILESERVER]);
            answer.packStr32(plugin_descr[PLUGIN_FILESERVER]);
            answer.pack((unsigned long)0);
        }
        if (this->data.owner.ICQPhone){
            nEntries++;
            answer.pack((char*)plugins[PLUGIN_ICQPHONE], sizeof(plugin));
            answer.pack((unsigned short)0);
            answer.pack((unsigned short)1);
            answer.packStr32(plugin_name[PLUGIN_ICQPHONE]);
            answer.packStr32(plugin_descr[PLUGIN_ICQPHONE]);
            answer.pack((unsigned long)0);
        }
        break;
    default:
        log(L_DEBUG, "Bad plugin type request %u", plugin_type);
    }
    unsigned long size = answer.size() + 8;
    info.pack((unsigned short)0);
    info.pack((unsigned short)1);
    switch (plugin_type){
    case PLUGIN_FOLLOWME:
        info.pack(this->data.owner.FollowMe);
        info.pack(time);
        info.pack((char)1);
        break;
    case PLUGIN_QUERYxSTATUS:
        info.pack((unsigned long)0);
        info.pack((unsigned long)0);
        info.pack((char)1);
    default:
        info.pack(time);
        info.pack(size);
        info.pack(typeAnswer);
        info.pack(nEntries);
        info.pack(answer.data(0), answer.size());
    }
}



