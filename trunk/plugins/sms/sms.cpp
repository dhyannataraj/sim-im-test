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

#include "xpm/cell_off.xpm"
#include "xpm/cell_on.xpm"
#include "xpm/nocell.xpm"

#include <qtimer.h>

static DataDef _smsUserData[] =
    {
        { "", DATA_ULONG, 1, SMS_SIGN },	// Sign
        { "", DATA_ULONG, 1, 0 },			// LastSend
        { "", DATA_UTF, 1, 0 },				// Name
        { "", DATA_UTF, 1, 0 },
        { "", DATA_ULONG, 1, 0 },			// Index
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

    icon.name = "nocell";
    icon.xpm  = nocell;
    eIcon.process();

    SerialPacket = registerType();
    getContacts()->addPacketType(SerialPacket, "Serial port", true);

    m_protocol = new SMSProtocol(this);
}

SMSPlugin::~SMSPlugin()
{
    delete m_protocol;
    getContacts()->removePacketType(SerialPacket);
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
        { "Port", DATA_STRING, 1, (unsigned)"modem" },
#endif
        { "InitString", DATA_STRING, 1, (unsigned)"E0" },
        { "BaudRate", DATA_ULONG, 1, 19200 },
        { "XonXoff", DATA_BOOL, 1, 0 },
        { NULL, 0, 0, 0 }
    };

SMSClient::SMSClient(Protocol *protocol, const char *cfg)
        : TCPClient(protocol, cfg)
{
    load_data(smsClientData, &data, cfg);
    m_ta = NULL;
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

string SMSClient::name()
{
    string res = "SMS.";
    if (getState() == Connected){
        res += m_ta->info();
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
    connect(m_ta, SIGNAL(phonebookEntry(int, const QString&, const QString&)), this, SLOT(phonebookEntry(int, const QString&, const QString&)));
    if (!m_ta->open(getDevice(), getBaudRate(), getXonXoff(), getInitString())){
        error_state("Can't open port", 0);
        return;
    }
}

void SMSClient::phonebookEntry(int index, const QString &phone, const QString &name)
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
        if (data){
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
            if (!bFound)
                contact->setPhones(contact->getPhones() + ";" + phone + ",,2/-");
            break;
        }
    }
    if (contact == NULL){
        contact = getContacts()->contactByPhone(phone.latin1());
        if (contact->getTemporary()){
            bNew = true;
            contact->setTemporary(0);
            contact = getContacts()->contact(0, true);
            contact->setName(name);
            contact->setPhones(phone + ",,2/-");
        }
    }
    smsUserData *data = (smsUserData*)contact->clientData.createData(this);
    set_str(&data->Phone, phone.utf8());
    set_str(&data->Name, name.utf8());
    data->Index = index;
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


