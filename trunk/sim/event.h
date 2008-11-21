/***************************************************************************
                          event.h  -  description
                             -------------------
    begin                : Sat Oct 28 2006
    copyright            : (C) 2006 by Christian Ehrlicher
    email                : ch.ehrlicher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SIM_EVENT_H
#define SIM_EVENT_H

#include <qpoint.h>
#include <qsize.h>
#include <qstringlist.h>

#include "buffer.h"

class CToolBar;
class QMainWindow;
class QPopupMenu;
class QTranslator;

namespace SIM {

class Client;
class ClientSocket;
class CommandsDef;
class Contact;
class Group;
class IP;
class Message;
class Plugin;
class ServerSocketNotify;
class TCPClient;
struct pluginInfo;

// ___________________________________________________________________________________
// Event receiver

const unsigned DefaultPriority  = 0x1000;
const unsigned HighPriority     = 0x0800;
const unsigned HighestPriority  = 0x0400;
const unsigned LowPriority      = 0x2000;
const unsigned LowestPriority   = 0x4000;

class EXPORT EventReceiver
{
public:
    EventReceiver(unsigned priority = DefaultPriority);
    virtual ~EventReceiver();
    // true  -> stop event handling
    // false -> don't stop event handling
    virtual bool processEvent(class Event*) { return false; }
    unsigned priority() const { return m_priority; }
    static void initList();
    static void destroyList();
protected:
    unsigned m_priority;
};

// _____________________________________________________________________________________
// Event

enum SIMEvent
{
    eEventLog	= 0x0001,	// Log Output
    eEventInit	= 0x0101,	// application init after all plugins are loaded
    eEventQuit	= 0x0102,	// last event until plugins are unloaded
    eEventExec	= 0x0110,	// execute an external program
    eEventSocketActive      = 0x0112,	// change socket activity state

    eEventArg               = 0x0201,   // get command line argument
    eEventGetArgs           = 0x0202,   // get all command line arguments

    eEventLanguageChanged   = 0x0301,   // i18n changed
    eEventPluginChanged     = 0x0302,   // a plugin was (un)loaded
    eEventGetPluginInfo     = 0x0303,   // get plugin at pluginidx n, ret: pluginInfo
    eEventApplyPlugin       = 0x0304,   // ?
    eEventLoadPlugin        = 0x0305,   // a plugin should be loaded
    eEventUnloadPlugin      = 0x0306,   // a plugin should be unloaded
    eEventPluginsLoad       = 0x0307,   // load all plugins
    eEventPluginsUnload     = 0x0308,   // unload all plugins
    eEventSaveState         = 0x0309,   // plugins should save their config
    eEventClientsChanged    = 0x0311,   // a client was added/removed

    eEventIconChanged       = 0x0401,   // icons changed
    eEventSetMainIcon       = 0x0402,   // set the main icon
    eEventSetMainText       = 0x0403,   // set main window text


    eEventToolbar           = 0x0501,   // add/show/remove a toolbar
    eEventToolbarChanged    = 0x050D,   // toolbar should be changed
    eEventMenu              = 0x0502,   // add/remove a menu
    eEventMenuShow          = 0x0503,
    eEventMenuGet           = 0x0504,
    eEventMenuGetDef        = 0x0505,
    eEventMenuProcess       = 0x050f,
    eEventCommandExec       = 0x0506,
    eEventCommandCreate     = 0x0507,
    eEventCommandChange     = 0x0508,
    eEventCommandRemove     = 0x0509,
    eEventCommandChecked    = 0x0513,   // fixme
    eEventCommandDisabled   = 0x0514,
    eEventCommandShow       = 0x0515,
    eEventCommandWidget     = 0x0516,

    eEventCheckCommandState = 0x0520,   // check state of a Command

    eEventAddWidget         = 0x0570,   // add a widget to main/statuswindow

    eEventAddPreferences    = 0x0580,
    eEventRemovePreferences = 0x0581,

    eEventClientChanged     = 0x0590,   // a client changed status

    eEventHomeDir           = 0x0601,   // get home dir for config
    eEventGoURL             = 0x0602,   // open url in browser / mail / ...
    eEventGetURL            = 0x0603,   // ???? win32 only
    eEventPlaySound         = 0x0604,   // play way-file
    eEventRaiseWindow       = 0x0605,   // raise this widget
    eEventGetProfile	    = 0x0606,	// get name of current profile

    eEventPaintView         = 0x0701,   // draw user list background
    eEventRepaintView       = 0x0702,   // repaint list view

    eEventAddHyperlinks     = 0x0801,   // replace all hyperlinks with correct html tags

    eEventGroup             = 0x0901,   // a group was added/changed/deleted
    eEventContact           = 0x0902,   // a contact was added/changed/deleted
    eEventAddContact        = 0x0930,   // add contact
    eEventDeleteContact     = 0x0931,   // delete contact
    eEventGetContactIP      = 0x0932,   // retrieve IP address for contact

    eEventSocketConnect     = 0x1001,   // connect to host:port
    eEventSocketListen      = 0x1002,   // listen to host:port

    eEventMessageReceived   = 0x1100,
    eEventMessageSent       = 0x1101,
    eEventMessageCancel     = 0x1102,
    eEventSent              = 0x1103,
    eEventOpenMessage       = 0x1104,
    eEventMessageRead       = 0x1105,
    eEventMessageAcked      = 0x1106,
    eEventMessageDeleted    = 0x1107,
    eEventMessageAccept     = 0x1108,
    eEventMessageDecline    = 0x1109,
    eEventMessageSend       = 0x110A,
    eEventSend              = 0x110B,

    eEventClientError       = 0x1301,
    eEventShowError         = 0x1302,

    // for _core plugin
    eEventCreateMessageType	= 0x1401,
    eEventRemoveMessageType	= 0x1402,
    eEventRealSendMessage	= 0x1403,
    eEventHistoryConfig	    = 0x1404,
    eEventTemplateExpand    = 0x1405,
    eEventTemplateExpanded  = 0x1406,
    eEventARRequest			= 0x1407,
    eEventClientStatus		= 0x1408,
    eEventLoadMessage		= 0x1409,
    eEventDefaultAction		= 0x1410,
    eEventContactClient     = 0x1411,
    eEventActiveContact		= 0x1414,
    eEventMessageRetry		= 0x1415,
    eEventHistoryColors	    = 0x1416,
    eEventCheckSend			= 0x1418,
    eEventCutHistory		= 0x1419,
    eEventTmplHelp			= 0x1420,
    eEventTmplHelpList		= 0x1421,
    eEventDeleteMessage		= 0x1422,
    eEventRewriteMessage	= 0x1423,
    eEventJoinAlert			= 0x1424,

    // for jabber-plugin
// not nice, but currently no other idea :(
// not handled ...
//    eEventAgentFound		= 0x1501,
    eEventAgentInfo		    = 0x1502,
    eEventAgentRegister 	= 0x1503,
    eEventJabberSearch		= 0x1504,
    eEventJabberSearchDone	= 0x1505,
    eEventDiscoItem		    = 0x1506,
    eEventVCard			    = 0x1507,
    eEventClientVersion	    = 0x1508,
    eEventClientLastInfo	= 0x1509,
    eEventClientTimeInfo	= 0x1510,

    // for jabber-plugin
    eEventICQSearch			= 0x16001,
    eEventICQSearchDone		= 0x16002,
// currently unhandled
//    eEventRandomChat          = 0x16003,
//    eEventRandomChatInfo      = 0x16004,

    // for ontop-plugins
    eEventInTaskManager	    = 0x1701,
    eEventOnTop     	    = 0x1702,
};


class EXPORT Event
{
public:
    // change after all is converted
    Event(SIMEvent type) : m_type(type) {}
    virtual ~Event();
    // change after all is converted
    SIMEvent type() const { return m_type; }
    bool process(EventReceiver *from = NULL);  // should return true/false
    // sometimes we need no processing / the event is processed another way.
    void setNoProcess() { m_bProcessed = true; }
protected:
    SIMEvent m_type;
    bool m_bProcessed;
};

class EXPORT EventLog : public Event
{
public:
    EventLog(unsigned logLevel, const QCString &logData)
        : Event(eEventLog), m_logLevel(logLevel), m_logData(logData),
          m_packetID(0), m_addInfo(), m_Buffer()
    {}
    // FIXME: 0x10 / 0x20
    EventLog(const Buffer &packetBuf, bool bOut, unsigned packetID, const QCString addInfo = QCString())
        : Event(eEventLog), m_logLevel(bOut ? 0x20 : 0x10), m_logData(),
          m_packetID(packetID), m_addInfo(addInfo), m_Buffer(packetBuf)
    {}

    unsigned logLevel() const { return m_logLevel; }
    const QCString &logData() const { return m_logData; }
    unsigned packetID() const { return m_packetID; }
    const QCString &additionalInfo() const { return m_addInfo; }
    const Buffer &buffer() const { return m_Buffer; }
    bool isPacketLog() const { return m_Buffer.size() > 0; }

    static QString make_packet_string(const EventLog &l);
    static void log_packet(const Buffer &packetBuf, bool bOut, unsigned packetID, const QCString addInfo = QCString());
    static void log_packet(const Buffer &packetBuf, bool bOut, unsigned packetID, const QString addInfo);

protected:
    unsigned m_logLevel;
    QCString m_logData;
    unsigned m_packetID;
    QCString m_addInfo;
    Buffer m_Buffer;
};

class EXPORT EventInit : public Event
{
public:
	EventInit() : Event(eEventInit), m_bAbortLoading(false) {}
    void setAbortLoading() { m_bAbortLoading = true; }
    bool abortLoading() const { return m_bAbortLoading; }
protected:
    bool    m_bAbortLoading;
};

class EXPORT EventQuit : public Event
{
public:
	EventQuit() : Event(eEventQuit) {}
};

class EXPORT EventExec : public Event
{
public:
	EventExec(const QString &cmd, const QStringList &args)
		: Event(eEventExec), m_cmd(cmd), m_args(args) {}
	const QString &cmd() const { return m_cmd; }
	const QStringList &args() const { return m_args; }
	// out
	void setPid(unsigned long pid) { m_pid = pid; }
	unsigned long pid() const { return m_pid; }
protected:
	QString		m_cmd;
	QStringList m_args;
    unsigned long m_pid;
};

class EXPORT EventSocketActive : public Event
{
public:
	EventSocketActive(bool bActive)
		: Event(eEventSocketActive), m_bActive(bActive) {}
	bool active() const { return m_bActive; }
protected:
	bool		m_bActive;
};

class EXPORT EventArg : public Event
{
public:
	EventArg(const QString &arg, const QString &desc = QString())
		: Event(eEventArg), m_arg(arg), m_desc(desc) {}
	const QString &arg()  const { return m_arg; }
	const QString &desc() const { return m_desc; }
	// out
	void setValue(const QString &value) { m_value = value; }
	const QString &value() const { return m_value; }
protected:
	QString		m_arg;
	QString		m_desc;
	QString		m_value;
};

class EXPORT EventGetArgs : public Event
{
public:
	EventGetArgs() : Event(eEventGetArgs) {}
    // FIXME for Qt4!
    void setArgs(unsigned argc, char **argv)
    {
        m_argc = argc;
        m_argv = argv;
    }
    unsigned argc() const { return m_argc; }
    char **argv() const { return m_argv; }
protected:
	unsigned      m_argc;
    char        **m_argv;
};

class EXPORT EventLanguageChanged : public Event
{
public:
	EventLanguageChanged(QTranslator *translator)
		: Event(eEventLanguageChanged), m_translator(translator) {}
	QTranslator *translator() const { return m_translator; }
protected:
	QTranslator *m_translator;
};

class EXPORT EventPluginChanged : public Event
{
public:
	EventPluginChanged(pluginInfo *info)
		: Event(eEventPluginChanged), m_info(info) {}
	pluginInfo *info() const { return m_info; }
protected:
	pluginInfo *m_info;
};

class EXPORT EventGetPluginInfo : public Event
{
public:
	EventGetPluginInfo(unsigned idx)
		: Event(eEventGetPluginInfo), m_idx(idx), m_pluginName(), m_info(0) {}
	EventGetPluginInfo(const QString &pluginName)
		: Event(eEventGetPluginInfo), m_idx(0), m_pluginName(pluginName), m_info(0) {}
    unsigned idx() const { return m_idx; }
    const QString &pluginName() const { return m_pluginName; }
    // out
    void setInfo(pluginInfo *info) { m_info = info; }
	pluginInfo *info() const { return m_info; }
protected:
    unsigned    m_idx;
    QString     m_pluginName;
	pluginInfo *m_info;
};

class EXPORT EventApplyPlugin : public Event
{
public:
	EventApplyPlugin(const QString &pluginName)
		: Event(eEventApplyPlugin), m_pluginName(pluginName) {}
	const QString &pluginName() const { return m_pluginName; }
protected:
	QString     m_pluginName;
};

class EXPORT EventLoadPlugin : public Event
{
public:
	EventLoadPlugin(const QString &pluginName)
		: Event(eEventLoadPlugin), m_pluginName(pluginName) {}
	const QString &pluginName() const { return m_pluginName; }
protected:
	QString     m_pluginName;
};

class EXPORT EventUnloadPlugin : public Event
{
public:
	EventUnloadPlugin(const QString &pluginName)
		: Event(eEventUnloadPlugin), m_pluginName(pluginName) {}
	const QString &pluginName() const { return m_pluginName; }
protected:
	QString     m_pluginName;
};

class EXPORT EventPluginsLoad : public Event
{
public:
    EventPluginsLoad()
        : Event(eEventPluginsLoad), m_bAbort(true), m_plugin(NULL) {}
	EventPluginsLoad(Plugin *plugin)
		: Event(eEventPluginsLoad), m_bAbort(false), m_plugin(plugin) {}
    bool abortLoading() const { return m_bAbort; }
	Plugin *plugin() const { return m_plugin; }
protected:
    bool       m_bAbort;
	Plugin    *m_plugin;
};

class EXPORT EventPluginsUnload : public Event
{
public:
	EventPluginsUnload(Plugin *plugin)
		: Event(eEventPluginsUnload), m_plugin(plugin) {}
	Plugin *plugin() const { return m_plugin; }
protected:
	Plugin    *m_plugin;
};

class EXPORT EventSaveState : public Event
{
public:
    EventSaveState() : Event(eEventSaveState) {}
};

class EXPORT EventClientsChanged : public Event
{
public:
    EventClientsChanged() : Event(eEventClientsChanged) {}
};

class EXPORT EventIconChanged : public Event
{
public:
    EventIconChanged() : Event(eEventIconChanged) {}
};

class EXPORT EventSetMainIcon : public Event
{
public:
    EventSetMainIcon(const QString &icon)
        : Event(eEventSetMainIcon), m_icon(icon) {}
    const QString &icon() const { return m_icon; }
protected:
    QString m_icon;
};

class EXPORT EventSetMainText : public Event
{
public:
    EventSetMainText(const QString &text)
        : Event(eEventSetMainText), m_text(text) {}
    const QString &text() const { return m_text; }
protected:
    QString m_text;
};

class EXPORT EventHomeDir : public Event
{
public:
    EventHomeDir(const QString &dir)
        : Event(eEventHomeDir), m_dir(dir) {}
    const QString &homeDir()  const { return m_dir; }
    void setHomeDir(const QString &dir) { m_dir = dir; }
protected:
    QString m_dir;
};

class EXPORT EventGoURL : public Event
{
public:
    EventGoURL(const QString &url)
        : Event(eEventGoURL), m_url(url) {}
    const QString &url()  const { return m_url; }
protected:
    QString m_url;
};

class EXPORT EventPlaySound : public Event
{
public:
    EventPlaySound(const QString &sound)
        : Event(eEventPlaySound), m_sound(sound) {}
    const QString &sound()  const { return m_sound; }
protected:
    QString m_sound;
};

class EXPORT EventRaiseWindow : public Event
{
public:
    EventRaiseWindow(QWidget *widgetToRaise)
        : Event(eEventRaiseWindow), m_widget(widgetToRaise) {}
    QWidget *widget()  const { return m_widget; }
protected:
    QWidget *m_widget;
};

class EXPORT EventGetProfile : public Event
{
public:
    EventGetProfile()
        : Event(eEventGetProfile), m_profile(QString::null) {}
    const QString &getProfile()  const { return m_profile; }
    void setProfileValue(const QString &profile) { m_profile = profile; }
protected:
    QString m_profile;
};


class EXPORT EventGetURL : public Event
{
public:
    EventGetURL() : Event(eEventGetURL) {}
    // out!
    void setUrl(const QString &url) { m_url = url; }
    const QString &url()  const { return m_url; }
protected:
    QString m_url;
};

class EXPORT EventPaintView : public Event
{
public:
    struct PaintView {
        QPainter *p;        // painter
        QPoint   pos;       // position
        QSize    size;      // size
        int      height;    // item height
        int      margin;    // icon margin
        QWidget  *win;      // window
        bool     isStatic;  // need static background
        bool     isGroup;   // draw group
    };
public:
    EventPaintView(PaintView *pv)
        : Event(eEventPaintView), m_pv(pv) {}
    PaintView *paintView() const { return m_pv; }
protected:
    PaintView *m_pv;
};

class EXPORT EventRepaintView : public Event
{
public:
    EventRepaintView() : Event(eEventRepaintView) {}
};

class EXPORT EventAddHyperlinks : public Event
{
public:
    EventAddHyperlinks(const QString &text)
        : Event(eEventAddHyperlinks), m_text(text) {}
    const QString &text()  const { return m_text; }
    void setText(const QString &text) { m_text = text; }
protected:
    QString m_text;
};

class EXPORT EventGroup : public Event
{
public:
    enum Action {
        eAdded,
        eDeleted,
        eChanged,
    };
public:
    EventGroup(Group *grp, enum Action action)
        : Event(eEventGroup), m_grp(grp), m_action(action) {}
    Group *group() const { return m_grp; }
    Action action() const { return m_action; }
protected:
    Group *m_grp;
    Action m_action;
};

class EXPORT EventContact : public Event
{
public:
    enum Action {
        eCreated,
        eDeleted,
        eChanged,   // contact changed
        eFetchInfoFailed,   // fetch contact info from server failed
        eOnline,    // contact goes online
        eStatus,    // contact changed status
    };
public:
    EventContact(Contact *contact, enum Action action)
        : Event(eEventContact), m_contact(contact), m_action(action) {}
    Contact *contact() const { return m_contact; }
    Action action() const { return m_action; }
protected:
    Contact *m_contact;
    Action m_action;
};

class EXPORT EventAddContact : public Event
{
public:
    struct AddContact {
        QString         proto;
        QString         addr;
        QString         nick;
        unsigned        group;
    };
public:
    EventAddContact(AddContact *ac)
        : Event(eEventAddContact), m_ac(ac), m_contact(NULL) {}
    AddContact *addContact() const { return m_ac; }
    // out
    void setContact(Contact *c) { m_contact = c; }
    Contact *contact() const { return m_contact; }
protected:
    AddContact *m_ac;
    Contact *m_contact;
};

class EXPORT EventDeleteContact : public Event
{
public:
    EventDeleteContact(const QString &alias)
        : Event(eEventDeleteContact), m_alias(alias) {}
    const QString &alias() const { return m_alias; }
protected:
    QString m_alias;
};

class EXPORT EventGetContactIP : public Event
{
public:
    EventGetContactIP(Contact *contact)
        : Event(eEventGetContactIP), m_contact(contact), m_ip(NULL) {}
    Contact *contact() const { return m_contact; }
    // out
    void setIP(IP *ip) { m_ip = ip; }
    IP *ip() const { return m_ip; }
protected:
    Contact *m_contact;
    IP *m_ip;
};

class EXPORT EventSocketConnect : public Event
{
public:
    EventSocketConnect(ClientSocket *socket, TCPClient *client, QString host, unsigned short port)
        : Event(eEventSocketConnect),
          m_socket(socket), m_client(client), m_host(host), m_port(port) {}
    ClientSocket *socket() const { return m_socket; }
    TCPClient *client()    const { return m_client; }
    const QString &host()  const { return m_host; }
    unsigned short port()  const { return m_port; }
protected:
    ClientSocket  *m_socket;
    TCPClient     *m_client;
    QString        m_host;
    unsigned short m_port;
};

class EXPORT EventSocketListen : public Event
{
public:
    EventSocketListen(ServerSocketNotify *notify, TCPClient *client)
        : Event(eEventSocketListen), m_notify(notify), m_client(client) {}
    ServerSocketNotify *notify() const { return m_notify; }
    TCPClient *client() const { return m_client; }
protected:
    ServerSocketNotify *m_notify;
    TCPClient    *m_client;
};

class EXPORT EventError : public Event
{
public:
    struct ClientErrorData {
        Client     *client;
        QString     err_str;
        const char *options;
        QString     args;
        unsigned    code;
        enum { E_ERROR = 0x0001, E_INFO = 0x0002 } flags;
        unsigned    id;
    };
public:
    EventError(SIMEvent ev, const ClientErrorData &data)
        : Event(ev), m_data(data) {}
    const ClientErrorData &data() const { return m_data; }
protected:
    ClientErrorData m_data;
};

class EXPORT EventClientError : public EventError
{
public:
    EventClientError(const ClientErrorData &data)
        : EventError(eEventClientError, data) {}
};

class EXPORT EventShowError : public EventError
{
public:
    EventShowError(const ClientErrorData &data)
        : EventError(eEventShowError, data) {}
};

/* Base bar for mainwindow */
// FIXME: put them into class EventToolbar?
const unsigned long ToolBarMain      = 1;
const unsigned long ToolBarContainer = 2;
const unsigned long ToolBarTextEdit  = 3;
const unsigned long ToolBarMsgEdit  = 4;

class EXPORT EventToolbar : public Event
{
public:
    enum Action {
        eAdd,
        eShow,
        eRemove,
    };
public:
    EventToolbar(unsigned long id, QMainWindow *parent, Action action = eShow)
        : Event(eEventToolbar), m_id(id), m_parent(parent), m_action(action) {}
    EventToolbar(unsigned long id, Action action)
        : Event(eEventToolbar), m_id(id), m_parent(NULL), m_action(action) {}

    unsigned long id() const { return m_id; }
    QMainWindow *parent() const { return m_parent; }
    Action action() const { return m_action; }
    // out
    void setToolbar(CToolBar *bar) { m_bar = bar; }
    CToolBar *toolBar() const { return m_bar; }
protected:
    unsigned long m_id;
    QMainWindow *m_parent;
    Action m_action;
    CToolBar *m_bar;
};

class EXPORT EventToolbarChanged : public Event
{
public:
    EventToolbarChanged(CommandsDef *defs)
        : Event(eEventToolbarChanged), m_defs(defs) {}
    
    CommandsDef *defs() const { return m_defs; }
protected:
    CommandsDef *m_defs;
};

/* Base menu for mainwindow */
// FIXME: put them into class EventMenu?
const unsigned long MenuMain = 1;
const unsigned long MenuGroup = 2;
const unsigned long MenuContact = 3;
const unsigned long MenuContactGroup = 4;
const unsigned long MenuContainer = 5;
const unsigned long MenuMessage = 6;

class EXPORT EventMenu : public Event
{
public:
    enum Action {
        eAdd,
        eRemove,
        eCustomize, // execute customize dialog
    };
public:
    EventMenu(unsigned long id, Action action)
        : Event(eEventMenu), m_id(id), m_action(action) {}

    unsigned long id() const { return m_id; }
    Action action() const { return m_action; }
protected:
    unsigned long m_id;
    Action m_action;
};

struct CommandDef;
class EXPORT EventMenuShow : public Event
{
public:
    EventMenuShow(CommandDef *def)
        : Event(eEventMenuShow), m_def(def) {}
    
    CommandDef *def() const { return m_def; }
protected:
    CommandDef *m_def;
};

class EXPORT EventMenuGet : public Event
{
public:
    EventMenuGet(CommandDef *def)
        : Event(eEventMenuGet), m_def(def), m_menu(NULL) {}
    
    CommandDef *def() const { return m_def; }
    // out
    void setMenu(QPopupMenu *m) { m_menu = m; }
    QPopupMenu *menu() const { return m_menu; }
protected:
    CommandDef *m_def;
    QPopupMenu *m_menu;
};

class EXPORT EventMenuGetDef : public Event
{
public:
    EventMenuGetDef(unsigned long id)
        : Event(eEventMenuGetDef), m_id(id), m_defs(NULL) {}
    
    unsigned long id() const { return m_id; }
    // out
    void setCommandsDef(CommandsDef *defs) { m_defs = defs; }
    CommandsDef *defs() const { return m_defs; }
protected:
    unsigned long m_id;
    CommandsDef *m_defs;
};

class EventMenuProcess : public Event
{
public:
    EventMenuProcess(unsigned long id, void *param, int key = 0)
        : Event(eEventMenuProcess), m_id(id), m_param(param), m_key(key), m_menu(NULL) {}
    
    unsigned long id() const { return m_id; }
    void *param() const { return m_param; }
    int key() const { return m_key; }
    // out
    void setMenu(QPopupMenu *m) { m_menu = m; }
    QPopupMenu *menu() const { return m_menu; }
protected:
    unsigned long m_id;
    void *m_param;
    int m_key;
    QPopupMenu *m_menu;
};


class EXPORT EventAddWidget : public Event
{
public:
    enum Place {
        eMainWindow,
        eStatusWindow,
    };
public:
    EventAddWidget(QWidget *w, bool bDown, Place place)
        : Event(eEventAddWidget), m_widget(w), m_bDown(bDown), m_place(place) {}
    
    QWidget *widget() const { return m_widget; }
    bool down() const { return m_bDown; }
    Place place() const { return m_place; }
protected:
    QWidget *m_widget;
    bool m_bDown;
    Place m_place;
};

// CommandDef->param is getPreferencesWindow
typedef QWidget* (*getPreferencesWindow)(QWidget *parent, void *data);
class EventAddPreferences : public Event
{
public:
    EventAddPreferences(CommandDef *def)
        : Event(eEventAddPreferences), m_def(def) {}

    CommandDef *def() const { return m_def; }
protected:
    CommandDef *m_def;
};

class EventRemovePreferences : public Event
{
public:
    EventRemovePreferences(unsigned long id)
        : Event(eEventRemovePreferences), m_id(id) {}

    unsigned long id() const { return m_id; }
protected:
    unsigned long m_id;
};

class EventClientChanged : public Event
{
public:
    EventClientChanged(Client *c)
        : Event(eEventClientChanged), m_client(c) {}

    Client *client() const { return m_client; }
protected:
    Client *m_client;
};

/* Commands - create command
   menu items & bar buttons
   param is CommandDef* */

struct EXPORT CommandDef
{
    unsigned long   id;        // Command ID
    QString     text;          // Command name
    QString     icon;          // Icon
    QString     icon_on;       // Icon for checked state
    QString     accel;         // Accel
    unsigned    bar_id;        // Toolbar ID
    unsigned    bar_grp;       // Toolbar GRP
    unsigned    menu_id;       // Menu ID
    unsigned    menu_grp;      // Menu GRP
    unsigned    popup_id;      // Popup ID
    unsigned    flags;         // Command flags
    void        *param;        // Paramether from MenuSetParam
    QString     text_wrk;      // Text for check state

    void clear()
    {
        id		 = 0;
        text	 = QString::null;
        icon	 = QString::null;
        icon_on  = QString::null;
        accel	 = QString::null;
        bar_id   = 0;
        bar_grp	 = 0;
        menu_id	 = 0;
        menu_grp = 0;
        popup_id = 0;
        flags	 = 0;
        param	 = NULL;
        text_wrk = QString::null;
    }
    CommandDef(unsigned long _id = 0, const QString &_text = QString::null,
               const QString &_icon = QString::null, const QString &_icon_on = QString::null,
               const QString &_accel = QString::null, unsigned _bar_id = 0, unsigned _bar_grp = 0,
               unsigned _menu_id = 0, unsigned _menu_grp = 0, unsigned _popup_id = 0,
               unsigned _flags = 0, void *_param = NULL, const QString &_text_wrk = QString::null)
               : id(_id), text(_text), icon(_icon), icon_on(_icon_on), accel(_accel), bar_id(_bar_id),
                 bar_grp(_bar_grp), menu_id(_menu_id), menu_grp(_menu_grp), popup_id(_popup_id),
                 flags(_flags), param(_param), text_wrk(_text_wrk)
    {}
    const CommandDef &operator =(const CommandDef &c) {
        id		 = c.id;
        text	 = c.text;
        icon	 = c.icon;
        icon_on  = c.icon_on;
        accel	 = c.accel;
        bar_id   = c.bar_id;
        bar_grp	 = c.bar_grp;
        menu_id	 = c.menu_id;
        menu_grp = c.menu_grp;
        popup_id = c.popup_id;
        flags	 = c.flags;
        param	 = c.param;
        text_wrk = c.text_wrk;
        return *this;
    }
};

class EXPORT Command
{
public:
    Command();
    CommandDef &operator=(const CommandDef&);
    CommandDef *operator->() { return &m_cmd; }
    operator void*()         { return &m_cmd; }
    operator CommandDef*()   { return &m_cmd; }
protected:
    CommandDef m_cmd;
};

const unsigned COMMAND_DEFAULT      = 0;
const unsigned COMMAND_IMPORTANT    = 0x0001;
const unsigned COMMAND_TITLE        = 0x0002;
const unsigned COMMAND_CHECKED      = 0x0004;
const unsigned COMMAND_CHECK_STATE  = 0x0008;
const unsigned COMMAND_DISABLED     = 0x0010;
const unsigned COMMAND_GLOBAL_ACCEL = 0x0020;
const unsigned COMMAND_RECURSIVE    = 0x0040;
const unsigned COMMAND_NEW_POPUP    = 0x0080;

const unsigned BTN_TYPE             = 0xF000;
const unsigned BTN_DEFAULT          = 0x0000;
const unsigned BTN_PICT             = 0x1000;
const unsigned BTN_COMBO            = 0x2000;
const unsigned BTN_COMBO_CHECK      = 0x3000;
const unsigned BTN_EDIT             = 0x4000;
const unsigned BTN_LABEL            = 0x5000;

const unsigned BTN_HIDE             = 0x10000;
const unsigned BTN_NO_BUTTON        = 0x20000;
const unsigned BTN_DIV              = 0x40000;

class EXPORT EventCommand : public Event
{
public:
    EventCommand(SIMEvent e, CommandDef *cmd)
        : Event(e), m_cmd(cmd) {}

    CommandDef *cmd() const { return m_cmd; }
protected:
    CommandDef *m_cmd;
};

class EXPORT EventCommandExec : public EventCommand
{
public:
    EventCommandExec(CommandDef *cmd)
        : EventCommand(eEventCommandExec, cmd) {}
};

class EXPORT EventCommandCreate : public EventCommand
{
public:
    EventCommandCreate(CommandDef *cmd)
        : EventCommand(eEventCommandCreate, cmd) {}
};

class EXPORT EventCommandChange : public EventCommand
{
public:
    EventCommandChange(CommandDef *cmd)
        : EventCommand(eEventCommandChange, cmd) {}
};

class EXPORT EventCommandRemove : public Event
{
public:
    EventCommandRemove(unsigned long id)
        : Event(eEventCommandRemove), m_id(id) {}

    unsigned long id() const { return m_id; }
protected:
    unsigned long m_id;
};

class EXPORT EventCommandChecked : public EventCommand
{
public:
    EventCommandChecked(CommandDef *cmd)
        : EventCommand(eEventCommandChecked, cmd) {}
};

class EXPORT EventCommandDisabled : public EventCommand
{
public:
    EventCommandDisabled(CommandDef *cmd)
        : EventCommand(eEventCommandDisabled, cmd) {}
};

class EXPORT EventCommandShow : public EventCommand
{
public:
    EventCommandShow(CommandDef *cmd)
        : EventCommand(eEventCommandShow, cmd) {}
};

class EXPORT EventCommandWidget : public EventCommand
{
public:
    EventCommandWidget(CommandDef *cmd)
        : EventCommand(eEventCommandWidget, cmd), m_widget(NULL) {}
    // out
    void setWidget(QWidget *w) { m_widget = w; }
    QWidget *widget() const { return m_widget; }
protected:
    QWidget *m_widget;
};

class EXPORT EventCheckCommandState : public EventCommand
{
public:
    EventCheckCommandState(CommandDef *cmd)
        : EventCommand(eEventCheckCommandState, cmd) {}
};

class EventMessage : public Event
{
public:
    EventMessage(SIMEvent e, Message *msg)
        : Event(e), m_msg(msg) {}

    Message *msg() const { return m_msg; }
protected:
    Message *m_msg;
};


class EventMessageReceived : public EventMessage
{
public:
    EventMessageReceived(Message *msg)
        : EventMessage(eEventMessageReceived, msg) {}
};

class EventMessageSent : public EventMessage
{
public:
    EventMessageSent(Message *msg)
        : EventMessage(eEventMessageSent, msg) {}
};

class EventMessageCancel : public EventMessage
{
public:
    EventMessageCancel(Message *msg)
        : EventMessage(eEventMessageCancel, msg) {}
};

class EventSent : public EventMessage
{
public:
    EventSent(Message *msg)
        : EventMessage(eEventSent, msg) {}
};

class EventOpenMessage : public EventMessage
{
public:
    EventOpenMessage(Message *msg)
        : EventMessage(eEventOpenMessage, msg) {}
};

class EventMessageRead : public EventMessage
{
public:
    EventMessageRead(Message *msg)
        : EventMessage(eEventMessageRead, msg) {}
};

class EventMessageAcked : public EventMessage
{
public:
    EventMessageAcked(Message *msg)
        : EventMessage(eEventMessageAcked, msg) {}
};

class EventMessageDeleted : public EventMessage
{
public:
    EventMessageDeleted(Message *msg)
        : EventMessage(eEventMessageDeleted, msg) {}
};

// Move this into EventMessageAccept? Better into class FileTransfer ?
enum OverwriteMode {
    Ask,
    Skip,
    Resume,
    Replace
};
class EventMessageAccept : public EventMessage
{
public:
    EventMessageAccept(Message *msg, const QString &dir, OverwriteMode mode)
        : EventMessage(eEventMessageAccept, msg), m_dir(dir), m_mode(mode) {}

    const QString &dir() const { return m_dir; }
    OverwriteMode mode() const { return m_mode; }
protected:
    QString m_dir;
    OverwriteMode m_mode;
};

class EventMessageDecline : public EventMessage
{
public:
    EventMessageDecline(Message *msg, const QString &reason = QString::null)
        : EventMessage(eEventMessageDecline, msg), m_reason(reason) {}

    const QString &reason() const { return m_reason; }
protected:
    QString m_reason;
};

class EventMessageSend : public EventMessage
{
public:
    EventMessageSend(Message *msg)
        : EventMessage(eEventMessageSend, msg) {}
};

class EventSend : public EventMessage
{
public:
    EventSend(Message *msg, const QCString &localeText)
        : EventMessage(eEventSend, msg), m_text(localeText) {}

    // in & out
    const QCString &localeText() const { return m_text; }
    void setLocaleText(const QCString &text) { m_text = text; }
protected:
    QCString m_text;
};

} // namespace SIM

#endif // SIM_EVENT_H
