/***************************************************************************
                          livejournal.cpp  -  description
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

#include "livejournal.h"
#include "livejournalcfg.h"
#include "msgjournal.h"
#include "journalsearch.h"
#include "fetch.h"
#include "html.h"
#include "core.h"

#ifdef USE_OPENSSL
#include <openssl/md5.h>
#else
#include "md5.h"
#endif

#ifdef TM_IN_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif

#include <qtimer.h>
#include <qregexp.h>

#include "xpm/livejournal.xpm"
#include "xpm/livejournal_off.xpm"
#include "xpm/livejournal_upd.xpm"

Plugin *createLiveJournalPlugin(unsigned base, bool, const char*)
{
    Plugin *plugin = new LiveJournalPlugin(base);
    return plugin;
}

static PluginInfo info =
    {
        NULL,
        NULL,
        VERSION,
        createLiveJournalPlugin,
        PLUGIN_PROTOCOL
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static DataDef journalMessageData[] =
    {
        { "Subject", DATA_UTF, 1, 0 },
        { "Private", DATA_ULONG, 1, 0 },
        { "Time", DATA_ULONG, 1, 0 },
        { "ItemID", DATA_ULONG, 1, 0 },
        { "", DATA_ULONG, 1, 0 },		// oldID
        { "Mood", DATA_ULONG, 1, 0 },
        { NULL, 0, 0, 0 }
    };

JournalMessage::JournalMessage(const char *cfg)
        : Message(MessageJournal, cfg)
{
    load_data(journalMessageData, &data, cfg);
}

JournalMessage::~JournalMessage()
{
    free_data(journalMessageData, &data);
}

string JournalMessage::save()
{
    string cfg = Message::save();
    string my_cfg = save_data(journalMessageData, &data);
    if (!my_cfg.empty()){
        if (!cfg.empty())
            cfg += "\n";
        cfg += my_cfg;
    }
    return cfg;
}

QString JournalMessage::presentation()
{
    QString subj = getSubject();
    QString res;
    if (!subj.isEmpty())
        res = i18n("<p>Subject: %1</p>") .arg(subj);
    res += Message::presentation();
    return res;
}

#if 0
i18n("LiveJournal post", "%n LiveJournal posts", 1);
i18n("Friends updated", "Friends updated %n", 1);
#endif

static Message *createJournalMessage(const char *cfg)
{
    return new JournalMessage(cfg);
}

static QObject* generateJournalMessage(MsgEdit *w, Message *msg)
{
    return new MsgJournal(w, msg);
}

static CommandDef journalMessageCommands[] =
    {
        {
            CmdDeleteJournalMessage,
            I18N_NOOP("&Remove from journal"),
            "remove",
            NULL,
            NULL,
            ToolBarMsgEdit,
            0x1080,
            MenuMessage,
            0,
            0,
            COMMAND_DEFAULT,
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
            COMMAND_DEFAULT,
            NULL,
            NULL
        }
    };

static MessageDef defJournalMessage =
    {
        NULL,
        journalMessageCommands,
        MESSAGE_SENDONLY,
        "LiveJournal post",
        "%n LiveJournal posts",
        createJournalMessage,
        generateJournalMessage,
        NULL
    };

static MessageDef defWWWJournalMessage =
    {
        NULL,
        NULL,
        MESSAGE_SENDONLY | MESSAGE_SILENT,
        NULL,
        NULL,
        createJournalMessage,
        NULL,
        NULL
    };

static Message *createUpdatedMessage(const char *cfg)
{
    return new Message(MessageUpdated, cfg);
}

static MessageDef defUpdatedMessage =
    {
        NULL,
        NULL,
        MESSAGE_SYSTEM,
        "Friends updated",
        "Friends updated %n",
        createUpdatedMessage,
        NULL,
        NULL
    };

CorePlugin *LiveJournalPlugin::core = NULL;
unsigned LiveJournalPlugin::MenuCount = 0;

LiveJournalPlugin::LiveJournalPlugin(unsigned base)
        : Plugin(base)
{
    IconDef icon;
    icon.name = "livejournal";
    icon.xpm  = livejournal;
    icon.isSystem = false;

    Event eIcon(EventAddIcon, &icon);
    eIcon.process();

    icon.name = "livejournal_off";
    icon.xpm  = livejournal_off;
    eIcon.process();

    icon.name = "livejournal_upd";
    icon.xpm  = livejournal_upd;
    eIcon.process();

    m_protocol = new LiveJournalProtocol(this);

    Event eMenu(EventMenuCreate, (void*)MenuWeb);
    eMenu.process();

    Command cmd;
    cmd->id			 = CmdMenuWeb;
    cmd->text		 = "_";
    cmd->menu_id	 = MenuWeb;
    cmd->menu_grp	 = 0x1000;
    cmd->flags		 = COMMAND_CHECK_STATE;
    Event eCmd(EventCommandCreate, cmd);
    eCmd.process();

    cmd->id			 = MessageJournal;
    cmd->text		 = I18N_NOOP("LiveJournal &post");
    cmd->icon		 = "livejournal";
    cmd->accel		 = "Ctrl+P";
    cmd->menu_grp	 = 0x3080;
    cmd->flags		 = COMMAND_DEFAULT;
    cmd->param		 = &defJournalMessage;
    Event eMsg(EventCreateMessageType, cmd);
    eMsg.process();

    cmd->id			 = CmdMenuWeb;
    cmd->text		 = I18N_NOOP("LiveJournal &WWW");
    cmd->icon		 = NULL;
    cmd->accel		 = NULL;
    cmd->menu_grp	 = 0x3090;
    cmd->popup_id	 = MenuWeb;
    cmd->flags		 = COMMAND_DEFAULT;
    cmd->param		 = &defWWWJournalMessage;
    eMsg.process();

    cmd->id			 = MessageUpdated;
    cmd->text		 = I18N_NOOP("Friends updated");
    cmd->icon		 = "livejournal_upd";
    cmd->accel		 = NULL;
    cmd->menu_grp	 = 0;
    cmd->popup_id	 = 0;
    cmd->flags		 = COMMAND_DEFAULT;
    cmd->param		 = &defUpdatedMessage;
    eMsg.process();

    Event ePlugin(EventGetPluginInfo, (void*)"_core");
    pluginInfo *info = (pluginInfo*)(ePlugin.process());
    core = static_cast<CorePlugin*>(info->plugin);
}

LiveJournalPlugin::~LiveJournalPlugin()
{
    Event eMenu(EventMenuRemove, (void*)MenuWeb);
    eMenu.process();
    Event eMsg(EventRemoveMessageType, (void*)MessageJournal);
    eMsg.process();
    Event eMsgWWW(EventRemoveMessageType, (void*)CmdMenuWeb);
    eMsgWWW.process();
    Event eMsgUpd(EventRemoveMessageType, (void*)MessageUpdated);
    eMsgUpd.process();
    delete m_protocol;
}

LiveJournalProtocol::LiveJournalProtocol(Plugin *plugin)
        : Protocol(plugin)
{
}

LiveJournalProtocol::~LiveJournalProtocol()
{
}

Client *LiveJournalProtocol::createClient(const char *cfg)
{
    return new LiveJournalClient(this, cfg);
}

static CommandDef livejournal_descr =
    {
        0,
        I18N_NOOP("LiveJournal"),
        "livejournal",
        NULL,
        NULL,
        0,
        0,
        0,
        0,
        0,
        PROTOCOL_NOSMS | PROTOCOL_SEARCH,
        NULL,
        NULL
    };

const CommandDef *LiveJournalProtocol::description()
{
    return &livejournal_descr;
}

static CommandDef livejournal_status_list[] =
    {
        {
            STATUS_ONLINE,
            I18N_NOOP("Online"),
            "livejournal",
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
            STATUS_OFFLINE,
            I18N_NOOP("Offline"),
            "livejournal_off",
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
        }
    };

const CommandDef *LiveJournalProtocol::statusList()
{
    return livejournal_status_list;
}

static DataDef liveJournalUserData[] =
    {
        { "", DATA_ULONG, 1, LIVEJOURNAL_SIGN },		// Sign
        { "LastSend", DATA_ULONG, 1, 0 },
        { "User", DATA_UTF, 1, 0 },
        { "Shared", DATA_BOOL, 1, 0 },
        { "", DATA_BOOL, 1, 0 },						// bChecked
        { NULL, 0, 0, 0 }
    };

/*
	char		*Server;
	char		*URL;
	unsigned	Port;
	unsigned	Interval;
*/

static DataDef liveJournalClientData[] =
    {
        { "Server", DATA_STRING, 1, (unsigned)"www.livejournal.com" },
        { "URL", DATA_STRING, 1, (unsigned)"/interface/flat" },
        { "Port", DATA_ULONG, 1, 80 },
        { "Interval", DATA_ULONG, 1, 5 },
        { "Mood", DATA_STRLIST, 1, 0 },
        { "Moods", DATA_ULONG, 1, 0 },
        { "Menu", DATA_STRLIST, 1, 0 },
        { "MenuURL", DATA_STRLIST, 1, 0 },
        { "FastServer", DATA_BOOL, 1, 0 },
        { "", DATA_STRING, 1, 0 },			// LastUpdate
        { "", DATA_STRUCT, sizeof(LiveJournalUserData) / sizeof(unsigned), (unsigned)liveJournalUserData },
        { NULL, 0, 0, 0 }
    };

const DataDef *LiveJournalProtocol::userDataDef()
{
    return liveJournalUserData;
}

LiveJournalClient::LiveJournalClient(Protocol *proto, const char *cfg)
        : TCPClient(proto, cfg)
{
    load_data(liveJournalClientData, &data, cfg);
    m_request = NULL;
    m_fetchId = 0;
    m_timer   = new QTimer(this);
}

LiveJournalClient::~LiveJournalClient()
{
    if (m_request)
        delete m_request;
    free_data(liveJournalClientData, &data);
}

string LiveJournalClient::getConfig()
{
    string cfg = TCPClient::getConfig();
    string my_cfg = save_data(liveJournalClientData, &data);
    if (!my_cfg.empty()){
        if (!cfg.empty())
            cfg += "\n";
        cfg += my_cfg;
    }
    return cfg;
}

class MessageRequest : public LiveJournalRequest
{
public:
    MessageRequest(LiveJournalClient *client, JournalMessage *msg, const char *journal);
    ~MessageRequest();
protected:
    void result(const char *key, const char *value);
    JournalMessage *m_msg;
    string		m_err;
    unsigned	m_id;
    bool		m_bResult;
    bool		m_bEdit;
};

class BRParser : public HTMLParser
{
public:
    BRParser(unsigned color);
    QString m_str;
    void parse(const QString&);
protected:
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &options);
    virtual void tag_end(const QString &tag);
    virtual void add_color();
    bool m_bSkip;
    unsigned m_color;
};

BRParser::BRParser(unsigned color)
{
    m_color = color;
    m_bSkip = false;
    add_color();
}

void BRParser::parse(const QString &str)
{
    HTMLParser::parse(str);
    m_str += "</span>";
}

void BRParser::text(const QString &text)
{
    if (m_bSkip)
        return;
    QString s = text;
    s = s.replace(QRegExp("\r"), "");
    s = s.replace(QRegExp("\n"), "");
    m_str += s;
}

void BRParser::tag_start(const QString &tag, const list<QString> &attrs)
{
    if (m_bSkip)
        return;
    if (tag == "body"){
        m_str = "";
        add_color();
        return;
    }
    if (tag == "p"){
        return;
    }
    if (tag == "br"){
        m_str += "</span>\n";
        add_color();
        return;
    }
    m_str += "<";
    m_str += tag;
    for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
        QString name = *it;
        ++it;
        QString value = *it;
        m_str += " ";
        m_str += name;
        if (!value.isEmpty()){
            m_str += "=\'";
            m_str += quoteString(value);
            m_str += "\'";
        }
    }
    m_str += ">";
}

void BRParser::tag_end(const QString &tag)
{
    if (m_bSkip)
        return;
    if (tag == "body"){
        m_bSkip = true;
        return;
    }
    if (tag == "p"){
        m_str += "</span>\n";
        add_color();
        return;
    }
    m_str += "</";
    m_str += tag;
    m_str += ">";
}

void BRParser::add_color()
{
    QString s;
    s.sprintf("<span style=\"background-color:#%06X\">", m_color & 0xFFFFFF);
    m_str += s;
}

MessageRequest::MessageRequest(LiveJournalClient *client, JournalMessage *msg, const char *journal)
        : LiveJournalRequest(client, msg->getID() ? "editevent" : "postevent")
{
    m_msg = msg;
    m_bEdit   = (msg->getID() != 0);
    m_bResult = false;
    if (msg->getRichText().isEmpty()){
        addParam("event", "");
    }else{
        BRParser parser(msg->getBackground());
        parser.parse(msg->getRichText());
        addParam("event", parser.m_str.utf8());
        addParam("subject", msg->getSubject());
    }
    addParam("lineendings", "unix");
    if (msg->getID())
        addParam("itemid", number(msg->getID()).c_str());
    if (msg->getTime() == 0){
        time_t now;
        time(&now);
        msg->setTime(now);
    }
    time_t now;
    now = msg->getTime();
    struct tm *tm = localtime(&now);
    addParam("year", number(tm->tm_year + 1900).c_str());
    addParam("mon", number(tm->tm_mon + 1).c_str());
    addParam("day", number(tm->tm_mday).c_str());
    addParam("hour", number(tm->tm_hour).c_str());
    addParam("min", number(tm->tm_min).c_str());
    if (msg->getMood()){
        string value = "current_mood_id=";
        value += number(msg->getMood());
        addParam("prop_name", value.c_str());
    }
    if (journal)
        addParam("usejournal", journal);
}

MessageRequest::~MessageRequest()
{
    m_bResult = true;
    if (m_bResult){
        if ((m_msg->getFlags() & MESSAGE_NOHISTORY) == 0){
            if (m_bEdit){
                m_msg->setId(m_msg->getOldID());
                if (m_msg->getRichText().isEmpty()){
                    Event e(EventDeleteMessage, m_msg);
                    e.process();
                }else{
                    Event e(EventRewriteMessage, m_msg);
                    e.process();
                }
            }else{
                m_msg->setID(m_id);
                Event e(EventSent, m_msg);
                e.process();
            }
        }
    }else{
        if (m_err.empty())
            m_err = I18N_NOOP("Posting failed");
        m_msg->setError(m_err.c_str());
    }
    Event e(EventMessageSent, m_msg);
    e.process();
    delete m_msg;
}

void MessageRequest::result(const char *key, const char *value)
{
    if (!strcmp(key, "errmsg"))
        m_err = value;
    if (!strcmp(key, "success") && !strcmp(value, "OK"))
        m_bResult = true;
    if (!strcmp(key, "itemid"))
        m_id = atol(value);
}

bool LiveJournalClient::send(Message *msg, void *_data)
{
    if (!canSend(msg->type(), _data))
        return false;
    LiveJournalUserData *data = (LiveJournalUserData*)_data;
    const char *journal = NULL;
    if (data->User && strcmp(data->User, this->data.owner.User))
        journal = data->User;
    m_requests.push_back(new MessageRequest(this, static_cast<JournalMessage*>(msg), journal));
    msg->setClient(dataName(_data).c_str());
    send();
    return true;
}

bool LiveJournalClient::canSend(unsigned type, void *_data)
{
    if ((_data == NULL) || (((clientData*)_data)->Sign != LIVEJOURNAL_SIGN))
        return false;
    if (type == MessageJournal){
        if (getState() != Connected)
            return false;
        return true;
    }
    if (type == CmdMenuWeb){
        LiveJournalUserData *data = (LiveJournalUserData*)_data;
        if ((data->User == NULL) || strcmp(data->User, this->data.owner.User))
            return false;
        return true;
    }
    return false;
}

void LiveJournalClient::setupContact(Contact*, void*)
{
}

bool LiveJournalClient::createData(clientData*&, Contact*)
{
    return false;
}

bool LiveJournalClient::isMyData(clientData *&data, Contact*&)
{
    if (data->Sign != LIVEJOURNAL_SIGN)
        return false;
    return false;
}

string LiveJournalClient::dataName(void *data)
{
    string res = name();
    res += ".";
    res += ((LiveJournalUserData*)data)->User;
    return res;
}

string LiveJournalClient::name()
{
    string res;
    res = "LiveJournal.";
    if (data.owner.User)
        res += data.owner.User;
    return res;
}

QWidget	*LiveJournalClient::setupWnd()
{
    return new LiveJournalCfg(NULL, this, true);
}

void LiveJournalClient::socketConnect()
{
    connect_ready();
    setStatus(STATUS_ONLINE);
}

const unsigned MAIN_INFO = 1;

static CommandDef cfgLiveJournalWnd[] =
    {
        {
            MAIN_INFO,
            "",
            "livejournal",
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
        }
    };

CommandDef *LiveJournalClient::configWindows()
{
    QString title = QString::fromUtf8(name().c_str());
    int n = title.find(".");
    if (n > 0)
        title = title.left(n) + " " + title.mid(n + 1);
    cfgLiveJournalWnd[0].text_wrk = strdup(title.utf8());
    return cfgLiveJournalWnd;
}

QWidget *LiveJournalClient::configWindow(QWidget *parent, unsigned id)
{
    if (id == MAIN_INFO)
        return new LiveJournalCfg(parent, this, false);
    return NULL;
}

bool LiveJournalClient::add(const char *name)
{
    Contact *contact;
    LiveJournalUserData *data = findContact(name, contact, false);
    if (data)
        return false;
    findContact(name, contact);
    return true;
}

LiveJournalUserData *LiveJournalClient::findContact(const char *user, Contact *&contact, bool bCreate)
{
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        LiveJournalUserData *data;
        ClientDataIterator itc(contact->clientData, this);
        while ((data = (LiveJournalUserData*)(++itc)) != NULL){
            if (!strcmp(data->User, user))
                return data;
        }
    }
    QString sname = QString::fromUtf8(user);
    it.reset();
    while ((contact = ++it) != NULL){
        if (contact->getName().lower() == sname.lower())
            break;;
    }
    if (contact == NULL){
        if (!bCreate)
            return NULL;
        contact = getContacts()->contact(0, true);
        contact->setName(sname);
    }
    LiveJournalUserData *data = (LiveJournalUserData*)(contact->clientData.createData(this));
    set_str(&data->User, user);
    Event e(EventContactChanged, contact);
    e.process();
    return data;
}

void LiveJournalClient::auth_ok()
{
    m_status = STATUS_ONLINE;
    setState(Connected);
    setPreviousPassword(NULL);
    statusChanged();
    list<Contact*> forDelete;
    Contact *contact;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        LiveJournalUserData *data;
        ClientDataIterator itc(contact->clientData, this);
        while ((data = (LiveJournalUserData*)(++itc)) != NULL){
            if (!data->bChecked)
                continue;
            if (data->bChecked)
                continue;
            contact->clientData.freeData(data);
            if (contact->clientData.size() == 0)
                forDelete.push_back(contact);
            break;
        }
    }
    for (list<Contact*>::iterator itc = forDelete.begin(); itc != forDelete.end(); ++itc)
        delete (*itc);
    QTimer::singleShot(0, this, SLOT(timeout()));
}

void LiveJournalClient::statusChanged()
{
    Contact *contact = NULL;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        ClientDataIterator itc(contact->clientData, this);
        if ((++itc) != NULL){
            Event e(EventContactChanged, contact);
            e.process();
        }
    }
    findContact(data.owner.User, contact);
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

void LiveJournalClient::contactInfo(void*, unsigned long &curStatus, unsigned&, const char *&statusIcon, string *icons)
{
    unsigned long status = STATUS_OFFLINE;
    const char *dicon = "livejournal_off";
    if ((getState() == Connected) && (m_status != STATUS_OFFLINE)){
        status = STATUS_ONLINE;
        dicon = "livejournal";
    }
    if (status > curStatus){
        curStatus = status;
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
}

typedef struct Mood
{
    unsigned	id;
    string		name;
} Mood;

class LoginRequest : public LiveJournalRequest
{
public:
    LoginRequest(LiveJournalClient *client);
    ~LoginRequest();
protected:
    void result(const char *key, const char *value);
    bool m_bOK;
    bool m_bResult;
    vector<Mood> m_moods;
    string m_err;
};

LoginRequest::LoginRequest(LiveJournalClient *client)
: LiveJournalRequest(client, "login")
{
    m_bOK     = false;
    m_bResult = false;
}

LoginRequest::~LoginRequest()
{
    if (m_bOK){
        for (unsigned i = 0; i < m_moods.size(); i++){
            if (m_moods[i].name.empty())
                continue;
            if (m_client->getMoods() < m_moods[i].id)
                m_client->setMoods(m_moods[i].id);
            m_client->setMood(i, m_moods[i].name.c_str());
        }
        m_client->auth_ok();
    }else{
        if (!m_bResult)
            return;
        if (m_err.empty())
            m_err = I18N_NOOP("Login failed");
        m_client->auth_fail(m_err.c_str());
    }
    Event e(EventClientChanged, m_client);
    e.process();
}

void LoginRequest::result(const char *key, const char *value)
{
    m_bResult = true;
    if (!strcmp(key, "success") && !strcmp(value, "OK")){
        m_bOK = true;
        return;
    }
    if (!strcmp(key, "errmsg")){
        m_err = value;
        return;
    }
    string k = key;
    string prefix = getToken(k, '_');
    if (prefix == "mood"){
        prefix = getToken(k, '_');
        unsigned id = atol(prefix.c_str());
        if (id == 0)
            return;
        while (m_moods.size() <= id){
            Mood m;
            m_moods.push_back(m);
        }
        if (k == "id")
            m_moods[id].id = atol(value);
        if (k == "name")
            m_moods[id].name = value;
    }
    if (prefix == "menu"){
        prefix = getToken(k, '_');
        unsigned menu_id = atol(prefix.c_str());
        prefix = getToken(k, '_');
        unsigned item_id = atol(prefix.c_str());
        if (item_id == 0)
            return;
        unsigned id = menu_id * 0x100 + item_id;
        if (k == "text")
            m_client->setMenu(id, value);
        if (k == "url")
            m_client->setMenuUrl(id, value);
        if (k == "sub"){
            string v = "@";
            v += value;
            m_client->setMenuUrl(id, v.c_str());
        }
    }
    if (prefix == "access"){
        if (atol(k.c_str()) == 0)
            return;
        Contact *contact;
        LiveJournalUserData *data = m_client->findContact(value, contact);
        if (data){
            data->bChecked = true;
            data->Shared   = true;
        }
    }
}

void LiveJournalClient::setStatus(unsigned status)
{
    if (status == STATUS_OFFLINE)
        return;
    Contact *contact;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        LiveJournalUserData *data;
        ClientDataIterator itc(contact->clientData, this);
        while ((data = (LiveJournalUserData*)(++itc)) != NULL){
            data->bChecked = false;
            if (data->User && this->data.owner.User && !strcmp(data->User, this->data.owner.User))
                data->bChecked = true;
        }
    }
    LiveJournalRequest *req = new LoginRequest(this);
    string version;
#ifdef WIN32
    version = "Win32";
#else
#ifdef QT_MACOSX_VERSION
    version = "MacOS";
#else
    version = "Qt";
#endif
#endif
    version += "-" PACKAGE "/" VERSION;
    req->addParam("clientversion", version.c_str());
    req->addParam("getmoods", number(getMoods()).c_str());
    req->addParam("getmenus", "1");
    m_requests.push_back(req);
    send();
}

void LiveJournalClient::disconnected()
{
    m_timer->stop();
    statusChanged();
}

void LiveJournalClient::packet_ready()
{
}

void LiveJournalClient::auth_fail(const char *err)
{
    m_reconnect = NO_RECONNECT;
    error_state(err, AuthError);
}

QWidget *LiveJournalClient::searchWindow()
{
    return new JournalSearch(this);
}

void *LiveJournalClient::processEvent(Event *e)
{
    if (e->type() == EventOpenMessage){
        Message *msg = (Message*)(e->param());
        if (msg->type() != MessageUpdated)
            return NULL;
        if (dataName(&data.owner) != msg->client())
            return NULL;
        Event eDel(EventMessageDeleted, msg);
        eDel.process();
        string url = "http://";
        url += getServer();
        if (getPort() != 80){
            url += ":";
            url += number(getPort());
        }
        url += "/";
        Event eGo(EventGoURL, (void*)url.c_str());
        eGo.process();
        if (getState() == Connected)
            m_timer->start(getInterval() * 60 * 1000, true);
        return e->param();
    }
    if (e->type() == EventFetchDone){
        fetchData *data = (fetchData*)e->param();
        if (data->req_id != m_fetchId)
            return NULL;
        m_fetchId = 0;
        if (data->result == 200){
            m_request->result(data->data);
        }else{
            string err = "Fetch error ";
            err += number(data->result);
            error_state(err.c_str(), 0);
            statusChanged();
        }
        delete m_request;
        m_request = NULL;
        send();
    }
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->id == CmdDeleteJournalMessage){
            Message *msg = (Message*)(cmd->param);
            Contact *contact = getContacts()->contact(msg->contact());
            if (contact == NULL)
                return NULL;
            LiveJournalUserData *data;
            ClientDataIterator it(contact->clientData, this);
            while ((data = (LiveJournalUserData*)(++it)) != NULL){
                if (dataName(data) == msg->client()){
                    JournalMessage *m = new JournalMessage(msg->save().c_str());
                    m->setContact(msg->contact());
                    m->setOldID(msg->id());
                    m->setText("");
                    if (!send(m, data))
                        delete m;
                    return e->param();
                }
            }
            return NULL;
        }
        unsigned menu_id = cmd->menu_id - MenuWeb;
        if (menu_id > LiveJournalPlugin::MenuCount)
            return NULL;
        unsigned item_id = cmd->id - CmdMenuWeb;
        if ((item_id == 0) || (item_id >= 0x100))
            return NULL;
        const char *url = getMenuUrl(menu_id * 0x100 + item_id);
        if ((url == NULL) || (*url == 0))
            return NULL;
        Event eUrl(EventGoURL, (void*)url);
        eUrl.process();
        return e->param();
    }
    if (e->type() == EventCheckState){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->id == CmdMenuWeb){
            unsigned menu_id = cmd->menu_id - MenuWeb;
            if (menu_id > LiveJournalPlugin::MenuCount)
                return NULL;
            unsigned nItems = 0;
            unsigned list_id = menu_id * 0x100 + 1;
            for (;;){
                const char *text = getMenu(list_id);
                if ((text == NULL) || (*text == 0))
                    break;
                nItems++;
                list_id++;
            }
            if (nItems == 0)
                return NULL;
            CommandDef *cmds = new CommandDef[nItems + 1];
            memset(cmds, 0, sizeof(CommandDef) * (nItems + 1));
            list_id = menu_id * 0x100 + 1;
            for (unsigned i = 0;; i++){
                const char *text = getMenu(list_id);
                if ((text == NULL) || (*text == 0))
                    break;
                cmds[i].text = "_";
                if (strcmp(text, "-")){
                    cmds[i].id = CmdMenuWeb + i + 1;
                    cmds[i].text = "_";
                    QString s = i18n(text);
                    cmds[i].text_wrk = strdup(s.utf8());
                    const char *url = getMenuUrl(list_id);
                    if (url && (*url == '@')){
                        unsigned nSub = atol(url + 1);
                        while (nSub > LiveJournalPlugin::MenuCount){
                            unsigned menu_id = MenuWeb + (++LiveJournalPlugin::MenuCount);
                            Event eMenu(EventMenuCreate, (void*)menu_id);
                            eMenu.process();
                            Command cmd;
                            cmd->id       = CmdMenuWeb;
                            cmd->text     = "_";
                            cmd->menu_id  = menu_id;
                            cmd->menu_grp = 0x1000;
                            cmd->flags    = COMMAND_CHECK_STATE;
                            Event e(EventCommandCreate, cmd);
                            e.process();
                        }
                        cmds[i].popup_id = MenuWeb + nSub;
                    }
                }else{
                    cmds[i].id = 0;
                }
                list_id++;
            }
            cmd->param = cmds;
            cmd->flags |= COMMAND_RECURSIVE;
            return e->param();
        }
    }
    return NULL;
}

void LiveJournalClient::send()
{
    if ((m_requests.size() == 0) || m_request)
        return;
    m_request = m_requests.front();
    m_requests.erase(m_requests.begin());
    string url;
    url = "http://";
    url += getServer();
    if (getPort() != 80){
        url += ":";
        url += number(getPort());
    }
    url += getURL();
    string headers = "Content-Type: application/x-www-form-urlencoded\r\n"
                     "Content-Length: ";
    headers += number(m_request->m_buffer.size());
    if (getFastServer())
        headers += "\r\nCookie: ljfastserver=1";
    m_fetchId = fetch(this, url.c_str(), &m_request->m_buffer);
}

bool LiveJournalClient::error_state(const char *err, unsigned code)
{
    return TCPClient::error_state(err, code);
}

class CheckFriendsRequest : public LiveJournalRequest
{
public:
    CheckFriendsRequest(LiveJournalClient *client);
    ~CheckFriendsRequest();
protected:
    void result(const char *key, const char *value);
    bool		m_bOK;
    bool		m_bChanged;
    unsigned	m_interval;
    string		m_err;
};

CheckFriendsRequest::CheckFriendsRequest(LiveJournalClient *client)
        : LiveJournalRequest(client, "checkfriends")
{
    m_bOK		= false;
    m_bChanged	= false;
    m_interval	= 0;
    addParam("lastupdate", client->getLastUpdate());
}

void LiveJournalClient::messageUpdated()
{
    Contact *contact;
    LiveJournalUserData *data = findContact(this->data.owner.User, contact);
    if (data == NULL)
        return;
    Message *msg = new Message(MessageUpdated);
    msg->setContact(contact->id());
    msg->setClient(dataName(data).c_str());
    msg->setFlags(MESSAGE_TEMP | MESSAGE_NOVIEW);
    Event e(EventMessageReceived, msg);
    if (!e.process())
        delete msg;
}

CheckFriendsRequest::~CheckFriendsRequest()
{
    if (m_bChanged){
        m_client->messageUpdated();
        return;
    }
    if (m_bOK){
        m_client->m_timer->start(m_interval, true);
        return;
    }
    m_client->error_state(m_err.c_str(), 0);
}

void CheckFriendsRequest::result(const char *key, const char *value)
{
    if (!strcmp(key, "success") && !strcmp(value, "OK")){
        m_bOK = true;
        return;
    }
    if (!strcmp(key, "lastupdate")){
        m_client->setLastUpdate(value);
        return;
    }
    if (!strcmp(key, "new")){
        if (atol(value))
            m_bChanged = true;
        return;
    }
    if (!strcmp(key, "interval")){
        m_interval = atol(value);
        return;
    }
    if (!strcmp(key, "errmsg")){
        m_err = value;
        return;
    }
}

void LiveJournalClient::timeout()
{
    if (getState() != Connected)
        return;
    m_timer->stop();
    m_requests.push_back(new CheckFriendsRequest(this));
    send();
}

LiveJournalRequest::LiveJournalRequest(LiveJournalClient *client, const char *mode)
{
    m_client = client;
    addParam("mode", mode);
    if (client->data.owner.User)
        addParam("user", client->data.owner.User);
    MD5_CTX md5;
    MD5_Init(&md5);
    MD5_Update(&md5, client->getPassword().utf8(), strlen(client->getPassword().utf8()));
    unsigned char pass[MD5_DIGEST_LENGTH];
    MD5_Final(pass, &md5);
    string hpass;
    for (unsigned i = 0; i < MD5_DIGEST_LENGTH; i++){
        char b[5];
        sprintf(b, "%02x", pass[i]);
        hpass += b;
    }
    addParam("hpassword", hpass.c_str());
}

LiveJournalRequest::~LiveJournalRequest()
{
}

void LiveJournalRequest::addParam(const char *key, const char *value)
{
    if (m_buffer.size())
        m_buffer.pack("&", 1);
    m_buffer.pack(key, strlen(key));
    m_buffer.pack("=", 1);
    for (const char *p = value; *p; p++){
        char c = *p;
        if (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9')) ||
                (c == '.') || (c == '-') || (c == '/') || (c == '_')){
            m_buffer.pack(&c, 1);
        }else{
            char buf[4];
            sprintf(buf, "%%%02X", c & 0xFF);
            m_buffer.pack(buf, 3);
        }
    }
}

void LiveJournalRequest::result(Buffer *b)
{
    for (;;){
        string key;
        string value;
        if (!getLine(b, key) || !getLine(b, value))
            break;
        log(L_DEBUG, "Result: %s=%s", key.c_str(), value.c_str());
        result(key.c_str(), value.c_str());
    }
}

bool LiveJournalRequest::getLine(Buffer *b, string &line)
{
    if (b == NULL)
        return false;
    if (!b->scan("\n", line))
        return false;
    if (line.length() && (line[line.length() - 1] == '\r'))
        line = line.substr(0, line.length() - 1);
    return true;
}

#if 0
I18N_NOOP("Invalid username")

I18N_NOOP("aggravated")
I18N_NOOP("angry")
I18N_NOOP("annoyed")
I18N_NOOP("anxious")
I18N_NOOP("bored")
I18N_NOOP("confused")
I18N_NOOP("crappy")
I18N_NOOP("cranky")
I18N_NOOP("depressed")
I18N_NOOP("discontent")
I18N_NOOP("energetic")
I18N_NOOP("enraged")
I18N_NOOP("enthralled")
I18N_NOOP("exhausted")
I18N_NOOP("happy")
I18N_NOOP("high")
I18N_NOOP("horny")
I18N_NOOP("hungry")
I18N_NOOP("infuriated")
I18N_NOOP("irate")
I18N_NOOP("jubilant")
I18N_NOOP("lonely")
I18N_NOOP("moody")
I18N_NOOP("pissed off")
I18N_NOOP("sad")
I18N_NOOP("satisfied")
I18N_NOOP("sore")
I18N_NOOP("stressed")
I18N_NOOP("thirsty")
I18N_NOOP("thoughtful")
I18N_NOOP("tired")
I18N_NOOP("touched")
I18N_NOOP("lazy")
I18N_NOOP("drunk")
I18N_NOOP("ditzy")
I18N_NOOP("mischievous")
I18N_NOOP("morose")
I18N_NOOP("gloomy")
I18N_NOOP("melancholy")
I18N_NOOP("drained")
I18N_NOOP("excited")
I18N_NOOP("relieved")
I18N_NOOP("hopeful")
I18N_NOOP("amused")
I18N_NOOP("determined")
I18N_NOOP("scared")
I18N_NOOP("frustrated")
I18N_NOOP("indescribable")
I18N_NOOP("sleepy")
I18N_NOOP("groggy")
I18N_NOOP("hyper")
I18N_NOOP("relaxed")
I18N_NOOP("restless")
I18N_NOOP("disappointed")
I18N_NOOP("curious")
I18N_NOOP("mellow")
I18N_NOOP("peaceful")
I18N_NOOP("bouncy")
I18N_NOOP("nostalgic")
I18N_NOOP("okay")
I18N_NOOP("rejuvenated")
I18N_NOOP("complacent")
I18N_NOOP("content")
I18N_NOOP("indifferent")
I18N_NOOP("silly")
I18N_NOOP("flirty")
I18N_NOOP("calm")
I18N_NOOP("refreshed")
I18N_NOOP("optimistic")
I18N_NOOP("pessimistic")
I18N_NOOP("giggly")
I18N_NOOP("pensive")
I18N_NOOP("uncomfortable")
I18N_NOOP("lethargic")
I18N_NOOP("listless")
I18N_NOOP("recumbent")
I18N_NOOP("exanimate")
I18N_NOOP("embarrassed")
I18N_NOOP("envious")
I18N_NOOP("sympathetic")
I18N_NOOP("sick")
I18N_NOOP("hot")
I18N_NOOP("cold")
I18N_NOOP("worried")
I18N_NOOP("loved")
I18N_NOOP("awake")
I18N_NOOP("working")
I18N_NOOP("productive")
I18N_NOOP("accomplished")
I18N_NOOP("busy")
I18N_NOOP("blah")
I18N_NOOP("full")
I18N_NOOP("grumpy")
I18N_NOOP("weird")
I18N_NOOP("nauseated")
I18N_NOOP("ecstatic")
I18N_NOOP("chipper")
I18N_NOOP("rushed")
I18N_NOOP("contemplative")
I18N_NOOP("nerdy")
I18N_NOOP("geeky")
I18N_NOOP("cynical")
I18N_NOOP("quixotic")
I18N_NOOP("crazy")
I18N_NOOP("creative")
I18N_NOOP("artistic")
I18N_NOOP("pleased")
I18N_NOOP("bitchy")
I18N_NOOP("guilty")
I18N_NOOP("irritated")
I18N_NOOP("blank")
I18N_NOOP("apathetic")
I18N_NOOP("dorky")
I18N_NOOP("impressed")
I18N_NOOP("naughty")
I18N_NOOP("predatory")
I18N_NOOP("dirty")
I18N_NOOP("giddy")
I18N_NOOP("surprised")
I18N_NOOP("shocked")
I18N_NOOP("rejected")
I18N_NOOP("numb")
I18N_NOOP("cheerful")
I18N_NOOP("good")
I18N_NOOP("distressed")
I18N_NOOP("intimidated")
I18N_NOOP("crushed")
I18N_NOOP("devious")
I18N_NOOP("thankful")
I18N_NOOP("grateful")
I18N_NOOP("jealous")
I18N_NOOP("nervous")

I18N_NOOP("Recent Entries")
I18N_NOOP("Calendar View")
I18N_NOOP("Friends View")
I18N_NOOP("Your Profile")
I18N_NOOP("Your To-Do List")
I18N_NOOP("Change Settings")
I18N_NOOP("Support")
I18N_NOOP("Personal Info")
I18N_NOOP("Journal Settings")

#endif

#ifdef WIN32
#include <windows.h>

/**
 * DLL's entry point
 **/
int WINAPI DllMain(HINSTANCE, DWORD, LPVOID)
{
    return TRUE;
}

/**
 * This is to prevent the CRT from loading, thus making this a smaller
 * and faster dll.
 **/
extern "C" BOOL __stdcall _DllMainCRTStartup( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return DllMain( hinstDLL, fdwReason, lpvReserved );
}

#endif

#ifndef WIN32
#include "livejournal.moc"
#endif


