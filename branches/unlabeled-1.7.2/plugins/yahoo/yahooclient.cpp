/***************************************************************************
                          yahooclient.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 * Based on libyahoo2
 *
 * Some code copyright (C) 2002-2004, Philip S Tellis <philip.tellis AT gmx.net>
 *
 * Yahoo Search copyright (C) 2003, Konstantin Klyagin <konst AT konst.org.ua>
 *
 * Much of this code was taken and adapted from the yahoo module for
 * gaim released under the GNU GPL.  This code is also released under the 
 * GNU GPL.
 *
 * This code is derivitive of Gaim <http://gaim.sourceforge.net>
 * copyright (C) 1998-1999, Mark Spencer <markster@marko.net>
 *             1998-1999, Adam Fritzler <afritz@marko.net>
 *             1998-2002, Rob Flynn <rob@marko.net>
 *             2000-2002, Eric Warmenhoven <eric@warmenhoven.org>
 *             2001-2002, Brian Macke <macke@strangelove.net>
 *                  2001, Anand Biligiri S <abiligiri@users.sf.net>
 *                  2001, Valdis Kletnieks
 *                  2002, Sean Egan <bj91704@binghamton.edu>
 *                  2002, Toby Gray <toby.gray@ntlworld.com>
 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "yahoo.h"
#include "yahooclient.h"
#include "yahoocfg.h"
#include "yahooinfo.h"
#include "yahoosearch.h"

#include "html.h"
#include "core.h"

#include <time.h>

#include <qtimer.h>
#include <qtextcodec.h>
#include <qregexp.h>

static char YAHOO_PACKET_SIGN[] = "YMSG";

const unsigned PING_TIMEOUT = 60;

const unsigned	YAHOO_LOGIN_OK		= 0;
const unsigned	YAHOO_LOGIN_PASSWD	= 13;
const unsigned	YAHOO_LOGIN_LOCK	= 14;
const unsigned	YAHOO_LOGIN_DUPL	= 99;

static DataDef yahooUserData[] =
    {
        { "", DATA_ULONG, 1, DATA(9) },		// Sign
        { "LastSend", DATA_ULONG, 1, 0 },
        { "Login", DATA_UTF, 1, 0 },
        { "Nick", DATA_UTF, 1, 0 },
        { "First", DATA_UTF, 1, 0 },
        { "Last", DATA_UTF, 1, 0 },
        { "EMail", DATA_UTF, 1, 0 },
        { "", DATA_ULONG, 1, DATA(-1) },	// Status
        { "", DATA_BOOL, 1, 0 },			// bAway
        { "", DATA_UTF, 1, 0 },				// AwayMessage
        { "StatusTime", DATA_ULONG, 1, 0 },
        { "OnlineTime", DATA_ULONG, 1, 0 },
        { "Group", DATA_STRING, 1, 0 },
        { "", DATA_BOOL, 1, 0 },			// bChecked
        { "", DATA_BOOL, 1, 0 },			// bTyping
        { "Encoding", DATA_STRING, 1, 0 },
        { NULL, 0, 0, 0 }
    };

static DataDef yahooClientData[] =
    {
        { "Server", DATA_STRING, 1, "scs.msg.yahoo.com" },
        { "Port", DATA_ULONG, 1, DATA(5050) },
        { "FTServer", DATA_STRING, 1, "filetransfer.msg.yahoo.com" },
        { "FYPort", DATA_ULONG, 1, DATA(80) },
        { "", DATA_STRING, 1, 0 },				// CookieY
        { "", DATA_STRING, 1, 0 },				// CookieT
        { "UseHTTP", DATA_BOOL, 1, 0 },
        { "AutoHTTP", DATA_BOOL, 1, DATA(1) },
        { "", DATA_STRUCT, sizeof(YahooUserData) / sizeof(Data), DATA(yahooUserData) },
        { NULL, 0, 0, 0 }
    };

const DataDef *YahooProtocol::userDataDef()
{
    return yahooUserData;
}

YahooClient::YahooClient(Protocol *protocol, const char *cfg)
        : TCPClient(protocol, cfg)
{
    load_data(yahooClientData, &data, cfg);
    m_status = STATUS_OFFLINE;
    m_bFirstTry = false;
}

YahooClient::~YahooClient()
{
	for (list<FileMessage*>::iterator it = m_waitMsg.begin(); it != m_waitMsg.end(); ++it){
		(*it)->setError(i18n("Cancel send"));
		Event e(EventMessageSent, *it);
		e.process();
	}
    TCPClient::setStatus(STATUS_OFFLINE, false);
    free_data(yahooClientData, &data);
}

string YahooClient::getConfig()
{
    string res = TCPClient::getConfig();
    if (res.length())
        res += "\n";
    res += save_data(yahooClientData, &data);
    return res;
}

bool YahooClient::send(Message *msg, void *_data)
{
    if ((getState() != Connected) || (_data == NULL))
        return false;
    YahooUserData *data = (YahooUserData*)_data;
    switch (msg->type()){
    case MessageTypingStart:
        sendTyping(data, true);
        return true;
    case MessageTypingStop:
        sendTyping(data, false);
        return true;
    case MessageGeneric:
        sendMessage(msg->getRichText(), msg, data);
        return true;
    case MessageUrl:{
            QString msgText = static_cast<UrlMessage*>(msg)->getUrl();
            if (!msg->getPlainText().isEmpty()){
                msgText += "<br>";
                msgText += msg->getRichText();
            }
            sendMessage(msgText, msg, data);
            return true;
        }
	case MessageFile:{
			m_waitMsg.push_back(static_cast<FileMessage*>(msg));
			new YahooFileTransfer(static_cast<FileMessage*>(msg), data, this);
			return true;
		}
    }
    return false;
}

bool YahooClient::canSend(unsigned type, void *_data)
{
    if ((_data == NULL) || (((clientData*)_data)->Sign.value != YAHOO_SIGN))
        return false;
    if (getState() != Connected)
        return false;
    switch (type){
    case MessageGeneric:
    case MessageUrl:
	case MessageFile:
        return true;
    }
    return false;
}

void YahooClient::packet_ready()
{
    log_packet(m_socket->readBuffer, false, YahooPlugin::YahooPacket);
    if (m_bHeader){
        char header[4];
        m_socket->readBuffer.unpack(header, 4);
        if (memcmp(header, YAHOO_PACKET_SIGN, 4)){
            m_socket->error_state("Bad packet sign");
            return;
        }
        m_socket->readBuffer.incReadPos(4);
        m_socket->readBuffer >> m_data_size >> m_service;
        unsigned long session_id;
        m_socket->readBuffer >> m_pkt_status >> session_id;
        if (m_data_size){
            m_socket->readBuffer.add(m_data_size);
            m_bHeader = false;
            return;
        }
    }
    log_packet(m_socket->readBuffer, false, YahooPlugin::YahooPacket);
    process_packet();
    m_socket->readBuffer.init(20);
    m_socket->readBuffer.packetStart();
    m_bHeader = true;
}

void YahooClient::sendPacket(unsigned short service, unsigned long status)
{
    if (m_bHTTP && !m_session_id.empty()){
        addParam(0, getLogin().utf8());
        addParam(24, m_session_id.c_str());
    }
    unsigned short size = 0;
    if (!m_values.empty()){
        for (list<PARAM>::iterator it = m_values.begin(); it != m_values.end(); ++it){
            size += 4;
            size += (*it).second.size();
            size += number((*it).first).length();
        }
    }
    m_socket->writeBuffer.packetStart();
    m_socket->writeBuffer.pack(YAHOO_PACKET_SIGN, 4);
    m_socket->writeBuffer << 0x000B0000L << size << service << status << m_session;
    if (size){
        for (list<PARAM>::iterator it = m_values.begin(); it != m_values.end(); ++it){
            m_socket->writeBuffer
            << number((*it).first).c_str()
            << (unsigned short)0xC080
            << (*it).second.c_str()
            << (unsigned short)0xC080;
        }
    }
    m_values.clear();
    log_packet(m_socket->writeBuffer, true, YahooPlugin::YahooPacket);
    m_socket->write();
}

void YahooClient::addParam(unsigned key, const char *value)
{
    if (value == NULL)
        value = "";
    m_values.push_back(PARAM(key, string(value)));
}

void YahooClient::connect_ready()
{
    m_bFirstTry = false;
    m_socket->readBuffer.init(20);
    m_socket->readBuffer.packetStart();
    m_session = rand();
    m_bHeader = true;
    log(L_DEBUG, "Connect ready");
    TCPClient::connect_ready();
    if (m_bHTTP){
        addParam(1, getLogin().utf8());
        sendPacket(YAHOO_SERVICE_AUTH);
    }else{
        sendPacket(YAHOO_SERVICE_VERIFY);
    }
}

class Params : public list<PARAM>
{
public:
    Params() {}
    const char *operator[](unsigned id);
};

const char *Params::operator [](unsigned id)
{
    for (iterator it = begin(); it != end(); ++it){
        if ((*it).first == id)
            return (*it).second.c_str();
    }
    return NULL;
}

void YahooClient::process_packet()
{
    Params params;
    Params::iterator it;
    for (;;){
        string key;
        string value;
        if (!m_socket->readBuffer.scan("\xC0\x80", key) || !m_socket->readBuffer.scan("\xC0\x80", value))
            break;
        unsigned key_id = atol(key.c_str());
        params.push_back(PARAM(key_id, value));
        log(L_DEBUG, "Param: %u %s", key_id, value.c_str());
    }
    switch (m_service){
    case YAHOO_SERVICE_VERIFY:
        if (m_pkt_status != 1){
            m_reconnect = NO_RECONNECT;
            m_socket->error_state(I18N_NOOP("Yahoo! login lock"));
            return;
        }
        addParam(1, getLogin().utf8());
        sendPacket(YAHOO_SERVICE_AUTH);
        break;
    case YAHOO_SERVICE_AUTH:
        process_auth(params[13], params[94], params[1]);
        break;
    case YAHOO_SERVICE_AUTHRESP:
        m_pkt_status = 0;
        if (params[66])
            m_pkt_status = atol(params[66]);
        switch (m_pkt_status){
        case YAHOO_LOGIN_OK:
            authOk();
            return;
        case YAHOO_LOGIN_PASSWD:
            m_reconnect = NO_RECONNECT;
            m_socket->error_state(I18N_NOOP("Login failed"), AuthError);
            return;
        case YAHOO_LOGIN_LOCK:
            m_reconnect = NO_RECONNECT;
            m_socket->error_state(I18N_NOOP("Your account has been locked"), AuthError);
            return;
        case YAHOO_LOGIN_DUPL:
            m_reconnect = NO_RECONNECT;
            m_socket->error_state(I18N_NOOP("Your account is being used from another location"));
            return;
        default:
            m_socket->error_state(I18N_NOOP("Login failed"));
        }
        break;
    case YAHOO_SERVICE_LIST:
        authOk();
        loadList(params[87]);
        for (it = params.begin(); it != params.end(); ++it){
            if ((*it).first == 59){
                string s = (*it).second;
                string n = getToken(s, ' ');
                const char *p = s.c_str();
                for (; *p; ++p)
                    if (*p != ' ')
                        break;
                string cookie = p;
                s = getToken(cookie, ';');
                if (n == "Y")
                    setCookieY(s.c_str());
                if (n == "T")
                    setCookieT(s.c_str());
            }
        }
        break;
    case YAHOO_SERVICE_LOGOFF:
        if (m_pkt_status == (unsigned long)(-1)){
            m_reconnect = NO_RECONNECT;
            m_socket->error_state(I18N_NOOP("Your account is being used from another location"));
            return;
        }
    case YAHOO_SERVICE_LOGON:
        if (params[1]){
            if (params[24])
                m_session_id = params[24];
            authOk();
        }
    case YAHOO_SERVICE_USERSTAT:
    case YAHOO_SERVICE_ISAWAY:
    case YAHOO_SERVICE_ISBACK:
    case YAHOO_SERVICE_GAMELOGON:
    case YAHOO_SERVICE_GAMELOGOFF:
    case YAHOO_SERVICE_IDACT:
    case YAHOO_SERVICE_IDDEACT:
        if (params[7] && params[13])
            processStatus(m_service, params[7], params[10], params[19], params[47], params[137]);
        break;
    case YAHOO_SERVICE_IDLE:
    case YAHOO_SERVICE_MAILSTAT:
    case YAHOO_SERVICE_CHATINVITE:
    case YAHOO_SERVICE_CALENDAR:
    case YAHOO_SERVICE_NEWPERSONALMAIL:
    case YAHOO_SERVICE_ADDIDENT:
    case YAHOO_SERVICE_ADDIGNORE:
    case YAHOO_SERVICE_PING:
    case YAHOO_SERVICE_GOTGROUPRENAME:
    case YAHOO_SERVICE_GROUPRENAME:
    case YAHOO_SERVICE_PASSTHROUGH2:
    case YAHOO_SERVICE_CHATLOGON:
    case YAHOO_SERVICE_CHATLOGOFF:
    case YAHOO_SERVICE_CHATMSG:
    case YAHOO_SERVICE_REJECTCONTACT:
    case YAHOO_SERVICE_PEERTOPEER:
        break;
    case YAHOO_SERVICE_MESSAGE:
        if (params[4] && params[14])
            process_message(params[4], params[14], params[97]);
        break;
    case YAHOO_SERVICE_NOTIFY:
        if (params[4] && params[49])
            notify(params[4], params[49], params[13]);
        break;
    case YAHOO_SERVICE_NEWCONTACT:
        if (params[1]){
            contact_added(params[3], params[14]);
            return;
        }
        if (params[7]){
            processStatus(m_service, params[7], params[10], params[14], params[47], params[137]);
            return;
        }
        if (m_pkt_status == 7)
            contact_rejected(params[3], params[14]);
        break;
    case YAHOO_SERVICE_P2PFILEXFER:
        if ((params[49] == NULL) || strcmp(params[49], "FILEXFER")){
            log(L_WARN, "Unhandled p2p type %s", params[49]);
            break;
        }
    case YAHOO_SERVICE_FILETRANSFER:
        if (params[4] && params[27] && params[28] && params[14] && params[20])
            process_file(params[4], params[27], params[28], params[14], params[20]);
        break;
    default:
        log(L_WARN, "Unknown service %X", m_service);
    }
}

class TextParser
{
public:
    TextParser(YahooClient *client, YahooUserData *data);
    QString parse(const char *msg);

    class Tag
    {
    public:
        Tag(const QString &str);
        bool operator == (const Tag &t) const;
        QString open_tag() const;
        QString close_tag() const;
    protected:
        QString	m_tag;
    };

class FaceSizeParser : public HTMLParser
    {
    public:
        FaceSizeParser(const QString&);
        QString face;
        QString size;
    protected:
        virtual void text(const QString &text);
        virtual void tag_start(const QString &tag, const list<QString> &options);
        virtual void tag_end(const QString &tag);
    };

protected:
    void addText(const char *str, unsigned size);
    unsigned m_state;
    QString color;
    QString face;
    QString size;
    bool m_bChanged;
    stack<Tag> m_tags;
    void setState(unsigned code, bool bSet);
    void clearState(unsigned code);
    void put_color(unsigned color);
    void put_style();
    void push_tag(const QString &tag);
    void pop_tag(const QString &tag);
    YahooUserData *m_data;
    YahooClient   *m_client;
    QString m_text;
};

TextParser::FaceSizeParser::FaceSizeParser(const QString &str)
{
    parse(str);
}

void TextParser::FaceSizeParser::text(const QString&)
{
}

void TextParser::FaceSizeParser::tag_start(const QString &tag, const list<QString> &options)
{
    if (tag != "font")
        return;
    for (list<QString>::const_iterator it = options.begin(); it != options.end(); ++it){
        QString key = *it;
        ++it;
        if (key == "face")
            face = QString("font-family:") + *it;
        if (key == "size")
            size = QString("font-size:") + *it + "pt";
    }
}

void TextParser::FaceSizeParser::tag_end(const QString&)
{
}

TextParser::Tag::Tag(const QString &tag)
{
    m_tag	= tag;
}

bool TextParser::Tag::operator == (const Tag &t) const
{
    return close_tag() == t.close_tag();
}

QString TextParser::Tag::open_tag() const
{
    QString res;
    res += "<";
    res += m_tag;
    res += ">";
    return res;
}

QString TextParser::Tag::close_tag() const
{
    int n = m_tag.find(" ");
    QString res;
    res += "</";
    if (n >= 0){
        res += m_tag.left(n);
    }else{
        res += m_tag;
    }
    res += ">";
    return res;
}

TextParser::TextParser(YahooClient *client, YahooUserData *data)
{
    m_data     = data;
    m_client   = client;
    m_bChanged = false;
    m_state    = 0;
}

static unsigned esc_colors[] =
    {
        0x000000,
        0x0000FF,
        0x008080,
        0x808080,
        0x008000,
        0xFF0080,
        0x800080,
        0xFF8000,
        0xFF0000,
        0x808000
    };

QString TextParser::parse(const char *msg)
{
    Buffer b;
    b.pack(msg, strlen(msg));
    for (;;){
        string part;
        if (!b.scan("\x1B\x5B", part))
            break;
        addText(part.c_str(), part.length());

        if (!b.scan("m", part))
            break;
        if (part.empty())
            continue;
        if (part[0] == 'x'){
            unsigned code = atol(part.c_str() + 1);
            switch (code){
            case 1:
            case 2:
            case 4:
                setState(code, false);
                break;
            }
            continue;
        }
        if (part[0] == '#'){
            put_color(strtoul(part.c_str() + 1, NULL, 16));
            continue;
        }
        unsigned code = atol(part.c_str());
        switch (code){
        case 1:
        case 2:
        case 4:
            setState(code, true);
            break;
        default:
            if ((code >= 30) && (code < 40))
                put_color(esc_colors[code - 30]);
        }
    }
    addText(b.data(b.readPos()), b.writePos() - b.readPos());
    while (!m_tags.empty()){
        m_text += m_tags.top().close_tag();
        m_tags.pop();
    }
    return m_text;
}

void TextParser::setState(unsigned code, bool bSet)
{
    if (bSet){
        if ((m_state & code) == code)
            return;
        m_state |= code;
    }else{
        if ((m_state & code) == 0)
            return;
        m_state &= ~code;
    }
    QString tag;
    switch (code){
    case 1:
        tag = "b";
        break;
    case 2:
        tag = "i";
        break;
    case 4:
        tag = "u";
        break;
    default:
        return;
    }
    if (bSet){
        push_tag(tag);
    }else{
        pop_tag(tag);
    }
}

void TextParser::put_color(unsigned _color)
{
    color.sprintf("color:#%06X", _color & 0xFFFFFF);
    m_bChanged = true;
}

void TextParser::put_style()
{
    if (!m_bChanged)
        return;
    m_bChanged = false;
    QString style;
    if (!color.isEmpty())
        style = color;
    if (!face.isEmpty()){
        if (!style.isEmpty())
            style += ";";
        style += face;
    }
    if (!size.isEmpty()){
        if (!style.isEmpty())
            style += ";";
        style += size;
    }
    QString tag("span style=\"");
    tag += style;
    tag += "\"";
    pop_tag(tag);
    push_tag(tag);
}

void TextParser::push_tag(const QString &tag)
{
    Tag t(tag);
    m_tags.push(t);
    m_text += t.open_tag();
}

void TextParser::pop_tag(const QString &tag)
{
    Tag t(tag);
    stack<Tag> tags;
    bool bFound = false;
    QString text;
    while (!m_tags.empty()){
        Tag top = m_tags.top();
        m_tags.pop();
        text += top.close_tag();
        if (top == t){
            bFound = true;
            break;
        }
        tags.push(top);
    }
    if (bFound)
        m_text += text;
    while (!tags.empty()){
        Tag top = tags.top();
        tags.pop();
        if (bFound)
            m_text += top.open_tag();
        m_tags.push(top);
    }
}

void TextParser::addText(const char *str, unsigned s)
{
    if (s == 0)
        return;
    QString text;
    if (m_data){
        text = m_client->toUnicode(str, m_data);
    }else{
        text = QString::fromUtf8(str, s);
    }
    while (!text.isEmpty()){
        bool bFace = false;
        int n1 = text.find("<font size=\"");
        int n2 = text.find("<font face=\"");
        int n = -1;
        if (n1 >= 0)
            n = n1;
        if ((n2 >= 0) && ((n == -1) || (n2 < n1))){
            n = n2;
            bFace = true;
        }
        if (n < 0){
            if (!text.isEmpty())
                put_style();
            m_text += quoteString(text);
            break;
        }
        if (n)
            put_style();
        m_text += quoteString(text.left(n));
        text = text.mid(n);
        n = text.find(">");
        if (n < 0)
            break;
        FaceSizeParser p(text.left(n + 1));
        text = text.mid(n + 1);
        if (!p.face.isEmpty()){
            face = p.face;
            m_bChanged = true;
        }
        if (!p.size.isEmpty()){
            size = p.size;
            m_bChanged = true;
        }
    }
}

void YahooClient::process_message(const char *id, const char *msg, const char *utf)
{
    bool bUtf = false;
    if (utf && atol(utf))
        bUtf = true;
    YahooUserData *data = NULL;
    if (utf == NULL)
        data = &this->data.owner;
    Message *m = new Message(MessageGeneric);
    m->setFlags(MESSAGE_RICHTEXT);
    TextParser parser(this, data);
    m->setText(parser.parse(msg));
    messageReceived(m, id);
}

void YahooClient::notify(const char *id, const char *msg, const char *state)
{
    Contact *contact;
    YahooUserData *data = findContact(id, NULL, contact);
    if (data == NULL)
        return;
    bool bState = false;
    if (state && atol(state))
        bState = true;
    if (!strcasecmp(msg, "TYPING")){
        if (data->bTyping.bValue != bState){
            data->bTyping.bValue = bState;
            Event e(EventContactStatus, contact);
            e.process();
        }
    }
}

void YahooClient::contact_added(const char *id, const char *message)
{
    Message *msg = new AuthMessage(MessageAdded);
    if (message)
        msg->setText(QString::fromUtf8(message));
    messageReceived(msg, id);
}

void YahooClient::contact_rejected(const char *id, const char *message)
{
    Message *msg = new AuthMessage(MessageRemoved);
    if (message)
        msg->setText(QString::fromUtf8(message));
    messageReceived(msg, id);
}

static bool _cmp(const char *s1, const char *s2)
{
    if (s1 == NULL)
        s1 = "";
    if (s2 == NULL)
        s2 = "";
    return strcmp(s1, s2) != 0;
}

void YahooClient::processStatus(unsigned short service, const char *id,
                                const char *_state, const char *_msg,
                                const char *_away, const char *_idle)
{
    Contact *contact;
    YahooUserData *data = findContact(id, NULL, contact);
    if (data == NULL)
        return;
    unsigned state = 0;
    unsigned away  = 0;
    unsigned idle  = 0;
    if (_state)
        state = atol(_state);
    if (_away)
        away  = atol(_away);
    if (_idle)
        idle  = atol(_idle);
    if (service == YAHOO_SERVICE_LOGOFF)
        state = YAHOO_STATUS_OFFLINE;
    if ((state != data->Status.value) ||
            ((state == YAHOO_STATUS_CUSTOM) &&
             (((away != 0) != data->bAway.bValue) || _cmp(_msg, data->AwayMessage.ptr)))){

        unsigned long old_status = STATUS_UNKNOWN;
        unsigned style  = 0;
        const char *statusIcon = NULL;
        contactInfo(data, old_status, style, statusIcon);

        time_t now;
        time(&now);
        now -= idle;
        if (data->Status.value == YAHOO_STATUS_OFFLINE)
            data->OnlineTime.value = now;
        data->Status.value = state;
        data->bAway.bValue = (away != 0);
        data->StatusTime.value = now;

        unsigned long new_status = STATUS_UNKNOWN;
        contactInfo(data, old_status, style, statusIcon);

        if (old_status != new_status){
            StatusMessage m;
            m.setContact(contact->id());
            m.setClient(dataName(data).c_str());
            m.setFlags(MESSAGE_RECEIVED);
            m.setStatus(STATUS_OFFLINE);
            Event e(EventMessageReceived, &m);
            e.process();
            if ((new_status == STATUS_ONLINE) && !contact->getIgnore()){
                Event e(EventContactOnline, contact);
                e.process();
            }
        }else{
            Event e(EventContactStatus, contact);
            e.process();
        }
    }
}

string YahooClient::name()
{
    string res = "Yahoo.";
    if (data.owner.Login.ptr)
        res += data.owner.Login.ptr;
    return res;
}

string YahooClient::dataName(void *_data)
{
    string res = name();
    YahooUserData *data = (YahooUserData*)_data;
    res += "+";
    res += data->Login.ptr;
    return res;
}

void YahooClient::setStatus(unsigned status)
{
    if (status  == m_status)
        return;
    time_t now;
    time(&now);
    if (m_status == STATUS_OFFLINE)
        data.owner.OnlineTime.value = now;
    data.owner.StatusTime.value = now;
    m_status = status;
    data.owner.Status.value = m_status;
    Event e(EventClientChanged, static_cast<Client*>(this));
    e.process();
    if (status == STATUS_OFFLINE){
        if (m_status != STATUS_OFFLINE){
            m_status = status;
            data.owner.Status.value = status;
            time_t now;
            time(&now);
            data.owner.StatusTime.value = now;
        }
        return;
    }
    unsigned yahoo_status = YAHOO_STATUS_OFFLINE;
    switch (status){
    case STATUS_ONLINE:
        yahoo_status = YAHOO_STATUS_AVAILABLE;
        break;
    case STATUS_DND:
        yahoo_status = YAHOO_STATUS_BUSY;
        break;
    }
    if (yahoo_status != YAHOO_STATUS_OFFLINE){
        m_status = status;
        sendStatus(yahoo_status);
        return;
    }
    ARRequest ar;
    ar.contact  = NULL;
    ar.status   = status;
    ar.receiver = this;
    ar.param	= (void*)status;
    Event eAR(EventARRequest, &ar);
    eAR.process();
}

void YahooClient::process_file(const char *id, const char *fileName, const char *fileSize, const char *msg, const char *url)
{
    YahooFileMessage *m = new YahooFileMessage;
    m->addFile(toUnicode(fileName, NULL), atol(fileSize));
    m->setUrl(url);
    m->setMsgText(msg);
    messageReceived(m, id);
}

void YahooClient::disconnected()
{
    m_values.clear();
    m_session_id = "";
    Contact *contact;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        YahooUserData *data;
        ClientDataIterator it(contact->clientData, this);
        while ((data = (YahooUserData*)(++it)) != NULL){
            if (data->Status.value != YAHOO_STATUS_OFFLINE){
                data->Status.value = YAHOO_STATUS_OFFLINE;
                StatusMessage m;
                m.setContact(contact->id());
                m.setClient(dataName(data).c_str());
                m.setStatus(STATUS_OFFLINE);
                m.setFlags(MESSAGE_RECEIVED);
                Event e(EventMessageReceived, &m);
                e.process();
            }
        }
    }
}

bool YahooClient::isMyData(clientData *&_data, Contact*&)
{
    if (_data->Sign.value != YAHOO_SIGN)
        return false;
    return true;
}

bool YahooClient::createData(clientData *&_data, Contact *contact)
{
    YahooUserData *data = (YahooUserData*)_data;
    YahooUserData *new_data = (YahooUserData*)(contact->clientData.createData(this));
    set_str(&new_data->Nick.ptr, data->Nick.ptr);
    _data = (clientData*)new_data;
    return true;
}

void YahooClient::setupContact(Contact*, void*)
{
}

QWidget	*YahooClient::setupWnd()
{
    return new YahooConfig(NULL, this, false);
}

QString YahooClient::getLogin()
{
    if (data.owner.Login.ptr)
        return QString::fromUtf8(data.owner.Login.ptr);
    return "";
}

void YahooClient::setLogin(const QString &login)
{
    set_str(&data.owner.Login.ptr, login.utf8());
}

void YahooClient::authOk()
{
    if (getState() == Connected)
        return;
    if (m_bHTTP && m_session_id.empty())
        return;
    setState(Connected);
    setPreviousPassword(NULL);
    setStatus(m_logonStatus);
    QTimer::singleShot(PING_TIMEOUT * 1000, this, SLOT(ping()));
}

void YahooClient::loadList(const char *str)
{
    Contact *contact;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        YahooUserData *data;
        ClientDataIterator itd(contact->clientData, this);
        while ((data = (YahooUserData*)(++itd)) != NULL){
            data->bChecked.bValue = (contact->getGroup() == 0);
        }
    }
    if (str){
        string s = str;
        while (!s.empty()){
            string line = getToken(s, '\n');
            string grp = getToken(line, ':');
            if (line.empty()){
                line = grp;
                grp = "";
            }
            while (!line.empty()){
                string id = getToken(line, ',');
                Contact *contact;
                YahooUserData *data = findContact(id.c_str(), grp.c_str(), contact, false);
                data->bChecked.bValue = true;
            }
        }
    }
    it.reset();
    list<Contact*> forRemove;
    while ((contact = ++it) != NULL){
        YahooUserData *data;
        ClientDataIterator itd(contact->clientData, this);
        list<YahooUserData*> dataForRemove;
        bool bChanged = false;
        while ((data = (YahooUserData*)(++itd)) != NULL){
            if (!data->bChecked.bValue){
                dataForRemove.push_back(data);
                bChanged = true;
            }
        }
        if (!bChanged)
            continue;
        for (list<YahooUserData*>::iterator it = dataForRemove.begin(); it != dataForRemove.end(); ++it)
            contact->clientData.freeData(*it);
        if (contact->clientData.size()){
            Event e(EventContactChanged, contact);
            e.process();
        }else{
            forRemove.push_back(contact);
        }
    }
    for (list<Contact*>::iterator itr = forRemove.begin(); itr != forRemove.end(); ++itr)
        delete *itr;
}

YahooUserData *YahooClient::findContact(const char *id, const char *grpname, Contact *&contact, bool bSend)
{
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        YahooUserData *data;
        ClientDataIterator itd(contact->clientData);
        while ((data = (YahooUserData*)(++itd)) != NULL){
            if (data->Login.ptr && !strcmp(id, data->Login.ptr))
                return data;
        }
    }
    it.reset();
    while ((contact = ++it) != NULL){
        if (contact->getName() == id){
            YahooUserData *data = (YahooUserData*)contact->clientData.createData(this);
            set_str(&data->Login.ptr, id);
            set_str(&data->Group.ptr, grpname);
            Event e(EventContactChanged, contact);
            e.process();
            return data;
        }
    }
    if (grpname == NULL)
        return NULL;
    Group *grp = NULL;
    if (*grpname){
        ContactList::GroupIterator it;
        while ((grp = ++it) != NULL)
            if (grp->getName() == toUnicode(grpname, NULL))
                break;
        if (grp == NULL){
            grp = getContacts()->group(0, true);
            grp->setName(toUnicode(grpname, NULL));
            Event e(EventGroupChanged, grp);
            e.process();
        }
    }
    if (grp == NULL)
        grp = getContacts()->group(0);
    contact = getContacts()->contact(0, true);
    YahooUserData *data = (YahooUserData*)(contact->clientData.createData(this));
    set_str(&data->Login.ptr, id);
    contact->setName(id);
    contact->setGroup(grp->id());
    Event e(EventContactChanged, contact);
    e.process();
    if (bSend)
        addBuddy(data);
    return data;
}

void YahooClient::messageReceived(Message *msg, const char *id)
{
    msg->setFlags(msg->getFlags() | MESSAGE_RECEIVED);
    if (msg->contact() == 0){
        Contact *contact;
        YahooUserData *data = findContact(id, NULL, contact);
        if (data == NULL){
            data = findContact(id, "", contact);
            if (data == NULL){
                delete msg;
                return;
            }
            contact->setTemporary(CONTACT_TEMP);
            Event e(EventContactChanged, contact);
            e.process();
        }
        msg->setClient(dataName(data).c_str());
        msg->setContact(contact->id());
    }
    Event e(EventMessageReceived, msg);
    e.process();
}

static void addIcon(string *s, const char *icon, const char *statusIcon)
{
    if (s == NULL)
        return;
    if (statusIcon && !strcmp(statusIcon, icon))
        return;
    string str = *s;
    while (!str.empty()){
        string item = getToken(str, ',');
        if (item == icon)
            return;
    }
    if (!s->empty())
        *s += ',';
    *s += icon;
}

void YahooClient::contactInfo(void *_data, unsigned long &status, unsigned&, const char *&statusIcon, string *icons)
{
    YahooUserData *data = (YahooUserData*)_data;
    unsigned cmp_status = STATUS_OFFLINE;
    switch (data->Status.value){
    case YAHOO_STATUS_AVAILABLE:
        cmp_status = STATUS_ONLINE;
        break;
    case YAHOO_STATUS_BUSY:
        cmp_status = STATUS_DND;
        break;
    case YAHOO_STATUS_NOTATHOME:
    case YAHOO_STATUS_NOTATDESK:
    case YAHOO_STATUS_NOTINOFFICE:
    case YAHOO_STATUS_ONVACATION:
        cmp_status = STATUS_NA;
        break;
    case YAHOO_STATUS_OFFLINE:
        break;
    case YAHOO_STATUS_CUSTOM:
        cmp_status = data->bAway.bValue ? STATUS_AWAY : STATUS_ONLINE;
        break;
    default:
        cmp_status = STATUS_AWAY;
    }

    const CommandDef *def;
    for (def = protocol()->statusList(); def->text; def++){
        if (def->id == cmp_status)
            break;
    }
    if (cmp_status > status){
        status = cmp_status;
        if (statusIcon && icons){
            string iconSave = *icons;
            *icons = statusIcon;
            if (iconSave.length())
                addIcon(icons, iconSave.c_str(), statusIcon);
        }
        statusIcon = def->icon;
    }else{
        if (statusIcon){
            addIcon(icons, def->icon, statusIcon);
        }else{
            statusIcon = def->icon;
        }
    }
    if (icons && data->bTyping.bValue)
        addIcon(icons, "typing", statusIcon);
}

QString YahooClient::contactTip(void *_data)
{
    YahooUserData *data = (YahooUserData*)_data;
    unsigned long status = STATUS_UNKNOWN;
    unsigned style  = 0;
    const char *statusIcon = NULL;
    contactInfo(data, status, style, statusIcon);
    QString res;
    res += "<img src=\"icon:";
    res += statusIcon;
    res += "\">";
    QString statusText;
    for (const CommandDef *cmd = protocol()->statusList(); cmd->text; cmd++){
        if (!strcmp(cmd->icon, statusIcon)){
            res += " ";
            statusText = i18n(cmd->text);
            res += statusText;
            break;
        }
    }
    res += "<br>";
    res += QString::fromUtf8(data->Login.ptr);
    res += "</b>";
    if (data->Status.value == YAHOO_STATUS_OFFLINE){
        if (data->StatusTime.value){
            res += "<br><font size=-1>";
            res += i18n("Last online");
            res += ": </font>";
            res += formatDateTime(data->StatusTime.value);
        }
    }else{
        if (data->OnlineTime.value){
            res += "<br><font size=-1>";
            res += i18n("Online");
            res += ": </font>";
            res += formatDateTime(data->OnlineTime.value);
        }
        if (data->Status.value != YAHOO_STATUS_AVAILABLE){
            res += "<br><font size=-1>";
            res += statusText;
            res += ": </font>";
            res += formatDateTime(data->StatusTime.value);
            QString msg;
            switch (data->Status.value){
            case YAHOO_STATUS_BRB:
                msg = i18n("Be right back");
                break;
            case YAHOO_STATUS_NOTATHOME:
                msg = i18n("Not at home");
                break;
            case YAHOO_STATUS_NOTATDESK:
                msg = i18n("Not at my desk");
                break;
            case YAHOO_STATUS_NOTINOFFICE:
                msg = i18n("Not in the office");
                break;
            case YAHOO_STATUS_ONPHONE:
                msg = i18n("On the phone");
                break;
            case YAHOO_STATUS_ONVACATION:
                msg = i18n("On vacation");
                break;
            case YAHOO_STATUS_OUTTOLUNCH:
                msg = i18n("Out to lunch");
                break;
            case YAHOO_STATUS_STEPPEDOUT:
                msg = i18n("Stepped out");
                break;
            case YAHOO_STATUS_CUSTOM:
                if (data->AwayMessage.ptr)
                    msg = QString::fromUtf8(data->AwayMessage.ptr);
            }
            if (!msg.isEmpty()){
                res += "<br>";
                res += quoteString(msg);
            }
        }
    }
    return res;
}

const unsigned MAIN_INFO = 1;
const unsigned NETWORK	 = 2;

static CommandDef yahooWnd[] =
    {
        {
            MAIN_INFO,
            "",
            "Yahoo!_online",
            NULL,
            NULL,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            NULL
        },
        {
            0,
            NULL,
            NULL,
            NULL,
            NULL,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            NULL
        },
    };

static CommandDef cfgYahooWnd[] =
    {
        {
            MAIN_INFO,
            "",
            "Yahoo!_online",
            NULL,
            NULL,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            NULL
        },
        {
            NETWORK,
            I18N_NOOP("Network"),
            "network",
            NULL,
            NULL,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            NULL
        },
        {
            0,
            NULL,
            NULL,
            NULL,
            NULL,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            NULL
        },
    };

CommandDef *YahooClient::infoWindows(Contact*, void *_data)
{
    YahooUserData *data = (YahooUserData*)_data;
    QString name = i18n(protocol()->description()->text);
    name += " ";
    name += QString::fromUtf8(data->Login.ptr);
    yahooWnd[0].text_wrk = strdup(name.utf8());
    return yahooWnd;
}

CommandDef *YahooClient::configWindows()
{
    QString name = i18n(protocol()->description()->text);
    name += " ";
    name += QString::fromUtf8(data.owner.Login.ptr);
    cfgYahooWnd[0].text_wrk = strdup(name.utf8());
    return cfgYahooWnd;
}

QWidget *YahooClient::infoWindow(QWidget *parent, Contact*, void *_data, unsigned id)
{
    YahooUserData *data = (YahooUserData*)_data;
    switch (id){
    case MAIN_INFO:
        return new YahooInfo(parent, data, this);
    }
    return NULL;
}

QWidget *YahooClient::configWindow(QWidget *parent, unsigned id)
{
    switch (id){
    case MAIN_INFO:
        return new YahooInfo(parent, NULL, this);
    case NETWORK:
        return new YahooConfig(parent, this, true);
    }
    return NULL;
}

void YahooClient::ping()
{
    if (getState() != Connected)
        return;
    sendPacket(YAHOO_SERVICE_PING);
    QTimer::singleShot(PING_TIMEOUT * 1000, this, SLOT(ping()));
}

class YahooParser : public HTMLParser
{
public:
    YahooParser(const QString&);
    string res;
    bool bUtf;
protected:
    typedef struct style
    {
        QString		tag;
        QString		face;
        unsigned	size;
        unsigned	color;
        unsigned	state;
    } style;
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &options);
    virtual void tag_end(const QString &tag);
    void set_style(const style &s);
    void set_state(unsigned oldState, unsigned newState, unsigned st);
    void escape(const char *str);
    bool	m_bFirst;
    string   esc;
    stack<style>	tags;
    style	curStyle;
};

YahooParser::YahooParser(const QString &str)
{
    bUtf  = false;
    m_bFirst = true;
    curStyle.face  = "Arial";
    curStyle.size  = 10;
    curStyle.color = 0;
    curStyle.state = 0;
    parse(str);
}

void YahooParser::text(const QString &str)
{
    if (str.isEmpty())
        return;
    if (!bUtf){
        for (int i = 0; i < (int)str.length(); i++){
            if (str[i].unicode() > 0x7F){
                bUtf = true;
                break;
            }
        }
    }
    res += esc;
    esc = "";
    res += str.utf8();
}

void YahooParser::tag_start(const QString &tag, const list<QString> &options)
{
    if (tag == "img"){
        QString src;
        for (list<QString>::const_iterator it = options.begin(); it != options.end(); ++it){
            QString name = (*it);
            ++it;
            QString value = (*it);
            if (name == "src"){
                src = value;
                break;
            }
        }
        if (src.left(10) != "icon:smile")
            return;
        bool bOK;
        unsigned nSmile = src.mid(10).toUInt(&bOK, 16);
        if (!bOK)
            return;
        const smile *p = smiles(nSmile);
        if (p)
            text(p->paste);
        return;
    }
    if (tag == "br"){
        res += "\n";
        return;
    }
    style s = curStyle;
    s.tag = tag;
    tags.push(s);
    if (tag == "p"){
        if (!m_bFirst)
            res += "\n";
        m_bFirst = false;
    }
    if (tag == "font"){
        for (list<QString>::const_iterator it = options.begin(); it != options.end(); ++it){
            QString name = *it;
            ++it;
            if (name == "color"){
                QColor c;
                c.setNamedColor(*it);
                s.color = c.rgb() & 0xFFFFFF;
            }
        }
    }
    if (tag == "b"){
        s.state |= 1;
        return;
    }
    if (tag == "i"){
        s.state |= 2;
        return;
    }
    if (tag == "u"){
        s.state |= 4;
        return;
    }
    for (list<QString>::const_iterator it = options.begin(); it != options.end(); ++it){
        QString name = *it;
        ++it;
        if (name != "style")
            continue;
        list<QString> styles = parseStyle(*it);
        for (list<QString>::iterator its = styles.begin(); its != styles.end(); ++its){
            QString name = *its;
            ++its;
            if (name == "color"){
                QColor c;
                c.setNamedColor(*its);
                s.color = c.rgb() & 0xFFFFFF;
            }
            if (name == "font-size"){
                unsigned size = atol((*its).latin1());
                if (size)
                    s.size = size;
            }
            if (name == "font-family")
                s.face = (*its);
            if (name == "font-weight")
                s.state &= ~1;
            if (atol((*its).latin1()) >= 600)
                s.state |= 1;
            if ((name == "font-style") && ((*its) == "italic"))
                s.state |= 2;
            if ((name == "text-decoration") && ((*its) == "underline"))
                s.state |= 4;
        }
    }
    set_style(s);
}

void YahooParser::tag_end(const QString &tag)
{
    style saveStyle =curStyle;
    while (!tags.empty()){
        saveStyle = tags.top();
        tags.pop();
        if (saveStyle.tag == tag)
            break;
    }
    set_style(saveStyle);
}

void YahooParser::set_state(unsigned oldState, unsigned newState, unsigned st)
{
    string part;
    if ((oldState & st) == (newState & st))
        return;
    if ((newState & st) == 0)
        part = "x";
    part += number(st);
    escape(part.c_str());
}

void YahooParser::set_style(const style &s)
{
    set_state(curStyle.state, s.state, 1);
    set_state(curStyle.state, s.state, 2);
    set_state(curStyle.state, s.state, 4);
    curStyle.state = s.state;
    if (curStyle.color != s.color){
        curStyle.color = s.color;
        unsigned i;
        for (i = 0; i < 10; i++){
            if (esc_colors[i] == s.color){
                escape(number(30 + i).c_str());
                break;
            }
        }
        if (i >= 10){
            char b[10];
            sprintf(b, "#%06X", s.color & 0xFFFFFF);
            escape(b);
        }
    }
    QString fontAttr;
    if (curStyle.size != s.size){
        curStyle.size = s.size;
        fontAttr = QString(" size=\"%1\"") .arg(s.size);
    }
    if (curStyle.face != s.face){
        curStyle.face = s.face;
        fontAttr += QString(" face=\"%1\"") .arg(s.face);
    }
    if (!fontAttr.isEmpty()){
        esc += "<font";
        esc += fontAttr.utf8();
        esc += ">";
    }
}

void YahooParser::escape(const char *str)
{
    esc += "\x1B\x5B";
    esc += str;
    esc += "m";
}

void YahooClient::sendMessage(const QString &msgText, Message *msg, YahooUserData *data)
{
    YahooParser p(msgText);
    addParam(0, getLogin().utf8());
    addParam(1, getLogin().utf8());
    addParam(5, data->Login.ptr);
    addParam(14, p.res.c_str());
    if(p.bUtf)
        addParam(97, "1");
    addParam(63, ";0");
    addParam(64, "0");
    sendPacket(YAHOO_SERVICE_MESSAGE, 0x5A55AA56);
    if ((msg->getFlags() & MESSAGE_NOHISTORY) == 0){
        msg->setClient(dataName(data).c_str());
        Event e(EventSent, msg);
        e.process();
    }
    Event e(EventMessageSent, msg);
    e.process();
    delete msg;
}

void YahooClient::sendTyping(YahooUserData *data, bool bState)
{
    addParam(5, data->Login.ptr);
    addParam(4, getLogin().utf8());
    addParam(14, " ");
    addParam(13, bState ? "1" : "0");
    addParam(49, "TYPING");
    sendPacket(YAHOO_SERVICE_NOTIFY, 0x16);
}

void YahooClient::addBuddy(YahooUserData *data)
{
    if ((getState() != Connected) || (data->Group.ptr == NULL))
        return;
    addParam(1, getLogin().utf8());
    addParam(7, data->Login.ptr);
    addParam(65, data->Group.ptr ? data->Group.ptr : "");
    sendPacket(YAHOO_SERVICE_ADDBUDDY);
}

void YahooClient::removeBuddy(YahooUserData *data)
{
    if ((getState() != Connected) || (data->Group.ptr == NULL))
        return;
    addParam(1, getLogin().utf8());
    addParam(7, data->Login.ptr);
    addParam(65, data->Group.ptr ? data->Group.ptr : "");
    sendPacket(YAHOO_SERVICE_REMBUDDY);
    set_str(&data->Group.ptr, NULL);
}

void YahooClient::moveBuddy(YahooUserData *data, const char *grp)
{
    if (getState() != Connected)
        return;
    if (data->Group.ptr == NULL){
        if ((grp == NULL) || (*grp == 0))
            return;
        set_str(&data->Group.ptr, grp);
        addBuddy(data);
        return;
    }
    if ((grp == NULL) || (*grp == 0)){
        removeBuddy(data);
        return;
    }
    if (!strcmp(data->Group.ptr, grp))
        return;
    addParam(1, getLogin().utf8());
    addParam(7, data->Login.ptr);
    addParam(65, grp);
    sendPacket(YAHOO_SERVICE_ADDBUDDY);
    addParam(1, getLogin().utf8());
    addParam(7, data->Login.ptr);
    addParam(65, data->Group.ptr ? data->Group.ptr : "");
    sendPacket(YAHOO_SERVICE_REMBUDDY);
    set_str(&data->Group.ptr, grp);
}

void *YahooClient::processEvent(Event *e)
{
    TCPClient::processEvent(e);
    if (e->type() == EventContactChanged){
        Contact *contact = (Contact*)(e->param());
        string grpName;
        string name;
        name = contact->getName().utf8();
        Group *grp = NULL;
        if (contact->getGroup())
            grp = getContacts()->group(contact->getGroup());
        if (grp)
            grpName = grp->getName().utf8();
        ClientDataIterator it(contact->clientData, this);
        YahooUserData *data;
        while ((data = (YahooUserData*)(++it)) != NULL){
            moveBuddy(data, grpName.c_str());
        }
    }
    if (e->type() == EventContactDeleted){
        Contact *contact = (Contact*)(e->param());
        ClientDataIterator it(contact->clientData, this);
        YahooUserData *data;
        while ((data = (YahooUserData*)(++it)) != NULL){
            removeBuddy(data);
        }
    }
    if (e->type() == EventTemplateExpanded){
        TemplateExpand *t = (TemplateExpand*)(e->param());
        sendStatus(YAHOO_STATUS_CUSTOM, fromUnicode(t->tmpl, NULL).c_str());
    }
    return NULL;
}

QWidget *YahooClient::searchWindow()
{
    return new YahooSearch(this);
}

void YahooClient::setInvisible(bool bState)
{
    if (bState == getInvisible())
        return;
    TCPClient::setInvisible(bState);
    if (getState() != Connected)
        return;
    sendStatus(data.owner.Status.value, data.owner.AwayMessage.ptr);
}

void YahooClient::sendStatus(unsigned long _status, const char *msg)
{
    unsigned long status = _status;
    if (getInvisible())
        status = YAHOO_STATUS_INVISIBLE;
    unsigned long service = YAHOO_SERVICE_ISAWAY;
    if (msg)
        status = YAHOO_STATUS_CUSTOM;
    if (data.owner.Status.value == YAHOO_STATUS_AVAILABLE)
        service = YAHOO_SERVICE_ISBACK;
    addParam(10, number(status).c_str());
    if ((status == YAHOO_STATUS_CUSTOM) && msg) {
        addParam(19, msg);
        addParam(47, "1");
    }
    sendPacket(service);
    if (data.owner.Status.value != status){
        time_t now;
        time(&now);
        data.owner.StatusTime.value = now;
    }
    data.owner.Status.value = _status;
    set_str(&data.owner.AwayMessage.ptr, msg);
}

void YahooClient::sendFile(FileMessage *msg, FileMessage::Iterator &it, YahooUserData *data)
{
    const QString *name = ++it;
    if (name == NULL)
        return;
	QString fn = *name;
#ifdef WIN32
	fn = fn.replace(QRegExp("\\\\"), "/");
#endif
	int n = fn.findRev("/");
	if (n > 0)
		fn = fn.mid(n + 1);
    QString m = msg->getPlainText();
    addParam(0, getLogin().utf8());
    addParam(5, data->Login.ptr);
    addParam(14, fromUnicode(m, data).c_str());
    addParam(27, fromUnicode(fn, data).c_str());
    addParam(28, number(it.size()).c_str());
    sendPacket(YAHOO_SERVICE_FILETRANSFER);

    string url;
    string cookies;
    url = "http://";
    url += getFTServer();
    url += ":";
    url += number(getFTPort());
    url += "/notifyft";
    cookies = "Y=";
    cookies += getCookieY();
    cookies += "; T=";
    cookies += getCookieT();
}

const unsigned MessageYahooFile	= 0x700;

static Message *createYahooFile(const char *cfg)
{
    return new YahooFileMessage(cfg);
}

static MessageDef defYahooFile =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT,
        "File",
        "%n files",
        createYahooFile,
        NULL,
        NULL
    };

static DataDef yahoMessageFile[] =
    {
        { "ServerText", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },				// URL
        { NULL, 0, 0, 0 }
    };

YahooFileMessage::YahooFileMessage(const char *cfg)
        : FileMessage(MessageYahooFile, cfg)
{
    load_data(yahoMessageFile, &data, cfg);
}

YahooFileMessage::~YahooFileMessage()
{
    free_data(yahoMessageFile, &data);
}

string YahooFileMessage::save()
{
    return save_data(yahoMessageFile, &data);
}

QString YahooFileMessage::getText() const
{
    const char *serverText = getMsgText();
    if ((serverText == NULL) || (*serverText == 0))
        return Message::getText();
    return YahooClient::toUnicode(serverText, client(), contact());
}

void YahooPlugin::registerMessages()
{
    Command cmd;
    cmd->id			= MessageYahooFile;
    cmd->text		= "YahooFile";
    cmd->icon		= "file";
    cmd->param		= &defYahooFile;
    Event eMsg(EventCreateMessageType, cmd);
    eMsg.process();
}

void YahooPlugin::unregisterMessages()
{
    Event eFile(EventRemoveMessageType, (void*)MessageYahooFile);
    eFile.process();
}

QTextCodec *YahooClient::getCodec(const char *encoding)
{
    if ((encoding == NULL) || (*encoding == 0))
        encoding = data.owner.Encoding.ptr;
	return _getCodec(encoding);
}

QTextCodec *YahooClient::_getCodec(const char *encoding)
{
    QTextCodec *codec = NULL;
    if (encoding)
        codec = QTextCodec::codecForName(encoding);
    if (codec == NULL){
        codec = QTextCodec::codecForLocale();
        const ENCODING *e;
        for (e = YahooPlugin::core->encodings; e->language; e++){
            if (!strcmp(codec->name(), e->codec))
                break;
        }
        if (e->language && !e->bMain){
            for (e++; e->language; e++){
                if (e->bMain){
                    codec = QTextCodec::codecForName(e->codec);
                    break;
                }
            }
        }
        if (codec == NULL)
            codec= QTextCodec::codecForLocale();
    }
    return codec;
}

QString YahooClient::toUnicode(const char *str, YahooUserData *client_data)
{
    if ((str == NULL) || (*str == 0))
        return QString();
    if (client_data != NULL)
    {
        QTextCodec *codec = getCodec(client_data->Encoding.ptr);
        return codec->toUnicode(str, strlen(str));
    }
    else
    {
        QTextCodec *codec = getCodec(NULL);
        return codec->toUnicode(str, strlen(str));
    }
}

string YahooClient::fromUnicode(const QString &str, YahooUserData *client_data)
{
    string res;
    if (str.isEmpty())
        return res;
    QString s = str;
    s.replace(QRegExp("\r"), "");
    s.replace(QRegExp("\n"), "\r\n");
    QTextCodec *codec = getCodec(client_data ? client_data->Encoding.ptr : NULL);
    QCString cstr = codec->fromUnicode(s);
    res = (const char*)cstr;
    return res;
}

QString YahooClient::toUnicode(const char *serverText, const char *clientName, unsigned contactId)
{
    Contact *c = getContacts()->contact(contactId);
    if (c){
        void *data;
        ClientDataIterator it(c->clientData);
        while ((data = ++it) != NULL){
            if (it.client()->dataName(data) != clientName)
                continue;
            QString res = static_cast<YahooClient*>(it.client())->toUnicode(serverText, (YahooUserData*)data);
            return res.replace(QRegExp("\r"), "");
        }
    }
    QTextCodec *codec = _getCodec(NULL);
    QString res = codec->toUnicode(serverText, strlen(serverText));
    return res.replace(QRegExp("\r"), "");
}

YahooFileTransfer::YahooFileTransfer(FileMessage *msg, YahooUserData *data, YahooClient *client)
: FileTransfer(msg), m_it(*msg)
{
	m_client = client;
	m_data   = data;
	m_client->sendFile(m_msg, m_it, m_data);
}

void YahooFileTransfer::startReceive(unsigned)
{
}

#ifndef WIN32
#include "yahooclient.moc"
#endif




