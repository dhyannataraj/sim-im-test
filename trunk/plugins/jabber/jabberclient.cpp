/***************************************************************************
                          jabberclient.cpp  -  description
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

#include "jabber.h"
#include "jabberclient.h"
#include "jabberconfig.h"
#include "jabber_ssl.h"
#include "jabberadd.h"
#include "jabberinfo.h"
#include "jabberhomeinfo.h"
#include "jabberworkinfo.h"
#include "jabberaboutinfo.h"
#include "jabbermessage.h"
#include "services.h"
#include "html.h"

#include "core.h"

#include <qtimer.h>
#include <qregexp.h>
#include <time.h>

#ifndef WIN32
#include <ctype.h>
#endif

#ifndef XML_STATUS_OK
#define XML_STATUS_OK    1
#define XML_STATUS_ERROR 0
#endif

unsigned PING_TIMEOUT = 50;

/*
typedef struct JabberUserData
{
    char		*ID;
    char		*Resource;
	unsigned	Status;
	char		*FirstName;
	char		*Nick;
	char		*Desc;
	char		*Bday;
	char		*Url;
	char		*OrgName;
	char		*OrgUnit;
	char		*Title;
	char		*Role;
	char		*Street;
	char		*City;
	char		*Region;
	char		*PCode;
	char		*Country;
} JabberUserData;
*/

static DataDef jabberUserData[] =
    {
        { "", DATA_ULONG, 1, JABBER_SIGN },		// Sign
        { "LastSend", DATA_ULONG, 1, 0 },
        { "ID", DATA_UTF, 1, 0 },
        { "Resource", DATA_UTF, 1, 0 },
        { "Name", DATA_UTF, 1, 0 },
        { "", DATA_ULONG, 1, STATUS_OFFLINE },		// Status
        { "FirstName", DATA_UTF, 1, 0 },
        { "Nick", DATA_UTF, 1, 0 },
        { "Desc", DATA_UTF, 1, 0 },
        { "BirthDay", DATA_UTF, 1, 0 },
        { "Url", DATA_UTF, 1, 0 },
        { "OrgName", DATA_UTF, 1, 0 },
        { "OrgUnit", DATA_UTF, 1, 0 },
        { "Role", DATA_UTF, 1, 0 },
        { "Title", DATA_UTF, 1, 0 },
        { "Street", DATA_UTF, 1, 0 },
        { "ExtAddr", DATA_UTF, 1, 0 },
        { "City", DATA_UTF, 1, 0 },
        { "Region", DATA_UTF, 1, 0 },
        { "PCode", DATA_UTF, 1, 0 },
        { "Country", DATA_UTF, 1, 0 },
        { "EMail", DATA_UTF, 1, 0 },
        { "Phone", DATA_UTF, 1, 0 },
        { "", DATA_UTF, 1, 0 },
        { "StatusTime", DATA_ULONG, 1, 0 },
        { "OnlineTime", DATA_ULONG, 1, 0 },
        { "Subscribe", DATA_ULONG, 1, 0 },
        { "Group", DATA_UTF, 1, 0 },
        { "", DATA_BOOL, 1, 0 },			// bChecked
        { "", DATA_STRING, 1, 0 },			// TypingId
        { "", DATA_ULONG, 1, 0 },			// ComposeId
        { "", DATA_ULONG, 1, 0 },			// richText
        { NULL, 0, 0, 0 }
    };

/*
typedef struct JabberClientData
{
    char		*ID;
    char		*Server;
    unsigned	Port;
	unsigned	UseSSL;
	unsigned	UsePlain;
	unsigned	UseVHost;
	unsigned	Register;
	char		*ListRequest;
} JabberClientData;
*/
static DataDef jabberClientData[] =
    {
        { "Server", DATA_STRING, 1, (unsigned)"jabber.org" },
        { "Port", DATA_ULONG, 1, 5222 },
        { "UseSSL", DATA_BOOL, 1, 0 },
        { "UsePlain", DATA_BOOL, 1, 0 },
        { "UseVHost", DATA_BOOL, 1, 0 },
        { "", DATA_BOOL, 1, 0 },
        { "Priority", DATA_ULONG, 1, 5 },
        { "ListRequest", DATA_UTF, 1, 0 },
        { "VHost", DATA_UTF, 1, 0 },
        { "Typing", DATA_BOOL, 1, 1 },
        { "ProtocolIcons", DATA_BOOL, 1, 1 },
        { "", DATA_STRUCT, sizeof(JabberUserData) / sizeof(unsigned), (unsigned)jabberUserData },
        { NULL, 0, 0, 0 }
    };

JabberClient::JabberClient(JabberProtocol *protocol, const char *cfg)
        : TCPClient(protocol, cfg)
{
    load_data(jabberClientData, &data, cfg);
    QString jid = QString::fromUtf8(data.owner.ID);
    if (!jid.isEmpty() && (jid.find('@') < 0)){
        QString server;
        if (data.UseVHost && data.VHost && *data.VHost){
            server = QString::fromUtf8(data.VHost);
        }else if (data.Server){
            server = QString::fromUtf8(data.Server);
        }
        if (!server.isEmpty()){
            jid += "@";
            jid += server;
        }
        set_str(&data.owner.ID, jid.utf8());
    }
    if (data.owner.Resource == NULL){
        string resource = PACKAGE;
        resource += "_";
        resource += VERSION;
#ifdef WIN32
        resource += "/win32";
#endif
        set_str(&data.owner.Resource, resource.c_str());
    }

    QString listRequests = getListRequest();
    while (!listRequests.isEmpty()){
        QString item = getToken(listRequests, ';', false);
        JabberListRequest lr;
        lr.bDelete = false;
        lr.jid = getToken(item, ',').utf8();
        lr.grp = getToken(item, ',').utf8();
        if (!item.isEmpty())
            lr.bDelete = true;
        m_listRequests.push_back(lr);
    }
    setListRequest(NULL);

    m_bXML		 = false;
    m_bSSL		 = false;
    m_curRequest = NULL;
    m_msg_id	 = 0;
    init();
}

JabberClient::~JabberClient()
{
    TCPClient::setStatus(STATUS_OFFLINE, false);
    free_data(jabberClientData, &data);
    freeData();
}

const DataDef *JabberProtocol::userDataDef()
{
    return jabberUserData;
}

bool JabberClient::compareData(void *d1, void *d2)
{
    JabberUserData *data1 = (JabberUserData*)d1;
    JabberUserData *data2 = (JabberUserData*)d2;
    return strcmp(data1->ID, data2->ID) == 0;
}

void JabberClient::setID(const QString &id)
{
    set_str(&data.owner.ID, id.utf8());
}

string JabberClient::getConfig()
{
    QString lr;
    for (list<JabberListRequest>::iterator it = m_listRequests.begin(); it != m_listRequests.end(); ++it){
        if (!lr.isEmpty())
            lr += ";";
        lr += quoteChars(QString::fromUtf8((*it).jid.c_str()), ",;");
        lr += ",";
        lr += quoteChars(QString::fromUtf8((*it).grp.c_str()), ",;");
        if ((*it).bDelete)
            lr += ",1";
    }
    setListRequest(lr);

    string res = Client::getConfig();
    if (res.length())
        res += "\n";
    return res += save_data(jabberClientData, &data);
}

string JabberClient::name()
{
    string res = "Jabber.";
    if (data.owner.ID)
        res += data.owner.ID;
    return res;
}

QWidget	*JabberClient::setupWnd()
{
    return new JabberConfig(NULL, this, false);
}

bool JabberClient::isMyData(clientData *&_data, Contact *&contact)
{
    if (_data->Sign != JABBER_SIGN)
        return false;
    JabberUserData *data = (JabberUserData*)_data;
    JabberUserData *my_data = findContact(data->ID, NULL, false, contact);
    if (my_data){
        data = my_data;
    }else{
        contact = NULL;
    }
    return true;
}

bool JabberClient::createData(clientData *&_data, Contact *contact)
{
    JabberUserData *data = (JabberUserData*)_data;
    JabberUserData *new_data = (JabberUserData*)(contact->clientData.createData(this));
    set_str(&new_data->ID, data->ID);
    _data = (clientData*)new_data;
    return true;
}

void JabberClient::connect_ready()
{
    if (!getUseSSL() || m_bSSL){
        connected();
        return;
    }
#ifdef USE_OPENSSL
    m_bSSL = true;
    SSLClient *ssl = new JabberSSL(m_socket->socket());
    m_socket->setSocket(ssl);
    if (!ssl->init()){
        m_socket->error_state("SSL init error");
        return;
    }
    ssl->connect();
    ssl->process();
#endif
}

void JabberClient::connected()
{
    m_socket->readBuffer.init(0);
    m_socket->readBuffer.packetStart();
    m_socket->setRaw(true);
    log(L_DEBUG, "Connect ready");
    startHandshake();
    TCPClient::connect_ready();
    if (!m_bXML){
        m_parser = XML_ParserCreate("UTF-8");
        XML_SetUserData(m_parser, this);
        XML_SetElementHandler(m_parser, p_element_start, p_element_end);
        XML_SetCharacterDataHandler(m_parser, p_char_data);
        m_bXML = true;
    }
}

void JabberClient::packet_ready()
{
    if (m_socket->readBuffer.writePos() == 0)
        return;
    JabberPlugin *plugin = static_cast<JabberPlugin*>(protocol()->plugin());
    log_packet(m_socket->readBuffer, false, plugin->JabberPacket);
    if (XML_Parse(m_parser,
                  m_socket->readBuffer.data(), m_socket->readBuffer.writePos(), false) != XML_STATUS_OK)
        m_socket->error_state("XML parse error");
    m_socket->readBuffer.init(0);
    m_socket->readBuffer.packetStart();
}

static bool cmp(const string &str, const char *s)
{
    if (s == NULL)
        return str.empty();
    return str == s;
}

void *JabberClient::processEvent(Event *e)
{
    if (e->type() == EventTemplateExpanded){
        TemplateExpand *t = (TemplateExpand*)(e->param());
        setStatus((unsigned)(t->param), quoteString(t->tmpl, false).utf8());
    }
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
        JabberUserData *data;
        while ((data = (JabberUserData*)(++it)) != NULL){
            if (!cmp(grpName, data->Group)){
                listRequest(data, name.c_str(), grpName.c_str(), false);
                continue;
            }
            if (data->Name && *data->Name){
                if (!cmp(name, data->Name))
                    listRequest(data, name.c_str(), grpName.c_str(), false);
                continue;
            }
            if (!cmp(name, data->ID))
                listRequest(data, name.c_str(), grpName.c_str(), false);
        }
        return NULL;
    }
    if (e->type() == EventContactDeleted){
        Contact *contact = (Contact*)(e->param());
        ClientDataIterator it(contact->clientData, this);
        JabberUserData *data;
        while ((data = (JabberUserData*)(++it)) != NULL){
            listRequest(data, NULL, NULL, true);
        }
        return NULL;
    }
    if (e->type() == EventGroupChanged){
        Group *grp = (Group*)(e->param());
        string grpName;
        grpName = grp->getName().utf8();
        ContactList::ContactIterator itc;
        Contact *contact;
        while ((contact = ++itc) != NULL){
            if (contact->getGroup() != grp->id())
                continue;
            ClientDataIterator it(contact->clientData, this);
            JabberUserData *data;
            while ((data = (JabberUserData*)(++it)) != NULL){
                if (!cmp(grpName, data->Group))
                    listRequest(data, contact->getName().utf8(), grpName.c_str(), false);
            }
        }
    }
    return NULL;
}

void JabberClient::setStatus(unsigned status)
{
    if (status == STATUS_OFFLINE){
        setStatus(status, NULL);
        return;
    }
    if (getInvisible()){
        if (m_status != status){
            m_status = status;
            Event e(EventClientChanged, static_cast<Client*>(this));
            e.process();
        }
        return;
    }
    if (status == STATUS_ONLINE){
        setStatus(status, NULL);
        return;
    }
    ARRequest ar;
    ar.contact  = NULL;
    ar.status   = status;
    ar.receiver = this;
    ar.param	= (void*)status;
    Event e(EventARRequest, &ar);
    e.process();
}

void JabberClient::setStatus(unsigned status, const char *ar)
{
    if (status  != m_status){
        time_t now;
        time(&now);
        data.owner.StatusTime = now;
        if (m_status == STATUS_OFFLINE)
            data.owner.OnlineTime = now;
        m_status = status;
        m_socket->writeBuffer.packetStart();
        string priority = number(getPriority());
        const char *show = NULL;
        const char *type = NULL;
        if (getInvisible()){
            type = "invisible";
        }else{
            switch (status){
            case STATUS_AWAY:
                show = "away";
                break;
            case STATUS_NA:
                show = "xa";
                break;
            case STATUS_DND:
                show = "dnd";
                break;
            case STATUS_FFC:
                show = "chat";
                break;
            case STATUS_OFFLINE:
                priority = "";
                type = "unavailable";
                break;
            }
        }
        m_socket->writeBuffer << "<presence";
        if (type)
            m_socket->writeBuffer << " type=\"" << type << "\"";
        m_socket->writeBuffer << ">\n";
        if (show && *show)
            m_socket->writeBuffer << "<show>" << show << "</show>\n";
        if (ar && *ar){
            m_socket->writeBuffer << "<status>" << ar << "</status>\n";
        }
        if (!priority.empty())
            m_socket->writeBuffer << "<priority>" << priority.c_str() << "</priority>\n";
        m_socket->writeBuffer << "</presence>";
        sendPacket();
        Event e(EventClientChanged, static_cast<Client*>(this));
        e.process();
    }
    if (status == STATUS_OFFLINE){
        if (m_socket){
            m_socket->writeBuffer.packetStart();
            m_socket->writeBuffer
            << "</stream:stream>\n";
            sendPacket();
        }
        Contact *contact;
        ContactList::ContactIterator it;
        time_t now;
        time(&now);
        data.owner.StatusTime = now;
        while ((contact = ++it) != NULL){
            JabberUserData *data;
            ClientDataIterator it(contact->clientData, this);
            while ((data = (JabberUserData*)(++it)) != NULL){
                if (data->Status == STATUS_OFFLINE)
                    continue;
                data->StatusTime = now;
                setOffline(data);
                StatusMessage m;
                m.setContact(contact->id());
                m.setClient(dataName(data).c_str());
                m.setFlags(MESSAGE_RECEIVED);
                m.setStatus(STATUS_OFFLINE);
                Event e(EventMessageReceived, &m);
                e.process();
            }
        }
    }
}

void JabberClient::disconnected()
{
    if (m_bXML){
        XML_ParserFree(m_parser);
        m_bXML = false;
    }
    for (list<ServerRequest*>::iterator it = m_requests.begin(); it != m_requests.end(); ++it)
        delete *it;
    m_requests.clear();
    if (m_curRequest){
        delete m_curRequest;
        m_curRequest = NULL;
    }
    init();
}

void JabberClient::init()
{
    m_id = "";
    m_depth = 0;
    m_id_seed = 0xAAAA;
    m_bSSL = false;
}

void JabberClient::sendPacket()
{
    JabberPlugin *plugin = static_cast<JabberPlugin*>(protocol()->plugin());
    log_packet(m_socket->writeBuffer, true, plugin->JabberPacket);
    m_socket->write();
}

void JabberClient::p_element_start(void *data, const char *el, const char **attr)
{
    ((JabberClient*)data)->element_start(el, attr);
}

void JabberClient::p_element_end(void *data, const char *el)
{
    ((JabberClient*)data)->element_end(el);
}

void JabberClient::p_char_data(void *data, const char *str, int len)
{
    ((JabberClient*)data)->char_data(str, len);
}

string JabberClient::get_attr(const char *name, const char **attr)
{
    for (const char **p = attr; *p; ){
        string tag = to_lower(*(p++));
        if (tag == name){
            return *p;
        }
    }
    return "";
}

void JabberClient::element_start(const char *el, const char **attr)
{
    string element = to_lower(el);
    const char *id = NULL;
    if (m_depth){
        if (m_curRequest){
            m_curRequest->element_start(element.c_str(), attr);
        }else{
            if (element == "iq"){
                string id = get_attr("id", attr);
                if (id.empty()){
                    m_curRequest = new IqRequest(this);
                    m_curRequest->element_start(element.c_str(), attr);
                }else{
                    list<ServerRequest*>::iterator it;
                    for (it = m_requests.begin(); it != m_requests.end(); ++it){
                        if ((*it)->m_id == id)
                            break;
                    }
                    if (it != m_requests.end()){
                        m_curRequest = *it;
                        m_requests.erase(it);
                        m_curRequest->element_start(element.c_str(), attr);
                    }else{
                        log(L_DEBUG, "Request ID %s not found", id.c_str());
                    }
                }
            }else if (element == "presence"){
                m_curRequest = new PresenceRequest(this);
                m_curRequest->element_start(element.c_str(), attr);
            }else if (element == "message"){
                m_curRequest = new MessageRequest(this);
                m_curRequest->element_start(element.c_str(), attr);
            }else{
                log(L_DEBUG, "Bad tag %s", element.c_str());
            }
        }
    }else{
        if (element == "stream:stream"){
            for (const char **p = attr; *p; ){
                string tag = to_lower(*(p++));
                if (tag == "id"){
                    id = *p;
                    break;
                }
            }
        }
        handshake(id);
    }
    m_depth++;
}

void JabberClient::element_end(const char *el)
{
    m_depth--;
    if (m_curRequest){
        string element = to_lower(el);
        m_curRequest->element_end(element.c_str());
        if (m_depth == 1){
            delete m_curRequest;
            m_curRequest = NULL;
        }
    }
}

void JabberClient::char_data(const char *str, int len)
{
    if (m_curRequest)
        m_curRequest->char_data(str, len);
}

string JabberClient::get_unique_id()
{
    char b[10];
    sprintf(b, "a%x", m_id_seed);
    m_id_seed += 0x10;
    return b;
}

JabberClient::ServerRequest::ServerRequest(JabberClient *client, const char *type,
        const char *from, const char *to)
{
    m_client = client;
    if (type == NULL)
        return;
    m_id  = m_client->get_unique_id();
    m_client->m_socket->writeBuffer.packetStart();
    m_client->m_socket->writeBuffer
    << "<iq type=\"" << type << "\" id=\""
    << m_id.c_str()
    << "\"";;
    if (from)
        m_client->m_socket->writeBuffer << " from=\"" << from << "\"";
    if (to)
        m_client->m_socket->writeBuffer << " to=\"" << to << "\"";
    m_client->m_socket->writeBuffer << ">\n";
}

JabberClient::ServerRequest::~ServerRequest()
{
}

void JabberClient::ServerRequest::send()
{
    end_element(false);
    while (!m_els.empty()){
        end_element(false);
    }
    m_client->m_socket->writeBuffer
    << "</iq>\n";
    m_client->sendPacket();
}

void JabberClient::ServerRequest::element_start(const char*, const char**)
{
}

void JabberClient::ServerRequest::element_end(const char*)
{
}

void JabberClient::ServerRequest::char_data(const char*, int)
{
}

void JabberClient::ServerRequest::start_element(const char *name)
{
    end_element(true);
    m_client->m_socket->writeBuffer
    << "<" << name;
    m_element = name;
}

void JabberClient::ServerRequest::add_attribute(const char *name, const char *value)
{
    m_client->m_socket->writeBuffer
    << " " << name << "=\"" << JabberClient::encodeXML(value) << "\"";
}

void JabberClient::ServerRequest::end_element(bool bNewLevel)
{
    if (bNewLevel){
        if (m_element.length()){
            m_client->m_socket->writeBuffer << ">\n";
            m_els.push(m_element);
        }
    }else{
        if (m_element.length()){
            m_client->m_socket->writeBuffer << "/>\n";
        }else if (m_els.size()){
            m_element = m_els.top();
            m_els.pop();
            m_client->m_socket->writeBuffer << "</" << m_element.c_str() << ">\n";
        }
    }
    m_element = "";
}

void JabberClient::ServerRequest::text_tag(const char *name, const char *value)
{
    if ((value == NULL) || (*value == 0))
        return;
    end_element(true);
    m_client->m_socket->writeBuffer
    << "<" << name << ">"
    << JabberClient::encodeXML(QString::fromUtf8(value))
    << "</" << name << ">\n";
}

void JabberClient::ServerRequest::add_condition(const char *condition)
{
    QString cond = QString::fromUtf8(condition);
    while (cond.length()){
        QString item = getToken(cond, ';');
        QString key = getToken(item, '=');
        text_tag(key.utf8(), item.utf8());
    }
}

const char *JabberClient::ServerRequest::_GET = "get";
const char *JabberClient::ServerRequest::_SET = "set";

void JabberClient::startHandshake()
{
    m_socket->writeBuffer.packetStart();
    m_socket->writeBuffer
    << "<stream:stream to=\""
    << encodeXML(getServer())
    << "\" xmlns=\"jabber:client\" xmlns:stream=\"http://etherx.jabber.org/streams\">\n";
    sendPacket();
}

void JabberClient::handshake(const char *id)
{
    if (id == NULL){
        m_socket->error_state("Bad session ID");
        return;
    }
    m_id = id;
    if (getRegister()){
        auth_register();
    }else{
#ifdef USE_OPENSSL
        if (getUsePlain()){
            auth_plain();
        }else{
            auth_digest();
        }
#else
        auth_plain();
#endif
    }
}

void JabberClient::auth_ok()
{
    if (getRegister()){
        setRegister(false);
        setClientStatus(STATUS_OFFLINE);
        TCPClient::setStatus(getManualStatus(), getCommonStatus());
        return;
    }
    setState(Connected);
    setPreviousPassword(NULL);
    rosters_request();
    info_request(NULL);
    setStatus(m_logonStatus);
    QTimer::singleShot(PING_TIMEOUT * 1000, this, SLOT(ping()));
}

void JabberClient::auth_failed()
{
    m_reconnect = NO_RECONNECT;
    m_socket->error_state(I18N_NOOP("Login failed"), AuthError);
}

string JabberClient::to_lower(const char *s)
{
    string res;
    if (s == NULL)
        return res;
    for (; *s; s++)
        res += (char)tolower(*s);
    return res;
}

QCString JabberClient::encodeXML(const QString &str)
{
    return quoteString(str, false).utf8();
}

JabberUserData *JabberClient::findContact(const char *alias, const char *name, bool bCreate, Contact *&contact)
{
    char *resource = NULL;
    char *p = strchr((char*)alias, '@');
    if (p){
        p = strchr(p, '/');
        if (p){
            *p = 0;
            resource = p + 1;
        }
    }
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        JabberUserData *data;
        ClientDataIterator it(contact->clientData, this);
        while ((data = (JabberUserData*)(++it)) != NULL){
            if (strcmp(data->ID, alias))
                continue;
            if (resource)
                set_str(&data->Resource, resource);
            if (name)
                set_str(&data->Name, name);
            return data;
        }
    }
    if (!bCreate)
        return NULL;
    it.reset();
    QString sname;
    if (name && *name){
        sname = QString::fromUtf8(name).lower();
    }else{
        sname = QString::fromUtf8(alias).lower();
        int pos = sname.find('@');
        if (pos > 0)
            sname = sname.left(pos);
    }
    while ((contact = ++it) != NULL){
        if (contact->getName().lower() == sname.lower()){
            JabberUserData *data = (JabberUserData*)(contact->clientData.createData(this));
            set_str(&data->ID, alias);
            if (resource)
                set_str(&data->Resource, resource);
            if (name)
                set_str(&data->Name, name);
            info_request(data);
            Event e(EventContactChanged, contact);
            e.process();
            return data;
        }
    }
    contact = getContacts()->contact(0, true);
    JabberUserData *data = (JabberUserData*)(contact->clientData.createData(this));
    set_str(&data->ID, alias);
    if (resource)
        set_str(&data->Resource, resource);
    if (name)
        set_str(&data->Name, name);
    contact->setName(sname);
    info_request(data);
    Event e(EventContactChanged, contact);
    e.process();
    return data;
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


void JabberClient::contactInfo(void *_data, unsigned long &curStatus, unsigned &style, const char *&statusIcon, string *icons)
{
    JabberUserData *data = (JabberUserData*)_data;
    const CommandDef *def = protocol()->statusList();
    for (; def->text; def++){
        if (def->id == data->Status)
            break;
    }
    if ((def == NULL) || (def->text == NULL))
        return;
    const char *dicon = def->icon;
    if (getProtocolIcons()){
        char *host = strchr(data->ID, '@');
        if (host){
            string h = host + 1;
            char *p = strchr((char*)(h.c_str()), '.');
            if (p)
                *p = 0;
            if (strcmp(h.c_str(), "icq") == 0){
                switch (data->Status){
                case STATUS_ONLINE:
                    dicon = "ICQ_online";
                    break;
                case STATUS_OFFLINE:
                    dicon = "ICQ_offline";
                    break;
                case STATUS_AWAY:
                    dicon = "ICQ_away";
                    break;
                case STATUS_NA:
                    dicon = "ICQ_na";
                    break;
                case STATUS_DND:
                    dicon = "ICQ_dnd";
                    break;
                case STATUS_FFC:
                    dicon = "ICQ_ffc";
                    break;
                }
            }else if (strcmp(h.c_str(), "aim") == 0){
                switch (data->Status){
                case STATUS_ONLINE:
                    dicon = "AIM_online";
                    break;
                case STATUS_OFFLINE:
                    dicon = "AIM_offline";
                    break;
                case STATUS_AWAY:
                    dicon = "AIM_away";
                    break;
                }
            }else if (strcmp(h.c_str(), "msn") == 0){
                switch (data->Status){
                case STATUS_ONLINE:
                    dicon = "MSN_online";
                    break;
                case STATUS_OFFLINE:
                    dicon = "MSN_offline";
                    break;
                case STATUS_AWAY:
                    dicon = "MSN_away";
                    break;
                case STATUS_NA:
                    dicon = "MSN_na";
                    break;
                case STATUS_DND:
                    dicon = "MSN_dnd";
                    break;
                }
            }
        }
    }
    if (data->Status > curStatus){
        curStatus = data->Status;
        if (statusIcon && icons){
            string iconSave = *icons;
            *icons = statusIcon;
            if (iconSave.length())
                addIcon(icons, iconSave.c_str(), statusIcon);
        }
        statusIcon = dicon;
    }else{
        if (statusIcon){
            addIcon(icons, dicon, statusIcon);
        }else{
            statusIcon = dicon;
        }
    }
    if (((data->Subscribe & SUBSCRIBE_TO) == 0) && !isAgent(data->ID))
        style |= CONTACT_UNDERLINE;
    if (icons && data->TypingId && *data->TypingId)
        addIcon(icons, "typing", statusIcon);
}

string JabberClient::buildId(JabberUserData *data)
{
    string res = data->ID;
    return res;
}

QWidget *JabberClient::searchWindow()
{
    return new JabberAdd(this);
}

void JabberClient::ping()
{
    if (getState() != Connected)
        return;
    m_socket->writeBuffer.packetStart();
    m_socket->writeBuffer << "\n";
    sendPacket();
    QTimer::singleShot(PING_TIMEOUT * 1000, this, SLOT(ping()));
}

QString JabberClient::contactName(void *clientData)
{
    QString res = Client::contactName(clientData);
    res += ": ";
    JabberUserData *data = (JabberUserData*)clientData;
    QString name;
    if (data->ID)
        name = QString::fromUtf8(data->ID);
    if (data->Nick && *data->Nick){
        res += QString::fromUtf8(data->Nick);
        res += " (";
        res += name;
        res += ")";
    }else{
        res += name;
    }
    return res;
}


QString JabberClient::contactTip(void *_data)
{
    JabberUserData *data = (JabberUserData*)_data;
    QString res;
    QString statusText;
    unsigned long status = STATUS_OFFLINE;
    unsigned style  = 0;
    const char *statusIcon = NULL;
    contactInfo(data, status, style, statusIcon);
    if (statusIcon){
        res += "<img src=\"icon:";
        res += statusIcon;
        res += "\">";
        const char *pStatus = strchr(statusIcon, '_');
        if (pStatus){
            pStatus++;
        }else{
            pStatus = statusIcon;
        }
        for (const CommandDef *cmd = protocol()->statusList(); cmd->text; cmd++){
            const char *pCmd = strchr(cmd->icon, '_');
            if (pCmd){
                pCmd++;
            }else{
                pCmd = cmd->icon;
            }
            if (!strcmp(pCmd, pStatus)){
                res += " ";
                statusText += i18n(cmd->text);
                res += statusText;
                break;
            }
        }
        res += "<br>";
    }
    res += "ID: <b>";
    res += QString::fromUtf8(data->ID);
    res += "</b>";
    if (data->Status == STATUS_OFFLINE){
        if (data->StatusTime){
            res += "<br><font size=-1>";
            res += i18n("Last online");
            res += ": </font>";
            res += formatTime(data->StatusTime);
        }
    }else{
        if (data->OnlineTime){
            res += "<br><font size=-1>";
            res += i18n("Online");
            res += ": </font>";
            res += formatTime(data->OnlineTime);
        }
        if (data->StatusTime != data->OnlineTime){
            res += "<br><font size=-1>";
            res += statusText;
            res += ": </font>";
            res += formatTime(data->StatusTime);
        }
    }
    if (data->Resource && *data->Resource){
        res += "<br>";
        res += QString::fromUtf8(data->Resource);
    }
    if (data->AutoReply && *data->AutoReply){
        res += "<br><br>";
        res += QString::fromUtf8(data->AutoReply);
    }
    return res;
}

void JabberClient::setOffline(JabberUserData *data)
{
    data->Status = STATUS_OFFLINE;
    data->composeId = 0;
    if (data->TypingId && *data->TypingId){
        set_str(&data->TypingId, NULL);
        Contact *contact;
        if (findContact(data->ID, NULL, false, contact)){
            Event e(EventContactStatus, contact);
            e.process();
        }
    }
}

const unsigned MAIN_INFO = 1;
const unsigned HOME_INFO = 2;
const unsigned WORK_INFO = 3;
const unsigned ABOUT_INFO = 4;
const unsigned SERVICES	 = 5;
const unsigned NETWORK	 = 6;

static CommandDef jabberWnd[] =
    {
        {
            MAIN_INFO,
            "",
            "Jabber_online",
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
            HOME_INFO,
            I18N_NOOP("Home info"),
            "home",
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
            WORK_INFO,
            I18N_NOOP("Work info"),
            "work",
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
            ABOUT_INFO,
            I18N_NOOP("About info"),
            "info",
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

static CommandDef cfgJabberWnd[] =
    {
        {
            MAIN_INFO,
            "",
            "Jabber_online",
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
            HOME_INFO,
            I18N_NOOP("Home info"),
            "home",
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
            WORK_INFO,
            I18N_NOOP("Work info"),
            "work",
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
            ABOUT_INFO,
            I18N_NOOP("About info"),
            "info",
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
            SERVICES,
            I18N_NOOP("Agents"),
            "configure",
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

CommandDef *JabberClient::infoWindows(Contact*, void *_data)
{
    JabberUserData *data = (JabberUserData*)_data;
    QString name = i18n(protocol()->description()->text);
    name += " ";
    name += QString::fromUtf8(data->ID);
    jabberWnd[0].text_wrk = strdup(name.utf8());
    return jabberWnd;
}

CommandDef *JabberClient::configWindows()
{
    QString name = i18n(protocol()->description()->text);
    name += " ";
    name += QString::fromUtf8(data.owner.ID);
    cfgJabberWnd[0].text_wrk = strdup(name.utf8());
    return cfgJabberWnd;
}

QWidget *JabberClient::infoWindow(QWidget *parent, Contact*, void *_data, unsigned id)
{
    JabberUserData *data = (JabberUserData*)_data;
    switch (id){
    case MAIN_INFO:
        return new JabberInfo(parent, data, this);
    case HOME_INFO:
        return new JabberHomeInfo(parent, data, this);
    case WORK_INFO:
        return new JabberWorkInfo(parent, data, this);
    case ABOUT_INFO:
        return new JabberAboutInfo(parent, data, this);
    }
    return NULL;
}

QWidget *JabberClient::configWindow(QWidget *parent, unsigned id)
{
    switch (id){
    case MAIN_INFO:
        return new JabberInfo(parent, NULL, this);
    case HOME_INFO:
        return new JabberHomeInfo(parent, NULL, this);
    case WORK_INFO:
        return new JabberWorkInfo(parent, NULL, this);
    case ABOUT_INFO:
        return new JabberAboutInfo(parent, NULL, this);
    case NETWORK:
        return new JabberConfig(parent, this, true);
    case SERVICES:
        return new Services(parent, this);
    }
    return NULL;
}

void JabberClient::updateInfo(Contact *contact, void *data)
{
    if (getState() != Connected){
        Client::updateInfo(contact, data);
        return;
    }
    if (data == NULL)
        data = &this->data.owner;
    info_request((JabberUserData*)data);
}

bool JabberClient::canSend(unsigned type, void *_data)
{
    if ((_data == NULL) || (((clientData*)_data)->Sign != JABBER_SIGN))
        return false;
    if (getState() != Connected)
        return false;
    JabberUserData *data = (JabberUserData*)_data;
    switch (type){
    case MessageGeneric:
        return true;
    case MessageAuthRequest:
        return ((data->Subscribe & SUBSCRIBE_TO) == 0) && !isAgent(data->ID);
    case MessageAuthGranted:
        return ((data->Subscribe & SUBSCRIBE_FROM) == 0) && !isAgent(data->ID);
    case MessageAuthRefused:
        return (data->Subscribe & SUBSCRIBE_FROM) && !isAgent(data->ID);
    case MessageJabberOnline:
        return isAgent(data->ID) && (data->Status == STATUS_OFFLINE);
    case MessageJabberOffline:
        return isAgent(data->ID) && (data->Status != STATUS_OFFLINE);
    }
    return false;
}

class JabberImageParser : public HTMLParser
{
public:
    JabberImageParser(unsigned bgColor);
    QString parse(const QString &text);
protected:
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &attrs);
    virtual void tag_end(const QString &tag);
	void startBody(const list<QString> &attrs);
	void endBody();
    QString res;
    bool		m_bPara;
	bool		m_bBody;
	unsigned	m_bgColor;
};

JabberImageParser::JabberImageParser(unsigned bgColor)
{
    m_bPara    = false;
	m_bBody    = true;
	m_bgColor  = bgColor;
}

QString JabberImageParser::parse(const QString &text)
{
	list<QString> attrs;
    startBody(attrs);
    HTMLParser::parse(text);
	endBody();
    return res;
}

void JabberImageParser::text(const QString &text)
{
	if (m_bBody)
		res += quoteString(text);
}

static const char *_tags[] =
{
	"abbr",
	"acronym",
	"address",
	"blockquote",
	"cite",
	"code",
	"dfn",
	"div",
	"em",
	"h1",
	"h2",
	"h3",
	"h4",
	"h5",
	"h6",
	"kbd",
	"p",
	"pre",
	"q",
	"samp",
	"span",
	"strong",
	"var",
	"a",
	"dl",
	"dt",
	"dd",
	"ol",
	"ul",
	"li",
	NULL
};

static const char *_styles[] =
{
	"color",
	"font-family",
	"font-size",
	"font-style",
	"font-weight",
	"text-align",
	"text-decoration",
	NULL
};

void JabberImageParser::startBody(const list<QString> &attrs)
{
		m_bBody = true;
		res = "<body>";
		list<QString> newStyles;
		list<QString>::const_iterator it;
		for (it = attrs.begin(); it != attrs.end(); ++it){
			QString name = *it;
			++it;
			QString value = *it;
			if (name == "style"){
				list<QString> styles = parseStyle(value);
				for (list<QString>::iterator it = styles.begin(); it != styles.end(); ++it){
					QString name = *it;
					++it;
					QString value = *it;
					for (const char **s = _styles; *s; s++){
						if (name == *s){
							newStyles.push_back(name);
							newStyles.push_back(value);
							break;
						}
					}
				}
			}
		}
		for (it = newStyles.begin(); it != newStyles.end(); ++it){
			QString name = *it;
			++it;
			if (name == "background-color")
				break;
		}
		if (it == newStyles.end()){
			char b[15];
			sprintf(b, "#%06X", m_bgColor & 0xFFFFFF);
			newStyles.push_back("background-color");
			newStyles.push_back(b);
		}
		res += "<span style=\'";
		res += makeStyle(newStyles);
		res += "\'>";
}

void JabberImageParser::endBody()
{
	if (m_bBody){
		res += "</span></body>";
		m_bBody = false;
	}
}

void JabberImageParser::tag_start(const QString &tag, const list<QString> &attrs)
{
	if (tag == "html"){
		m_bBody = false;
		res = "";
		return;
	}
	if (tag == "body"){
		startBody(attrs);
		return;
	}
	if (!m_bBody)
		return;
    if (tag == "img"){
        QString src;
        for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
            QString name = *it;
            ++it;
            QString value = *it;
            if (name == "src"){
                src = value;
                break;
            }
        }
        if (src.left(10) != "icon:smile")
            return;
        bool bOK;
        unsigned nIcon = src.mid(10).toUInt(&bOK, 16);
        if (!bOK)
            return;
        const smile *p = smiles(nIcon);
        if (p){
            res += p->paste;
            return;
        }
    }
    if (tag == "p"){
        if (m_bPara){
            res += "<br/>";
            m_bPara = false;
        }
        return;
    }
    if (tag == "br"){
        res += "<br/>";
        return;
    }
	for (const char **t = _tags; *t; t++){
		if (tag == *t){
		    res += "<";
			res += tag;
			for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
				QString name = *it;
				++it;
				QString value = *it;
				if (name == "style"){
					list<QString> styles = parseStyle(value);
					list<QString> newStyles;
					for (list<QString>::iterator it = styles.begin(); it != styles.end(); ++it){
						QString name = *it;
						++it;
						QString value = *it;
						for (const char **s = _styles; *s; s++){
							if (name == *s){
								newStyles.push_back(name);
								newStyles.push_back(value);
								break;
							}
						}
					}
					value = makeStyle(newStyles);
				}
				if ((name != "style") && (name != "href"))
					continue;
				res += " ";
				res += name;
				if (!value.isEmpty()){
					res += "=\'";
					res += quoteString(value);
					res += "\'";
				}
			}
			res += ">";
			return;
		}
	}
	if (tag == "b"){
		res += "<span style=\'font-weight:bold\'>";
		return;
	}
	if (tag == "i"){
		res += "<span style=\'font-style:italic\'>";
		return;
	}
	if (tag == "u"){
		res += "<span style=\'text-decoration:underline\'>";
		return;
	}
	if (tag == "font"){
		res += "<span";
		string style;
		for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
			QString name = *it;
			++it;
			QString value = *it;
			if (name == "color"){
				if (!style.empty())
					style += ";";
				style += "color: ";
				style += value.utf8();
				continue;
			}
		}
		if (!style.empty()){
			res += " style=\'";
			res += style.c_str();
			res += "\'";
		}
		res += ">";
		return;
	}
	return;
}

void JabberImageParser::tag_end(const QString &tag)
{
	if (tag == "body"){
		endBody();
		return;
	}
	if (!m_bBody)
		return;
    if (tag == "p"){
        m_bPara = true;
        return;
    }
	for (const char **t = _tags; *t; t++){
		if (tag == *t){
		    res += "</";
			res += tag;
			res += ">";
			return;
		}
	}
	if ((tag == "b") || (tag == "i") || (tag == "u") || (tag == "font")){
		res += "</span>";
		return;
	}
}

static QString removeImages(const QString &text, unsigned bgColor)
{
    JabberImageParser p(bgColor);
    return p.parse(text);
}

bool JabberClient::send(Message *msg, void *_data)
{
    if ((getState() != Connected) || (_data == NULL))
        return false;
    JabberUserData *data = (JabberUserData*)_data;
    switch (msg->type()){
    case MessageAuthRefused:{
            string grp;
            Group *group = NULL;
            Contact *contact = getContacts()->contact(msg->contact());
            if (contact && contact->getGroup())
                group = getContacts()->group(contact->getGroup());
            if (group)
                grp = group->getName().utf8();
            listRequest(data, data->Name, grp.c_str(), false);
            if (data->Subscribe & SUBSCRIBE_FROM){
                m_socket->writeBuffer.packetStart();
                m_socket->writeBuffer
                << "<presence to=\""
                << data->ID;
                m_socket->writeBuffer
                << "\" type=\"unsubscribed\"><status>"
                << (const char*)(quoteString(msg->getPlainText(), false).utf8())
                << "</status></presence>";
                sendPacket();
                if ((msg->getFlags() & MESSAGE_NOHISTORY) == 0){
                    msg->setClient(dataName(data).c_str());
                    Event e(EventSent, msg);
                    e.process();
                }
                Event e(EventMessageSent, msg);
                e.process();
                delete msg;
                return true;
            }
        }
    case MessageGeneric:{
            Contact *contact = getContacts()->contact(msg->contact());
            if ((contact == NULL) || (data == NULL))
                return false;
            m_socket->writeBuffer.packetStart();
            m_socket->writeBuffer
            << "<message type=\"chat\" to=\""
            << data->ID;
            m_socket->writeBuffer
            << "\"><body>"
            << (const char*)msg->getPlainText().utf8()
            << "</body>";
            if (data->richText){
                m_socket->writeBuffer
                << "<html xmlns='http://jabber.org/protocol/xhtml-im'>"
                << removeImages(msg->getRichText(), msg->getBackground()).utf8()
                << "</html>";
            }
            m_socket->writeBuffer
            << "</message>";
            sendPacket();
            if ((msg->getFlags() & MESSAGE_NOHISTORY) == 0){
                msg->setClient(dataName(data).c_str());
                Event e(EventSent, msg);
                e.process();
            }
            Event e(EventMessageSent, msg);
            e.process();
            delete msg;
            return true;
        }
    case MessageAuthRequest:{
            m_socket->writeBuffer.packetStart();
            m_socket->writeBuffer
            << "<presence to=\""
            << data->ID;
            m_socket->writeBuffer
            << "\" type=\"subscribe\"><status>"
            << (const char*)(quoteString(msg->getPlainText(), false).utf8())
            << "</status></presence>";
            sendPacket();
            if ((msg->getFlags() & MESSAGE_NOHISTORY) == 0){
                msg->setClient(dataName(data).c_str());
                Event e(EventSent, msg);
                e.process();
            }
            Event e(EventMessageSent, msg);
            e.process();
            delete msg;
            return true;
        }
    case MessageAuthGranted:{
            m_socket->writeBuffer.packetStart();
            m_socket->writeBuffer
            << "<presence to=\""
            << data->ID;
            m_socket->writeBuffer
            << "\" type=\"subscribed\"></presence>";
            sendPacket();
            if ((msg->getFlags() & MESSAGE_NOHISTORY) == 0){
                msg->setClient(dataName(data).c_str());
                Event e(EventSent, msg);
                e.process();
            }
            Event e(EventMessageSent, msg);
            e.process();
            delete msg;
            return true;
        }
    case MessageJabberOnline:
        if (isAgent(data->ID) && (data->Status == STATUS_OFFLINE)){
            m_socket->writeBuffer.packetStart();
            m_socket->writeBuffer
            << "<presence to=\""
            << data->ID
            << "\"></presence>";
            sendPacket();
            delete msg;
            return true;
        }
        break;
    case MessageJabberOffline:
        if (isAgent(data->ID) && (data->Status != STATUS_OFFLINE)){
            m_socket->writeBuffer.packetStart();
            m_socket->writeBuffer
            << "<presence to=\""
            << data->ID
            << "\" type=\"unavailable\"></presence>";
            sendPacket();
            delete msg;
            return true;
        }
        break;
    case MessageTypingStart:
        if (getTyping()){
            data->composeId = ++m_msg_id;
            string msg_id = "msg";
            msg_id += number(data->composeId);
            m_socket->writeBuffer.packetStart();
            m_socket->writeBuffer
            << "<message to=\""
            << data->ID
            << "\"><x xmlns='jabber:x:event'><composing/><id>"
            << msg_id.c_str()
            << "</id></x></message>";
            sendPacket();
            delete msg;
            return true;
        }
        break;
    case MessageTypingStop:
        if (getTyping()){
            if (data->composeId == 0)
                return false;
            string msg_id = "msg";
            msg_id += number(data->composeId);
            m_socket->writeBuffer.packetStart();
            m_socket->writeBuffer
            << "<message to=\""
            << data->ID
            << "\"><x xmlns='jabber:x:event'><id>"
            << msg_id.c_str()
            << "</id></x></message>";
            sendPacket();
            delete msg;
            return true;
        }
        break;
    }
    return false;
}

string JabberClient::dataName(void *_data)
{
    QString res(name().c_str());
    JabberUserData *data = (JabberUserData*)_data;
    res += "+";
    res += data->ID;
    res = res.replace(QRegExp("/"), "_");
    return string(res.utf8());
}

void JabberClient::listRequest(JabberUserData *data, const char *name, const char *grp, bool bDelete)
{
    string jid = data->ID;
    list<JabberListRequest>::iterator it;
    for (it = m_listRequests.begin(); it != m_listRequests.end(); ++it){
        if (jid == (*it).jid){
            m_listRequests.erase(it);
            break;
        }
    }
    JabberListRequest lr;
    lr.jid = jid;
    if (name)
        lr.name = name;
    if (grp)
        lr.grp = grp;
    lr.bDelete = bDelete;
    m_listRequests.push_back(lr);
    processList();
}

JabberListRequest *JabberClient::findRequest(const char *jid, bool bRemove)
{
    list<JabberListRequest>::iterator it;
    for (it = m_listRequests.begin(); it != m_listRequests.end(); ++it){
        if ((*it).jid == jid){
            if (bRemove){
                m_listRequests.erase(it);
                return NULL;
            }
            return &(*it);
        }
    }
    return NULL;
}


bool JabberClient::isAgent(const char *jid)
{
    const char *p = strrchr(jid, '/');
    if (p && !strcmp(p + 1, "registered"))
        return true;
    return false;
}


void JabberClient::auth_request(const char *jid, unsigned type, const char *text, bool bCreate)
{
    Contact *contact;
    JabberUserData *data = findContact(jid, NULL, false, contact);
    if (isAgent(jid)){
        switch (type){
        case MessageAuthRequest:{
                if (data == NULL)
                    data = findContact(jid, NULL, true, contact);
                m_socket->writeBuffer.packetStart();
                m_socket->writeBuffer
                << "<presence to=\""
                << data->ID
                << "\" type=\"subscribed\"></presence>";
                sendPacket();
                m_socket->writeBuffer.packetStart();
                m_socket->writeBuffer
                << "<presence to=\""
                << data->ID
                << "\" type=\"subscribe\"><status>"
                << "</status></presence>";
                sendPacket();
                Event e(EventContactChanged, contact);
                e.process();
                return;
            }
        case MessageAuthGranted:{
                if (data == NULL)
                    data = findContact(jid, NULL, true, contact);
                Event e(EventContactChanged, contact);
                e.process();
                return;
            }

        }
    }
    if ((data == NULL) && bCreate){
        data = findContact(jid, NULL, true, contact);
        contact->setTemporary(CONTACT_TEMP);
    }
    if (data == NULL)
        return;
    AuthMessage msg(type);
    msg.setContact(contact->id());
    msg.setClient(dataName(data).c_str());
    msg.setFlags(MESSAGE_RECEIVED);
    if (text)
        msg.setText(unquoteString(QString::fromUtf8(text)));
    Event e(EventMessageReceived, &msg);
    e.process();
}

void JabberClient::setInvisible(bool bState)
{
    if (getInvisible() == bState)
        return;
    TCPClient::setInvisible(bState);
    if (getStatus() == STATUS_OFFLINE)
        return;
    unsigned status = getStatus();
    m_status = STATUS_OFFLINE;
    if (getInvisible()){
        setStatus(status, NULL);
        return;
    }
    setStatus(status);
}

string JabberClient::VHost()
{
    if (data.UseVHost && data.VHost && *data.VHost)
        return data.VHost;
    return data.Server;
}

#ifndef WIN32
#include "jabberclient.moc"
#endif

