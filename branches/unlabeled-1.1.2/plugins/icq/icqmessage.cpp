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
#include "msgedit.h"
#include "textshow.h"
#include "listview.h"
#include "userwnd.h"
#include "searchresult.h"
#include "xml.h"

#include <qtimer.h>
#include <qlayout.h>
#include <qtextcodec.h>

#include <vector>

using namespace std;

#ifdef WIN32
#include <windows.h>
#include <ddeml.h>

class DDEbase
{
public:
    DDEbase();
    ~DDEbase();
    operator DWORD() { return m_idDDE; }
    static DDEbase *base;
protected:
    DWORD m_idDDE;
    static HDDEDATA CALLBACK DDECallback(UINT, UINT, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD);
};

DDEbase *DDEbase::base = NULL;

DDEbase::DDEbase()
{
    m_idDDE = 0;
    FARPROC lpDdeProc = MakeProcInstance((FARPROC) DDECallback, hInstance);
    DWORD res = DdeInitializeA((LPDWORD) &m_idDDE, (PFNCALLBACK) lpDdeProc,	APPCMD_CLIENTONLY, 0L);
    base = this;
}

DDEbase::~DDEbase()
{
    base = NULL;
    if (m_idDDE)
        DdeUninitialize(m_idDDE);
}

HDDEDATA CALLBACK DDEbase::DDECallback(UINT, UINT, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD)
{
    return NULL;
}

class DDEstring
{
public:
    DDEstring(const char *name);
    ~DDEstring();
    operator HSZ() { return hSz; }
protected:
    HSZ hSz;
};

DDEstring::DDEstring(const char *name) : hSz(NULL)
{
    hSz = DdeCreateStringHandleA(*DDEbase::base, name, CP_WINANSI);
}

DDEstring::~DDEstring()
{
    if (hSz)
        DdeFreeStringHandle(*DDEbase::base, hSz);
}

class DDEdataHandle
{
public:
    DDEdataHandle(const char *text);
    DDEdataHandle(HDDEDATA data);
    ~DDEdataHandle();
    operator HDDEDATA() { return hData; }
    operator const char *();
protected:
    HDDEDATA hData;
};

DDEdataHandle::DDEdataHandle(const char *text)
{
    hData = DdeCreateDataHandle(*DDEbase::base, (unsigned char*)text, strlen(text) + 1, 0, NULL, CF_TEXT, 0);
}

DDEdataHandle::DDEdataHandle(HDDEDATA data)
{
    hData = data;
}

DDEdataHandle::~DDEdataHandle()
{
    if (hData) DdeFreeDataHandle(hData);
}

DDEdataHandle::operator const char*()
{
    if (hData == NULL)
        return NULL;
    return (const char*)DdeAccessData(hData, NULL);
}

class DDEconversation
{
protected:
    HCONV hConv;
public:
    DDEconversation(const char *_server, const char *_topic);
    ~DDEconversation();
    operator HCONV() { return hConv; }
    HDDEDATA Execute(const char *cmd);
};

DDEconversation::DDEconversation(const char *_server, const char *_topic)
        : hConv(NULL)
{
    DDEstring server(_server);
    DDEstring topic(_topic);
    hConv = DdeConnect(*DDEbase::base, server, topic, NULL);
}

DDEconversation::~DDEconversation()
{
    if (hConv)
        DdeDisconnect(hConv);
}

HDDEDATA DDEconversation::Execute(const char *cmd)
{
    DDEstring c(cmd);
    DWORD res = NULL;
    HDDEDATA hData = DdeClientTransaction(NULL, 0, hConv, c, CF_TEXT, XTYP_REQUEST, 30000, &res);
    if (hData == NULL)
        unsigned errCode = DdeGetLastError((DWORD)DDEbase::base);
    return hData;
}

string getCurrentUrl()
{
    DWORD keyLen = 0;
    HKEY hKeyOpen;
    if (RegOpenKeyExA(HKEY_CLASSES_ROOT,
                      "HTTP\\Shell\\open\\ddeexec\\application",
                      0, KEY_READ | KEY_QUERY_VALUE, &hKeyOpen) != ERROR_SUCCESS)
        return "";
    if (RegQueryValueExA(hKeyOpen, "", 0, 0, 0, &keyLen) != ERROR_SUCCESS){
        RegCloseKey(hKeyOpen);
        return "";
    }
    string topic;
    topic.append(keyLen, 0);

    DWORD type;
    DWORD size = keyLen;
    if (RegQueryValueExA(hKeyOpen, "", 0, &type, (unsigned char*)(topic.c_str()), &keyLen) != ERROR_SUCCESS){
        RegCloseKey(hKeyOpen);
        return "";
    }
    RegCloseKey(hKeyOpen);

    DDEbase b;
    DDEconversation conv(topic.c_str(), "WWW_GetWindowInfo");
    DDEdataHandle answer(conv.Execute("-1"));
    const char *url = answer;
    if (url == NULL)
        return "";
    url++;
    char *end = strchr((char*)url, '\"');
    if (end)
        *end = 0;
    return url;
}

#endif

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

static DataDef urlMessageData[] =
    {
        { "Url", DATA_UTF, 1, 0 },
        { "ServerUrl", DATA_STRING, 1, 0 },
        { "ServerText", DATA_STRING, 1, 0 },
        { NULL, 0, 0, 0 }
    };

URLMessage::URLMessage(const char *cfg)
        : Message(MessageURL, cfg)
{
    load_data(urlMessageData, &data, cfg);
}

URLMessage::~URLMessage()
{
    free_data(urlMessageData, &data);
}

string URLMessage::save()
{
    string s = Message::save();
    string s1 = save_data(urlMessageData, &data);
    if (!s1.empty()){
        if (!s.empty())
            s += '\n';
        s += s1;
    }
    return s;
}

QString URLMessage::getText()
{
    const char *serverText = getServerText();
    if ((serverText == NULL) || (*serverText == 0))
        return Message::getText();
    return ICQClient::toUnicode(serverText, client(), contact());
}

QString URLMessage::getUrl()
{
    const char *serverText = data.ServerUrl;
    if ((serverText == NULL) || (*serverText == 0))
        return data.Url ? QString::fromUtf8(data.Url) : QString("");
    return ICQClient::toUnicode(serverText, client(), contact());
}

bool URLMessage::setUrl(const QString &url)
{
    return set_str(&data.Url, url.utf8());
}

QString URLMessage::presentation()
{
    QString url = quoteString(getUrl());
    QString res = "<p><a href=\"";
    res += url;
    res += "\">";
    res += url;
    res += "</a></p><p>";
    res += getRichText();
    res += "</p>";
    return res;
}

static DataDef contactMessageData[] =
    {
        { "Contacts", DATA_UTF, 1, 0 },
        { "ServerText", DATA_STRING, 1, 0 },
        { NULL, 0, 0, 0 }
    };

ContactMessage::ContactMessage(const char *cfg)
        : Message(MessageContact, cfg)
{
    load_data(contactMessageData, &data, cfg);
}

ContactMessage::~ContactMessage()
{
    free_data(contactMessageData, &data);
}

QString ContactMessage::getContacts()
{
    const char *serverText = getServerText();
    if ((serverText == NULL) || (*serverText == 0))
        return data.Contacts ? QString::fromUtf8(data.Contacts) : QString("");
    return ICQClient::toUnicode(serverText, client(), contact());
}

bool ContactMessage::setContacts(const QString &str)
{
    return set_str(&data.Contacts, str.utf8());
}

string ContactMessage::save()
{
    string s = Message::save();
    string s1 = save_data(contactMessageData, &data);
    if (!s1.empty()){
        if (!s.empty())
            s += '\n';
        s += s1;
    }
    return s;
}

QString ContactMessage::presentation()
{
    QString res;
    QString contacts = getContacts();
    while (contacts.length()){
        QString contact = getToken(contacts, ';');
        QString uin = getToken(contact, ',');
        contact = quoteString(contact);
        res += QString("<p><a href=\"icq://%1,%2\">%3</a> %4</p>")
               .arg(uin)
               .arg(contact)
               .arg(uin)
               .arg(contact);
    }
    return res;
}

static DataDef icqAuthMessageData[] =
    {
        { "ServerText", DATA_STRING, 1, 0 },
        { "Charset", DATA_STRING, 1, 0 },
        { NULL, 0, 0, 0 }
    };

ICQAuthMessage::ICQAuthMessage(unsigned type, const char *cfg)
        : AuthMessage(type, cfg)
{
    load_data(icqAuthMessageData, &data, cfg);
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
        r = c - '0';
        return true;
    }
    if ((c >= 'A') && (c <= 'F')){
        r = c - 'A' + 10;
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
            if (*str == (char)0xFE) break;
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
                msg->setText(ICQClient::parseRTF(str, encoding));
                msg->setFlags(MESSAGE_RICHTEXT);
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
    URLMessage *m = new URLMessage;
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
        unsigned uin = atol(c[i*2].c_str());
        if (uin == 0){
            log(L_WARN, "Bad contact UIN");
            return NULL;
        }
        if (!serverText.empty())
            serverText += ';';
        serverText += number(uin);
        serverText += ',';
        serverText += c[i*2+1];
    }
    ContactMessage *m = new ContactMessage;
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
    ICQAuthMessage *m = new ICQAuthMessage(MessageICQAuthRequest);
    m->setServerText(l[4].c_str());
    return m;
}

static Message *parseExtendedMessage(Buffer &packet)
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
    if (msgType.find("URL")){
        string info;
        b.unpackStr32(info);
        return parseURLMessage(info.c_str());
    }
    if (msgType == "Request For Contacts"){
        string info;
        b.unpackStr32(info);
        ICQAuthMessage *m = new ICQAuthMessage(MessageContactRequest);
        m->setServerText(info.c_str());
        return m;
    }
    if (msgType == "Contacts"){
        string p;
        b.unpackStr32(p);
        return parseContactMessage(p.c_str());
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
    log(L_WARN, "Unknown extended message type %s", msgType.c_str());
    return NULL;
}

Message *ICQClient::parseMessage(unsigned short type, unsigned long uin, string &p, Buffer &packet,
                                 unsigned short, unsigned short,
                                 unsigned long, unsigned long)
{
    if (uin == 0x0A){
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
            ICQUserData *data = findContact(uin, NULL, true, contact);
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
    case ICQ_MSGxEXT:
        msg = parseExtendedMessage(packet);
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
        MESSAGE_DEFAULT,
        MessageGeneric,
        NULL,
        NULL,
        createIcq,
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
        MESSAGE_DEFAULT,
        0,
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
        MESSAGE_DEFAULT,
        0,
        "Email pager message",
        "%n Email pager messages",
        createEmailPager,
        NULL,
        NULL
    };

static Message *createIcqAuthRequest(const char *cfg)
{
    return new ICQAuthMessage(MessageICQAuthRequest, cfg);
}

static MessageDef defIcqAuthRequest =
    {
        NULL,
        MESSAGE_DEFAULT,
        MessageAuthRequest,
        NULL,
        NULL,
        createIcqAuthRequest,
        NULL,
        NULL
    };

static Message *createIcqAuthGranted(const char *cfg)
{
    return new ICQAuthMessage(MessageICQAuthGranted, cfg);
}

static MessageDef defIcqAuthGranted =
    {
        NULL,
        MESSAGE_DEFAULT,
        MessageAuthGranted,
        NULL,
        NULL,
        createIcqAuthGranted,
        NULL,
        NULL
    };

static Message *createIcqAuthRefused(const char *cfg)
{
    return new ICQAuthMessage(MessageICQAuthRefused, cfg);
}

static MessageDef defIcqAuthRefused =
    {
        NULL,
        MESSAGE_DEFAULT,
        MessageAuthRefused,
        NULL,
        NULL,
        createIcqAuthRefused,
        NULL,
        NULL
    };

static Message *createContactRequest(const char *cfg)
{
    return new ICQAuthMessage(MessageContactRequest, cfg);
}

#if 0
i18n("Contact request", "%n contact requests", 1);
#endif

static MessageDef defContactRequest =
    {
        NULL,
        MESSAGE_DEFAULT,
        0,
        "Contact request",
        "%n contact requests",
        createContactRequest,
        NULL,
        NULL
    };

static Message *createUrl(const char *cfg)
{
    return new URLMessage(cfg);
}

static QObject *generateUrl(QWidget *p, Message *msg)
{
    return new MsgUrl(static_cast<CToolCustom*>(p), msg);
}

#if 0
i18n("URL", "%n URLs", 1);
#endif

static MessageDef defUrl =
    {
        NULL,
        MESSAGE_DEFAULT,
        0,
        "URL",
        "%n URLs",
        createUrl,
        NULL,
        generateUrl
    };

static Message *createContact(const char *cfg)
{
    return new ContactMessage(cfg);
}

static QObject *generateContact(QWidget *p, Message *msg)
{
    return new MsgContacts(static_cast<CToolCustom*>(p), msg, ICQPlugin::m_protocol);
}

#if 0
i18n("Contact list", "%n contact lists", 1);
#endif

static MessageDef defContact =
    {
        NULL,
        MESSAGE_DEFAULT,
        0,
        "Contact list",
        "%n contact lists",
        createContact,
        NULL,
        generateContact
    };

MsgUrl::MsgUrl(CToolCustom *parent, Message *msg)
        : QLineEdit(parent)
{
    m_client = msg->client();
    for (QWidget *p = parent->parentWidget(); p; p = p->parentWidget()){
        if (p->inherits("MsgEdit")){
            m_edit = static_cast<MsgEdit*>(p);
            break;
        }
    }
    m_edit->m_edit->setTextFormat(QTextEdit::PlainText);
    QString t = msg->getPlainText();
    if (!t.isEmpty())
        m_edit->m_edit->setText(t);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    parent->addWidget(this);
    parent->setText(i18n("URL:"));
    Command cmd;
    cmd->id    = CmdSend;
    cmd->param = m_edit;
    Event e(EventCommandWidget, cmd);
    btnSend = (QToolButton*)(e.process());
    connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(urlChanged(const QString&)));
    setText(static_cast<URLMessage*>(msg)->getUrl());
#ifdef WIN32
    if (text().isEmpty())
        setText(QString::fromUtf8(getCurrentUrl().c_str()));
#endif
    urlChanged(text());
    show();
}

void MsgUrl::init()
{
    if (text().isEmpty()){
        setFocus();
        return;
    }
    m_edit->m_edit->setFocus();
}

void MsgUrl::urlChanged(const QString &str)
{
    if (btnSend == NULL)
        return;
    btnSend->setEnabled(!str.isEmpty());
}

void *MsgUrl::processEvent(Event *e)
{
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if ((cmd->id == CmdSend) && (cmd->param == m_edit)){
            QString msgText = m_edit->m_edit->text();
            QString urlText = text();
            if (!urlText.isEmpty()){
                URLMessage *msg = new URLMessage;
                msg->setContact(m_edit->m_userWnd->id());
                msg->setText(msgText);
                msg->setUrl(urlText);
                msg->setClient(m_client.c_str());
                MsgSend s;
                s.edit = m_edit;
                s.msg  = msg;
                Event e(EventRealSendMessage, &s);
                e.process();
            }
            return e->param();
        }
    }
    if (e->type() == EventRemoveMessageType){
        if ((unsigned)(e->param()) == MessageURL)
            m_edit->m_userWnd->close();
    }
    return NULL;
}

MsgContacts::MsgContacts(CToolCustom *btn, Message *msg, Protocol *protocol)
        : QObject(btn)
{
    m_client = msg->client();
    m_msg  = NULL;
    for (QWidget *p = btn->parentWidget(); p; p = p->parentWidget()){
        if (p->inherits("MsgEdit")){
            m_edit = static_cast<MsgEdit*>(p);
            break;
        }
    }
    btn->setText(i18n("Drag users here"));
    m_contacts = new ListView(m_edit->m_frame);
    Command cmd;
    cmd->id    = CmdSend;
    cmd->param = m_edit;
    Event e(EventCommandWidget, cmd);
    btnSend = (QToolButton*)(e.process());
    connect(m_contacts, SIGNAL(destroyed()), this, SLOT(contactsDestroyed()));
    connect(m_contacts, SIGNAL(dragEnter(QMimeSource*)), this, SLOT(contactsDragEnter(QMimeSource*)));
    connect(m_contacts, SIGNAL(drop(QMimeSource*)), this, SLOT(contactsDrop(QMimeSource*)));
    connect(m_contacts, SIGNAL(deleteItem(QListViewItem*)), this, SLOT(deleteItem(QListViewItem*)));
    m_edit->m_layout->addWidget(m_contacts);
    m_edit->m_edit->hide();
    int wChar = QFontMetrics(m_contacts->font()).width('0');
    m_contacts->addColumn(i18n("UIN"), -10*wChar);
    m_contacts->setColumnAlignment(0, AlignRight);
    m_contacts->addColumn(i18n("Alias"), 20*wChar);
    m_contacts->addColumn("");
    m_contacts->addColumn(i18n("Name"));
    m_contacts->addColumn(i18n("Email"));
    m_contacts->setExpandingColumn(3);
    m_contacts->setSorting(0);
    m_contacts->show();
    m_protocol = protocol;
    ContactMessage *m = static_cast<ContactMessage*>(msg);
    QString contacts = m->getContacts();
    while (contacts.length()){
        QString item = getToken(contacts, ';');
        unsigned long uin = atol(getToken(item, ','));
        if (uin == 0)
            continue;
        new UserTblItem(m_contacts, uin, item);
    }
    changed();
}

MsgContacts::~MsgContacts()
{
    m_edit->m_edit->show();
    if (m_contacts)
        delete m_contacts;
}

void MsgContacts::contactsDragEnter(QMimeSource *s)
{
    if (!ContactDragObject::canDecode(s))
        return;
    Contact *contact = ContactDragObject::decode(s);
    if (contact == NULL)
        return;
    ClientDataIterator it(contact->clientData);
    void *data;
    while ((data = ++it) != NULL){
        if (it.client()->protocol() == m_protocol){
            m_contacts->acceptDrop(true);
            return;
        }
    }
}

void MsgContacts::contactsDrop(QMimeSource *s)
{
    if (!ContactDragObject::canDecode(s))
        return;
    Contact *contact = ContactDragObject::decode(s);
    if (contact == NULL)
        return;
    ClientDataIterator it(contact->clientData);
    void *_data;
    while ((_data = ++it) != NULL){
        if (it.client()->protocol() != m_protocol)
            continue;
        ICQUserData *data = (ICQUserData*)_data;
        QListViewItem *item;
        for (item = m_contacts->firstChild(); item; item = item->nextSibling()){
            if (item->text(0).toUInt() == data->Uin)
                break;;
        }
        if (item)
            continue;
        new UserTblItem(m_contacts, static_cast<ICQClient*>(it.client()), data);
    }
    changed();
}

void MsgContacts::deleteItem(QListViewItem *item)
{
    delete item;
    changed();
}

void MsgContacts::changed()
{
    if (btnSend == NULL)
        return;
    btnSend->setEnabled(m_contacts->firstChild() != NULL);
}

void MsgContacts::contactsDestroyed()
{
    m_contacts = NULL;
}

void MsgContacts::init()
{
    m_contacts->setFocus();
}

void *MsgContacts::processEvent(Event *e)
{
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if ((cmd->id == CmdSend) && (cmd->param == m_edit)){
            QString msgText = m_edit->m_edit->text();
            QString contacts;
            for (QListViewItem *item = m_contacts->firstChild(); item; item = item->nextSibling()){
                if (!contacts.isEmpty())
                    contacts += ";";
                contacts += item->text(0);
                contacts += ",";
                contacts += quoteChars(item->text(1), ";");
            }
            if (!contacts.isEmpty()){
                ContactMessage *msg = new ContactMessage;
                msg->setContact(m_edit->m_userWnd->id());
                msg->setContacts(contacts);
                msg->setClient(m_client.c_str());
                MsgSend s;
                s.edit = m_edit;
                s.msg  = msg;
                m_msg  = msg;
                Event e(EventRealSendMessage, &s);
                e.process();
            }
            return e->param();
        }
    }
    if (e->type() == EventRemoveMessageType){
        if ((unsigned)(e->param()) == MessageContact)
            m_edit->m_userWnd->close();
    }
    if (e->type() == EventMessageSent){
        Message *msg = (Message*)(e->param());
        if (msg == m_msg){
            m_msg = NULL;
            const char *err = msg->getError();
            if ((err == NULL) || (*err == 0))
                m_contacts->clear();
        }
    }
    return NULL;
}

void ICQPlugin::registerMessages()
{
    Command cmd;

    cmd->id			 = MessageURL;
    cmd->text		 = I18N_NOOP("&URL");
    cmd->icon		 = "url";
    cmd->accel		 = "Ctrl+U";
    cmd->menu_grp	 = 0x3012;
    cmd->flags		 = COMMAND_DEFAULT;
    cmd->param		 = &defUrl;
    Event eMsg(EventCreateMessageType, cmd);
    eMsg.process();

    cmd->id			 = MessageContact;
    cmd->text		 = I18N_NOOP("&Contact list");
    cmd->icon		 = "contacts";
    cmd->accel		 = "Ctrl+C";
    cmd->menu_grp	 = 0x3013;
    cmd->param		 = &defContact;
    eMsg.process();

    cmd->id			= MessageICQ;
    cmd->text		= "ICQ";
    cmd->icon		= "message";
    cmd->menu_grp	= 0;
    cmd->param		= &defIcq;
    eMsg.process();

    cmd->id			= MessageContactRequest;
    cmd->text		= "Contact Request";
    cmd->icon		= "contacts";
    cmd->menu_grp	= 0;
    cmd->param		= &defContactRequest;
    eMsg.process();

    cmd->id         = MessageICQAuthRequest;
    cmd->text       = "ICQ AuthRequest";
    cmd->icon       = "auth";
    cmd->menu_grp   = 0;
    cmd->param      = &defIcqAuthRequest;
    eMsg.process();


    cmd->id			= MessageICQAuthGranted;
    cmd->text		= "ICQ AuthGranted";
    cmd->icon		= "auth";
    cmd->menu_grp	= 0;
    cmd->param		= &defIcqAuthGranted;
    eMsg.process();

    cmd->id			= MessageICQAuthRefused;
    cmd->text		= "ICQ AuthRefused";
    cmd->icon		= "auth";
    cmd->menu_grp	= 0;
    cmd->param		= &defIcqAuthRefused;
    eMsg.process();

    cmd->id			= MessageWebPanel;
    cmd->text		= "Web panel";
    cmd->icon		= "web";
    cmd->menu_grp	= 0;
    cmd->param		= &defWebPanel;
    eMsg.process();

    cmd->id			= MessageEmailPager;
    cmd->text		= "Email pager";
    cmd->icon		= "mailpager";
    cmd->menu_grp	= 0;
    cmd->param		= &defEmailPager;
    eMsg.process();
}

void ICQPlugin::unregisterMessages()
{
    Event eUrl(EventRemoveMessageType, (void*)MessageURL);
    eUrl.process();

    Event eContact(EventRemoveMessageType, (void*)MessageContact);
    eContact.process();

    Event eIcq(EventRemoveMessageType, (void*)MessageICQ);
    eIcq.process();

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
}

#ifndef WIN32
#include "icqmessage.moc"
#endif



