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

class QTranslator;

namespace SIM {

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
    virtual void *processEvent(class Event*) { return NULL; }
    unsigned priority() const { return m_priority; }
    static void initList();
    static void destroyList();
protected:
    unsigned m_priority;
};

// _____________________________________________________________________________________
// Event

class EXPORT Event
{
public:
    Event(unsigned type, void *param = NULL) : m_type(type), m_param(param) {}
    virtual ~Event() {}
    unsigned type() const { return m_type; }
    void *param() { return m_param; }   // deprecated
    void *process(EventReceiver *from = NULL);  // should return true/false
protected:
    unsigned m_type;
    void *m_param;
};

enum SIMEvents
{
    eEventLog	= 0x0001,	// Log Output
	eEventInit	= 0x0101,	// application init after all plugins are loaded
	eEventQuit	= 0x0102,	// last event until plugins are unloaded
	eEventExec	= 0x0110,	// execute an external programm
	eEventSocketActive = 0x0112,	// change socket activity state
	eEventArg	    = 0x0201,	// get command line argument
    eEventGetArgs   = 0x0202,   // get all command line arguments
    eEventLanguageChanged   = 0x0301,   // i18n changed
    eEventPluginChanged     = 0x0302,   // a plugin was (un)loaded
    eEventGetPluginInfo     = 0x0303,   // get plugin at pluginidx n, ret: pluginInfo
    eEventApplyPlugin       = 0x0304,   // ?
    eEventLoadPlugin        = 0x0305,   // a plugin should be loaded
    eEventUnloadPlugin      = 0x0306,   // a plugin should be unloaded

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
	EventInit() : Event(eEventInit) {}
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
		: Event(eEventInit), m_cmd(cmd), m_args(args) {}
	const QString &cmd() const { return m_cmd; }
	const QStringList &args() const { return m_args; }
protected:
	QString		m_cmd;
	QStringList m_args;
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

// _____________________________________________________________________________________
// Default events

const unsigned EventPluginsUnload   = 0x0307;
const unsigned EventPluginsLoad     = 0x0308;

/* Event - save state
*/
const unsigned EventSaveState       = 0x0310;

/* Event clients changes
*/
const unsigned EventClientsChanged  = 0x0311;

/* Event icons changed */
const unsigned EventIconChanged     = 0x0400;

/* Toolbar (create and remove)
   param is toolbar id */
const unsigned EventToolbarCreate = 0x0501;
const unsigned EventToolbarRemove = 0x0502;

/* Base bar for mainwindow */
const unsigned long ToolBarMain      = 1;
const unsigned long ToolBarContainer = 2;
const unsigned long ToolBarTextEdit  = 3;
const unsigned long ToolBarMsgEdit  = 4;

/* Menu (create and remove)
   param is toolbar id */
const unsigned EventMenuCreate = 0x0503;
const unsigned EventMenuRemove = 0x0504;

/* Base menu for mainwindow */
const unsigned long MenuMain = 1;
const unsigned long MenuGroup = 2;
const unsigned long MenuContact = 3;
const unsigned long MenuContactGroup = 4;
const unsigned long MenuContainer = 5;
const unsigned long MenuMessage = 6;

/* Commands - process command
   param is CommandDef* */

const unsigned EventCommandExec = 0x0505;

/* Commands - create command
   menu items & bar buttons
   param is CommandDef* */

const unsigned EventCommandCreate = 0x0506;

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

const unsigned BTN_HIDE             = 0x10000;
const unsigned BTN_NO_BUTTON        = 0x20000;
const unsigned BTN_DIV              = 0x40000;

/* Command - remove command id */
const unsigned EventCommandRemove = 0x0507;


/* Event real create toolbar
   param is ToolBarShow*
*/
const unsigned EventShowBar = 0x0508;

typedef struct BarShow
{
    class QMainWindow *parent;
    unsigned    bar_id;
} ToolBarShow;

/* Event get real menu
   param is CommandDef
   return QPopupMenu*
*/
const unsigned EventGetMenu = 0x0509;

/* Event - add widget into main window param is WindowDef*
*/

const unsigned EventAddWindow = 0x050A;

/* Event - add widget into status param is WindowDef*
*/

const unsigned EventAddStatus = 0x050B;

typedef struct WindowDef
{
    class QWidget *widget;
    bool    bDown;
} WindowDef;

/* Event Check command state
   param is CommandDef*
*/
const unsigned EventCheckState  = 0x050C;

/* Event - toolbar changed
   param is ToolBarDef*
*/

const unsigned EventToolbarChanged = 0x050D;

/* Event get menu def
   param is menu_id
   return CommandsDef*
*/
const unsigned EventGetMenuDef = 0x050E;

/* Event menu customize
   param menu_id
*/
const unsigned EventMenuCustomize = 0x050F;

/* Event set menu param
   param is ProcessMenuParam*
   return QPopupMenu*
*/
const unsigned EventProcessMenu   = 0x0510;

typedef struct ProcessMenuParam
{
    unsigned id;
    void     *param;
    int      key;           // If key != 0 process accel
} ProcessMenuParam;

/* Event register user preferences
   param is CommandDef*
   CommandDef->param is getPreferencesWindow
*/
const unsigned EventAddPreferences = 0x0511;

typedef QWidget* (*getPreferencesWindow)(QWidget *parent, void *data);

/* Event remove user preferences
   param is id
*/

const unsigned EventRemovePreferences = 0x0512;

const unsigned EventSetMainIcon     = 0x0520;
const unsigned EventSetMainText     = 0x0521;

const unsigned EventCommandChange   = 0x0522;
const unsigned EventCommandChecked  = 0x0523;
const unsigned EventCommandDisabled = 0x0524;
const unsigned EventCommandShow     = 0x0525;
const unsigned EventCommandWidget   = 0x0526;

const unsigned EventClientChanged   = 0x0530;

/* Event - get home dir
   param is QString *fileName
*/

const unsigned EventHomeDir     = 0x0601;

/* Event go URL
   param is QString *url
*/

const unsigned EventGoURL       = 0x0602;

/* Event get URL
   return QString *url
*/

const unsigned EventGetURL      = 0x0603;

/* Event play sound
   param is QString *wav
*/

const unsigned EventPlaySound   = 0x0604;

/* Event raise window
   param is QWidget *
*/

const unsigned EventRaiseWindow = 0x0605;


/* Event - draw user list background
   param is PaintListView*
*/
const unsigned EventPaintView = 0x0701;

typedef struct PaintView
{
    class QPainter *p;        // painter
    QPoint   pos;       // position
    QSize    size;      // size
    int      height;    // item height
    int      margin;    // icon margin
    QWidget  *win;      // window
    bool     isStatic;  // need static background
    bool     isGroup;   // draw group
} PaintView;

/* Event repaint list view */
const unsigned EventRepaintView = 0x0702;

/* Event draw view item */
const unsigned EventDrawItem = 0x0703;

/* Event replace all hyperlinks with correct html tags
   param is QString*
*/
const unsigned EventAddHyperlinks = 0x0801;

/* Event group created
   param is Group*
*/
const unsigned EventGroupCreated = 0x0901;

/* Event group deleted
   param is Group*
*/
const unsigned EventGroupDeleted = 0x0902;

/* Event group changed
   param is Group*
*/
const unsigned EventGroupChanged = 0x0903;

/* Event group created
   param is Group*
*/
const unsigned EventContactCreated = 0x0911;

/* Event group deleted
   param is Group*
*/
const unsigned EventContactDeleted = 0x0912;

/* Event group changed
   param is Group*
*/
const unsigned EventContactChanged = 0x0913;
const unsigned EventFetchInfoFail  = 0x0914;
const unsigned EventContactOnline  = 0x0915;
const unsigned EventContactStatus  = 0x0916;

typedef struct addContact
{
    QString         proto;
    QString         addr;
    QString         nick;
    unsigned        group;
} addContact;

/* Event for add contact
   param is addContact*
   return Contact*
*/
const unsigned EventAddContact      = 0x0930;

/* Event for remove contact
   param is QString *address
*/
const unsigned EventDeleteContact   = 0x0931;

/* Event for get contact IP
   param is Contact*
*/
const unsigned EventGetContactIP    = 0x0932;

/* Event netvork state changed
*/
const unsigned EventNetworkChanged  = 0x1000;

/* Event socket connect
   param is ConnectParam *
*/
const unsigned EventSocketConnect   = 0x1001;

class ClientSocket;
class ServerSocketNotify;
class TCPClient;

typedef struct ConnectParam
{
    ClientSocket    *socket;
    TCPClient       *client;
    QString         host;
    unsigned short  port;
} ConnectParam;

/* Event socket listen
   param is ListenParam *
*/
const unsigned EventSocketListen     = 0x1002;

typedef struct ListenParam
{
    ServerSocketNotify  *notify;
    TCPClient           *client;
} ListenParam;

/* Event send & receive message
*/

class Message;

enum OverwriteMode
{
    Ask,
    Skip,
    Resume,
    Replace
};

typedef struct messageAccept
{
    Message         *msg;
    QString         dir;
    OverwriteMode   overwrite;
} messageAccept;

typedef struct messageDecline
{
    Message     *msg;
    QString     reason;
} messageDecline;

typedef struct messageSend
{
    Message     *msg;
    QCString    *text;      // locale dependent !
} messageSend;

// Param: Message*
// Make sure Message * is a pointer to the heap since the message
// maybe gets destroyed during it's way through the plugins
const unsigned EventMessageReceived = 0x1100;
const unsigned EventMessageSent     = 0x1101;
const unsigned EventMessageCancel   = 0x1102;
const unsigned EventSent            = 0x1103;
const unsigned EventOpenMessage     = 0x1104;
const unsigned EventMessageRead     = 0x1105;
const unsigned EventMessageAcked    = 0x1106;
const unsigned EventMessageDeleted  = 0x1107;
const unsigned EventMessageAccept   = 0x1108;
const unsigned EventMessageDecline  = 0x1109;
const unsigned EventMessageSend     = 0x110A;
const unsigned EventSend            = 0x110B;

const unsigned EventClientError     = 0x1301;
const unsigned EventShowError       = 0x1302;

const unsigned ERR_ERROR    = 0x0000;
const unsigned ERR_INFO     = 0x0001;

typedef struct clientErrorData
{
    class Client *client;
    QString     err_str;
    const char  *options;
    QString     args;
    unsigned    code;
    unsigned    flags;
    unsigned    id;
} clientErrorData;

const unsigned EventUser            = 0x10000;

} // namespace SIM

#endif // SIM_EVENT_H
