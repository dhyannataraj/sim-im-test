/***************************************************************************
                          sms.cpp  -  description
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

#include "sms.h"
#include "smssetup.h"
#include "serial.h"
#include "gsm_ta.h"
#include "core.h"
#include "maininfo.h"
#include "listview.h"

#include "xpm/cell_off.xpm"
#include "xpm/cell_on.xpm"
#include "xpm/simcard.xpm"

#include <qtimer.h>
#include <qapplication.h>
#include <qwidgetlist.h>
#include <qobjectlist.h>

const unsigned COL_TYPE	= 3;

static DataDef _smsUserData[] =
    {
        { "", DATA_ULONG, 1, SMS_SIGN },	// Sign
        { "", DATA_ULONG, 1, 0 },			// LastSend
        { "", DATA_UTF, 1, 0 },				// Name
        { "", DATA_UTF, 1, 0 },
        { "", DATA_ULONG, 1, 0 },			// Index
        { "", DATA_ULONG, 1, 0 },			// Type
        { NULL, 0, 0, 0 }
    };

Plugin *createSMSPlugin(unsigned base, bool, const char*)
{
    Plugin *plugin = new SMSPlugin(base);
    return plugin;
}

static PluginInfo info =
    {
        0,
        0,
        VERSION,
        createSMSPlugin,
        PLUGIN_PROTOCOL
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

unsigned SMSPlugin::SerialPacket = 0;

static Message *createPhoneCall(const char *cfg)
{
    return new Message(MessagePhoneCall, cfg);
}

static MessageDef defPhoneCall =
    {
        NULL,
        NULL,
        MESSAGE_INFO,
        "Phone call",
        "%n phone calls",
        createPhoneCall,
        NULL,
        NULL
    };

#if 0
i18n("Phone call", "%n phone calls", 1);
#endif

SMSPlugin::SMSPlugin(unsigned base)
        : Plugin(base)
{
    IconDef icon;
    icon.name = "cell_off";
    icon.xpm  = cell_off;
    icon.isSystem = false;

    Event eIcon(EventAddIcon, &icon);
    eIcon.process();

    icon.name = "cell_on";
    icon.xpm  = cell_on;
    eIcon.process();

    icon.name = "simcard";
    icon.xpm  = simcard;
    eIcon.process();

    SerialPacket = registerType();
    getContacts()->addPacketType(SerialPacket, "Serial port", true);

    Command cmd;
    cmd->id			 = MessagePhoneCall;
    cmd->text		 = I18N_NOOP("Phone call");
    cmd->icon		 = "phone";
    cmd->flags		 = COMMAND_DEFAULT;
    cmd->param		 = &defPhoneCall;

    Event eMsg(EventCreateMessageType, cmd);
    eMsg.process();

    m_protocol = new SMSProtocol(this);

    qApp->installEventFilter(this);
    setPhoneCol();
}

SMSPlugin::~SMSPlugin()
{
    removePhoneCol();
    delete m_protocol;
    getContacts()->removePacketType(SerialPacket);
    Event eCall(EventRemoveMessageType, (void*)MessagePhoneCall);
    eCall.process();
}

void SMSPlugin::setPhoneCol()
{
    QWidgetList  *list = QApplication::topLevelWidgets();
    QWidgetListIt it(*list);
    QWidget *w;
    while ((w=it.current()) != NULL){
        ++it;
        QObjectList * l = w->queryList("MainInfo");
        QObjectListIt itw(*l);
        QObject * obj;
        while ((obj=itw.current()) != NULL) {
            ++itw;
            setPhoneCol(static_cast<MainInfo*>(obj));
        }
        delete l;
    }
    delete list;
}

void SMSPlugin::removePhoneCol()
{
    QWidgetList  *list = QApplication::topLevelWidgets();
    QWidgetListIt it(*list);
    QWidget *w;
    while ((w=it.current()) != NULL){
        ++it;
        QObjectList * l = w->queryList("MainInfo");
        QObjectListIt itw(*l);
        QObject * obj;
        while ((obj=itw.current()) != NULL) {
            ++itw;
            removePhoneCol(static_cast<MainInfo*>(obj));
        }
        delete l;
    }
    delete list;
}

void SMSPlugin::setPhoneCol(MainInfo *w)
{
    w->lstPhones->addColumn(" ", 16);
}

void SMSPlugin::removePhoneCol(MainInfo *w)
{
    w->lstPhones->removeColumn(COL_TYPE);
}

bool SMSPlugin::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::ChildInserted){
        QChildEvent *ce = static_cast<QChildEvent*>(e);
        if (ce->child()->inherits("MainInfo"))
            setPhoneCol(static_cast<MainInfo*>(ce->child()));
    }
    return QObject::eventFilter(obj, e);
}

SMSProtocol::SMSProtocol(Plugin *plugin)
        : Protocol(plugin)
{
}

SMSProtocol::~SMSProtocol()
{
}

Client *SMSProtocol::createClient(const char *cfg)
{
    return new SMSClient(this, cfg);
}

static CommandDef sms_descr =
    {
        0,
        I18N_NOOP("SMS"),
        "cell_on",
        NULL,
        NULL,
        0,
        0,
        0,
        0,
        0,
        PROTOCOL_NOPROXY | PROTOCOL_TEMP_DATA | PROTOCOL_NODATA,
        NULL,
        NULL
    };

const CommandDef *SMSProtocol::description()
{
    return &sms_descr;
}

static CommandDef sms_status_list[] =
    {
        {
            STATUS_ONLINE,
            I18N_NOOP("Online"),
            "cell_on",
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
            "cell_off",
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

const CommandDef *SMSProtocol::statusList()
{
    return sms_status_list;
}

const DataDef *SMSProtocol::userDataDef()
{
    return _smsUserData;
}

static DataDef smsClientData[] =
    {
#ifdef WIN32
        { "Port", DATA_STRING, 1, (unsigned)"COM1" },
#else
        { "Port", DATA_STRING, 1, (unsigned)"cuaa0" },
#endif
        { "InitString", DATA_STRING, 1, (unsigned)"E0" },
        { "BaudRate", DATA_ULONG, 1, 19200 },
        { "XonXoff", DATA_BOOL, 1, 0 },
        { "", DATA_ULONG, 1, 0 },		// Charge
        { "", DATA_BOOL, 1, 0 },		// Charging
        { "", DATA_ULONG, 1, 0 },		// Quality
        { NULL, 0, 0, 0 }
    };

SMSClient::SMSClient(Protocol *protocol, const char *cfg)
        : TCPClient(protocol, cfg)
{
    load_data(smsClientData, &data, cfg);
    m_ta = NULL;
    m_call = NULL;
    m_callTimer = new QTimer(this);
    connect(m_callTimer, SIGNAL(timeout()), this, SLOT(callTimeout()));
}

SMSClient::~SMSClient()
{
    free_data(smsClientData, &data);
}

string SMSClient::getConfig()
{
    string cfg = TCPClient::getConfig();
    string my_cfg = save_data(smsClientData, &data);
    if (!my_cfg.empty()){
        if (!cfg.empty())
            cfg += "\n";
        cfg += my_cfg;
    }
    return cfg;
}

string SMSClient::model()
{
    if (getState() == Connected)
        return m_ta->model();
    return "";
}

string SMSClient::oper()
{
    if (getState() == Connected)
        return m_ta->oper();
    return "";
}

string SMSClient::name()
{
    string res = "SMS.";
    if (getState() == Connected){
        res += model();
        res += " ";
        res += oper();
    }else{
        res += getDevice();
    }
    return res;
}

string SMSClient::dataName(void*)
{
    return "";
}

bool SMSClient::isMyData(clientData*&, Contact*&)
{
    return false;
}

bool SMSClient::createData(clientData*&, Contact*)
{
    return false;
}

void SMSClient::setupContact(Contact*, void*)
{
}

bool SMSClient::send(Message*, void*)
{
    return false;
}

bool SMSClient::canSend(unsigned type, void *data)
{
    if ((data == NULL) && (type == MessageSMS))
        return true;
    return false;
}

QWidget	*SMSClient::setupWnd()
{
    return new SMSSetup(NULL, this);
}

const unsigned MAIN_INFO = 1;

static CommandDef cfgSmsWnd[] =
    {
        {
            MAIN_INFO,
            "",
            "cell_on",
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

CommandDef *SMSClient::configWindows()
{
    QString title = QString::fromUtf8(name().c_str());
    int n = title.find(".");
    if (n > 0)
        title = title.left(n) + " " + title.mid(n + 1);
    cfgSmsWnd[0].text_wrk = strdup(title.utf8());
    return cfgSmsWnd;
}

QWidget *SMSClient::configWindow(QWidget *parent, unsigned id)
{
    if (id == MAIN_INFO)
        return new SMSSetup(parent, this);
    return NULL;
}

void SMSClient::packet_ready()
{
}

void SMSClient::disconnected()
{
    setStatus(STATUS_OFFLINE);
}

void SMSClient::setStatus(unsigned status)
{
    if (status == STATUS_OFFLINE){
        Contact *contact;
        ContactList::ContactIterator it;
        while ((contact = ++it) != NULL)
            contact->clientData.freeClientData(this);
        if (m_ta){
            delete m_ta;
            m_ta = NULL;
        }
        return;
    }
    if (m_ta)
        return;
    m_ta = new GsmTA(this);
    connect(m_ta, SIGNAL(init_done()), this, SLOT(init()));
    connect(m_ta, SIGNAL(error()), this, SLOT(error()));
    connect(m_ta, SIGNAL(phonebookEntry(int, int, const QString&, const QString&)), this, SLOT(phonebookEntry(int, int, const QString&, const QString&)));
    connect(m_ta, SIGNAL(charge(bool, unsigned)), this, SLOT(charge(bool, unsigned)));
    connect(m_ta, SIGNAL(quality(unsigned)), this, SLOT(quality(unsigned)));
    connect(m_ta, SIGNAL(phoneCall(const QString&)), this, SLOT(phoneCall(const QString&)));
    if (!m_ta->open(getDevice(), getBaudRate(), getXonXoff(), getInitString())){
        error_state("Can't open port", 0);
        return;
    }
}

void SMSClient::phonebookEntry(int index, int type, const QString &phone, const QString &name)
{
    bool bNew = false;
    Contact *contact;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        smsUserData *data;
        ClientDataIterator itd(contact->clientData);
        while ((data = (smsUserData*)(++itd)) != NULL){
            if (name == QString::fromUtf8(data->Name))
                break;
        }
        if (data)
            break;
    }
    if (contact == NULL){
        contact = getContacts()->contactByPhone(phone.latin1());
        if (contact->getTemporary()){
            bNew = true;
            contact->setTemporary(0);
            contact->setName(name);
        }
    }
    QString phones = contact->getPhones();
    bool bFound = false;
    while (!phones.isEmpty()){
        QString item = getToken(phones, ';', false);
        QString number = getToken(item, ',');
        if (number == phone){
            bFound = true;
            break;
        }
    }
    if (!bFound){
        phones = contact->getPhones();
        if (!phones.isEmpty())
            phones += ";";
        contact->setPhones(phones + phone + ",,2/-");
    }
    smsUserData *data = (smsUserData*)contact->clientData.createData(this);
    set_str(&data->Phone, phone.utf8());
    set_str(&data->Name, name.utf8());
    data->Index = index;
    data->Type  = type;
    if (bNew){
        Event e(EventContactChanged, contact);
        e.process();
    }
}

const char *SMSClient::getServer()
{
    return NULL;
}

unsigned short SMSClient::getPort()
{
    return 0;
}

void SMSClient::socketConnect()
{
    connect_ready();
    setStatus(STATUS_ONLINE);
}

void SMSClient::error()
{
    QTimer::singleShot(0, this, SLOT(ta_error()));
}

void SMSClient::ta_error()
{
    error_state(I18N_NOOP("Port error"), 0);
}

void SMSClient::init()
{
    m_status = STATUS_ONLINE;
    setState(Connected);
    m_ta->getPhoneBook();
}

void SMSClient::charge(bool bCharge, unsigned capacity)
{
    bool bChange = false;
    if (bCharge != getCharging()){
        bChange = true;
        setCharging(bCharge);
    }
    if (capacity != getCharge()){
        bChange = true;
        setCharge(capacity);
    }
    if (bChange){
        Event e(EventClientChanged, this);
        e.process();
    }
}

void SMSClient::quality(unsigned quality)
{
    if (quality != getQuality()){
        setQuality(quality);
        Event e(EventClientChanged, this);
        e.process();
    }
}

void SMSClient::phoneCall(const QString &number)
{
    if (m_call && (number == m_callNumber))
        return;
    if (m_call){
        m_callTimer->stop();
        Event e(EventMessageDeleted, m_call);
        e.process();
        delete m_call;
        m_call = NULL;
    }
    m_callNumber = number;
    m_call = new Message(MessagePhoneCall);
    if (!number.isEmpty()){
        bool bNew = false;
        Contact *contact = getContacts()->contactByPhone(number.latin1());
        if (contact->getTemporary()){
            bNew = true;
            contact->setTemporary(0);
            contact->setName(number);
        }
        QString phones = contact->getPhones();
        bool bFound = false;
        while (!phones.isEmpty()){
            QString item = getToken(phones, ';', false);
            QString phone = getToken(item, ',');
            if (number == phone){
                bFound = true;
                break;
            }
        }
        if (!bFound){
            phones = contact->getPhones();
            if (!phones.isEmpty())
                phones += ";";
            contact->setPhones(phones + number + ",,2/-");
        }
        if (bNew){
            Event e(EventContactChanged, contact);
            e.process();
        }
        m_call->setContact(contact->id());
    }
    m_call->setFlags(MESSAGE_RECEIVED | MESSAGE_TEMP);
    Event e(EventMessageReceived, m_call);
    if (e.process()){
        m_call = NULL;
        return;
    }
    m_bCall = false;
    m_callTimer->start(12000);
}

void SMSClient::callTimeout()
{
    if (m_bCall){
        m_bCall = false;
        return;
    }
    if (m_call == NULL)
        return;
    Event e(EventMessageDeleted, m_call);
    e.process();
    delete m_call;
    m_call = NULL;
    m_callTimer->stop();
    m_callNumber = "";
}

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
#include "sms.moc"
#endif


