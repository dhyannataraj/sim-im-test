/***************************************************************************
                          simapi.h  -  description
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

#ifndef _SIMAPI_H
#define _SIMAPI_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "compatqtversion.h"

#if COMPAT_QT_VERSION >= 0x030000
#include <qtextedit.h>
#endif

#ifdef STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#else
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef WIN32
	#ifdef HAVE_STDBOOL_H
	  #include <stdbool.h>
	#else
	  #ifndef HAVE__BOOL
	    #ifdef __cplusplus
typedef bool _Bool;
		#else
typedef unsigned char _Bool;
		#endif
	  #endif
	  #define bool _Bool
	  #define false 0
	  #define true 1
	  #define __bool_true_false_are_defined 1
	#endif
#endif

#ifdef WIN32
#if _MSC_VER > 1020
#include <yvals.h>              
#pragma warning(disable: 4097)
#pragma warning(disable: 4244)  
#pragma warning(disable: 4275)
#pragma warning(disable: 4514)
#pragma warning(disable: 4710)  
#pragma warning(disable: 4786)  
#pragma warning(push)
#pragma warning(disable: 4018)  
#pragma warning(disable: 4100)  
#pragma warning(disable: 4146)  
#pragma warning(disable: 4511)  
#pragma warning(disable: 4512)  
#pragma warning(disable: 4530)  
#pragma warning(disable: 4663)  
#endif
#endif

#if defined(_MSC_VER) && defined(_DEBUG) && !defined(NO_CHECK_NEW)
#include <qnetworkprotocol.h>
#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#endif
#include <stdlib.h>
#include <crtdbg.h>
#include <memory>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#ifdef USE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/rand.h>
#endif

#include <string>
using namespace std;

#include <qwidget.h>
#if COMPAT_QT_VERSION >= 0x030000
#include <qdockwindow.h>
#endif

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(pop)
#endif
#endif

#ifdef SIMAPI_EXPORTS
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#ifdef SIMUI_EXPORTS
#define UI_EXPORT __declspec(dllexport)
#else
#define UI_EXPORT
#endif

#ifndef HAVE_STRCASECMP
EXPORT int strcasecmp(const char *a, const char *b);
#endif

#ifdef WIN32
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif

#ifndef COPY_RESTRICTED
#define COPY_RESTRICTED(A) private: A(const A&); A &operator = (const A&);
#endif

#ifdef USE_KDE

#include <klocale.h>
#if COMPAT_QT_VERSION < 0x030000
EXPORT QString i18n(const char *singular, const char *plural, unsigned long n);
#endif
#else
#include <qobject.h>
EXPORT QString i18n(const char *text);
EXPORT QString i18n(const char *text, const char *comment);
EXPORT QString i18n(const char *singular, const char *plural, unsigned long n);
#define I18N_NOOP(A)	A
#endif
#if !defined(USE_KDE) || (COMPAT_QT_VERSION < 0x030000)
EXPORT void resetPlural();
#endif

#if COMPAT_QT_VERSION < 0x030000

#include <qpoint.h>

class QContextMenuEvent
{
public:
QContextMenuEvent(const QPoint &pos) : p(pos) {}
    const QPoint &globalPos() { return p; }
    void accept() {}
protected:
    QPoint p;
};

#endif

class QFile;
class QWidget;
class QIconSet;
class QPixmap;
class QToolBar;
class QMainWindow;
class QComboBox;
class QLineEdit;

class KAboutData;
class Buffer;

namespace SIM
{

// _________________________________________________________________________________
/* PluginManager - base class for main application */

class PluginManagerPrivate;
class ContactList;
class SocketFactory;
class Client;

class EXPORT PluginManager
{
public:
    PluginManager(int argc, char **argv);
    ~PluginManager();
    bool isLoaded();
    static ContactList			*contacts;
    static SocketFactory		*factory;
private:
    PluginManagerPrivate *p;
};

// __________________________________________________________________________________
/* Plugin prototype */

struct pluginInfo;

class EXPORT Plugin
{
public:
    Plugin(unsigned base);
    virtual ~Plugin() {}
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual string getConfig();
    unsigned registerType();
    void boundTypes();
protected:
    unsigned m_current;
    unsigned m_base;
};

typedef Plugin *createPlugin(unsigned base, bool bStart, const char *add_info);
typedef QStyle *createStyle();

const unsigned PLUGIN_KDE_COMPILE    = 0x0001;
#ifdef USE_KDE
const unsigned PLUGIN_DEFAULT	     = PLUGIN_KDE_COMPILE;
#else
const unsigned PLUGIN_DEFAULT	     = 0x0000;
#endif
const unsigned PLUGIN_NO_CONFIG_PATH = 0x0002 | PLUGIN_DEFAULT;
const unsigned PLUGIN_NOLOAD_DEFAULT = 0x0004 | PLUGIN_DEFAULT;

const unsigned PLUGIN_PROTOCOL		 = 0x0008 | PLUGIN_NOLOAD_DEFAULT;
const unsigned PLUGIN_NODISABLE		 = 0x0010;
const unsigned PLUGIN_RELOAD		 = 0x0020;

const unsigned ABORT_LOADING		 = (unsigned)(-1);

typedef struct PluginInfo			// Information in plugin
{
    const char		*title;			// User title
    const char		*description;	// Description
    const char		*version;		// Version
    createPlugin	*create;		// create proc
    unsigned		flags;			// plugin falsg
} PluginInfo;

typedef struct pluginInfo
{
    Plugin			*plugin;
    char			*name;
    char			*config;		// config string
    bool			bDisabled;		// no load this plugin
    bool			bNoCreate;		// can't create plugin
    bool			bFromCfg;		// init state from config
    void			*module;		// so or dll handle
    PluginInfo		*info;
    unsigned		base;			// base for plugin types
} pluginInfo;

typedef struct StyleInfo
{
    const char		*title;
    createStyle		*create;
} StyleInfo;

#ifdef WIN32
#define EXPORT_PROC extern "C" __declspec(dllexport)
#else
#define EXPORT_PROC extern "C" 
#endif

EXPORT_PROC PluginInfo *GetPluginInfo();
EXPORT_PROC StyleInfo  *GetStyleInfo();

// _____________________________________________________________________________________
// Event

class EventReceiver;

class EXPORT Event
{
public:
Event(unsigned type, void *param = NULL) : m_type(type), m_param(param) {}
    virtual ~Event() {}
    unsigned type() { return m_type; }
    void *param() { return m_param; }
    void *process(EventReceiver *from = NULL);
protected:
    unsigned m_type;
    void *m_param;
};

// ___________________________________________________________________________________
// Event receiver

const unsigned DefaultPriority	= 0x1000;
const unsigned HighPriority		= 0x0800;
const unsigned HighestPriority	= 0x0400;
const unsigned LowPriority		= 0x2000;
const unsigned LowestPriority	= 0x4000;

class EXPORT EventReceiver
{
public:
    EventReceiver(unsigned priority = DefaultPriority);
    virtual ~EventReceiver();
    virtual void *processEvent(Event*) { return NULL; }
    unsigned priority() { return m_priority; }
    static void initList();
    static void destroyList();
protected:
    unsigned m_priority;
};

// _____________________________________________________________________________________
// Default events

/* Event for loggin */
/* Log one string (pram is LogInfo) */
const unsigned EventLog = 0x0001;

typedef struct LogInfo
{
    unsigned	log_level;
    void		*log_info;
    unsigned	packet_id;
    const char	*add_info;
} LogInfo;

/* Init application - after load all plugins */
const unsigned EventInit = 0x0101;

/* Execute cmd - param is ExecParam* */
const unsigned EventExec = 0x0110;

typedef struct ExecParam
{
    const char *cmd;
    const char *arg;
} ExecParam;

const unsigned EventQuit = 0x0111;

/* Event change socket activity state
*/
const unsigned EventSocketActive = 0x0112;

/* Get arg param is CmdParam */
const unsigned EventArg = 0x0201;

typedef struct CmdParam
{
    const char *arg;
    const char *descr;
    string *value;
} CmdParam;

const unsigned EventArgc = 0x0202;
const unsigned EventArgv = 0x0203;

/* Notification langauge changed
   param is new translator */
const unsigned EventLanguageChanged = 0x0301;

/* Notification about change plugin state
   param is pluginInfo* */
const unsigned EventPluginChanged	= 0x0302;

/* Event get plugin info param is plugin number
   return pluginInfo* 
*/
const unsigned EventPluginGetInfo	= 0x0303;
const unsigned EventApplyPlugin		= 0x0304;

const unsigned EventLoadPlugin		= 0x0305;
const unsigned EventUnloadPlugin	= 0x0306;

const unsigned EventPluginsUnload	= 0x0307;
const unsigned EventPluginsLoad		= 0x0308;

const unsigned EventGetPluginInfo	= 0x0309;

/* Event - save state
*/
const unsigned EventSaveState		= 0x0310;

/* Event clients changes
*/
const unsigned EventClientsChanged	= 0x0311;

/* Add icon (param is IconDef ) */
const unsigned EventAddIcon = 0x0401;

typedef struct IconDef
{
    const char *name;
    const char **xpm;
    const char *system;
    unsigned   flags;
    IconDef() : name(NULL), xpm(NULL), system(NULL), flags(0) {}
} IconDef;

const unsigned ICON_COLOR_MASK	= 0x000000FF;

/* Get icon (param is const char *name)
   return const QIconSet* or NULL */
const unsigned EventGetIcon = 0x0402;

/* Event icons changed */
const unsigned EventIconChanged = 0x0404;

/* Toolbar (create and remove)
   param is toolbar id */
const unsigned EventToolbarCreate = 0x0501;
const unsigned EventToolbarRemove = 0x0502;

/* Base bar for mainwindow */
const unsigned ToolBarMain      = 1;
const unsigned ToolBarContainer = 2;
const unsigned ToolBarTextEdit  = 3;
const unsigned ToolBarMsgEdit	= 4;

/* Menu (create and remove)
   param is toolbar id */
const unsigned EventMenuCreate = 0x0503;
const unsigned EventMenuRemove = 0x0504;

/* Base menu for mainwindow */
const unsigned MenuMain = 1;
const unsigned MenuGroup = 2;
const unsigned MenuContact = 3;
const unsigned MenuContactGroup = 4;
const unsigned MenuContainer = 5;
const unsigned MenuMessage = 6;

/* Commands - process command
   param is CommandDef* */

const unsigned EventCommandExec = 0x0505;

/* Commands - create command
   menu items & bar buttons
   param is CommandDef* */

const unsigned EventCommandCreate = 0x0506;

typedef struct CommandDef
{
    unsigned	id;				// Command ID
    const char	*text;			// Command name
    const char	*icon;			// Icon
    const char	*icon_on;		// Icon for checked state
    const char	*accel;			// Accel
    unsigned	bar_id;			// Toolbar ID
    unsigned	bar_grp;		// Toolbar GRP
    unsigned	menu_id;		// Menu ID
    unsigned	menu_grp;		// Menu GRP
    unsigned	popup_id;		// Popup ID
    unsigned	flags;			// Command flags
    void		*param;			// Paramether from MenuSetParam
    char		*text_wrk;		// Text for check state (utf8)
} CommandDef;

class EXPORT Command
{
public:
    Command();
    CommandDef &operator=(const CommandDef&);
    CommandDef *operator->() { return &m_cmd; }
    operator void*()		 { return &m_cmd; }
    operator CommandDef*()	 { return &m_cmd; }
protected:
    CommandDef m_cmd;
};

const unsigned COMMAND_DEFAULT		= 0;
const unsigned COMMAND_IMPORTANT	= 0x0001;
const unsigned COMMAND_TITLE		= 0x0002;
const unsigned COMMAND_CHECKED		= 0x0004;
const unsigned COMMAND_CHECK_STATE	= 0x0008;
const unsigned COMMAND_DISABLED		= 0x0010;
const unsigned COMMAND_GLOBAL_ACCEL	= 0x0020;
const unsigned COMMAND_RECURSIVE	= 0x0040;
const unsigned COMMAND_NEW_POPUP	= 0x0080;

const unsigned BTN_TYPE				= 0xF000;
const unsigned BTN_DEFAULT			= 0x0000;
const unsigned BTN_PICT				= 0x1000;
const unsigned BTN_COMBO			= 0x2000;
const unsigned BTN_COMBO_CHECK		= 0x3000;
const unsigned BTN_EDIT				= 0x4000;

const unsigned BTN_HIDE				= 0x10000;
const unsigned BTN_NO_BUTTON		= 0x20000;

/* Command - remove command id */
const unsigned EventCommandRemove = 0x0507;


/* Event real create toolbar
   param is ToolBarShow*
*/
const unsigned EventShowBar = 0x0508;

typedef struct BarShow
{
    QMainWindow	*parent;
    unsigned	bar_id;
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
    QWidget	*widget;
    bool	bDown;
} WindowDef;

/* Event Check command state
   param is CommandDef*
*/
const unsigned EventCheckState	= 0x050C;

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
    void	 *param;
    int		 key;			// If key != 0 process accel
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

const unsigned EventSetMainIcon		= 0x0520;
const unsigned EventSetMainText		= 0x0521;

const unsigned EventCommandChange	= 0x0522;
const unsigned EventCommandChecked	= 0x0523;
const unsigned EventCommandDisabled	= 0x0524;
const unsigned EventCommandWidget	= 0x0525;

const unsigned EventClientChanged	= 0x0530;

/* Event - get home dir
   param is string fileName
*/

const unsigned EventHomeDir		= 0x0601;

/* Event go URL
   param is char *url
*/

const unsigned EventGoURL		= 0x0602;

/* Event get URL
   return char *url
*/

const unsigned EventGetURL		= 0x0603;

/* Event play sound
   param is char *wav
*/

const unsigned EventPlaySound	= 0x0604;

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
    QPainter *p;		// painter
    QPoint	 pos;		// position
    QSize	 size;		// size
    int		 height;	// item height
    int		 margin;	// icon margin
    QWidget	 *win;		// window
    bool	 isStatic;	// need static background
} PaintView;

/* Event repaint list view */
const unsigned EventRepaintView = 0x0702;

/* Event draw view item */
const unsigned EventDrawItem = 0x0703;

/* Event encode message text for show
   param is string* (string in utf-8)
*/
const unsigned EventEncodeText = 0x0801;

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


/* Event set common icon
   param is const char *icon
*/
const unsigned EventCommonIcon		= 0x0920;

/* Event set common title
   param is const char *state
*/
const unsigned EventCommonTitle		= 0x0921;

typedef struct addContact
{
    const char		*proto;
    const char		*addr;
    const char		*nick;
    unsigned		group;
} addContact;

/* Event for add contact
   param is addContact*
   return Contact*
*/
const unsigned EventAddContact		= 0x0930;

/* Event for remove contact
   param is char *address
*/
const unsigned EventDeleteContact	= 0x0931;

/* Event for get contact IP
   param is Contact*
*/
const unsigned EventGetContactIP	= 0x0932;

/* Event netvork state changed
*/
const unsigned EventNetworkChanged	= 0x1000;

/* Event socket connect
   param is ConnectParam *
*/
const unsigned EventSocketConnect	= 0x1001;

class ClientSocket;
class ServerSocketNotify;
class TCPClient;

typedef struct ConnectParam
{
    ClientSocket	*socket;
    TCPClient		*client;
    const char		*host;
    unsigned short	port;
} ConnectParam;

/* Event socket listen
   param is ListenParam *
*/
const unsigned EventSocketListen	 = 0x1002;

typedef struct ListenParam
{
    ServerSocketNotify	*notify;
    TCPClient			*client;
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
    Message			*msg;
    const char		*dir;
    OverwriteMode	overwrite;
} messageAccept;

typedef struct messageDecline
{
    Message		*msg;
    const char	*reason;
} messageDecline;

typedef struct messageSend
{
    Message		*msg;
    string		*text;
} messageSend;

const unsigned EventMessageReceived	= 0x1100;
const unsigned EventMessageSent		= 0x1101;
const unsigned EventMessageCancel	= 0x1102;
const unsigned EventSent			= 0x1103;
const unsigned EventOpenMessage		= 0x1104;
const unsigned EventMessageRead		= 0x1105;
const unsigned EventMessageAcked	= 0x1106;
const unsigned EventMessageDeleted  = 0x1107;
const unsigned EventMessageAccept	= 0x1108;
const unsigned EventMessageDecline	= 0x1109;
const unsigned EventMessageSend		= 0x110A;
const unsigned EventSend			= 0x110B;

const unsigned EventFetchDone		= 0x1300;

typedef struct fetchData
{
    unsigned	req_id;
    unsigned	result;
    Buffer		*data;
    const char	*headers;
} fetchData;

const unsigned EventClientError		= 0x1301;

typedef struct clientErrorData
{
    Client		*client;
    const char	*err_str;
    char		*args;
    unsigned	code;
} clientErrorData;

const unsigned EventUser			= 0x10000;

// _____________________________________________________________________________________
// CommandsDef

class CommandsDefPrivate;
class CommandsListPrivate;
class CommandsDef;

class EXPORT CommandsList
{
public:
    CommandsList(CommandsDef &def, bool bFull = false);
    ~CommandsList();
    CommandDef *operator++();
private:
    CommandsListPrivate *p;
    friend class CommandsListPrivate;
};

class EXPORT CommandsDef
{
public:
    CommandsDef(unsigned id, bool bMenu);
    ~CommandsDef();
    unsigned id();
    bool isMenu();
    void setConfig(const char*);
    void set(CommandDef*);
private:
    CommandsDefPrivate *p;
    friend class CommandsList;
    friend class CommandsDefPrivate;
};

class CommandsMapPrivate;
class CommandsMapIteratorPrivate;

class EXPORT CommandsMap
{
public:
    CommandsMap();
    ~CommandsMap();
    CommandDef *find(unsigned id);
    bool add(CommandDef*);
    bool erase(unsigned id);
private:
    CommandsMapPrivate	*p;
    friend class CommandsMapIterator;
};

class EXPORT CommandsMapIterator
{
public:
    CommandsMapIterator(CommandsMap&);
    ~CommandsMapIterator();
    CommandDef *operator++();
private:
    CommandsMapIteratorPrivate *p;
};

// ____________________________________________________________________________________
// Configuration

typedef struct DataDef
{
    const char	*name;
    unsigned	type;
    unsigned	n_values;
    const char	*def_value;
} DataDef;

typedef union Data
{
    char			*ptr;
    unsigned long	value;
    bool			bValue;
} Data;

#define DATA(A)	((const char*)(A))

const unsigned DATA_STRING		= 0;
const unsigned DATA_LONG		= 1;
const unsigned DATA_ULONG		= 2;
const unsigned DATA_BOOL		= 3;
const unsigned DATA_STRLIST		= 4;
const unsigned DATA_UTF			= 5;
const unsigned DATA_IP			= 6;
const unsigned DATA_STRUCT		= 7;
const unsigned DATA_UTFLIST		= 8;
const unsigned DATA_OBJECT		= 9;

EXPORT void free_data(const DataDef *def, void *data);
EXPORT void load_data(const DataDef *def, void *data, const char *config);
EXPORT string save_data(const DataDef *def, void *data);

EXPORT bool set_str(char **str, const char *value);
EXPORT const char *get_str(const Data &strlist, unsigned index);
EXPORT void clear_list(Data *strlist);
EXPORT void set_str(Data *strlist, unsigned index, const char *value);
EXPORT unsigned long get_ip(Data &ip);
EXPORT const char *get_host(Data &ip);
EXPORT bool set_ip(Data *ip, unsigned long value, const char *host=NULL);

#define PROP_STRLIST(A)	\
	const char *get##A(unsigned index) const { return get_str(data.A, index); } \
	void set##A(unsigned index, const char *value) { set_str(&data.A, index, value); } \
	void clear##A()	{ clear_list(&data.A); }

#define PROP_UTFLIST(A)	\
	QString get##A(unsigned index) const { return QString::fromUtf8(get_str(data.A, index)); } \
	void set##A(unsigned index, const QString &value) { set_str(&data.A, index, value.utf8()); } \
	void clear##A()	{ clear_list(&data.A); }

#define PROP_STR(A) \
	const char *get##A() const { return data.A.ptr ? data.A.ptr : ""; } \
	bool set##A(const char *r) { return set_str(&data.A.ptr, r); }

#define PROP_UTF8(A) \
	QString get##A() const { return data.A.ptr ? QString::fromUtf8(data.A.ptr) : QString(""); } \
	bool set##A(const QString &r) { return set_str(&data.A.ptr, r.utf8()); }

#define VPROP_UTF8(A) \
	virtual QString get##A() const { return data.A.ptr ? QString::fromUtf8(data.A.ptr) : QString(""); } \
	bool set##A(const QString &r) { return set_str(&data.A.ptr, r.utf8()); }

#define PROP_LONG(A) \
	unsigned long get##A() const { return data.A.value; } \
	void set##A(unsigned long r) { data.A.value = r; }

#define PROP_ULONG(A) \
	unsigned long get##A() const { return data.A.value; } \
	void set##A(unsigned long r) { data.A.value = r; }

#define PROP_USHORT(A) \
	unsigned short get##A() const { return (unsigned short)(data.A.value); } \
	void set##A(unsigned short r) { data.A.value = r; }

#define PROP_BOOL(A) \
	bool get##A() const { return data.A.bValue; } \
	void set##A(bool r) { data.A.bValue = r; }

#define VPROP_BOOL(A) \
	bool get##A() const { return data.A.bValue; } \
	virtual void set##A(bool r) { data.A.bValue = r; }

#define PROP_IP(A)	\
	unsigned long get##A()	const { return get_ip(data.A); } \
	const char *host##A() { return get_host(data.A); } \
	void set##A(unsigned long r) { set_ip(&data.A, r); }

const int LEFT		= 0;
const int TOP		= 1;
const int WIDTH		= 2;
const int HEIGHT	= 3;
const int DESKTOP	= 4;

EXPORT void saveGeometry(QWidget*, Data[5]);
EXPORT void restoreGeometry(QWidget*, Data[5], bool bPos, bool bSize);
EXPORT void saveToolbar(QToolBar*, Data[7]);
EXPORT void restoreToolbar(QToolBar*, Data[7]);
EXPORT bool cmp(char *s1, char *s2);

// _____________________________________________________________________________________
// Utilities

/* Get full path */
EXPORT string app_file(const char *f);

/* Get user file */
EXPORT string user_file(const char *f);

/* Make directory */
EXPORT bool makedir(char *p);

/* Save state */
EXPORT void save_state();

EXPORT string number(unsigned n);
EXPORT string trim(const char *str);
EXPORT QString trim(const QString &str);
EXPORT QString formatDateTime(unsigned long t);
EXPORT QString formatDate(unsigned long t);
EXPORT QString formatAddr(Data &addr, unsigned port);
EXPORT bool getLine(QFile &f, string &s);
EXPORT string getToken(string &from, char c, bool bUnEscape=true);
EXPORT QString getToken(QString &from, char c, bool bUnEsacpe=true);
EXPORT string quoteChars(const char *from, const char *chars);
EXPORT QString quoteChars(const QString &from, const char *chars, bool bQuoteSlash=true);
EXPORT char fromHex(char);
EXPORT string unquoteString(const char *p);


// _____________________________________________________________________________________
// Message

const unsigned MessageGeneric		= 1;
const unsigned MessageSMS			= 2;
const unsigned MessageFile			= 3;
const unsigned MessageAuthRequest	= 4;
const unsigned MessageAuthGranted	= 5;
const unsigned MessageAuthRefused	= 6;
const unsigned MessageAdded			= 7;
const unsigned MessageRemoved		= 8;
const unsigned MessageStatus		= 9;
const unsigned MessageTypingStart	= 10;
const unsigned MessageTypingStop	= 11;
const unsigned MessageUrl			= 12;
const unsigned MessageContacts		= 13;

// Message flags:

const unsigned	MESSAGE_RECEIVED	= 0x00000001;
const unsigned	MESSAGE_RICHTEXT	= 0x00000002;
const unsigned	MESSAGE_SECURE		= 0x00000004;
const unsigned	MESSAGE_URGENT		= 0x00000008;
const unsigned	MESSAGE_LIST		= 0x00000010;
const unsigned	MESSAGE_NOVIEW		= 0x00000020;

const unsigned	MESSAGE_SAVEMASK	= 0x0000FFFF;

const unsigned	MESSAGE_TRANSLIT	= 0x00010000;
const unsigned	MESSAGE_1ST_PART	= 0x00020000;
const unsigned  MESSAGE_NOHISTORY	= 0x00040000;
const unsigned	MESSAGE_LAST		= 0x00080000;
const unsigned	MESSAGE_MULTIPLY	= 0x00100000;
const unsigned	MESSAGE_FORWARD		= 0x00200000;
const unsigned	MESSAGE_INSERT		= 0x00400000;
const unsigned	MESSAGE_OPEN		= 0x00800000;
const unsigned  MESSAGE_NORAISE		= 0x01000000;
const unsigned	MESSAGE_TEMP		= 0x10000000;

typedef struct MessageData
{
    Data		Text;			// Message text (UTF-8)
    Data		Flags;			//
    Data		Background;
    Data		Foreground;
    Data		Time;
    Data		Font;
    Data		Error;
    Data		RetryCode;
} MessageData;

class EXPORT Message
{
public:
    Message(unsigned type = MessageGeneric, const char *cfg = NULL);
    virtual ~Message();
    unsigned type() const { return m_type; }
    unsigned id() const { return m_id; }
    void setId(unsigned id) { m_id = id; }
    unsigned contact() const { return m_contact; }
    void setContact(unsigned contact) { m_contact = contact; }
    virtual string save();
    virtual unsigned baseType() { return m_type; }
    QString getPlainText();
    QString getRichText();
    VPROP_UTF8(Text)
    virtual bool setText(const char *text);
    PROP_ULONG(Flags)
    PROP_ULONG(Background)
    PROP_ULONG(Foreground)
    PROP_ULONG(Time)
    PROP_STR(Error);
    PROP_STR(Font);
    PROP_ULONG(RetryCode);
    const char *client() const { return m_client.c_str(); }
    void setClient(const char *client);
    virtual QString presentation();
protected:
    unsigned	m_id;
    unsigned	m_contact;
    unsigned	m_type;
    string		m_client;
    MessageData	data;
};

typedef struct MessageSMSData
{
    Data	Phone;
    Data	Network;
} MessageSMSData;

class EXPORT SMSMessage : public Message
{
public:
    SMSMessage(const char *cfg=NULL);
    ~SMSMessage();
    PROP_UTF8(Phone);
    PROP_UTF8(Network);
    virtual string save();
    virtual QString presentation();
protected:
    MessageSMSData data;
};

typedef struct MessageFileData
{
    Data		File;
    Data		Description;
    Data		Size;
} MessageFileData;

class FileMessageIteratorPrivate;
class FileMessage;

class FileTransferNotify
{
public:
    FileTransferNotify() {}
    virtual ~FileTransferNotify() {}
    virtual void process() = 0;
    virtual void transfer(bool) = 0;
    virtual void createFile(const QString &name, unsigned size, bool bCanResume) = 0;
};

const unsigned NO_FILE	= (unsigned)(-1);

class EXPORT FileTransfer
{
public:
    FileTransfer(FileMessage *msg);
    virtual ~FileTransfer();
    void setNotify(FileTransferNotify*);
    FileTransferNotify			*notify() { return m_notify; }
    unsigned file()				{ return m_nFile; }
    unsigned files()			{ return m_nFiles; }
    unsigned bytes()			{ return m_bytes; }
    unsigned totalBytes()		{ return m_totalBytes; }
    unsigned fileSize()			{ return m_fileSize; }
    unsigned totalSize()		{ return m_totalSize; }
    unsigned speed()			{ return m_speed; }
    unsigned transferBytes()	{ return m_transferBytes; }
    virtual void setSpeed(unsigned speed);
    QString  dir()				{ return m_dir; }
    void setDir(const QString &dir) { m_dir = dir; }
    OverwriteMode overwrite()	{ return m_overwrite; }
    void setOverwrite(OverwriteMode overwrite) { m_overwrite = overwrite; }
    enum State
    {
        Unknown,
        Listen,
        Connect,
        Negotiation,
        Read,
        Write,
        Done,

        Wait,
        Error
    };
    State state()	{ return m_state; }
    QFile			*m_file;
    virtual void	startReceive(unsigned pos) = 0;
    virtual void	setError();
    void	addFile(const QString &file, unsigned size);
protected:
    bool	openFile();
    FileMessage			*m_msg;
    FileTransferNotify	*m_notify;
    unsigned m_nFile;
    unsigned m_nFiles;
    unsigned m_bytes;
    unsigned m_totalBytes;
    unsigned m_fileSize;
    unsigned m_totalSize;
    unsigned m_speed;
    unsigned m_transferBytes;

    unsigned m_sendTime;
    unsigned m_sendSize;
    unsigned m_transfer;

    OverwriteMode m_overwrite;
    QString	 m_dir;
    State	 m_state;
    friend class FileMessage;
};

class EXPORT FileMessage : public Message
{
public:
    FileMessage(unsigned type=MessageFile, const char *cfg=NULL);
    ~FileMessage();
    PROP_UTF8(File);
    unsigned getSize();
    void     setSize(unsigned);
    virtual	string save();
    virtual QString presentation();
    virtual QString getDescription();
    bool    setDescription(const QString&);
    void	addFile(const QString&);
    void	addFile(const QString&, unsigned size);
    class EXPORT Iterator
    {
    public:
        Iterator(const FileMessage&);
        ~Iterator();
        const QString *operator++();
        const QString *operator[](unsigned);
        void reset();
        unsigned count();
        unsigned size();
    protected:
        FileMessageIteratorPrivate *p;
        friend class FileMessage;
    };
    FileTransfer	*m_transfer;
protected:
    MessageFileData	data;
    friend class FileTransfer;
    friend class Iterator;
};

class EXPORT AuthMessage : public Message
{
public:
    AuthMessage(unsigned type, const char *cfg=NULL)
: Message(type, cfg) {}
    virtual QString presentation();
};

typedef struct MessageUrlData
{
    Data	Url;
} MessageUrlData;

class EXPORT UrlMessage : public Message
{
public:
    UrlMessage(unsigned type=MessageUrl, const char *cfg=NULL);
    ~UrlMessage();
    virtual string  save();
    virtual QString presentation();
    VPROP_UTF8(Url)
protected:
    MessageUrlData data;
};

typedef struct MessageContactsData
{
    Data	Contacts;
} MessageContactsData;

class EXPORT ContactsMessage : public Message
{
public:
    ContactsMessage(unsigned type=MessageContacts, const char *cfg=NULL);
    ~ContactsMessage();
    virtual string  save();
    virtual QString presentation();
    VPROP_UTF8(Contacts);
protected:
    MessageContactsData data;
};

typedef struct MessageStatusData
{
    Data	Status;
} MessageStatusData;

class EXPORT StatusMessage : public Message
{
public:
    StatusMessage(const char *cfg=NULL);
    PROP_ULONG(Status);
    virtual string save();
    virtual QString presentation();
protected:
    MessageStatusData data;
};

// _____________________________________________________________________________________
// Contact list

class EXPORT UserData
{
public:
    UserData();
    ~UserData();
    string save();
    void load(unsigned long id, const DataDef *def, const char *cfg);
    void *getUserData(unsigned id, bool bCreate);
    void freeUserData(unsigned id);
protected:
    unsigned n_data;
    void **userData;
};

class EXPORT Client;
class ClientUserDataPrivate;
class ClientDataIterator;
class ClientDataIteratorPrivate;

typedef struct clientData		// Base struct for all clientData
{
    Data	Sign;
    Data	LastSend;
} clientData;

class EXPORT ClientUserData
{
public:
    ClientUserData();
    ~ClientUserData();
    string save();
    void load(Client *client, const char *cfg);
    void *getData(Client *client);
    bool have(void*);
    void *createData(Client *client);
    void freeData(void*);
    void freeClientData(Client *client);
    void sort();
    void join(ClientUserData &data);
    void join(clientData *cData, ClientUserData &data);
    unsigned size();
    Client *activeClient(void *&data, Client *client);
    string property(const char *name);
protected:
    ClientUserDataPrivate *p;
    friend class ClientDataIterator;
};

class EXPORT ClientDataIterator
{
public:
    ClientDataIterator(ClientUserData &data, Client *client=NULL);
    ~ClientDataIterator();
    clientData *operator ++();
    Client *client();
    void reset();
protected:
    ClientDataIteratorPrivate *p;
};

class EXPORT PacketType
{
public:
    PacketType(unsigned id, const char *name, bool bText);
    ~PacketType();
    unsigned id() { return m_id; }
    const char *name() { return m_name; }
    bool isText() { return m_bText; }
protected:
    unsigned m_id;
    char	 *m_name;
    bool	 m_bText;
};

const unsigned PHONE	= 0;
const unsigned FAX		= 1;
const unsigned CELLULAR	= 2;
const unsigned PAGER	= 3;

typedef struct ContactData
{
    Data			Group;		// Group ID
    Data			Name;		// Contact Display Name (UTF-8)
    Data			Ignore;		// In ignore list
    Data			LastActive;
    Data			EMails;
    Data			Phones;
    Data			PhoneStatus;
    Data			FirstName;
    Data			LastName;
    Data			Notes;
    Data			Temporary;
} ContactData;

const unsigned CONTACT_TEMP	= 1;
const unsigned CONTACT_DRAG	= 2;

class EXPORT Contact
{
public:
    Contact(unsigned long id = 0, const char *cfg = NULL);
    virtual ~Contact();
    unsigned long id() { return m_id; }
    PROP_ULONG(Group)
    PROP_UTF8(Name)
    PROP_BOOL(Ignore)
    PROP_ULONG(LastActive)
    PROP_UTF8(EMails)
    PROP_UTF8(Phones)
    PROP_ULONG(PhoneStatus)
    PROP_UTF8(FirstName)
    PROP_UTF8(LastName)
    PROP_UTF8(Notes)
    PROP_ULONG(Temporary)
    void *getUserData(unsigned id, bool bCreate = false);
    UserData userData;
    ClientUserData clientData;
    bool setFirstName(const QString &name, const char *client);
    bool setLastName(const QString &name, const char *client);
    bool setEMails(const QString &mails, const char *client);
    bool setPhones(const QString &phones, const char *client);
    unsigned long contactInfo(unsigned &style, const char *&statusIcon, string *icons = NULL);
    QString tipText();
    ContactData data;
    const DataDef *dataDef();
    void setup();
protected:
    unsigned long m_id;
    friend class ContactList;
    friend class ContactListPrivate;
};

typedef struct GroupData
{
    Data		Name;		// Display name (UTF-8)
} GroupData;

class EXPORT Group
{
public:
    Group(unsigned long id = 0, const char *cfg = NULL);
    virtual ~Group();
    unsigned long id() { return m_id; }
    PROP_UTF8(Name)
    void *getUserData(unsigned id, bool bCreate = false);
    UserData userData;
    ClientUserData clientData;
protected:
    unsigned long m_id;
    GroupData data;
    friend class ContactList;
    friend class ContactListPrivate;
};

const unsigned STATUS_UNKNOWN	= 0;
const unsigned STATUS_OFFLINE	= 1;
const unsigned STATUS_NA		= 10;
const unsigned STATUS_DND		= 20;
const unsigned STATUS_AWAY		= 30;
const unsigned STATUS_ONLINE	= 40;
const unsigned STATUS_FFC		= 50;

const unsigned CONTACT_UNDERLINE	= 0x0001;
const unsigned CONTACT_ITALIC		= 0x0002;
const unsigned CONTACT_STRIKEOUT	= 0x0004;

const unsigned PROTOCOL_INFO			= 0x00010000;
const unsigned PROTOCOL_SEARCH			= 0x00020000;
const unsigned PROTOCOL_SEARCH_ONLINE	= 0x00040000;
const unsigned PROTOCOL_INVISIBLE		= 0x00080000;
const unsigned PROTOCOL_AR				= 0x00100000;
const unsigned PROTOCOL_AR_USER			= 0x00200000;
const unsigned PROTOCOL_FOLLOWME		= 0x00400000;
const unsigned PROTOCOL_ANY_PORT		= 0x00800000;
const unsigned PROTOCOL_NOSMS			= 0x01000000;
const unsigned PROTOCOL_NOPROXY			= 0x02000000;
const unsigned PROTOCOL_TEMP_DATA		= 0x04000000;
const unsigned PROTOCOL_NODATA			= 0x08000000;
const unsigned PROTOCOL_NO_AUTH			= 0x10000000;

class ContactList;
class Client;

class EXPORT Protocol
{
public:
    Protocol(Plugin *plugin);
    virtual ~Protocol();
    Plugin  *plugin() { return m_plugin; }
    virtual Client	*createClient(const char *cfg) = 0;
    virtual const CommandDef *description() = 0;
    virtual const CommandDef *statusList() = 0;
    virtual const DataDef *userDataDef() = 0;
protected:
    Plugin *m_plugin;
};

typedef struct ClientData
{
    Data	ManualStatus;
    Data	CommonStatus;
    Data	Password;
    Data	SavePassword;
    Data	PreviousPassword;
    Data	Invisible;
    Data	LastSend;
} ClientData;

const unsigned AuthError = 1;

class EXPORT Client
{
public:
    Client(Protocol*, const char *cfg);
    virtual ~Client();
    enum State
    {
        Offline,
        Connecting,
        Connected,
        Error
    };
    virtual string name() = 0;
    virtual string dataName(void*) = 0;
    Protocol *protocol() { return m_protocol; }
    virtual QWidget	*setupWnd() = 0;
    virtual void setStatus(unsigned status, bool bCommon);
    virtual string getConfig();
    virtual bool compareData(void*, void*);
    virtual bool isMyData(clientData*&, Contact*&) = 0;
    virtual bool createData(clientData*&, Contact*) = 0;
    virtual void contactInfo(void *clientData, unsigned long &status, unsigned &style, const char *&statusIcon, string *icons = NULL);
    virtual QString ownerName();
    virtual QString contactName(void *clientData);
    virtual void setupContact(Contact*, void *data) = 0;
    virtual bool send(Message*, void *data) = 0;
    virtual bool canSend(unsigned type, void *data) = 0;
    virtual QString contactTip(void *clientData);
    virtual CommandDef *infoWindows(Contact *contact, void *clientData);
    virtual QWidget *infoWindow(QWidget *parent, Contact *contact, void *clientData, unsigned id);
    virtual CommandDef *configWindows();
    virtual QWidget *configWindow(QWidget *parent, unsigned id);
    virtual void updateInfo(Contact *contact, void *clientData);
    virtual void setClientInfo(void *data);
    virtual QWidget *searchWindow();
    void    setState(State, const char *text = NULL, unsigned code = 0);
    State   getState() { return m_state; }
    virtual unsigned getStatus();
    PROP_ULONG(ManualStatus)
    PROP_BOOL(CommonStatus)
    PROP_UTF8(Password)
    PROP_BOOL(SavePassword)
    PROP_UTF8(PreviousPassword)
    PROP_STRLIST(LastSend)
    VPROP_BOOL(Invisible)
protected:
    void  freeData();
    State m_state;
    unsigned m_status;
    ClientData	data;
    Protocol	*m_protocol;
};

typedef struct UserDataDef
{
    unsigned		id;
    string			name;
    const DataDef	*def;
} UserDataDef;

class ContactListPrivate;
class ContactIteratorPrivate;
class GroupIteratorPrivate;
class ClientIteratorPrivate;
class ProtocolIteratorPrivate;
class PacketIteratorPrivate;
class UserDataIteratorPrivate;

class EXPORT ContactList
{
public:
    ContactList();
    virtual ~ContactList();
    Contact *owner();
    void clear();
    void load();
    void save();
    void addClient(Client*);
    unsigned registerUserData(const char *name, const DataDef *def);
    void unregisterUserData(unsigned id);
    Contact *contact(unsigned long id=0, bool isNew=false);
    Group   *group(unsigned long id=0, bool isNew=false);
    void addContact(Contact *contact);
    int  groupIndex(unsigned long id);
    int	 groupCount();
    bool moveGroup(unsigned long id, bool bUp);
    bool moveClient(Client *client, bool bUp);
    class EXPORT GroupIterator
    {
    public:
        Group *operator++();
        GroupIterator();
        ~GroupIterator();
        void reset();
    protected:
        GroupIteratorPrivate *p;
        friend class ContactList;
    };
    class EXPORT ContactIterator
    {
    public:
        Contact *operator++();
        ContactIterator();
        ~ContactIterator();
        void reset();
    protected:
        ContactIteratorPrivate *p;
        friend class ContactList;
    };
    class EXPORT ProtocolIterator
    {
    public:
        Protocol *operator++();
        ProtocolIterator();
        ~ProtocolIterator();
        void reset();
    protected:
        ProtocolIteratorPrivate *p;
        friend class ContactList;
    };
    class EXPORT PacketIterator
    {
    public:
        PacketType *operator++();
        PacketIterator();
        ~PacketIterator();
        void reset();
    protected:
        PacketIteratorPrivate *p;
        friend class ContactList;
    };
    class EXPORT UserDataIterator
    {
    public:
        UserDataDef *operator++();
        UserDataIterator();
        ~UserDataIterator();
    protected:
        UserDataIteratorPrivate *p;
        friend class ContactList;
    };
    void *getUserData(unsigned id);
    unsigned nClients();
    Client *getClient(unsigned n);
    void clearClients();
    void addPacketType(unsigned id, const char *name, bool bText=false);
    void removePacketType(unsigned id);
    PacketType *getPacketType(unsigned i);
    Contact *contactByPhone(const char *phone);
    Contact *contactByMail(const QString &_mail, const QString &_name);
    static bool cmpPhone(const char *p1, const char *p2);
protected:
    ContactListPrivate *p;
    friend class Contact;
    friend class Group;
    friend class UserData;
    friend class GroupIterator;
    friend class GroupIteratorPrivate;
    friend class ContactIterator;
    friend class ContactIteratorPrivate;
    friend class Client;
    friend class ClientIterator;
    friend class ClientIteratorPrivate;
    friend class Protocol;
    friend class ProtocolIterator;
    friend class ProtocolIteratorPrivate;
    friend class PacketIterator;
    friend class PacketIteratorPrivate;
    friend class UserDataIterator;
};

EXPORT ContactList *getContacts();

// ____________________________________________________________________________________
// Url procs

enum quoteMode
{
    quoteHTML,
    quoteXML,
    quoteNOBR
};

EXPORT QString	unquoteText(const QString &text);
EXPORT QString  quoteString(const QString &str, quoteMode mode = quoteHTML);
EXPORT QString	unquoteString(const QString &str);

// ____________________________________________________________________________________
// Log procs

const unsigned short L_ERROR 		= 0x01;
const unsigned short L_WARN  		= 0x02;
const unsigned short L_DEBUG		= 0x04;

const unsigned short L_PACKET_IN	= 0x10;
const unsigned short L_PACKET_OUT	= 0x20;

EXPORT void log(unsigned short level, const char *fmt, ...);
EXPORT string make_packet_string(LogInfo *l);

// _____________________________________________________________________________________
// Data

typedef struct ext_info
{
    const char *szName;
    unsigned short nCode;
} ext_info;

EXPORT const ext_info *getCountries();
EXPORT const ext_info *getCountryCodes();

typedef struct pager_provider
{
    const char *szName;
    const char *szGate;
} pager_provider;

EXPORT const pager_provider *getProviders();

// _____________________________________________________________________________________
// Fetch Data

EXPORT unsigned fetch(const char *url, Buffer *postData=NULL, const char *headers=NULL, bool bRedirect=true);

// _____________________________________________________________________________________
// User interface

#ifdef WIN32
EXPORT void setWndProc(QWidget*);
EXPORT void mySetCaption(QWidget *w, const QString &caption);
EXPORT void translate();
EXPORT unsigned wndMessage();
#define SET_WNDPROC(A)	setWndProc(this);
#define setCaption(s)	mySetCaption(this, s);
#else
#ifndef QT_MACOSX_VERSION
EXPORT void setWndClass(QWidget*, const char*);
#define SET_WNDPROC(A)  setWndClass(this, A);
#else
#define SET_WNDPROC(A)
#endif
#endif

/* Raises widget w (on the KDE-Desktop desk) */
EXPORT bool raiseWindow(QWidget *w, unsigned desk = 0);
EXPORT void setButtonsPict(QWidget *w);

EXPORT const QIconSet *Icon(const char *name);
EXPORT QPixmap Pict(const char *name);

EXPORT void setAboutData(KAboutData*);
EXPORT KAboutData *getAboutData();

EXPORT void initCombo(QComboBox *cmb, unsigned short code, const ext_info *tbl, bool bAddEmpty = true, const ext_info *tbl1 = NULL);
EXPORT unsigned short getComboValue(QComboBox *cmb, const ext_info *tbl, const ext_info *tbl1 = NULL);
EXPORT void set_value(QLineEdit*, char *&value);
EXPORT void disableWidget(QWidget *w);

EXPORT QString toTranslit(const QString&);
EXPORT bool isLatin(const QString&);
EXPORT QString getPart(QString&, unsigned size);
EXPORT QString getRichTextPart(QString&, unsigned size);

typedef struct smile
{
    const char	*exp;
    const char	*paste;
    const char	*title;
} smile;

EXPORT const smile *smiles(unsigned n);
EXPORT const smile *defaultSmiles(unsigned n);
EXPORT void setSmiles(const char *smiles_str);

EXPORT unsigned screens();
EXPORT QRect screenGeometry(unsigned nScreen);
EXPORT QRect screenGeometry();

EXPORT unsigned get_random();

class EXPORT my_string
{
public:
    my_string(const char *str);
    my_string(const my_string &str);
    ~my_string();
    bool operator < (const my_string &str) const;
    void operator = (const my_string &str);
    const char *c_str() const { return m_str->c_str(); }
    string *m_str;
};

};

using namespace SIM;

EXPORT QString g_i18n(const char *text, Contact *contact);

#endif
