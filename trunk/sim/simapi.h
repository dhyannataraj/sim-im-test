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

#include <qglobal.h>
#include <qdockwindow.h>
#include <qstring.h>
#include <qwidget.h>

class KAboutData;
class QComboBox;
class QMainWindow;
class QFile;
class QIconSet;
class QIcon;
class QLibrary;
class QObject;
class QPixmap;
class QToolBar;

#ifdef Q_CC_MSVC
// "conditional expression is constant" (W4)
# pragma warning(disable: 4127)
// "'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'" (W1)
# pragma warning(disable: 4251)
// "function' : function not inlined" (W4)4786
# pragma warning(disable: 4710)
#endif

#ifdef Q_CC_MSVC
# define DEPRECATED __declspec(deprecated)
#elif defined Q_CC_GNU
# define DEPRECATED __attribute__ ((deprecated))
#else
# define DEPRECATED
#endif

#ifdef HAVE_GCC_VISIBILITY      // @linux: all gcc >= 4.0 have visibility support - please add a check for configure
# define SIM_EXPORT __attribute__ ((visibility("default")))
# define SIM_IMPORT __attribute__ ((visibility("default")))
# define EXPORT_PROC extern "C" __attribute__ ((visibility("default")))
#elif defined(_WIN32) || defined(_WIN64)
# define SIM_EXPORT __declspec(dllexport)
# define SIM_IMPORT __declspec(dllimport)
# define EXPORT_PROC extern "C" __declspec(dllexport)
#else
# define SIM_EXPORT
# define SIM_IMPORT
# define EXPORT_PROC extern "C"
#endif

#ifdef SIMAPI_EXPORTS   // should be set when simapi-lib is build - please add a check for configure
# define EXPORT SIM_EXPORT
#else
# define EXPORT SIM_IMPORT
#endif

#ifndef HAVE_STRCASECMP
EXPORT int strcasecmp(const char *a, const char *b);
#endif

#include <string>

#ifdef Q_OS_WIN
# ifndef snprintf
#  define snprintf _snprintf
# endif
#endif

#ifndef COPY_RESTRICTED
# define COPY_RESTRICTED(A) \
    private: \
        A(const A&); \
        A &operator = (const A&);
#endif

#ifdef USE_KDE
# include <klocale.h>
#else
EXPORT QString i18n(const char *text);
EXPORT QString i18n(const char *text, const char *comment);
EXPORT QString i18n(const char *singular, const char *plural, unsigned long n);
EXPORT inline QString tr2i18n(const char* message, const char* =0) { return i18n(message); }
EXPORT void resetPlural();
# ifndef I18N_NOOP
#  define I18N_NOOP(A)  A
# endif
#endif

#include "event.h"

namespace SIM
{

// _________________________________________________________________________________
/* PluginManager - base class for main application */

class ContactList;
class SocketFactory;
class Client;

class EXPORT PluginManager
{
public:
    PluginManager(int argc, char **argv);
    ~PluginManager();
    bool isLoaded();
    static ContactList          *contacts;
    static SocketFactory        *factory;
private:
    class PluginManagerPrivate *p;

    COPY_RESTRICTED(PluginManager)
};

// __________________________________________________________________________________
/* Plugin prototype */

class EXPORT Plugin
{
public:
    Plugin(unsigned base);
    virtual ~Plugin() {}
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual std::string getConfig();
    unsigned registerType();
    void boundTypes();
protected:
    unsigned m_current;
    unsigned m_base;
};

typedef Plugin *createPlugin(unsigned base, bool bStart, Buffer *cfg);
typedef QStyle *createStyle();

const unsigned PLUGIN_KDE_COMPILE    = 0x0001;
#ifdef USE_KDE
const unsigned PLUGIN_DEFAULT        = PLUGIN_KDE_COMPILE;
#else
const unsigned PLUGIN_DEFAULT        = 0x0000;
#endif
const unsigned PLUGIN_NO_CONFIG_PATH = 0x0002 | PLUGIN_DEFAULT;
const unsigned PLUGIN_NOLOAD_DEFAULT = 0x0004 | PLUGIN_DEFAULT;

const unsigned PLUGIN_PROTOCOL       = 0x0008 | PLUGIN_NOLOAD_DEFAULT;
const unsigned PLUGIN_NODISABLE      = 0x0010;
const unsigned PLUGIN_RELOAD         = 0x0020;

const unsigned long ABORT_LOADING    = (unsigned long)(-1);

typedef struct PluginInfo           // Information in plugin
{
    const char      *title;         // User title
    const char      *description;   // Description
    const char      *version;       // Version
    createPlugin    *create;        // create proc
    unsigned        flags;          // plugin flags
} PluginInfo;

typedef struct pluginInfo
{
    Plugin          *plugin;
    QString         name;
    QString         filePath;
    Buffer          *cfg;           // configuration data
    bool            bDisabled;      // no load this plugin
    bool            bNoCreate;      // can't create plugin
    bool            bFromCfg;       // init state from config
    QLibrary        *module;        // so or dll handle
    PluginInfo      *info;
    unsigned        base;           // base for plugin types
} pluginInfo;

SIM_EXPORT PluginInfo *GetPluginInfo();

// _____________________________________________________________________________________
// CommandsDef

struct CommandDef;
class CommandsDef;

class EXPORT CommandsList
{
public:
    CommandsList(CommandsDef &def, bool bFull = false);
    ~CommandsList();
    CommandDef *operator++();
    void reset();
private:
    class CommandsListPrivate *p;
    friend class CommandsListPrivate;

    COPY_RESTRICTED(CommandsList)
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
    class CommandsDefPrivate *p;
    friend class CommandsList;
    friend class CommandsDefPrivate;

    COPY_RESTRICTED(CommandsDef)
};

class EXPORT CommandsMap
{
public:
    CommandsMap();
    ~CommandsMap();
    CommandDef *find(unsigned id);
    bool add(CommandDef*);
    bool erase(unsigned id);
    void clear();
private:
    class CommandsMapPrivate  *p;
    friend class CommandsMapIterator;

    COPY_RESTRICTED(CommandsMap)
};

class EXPORT CommandsMapIterator
{
public:
    CommandsMapIterator(CommandsMap&);
    ~CommandsMapIterator();
    CommandDef *operator++();
private:
    class CommandsMapIteratorPrivate *p;

    COPY_RESTRICTED(CommandsMapIterator)
};

// ____________________________________________________________________________________
// Configuration

enum DataType {
    DATA_UNKNOWN = 0,
    DATA_STRING,
    DATA_LONG,
    DATA_ULONG,
    DATA_BOOL,
    DATA_STRLIST,
    DATA_STRMAP = DATA_STRLIST,
    DATA_UTF,
    DATA_IP,
    DATA_STRUCT,
    DATA_UTFLIST,
    DATA_OBJECT,
    DATA_BINARY,    // QByteArray
    DATA_CSTRING    // to store data in an unknown encoding (icq's ServerText)
};

typedef struct DataDef
{
    const char  *name;
    DataType    type;
    unsigned    n_values;
    const char  *def_value;
} DataDef;

class IP;
class EXPORT Data
{
public:
    typedef QMap<unsigned, QString> STRING_MAP;
public:
    Data();
    Data(const Data& d);
    Data &operator =(const Data &);

    ~Data() { clear(false); }

    // DataName
    void setName(const QString &name);
    const QString &name() const;

    // DataType
    void setType(DataType eType);
    DataType type() const;

    void clear(bool bNew = true);

    // QString
    const QString &str() const;
    QString &str();
    bool setStr(const QString &s);
    // StringMap
    const STRING_MAP &strMap() const;
    STRING_MAP &strMap();
    bool setStrMap(const STRING_MAP &s);
    // Long
    long toLong() const;
    long &asLong();
    bool setLong(long d);
    // ULong
    unsigned long toULong() const;
    unsigned long &asULong();
    bool setULong(unsigned long d);
    // Bool
    bool toBool() const;
    bool &asBool();
    bool setBool(bool d);
    // class QObject
    const QObject* object() const;
    QObject* object();
    bool setObject(const QObject *);
    // class IP
    const IP* ip() const;
    IP* ip();
    bool setIP(const IP *);
    // Binary
    const QByteArray &toBinary() const;
    QByteArray &asBinary();
    bool setBinary(const QByteArray &d);
    // QString
    const QCString &cstr() const;
    QCString &cstr();
    bool setCStr(const QCString &s);

protected:
    void checkType(DataType type) const;
    DataType m_type;
    QString m_name;
    class DataPrivate *data;
};

#define DATA(A) ((const char*)(A))

EXPORT void free_data(const DataDef *def, void *data);
EXPORT void load_data(const DataDef *def, void *data, Buffer *config);
EXPORT std::string save_data(const DataDef *def, void *data);

EXPORT const QString &get_str(const Data &strlist, unsigned index);
EXPORT void set_str(Data *strlist, unsigned index, const QString &value);
EXPORT unsigned long get_ip(const Data &ip);
EXPORT const char *get_host(const Data &ip);
EXPORT bool set_ip(Data *ip, unsigned long value, const char *host=NULL);

#define PROP_STRLIST(A) \
    QString get##A(unsigned index) { return SIM::get_str(data.A, index); } \
    void set##A(unsigned index, const QString &value) { SIM::set_str(&data.A, index, value); } \
    void clear##A()  { data.A.clear(); }

#define PROP_UTFLIST(A) \
    QString get##A(unsigned index) { return SIM::get_str(data.A, index); } \
    void set##A(unsigned index, const QString &value) { SIM::set_str(&data.A, index, value); } \
    void clear##A()  { data.A.clear(); }

#define PROP_STR(A) \
    QString get##A() const { return data.A.str(); } \
    bool set##A(const QString &r) { return data.A.setStr( r ); }

#define PROP_UTF8(A) \
    QString get##A() const { return data.A.str(); } \
    bool set##A(const QString &r) { return data.A.setStr( r ); }

#define VPROP_UTF8(A) \
    virtual QString get##A() const { return data.A.str(); } \
    virtual bool set##A(const QString &r) { return data.A.setStr( r ); }

#define PROP_LONG(A) \
    long get##A() const { return data.A.toLong(); } \
    void set##A(long r) { data.A.setLong(r); }

#define PROP_ULONG(A) \
    unsigned long get##A() const { return data.A.toULong(); } \
    void set##A(unsigned long r) { data.A.setULong(r); }

#define PROP_USHORT(A) \
    unsigned short get##A() const { return (unsigned short)(data.A.toULong()); } \
    void set##A(unsigned short r) { data.A.setULong(r); }

#define PROP_BOOL(A) \
    bool get##A() const { return data.A.toBool(); } \
    void set##A(bool r) { data.A.setBool(r); }

#define VPROP_BOOL(A) \
    virtual bool get##A() const { return data.A.toBool(); } \
    virtual void set##A(bool r) { data.A.setBool(r); }

#define PROP_IP(A)  \
    unsigned long get##A()  const { return (data.A.ip() ? data.A.ip()->ip() : 0); } \
    const char *host##A() { return (data.A.ip() ? data.A.ip()->host() : ""); } \
    void set##A(unsigned long r) { SIM::set_ip(&data.A, r); }

#define PROP_CSTR(A) \
    QCString get##A() const { return data.A.cstr(); } \
    bool set##A(const QCString &r) { return data.A.setCStr( r ); }

const int LEFT      = 0;
const int TOP       = 1;
const int WIDTH     = 2;
const int HEIGHT    = 3;
const int DESKTOP   = 4;

typedef Data Geometry[5];
EXPORT void saveGeometry(QWidget*, Geometry);
EXPORT void restoreGeometry(QWidget*, Geometry, bool bPos, bool bSize);

EXPORT void saveToolbar(QToolBar*, Data[7]);
EXPORT void restoreToolbar(QToolBar*, Data[7]);

// _____________________________________________________________________________________
// Utilities

/* Get full path */
EXPORT QString app_file(const QString &f);

/* Get user file */
EXPORT QString user_file(const QString &f);

/* Make directory */
EXPORT bool makedir(char *p);

/* Save state */
EXPORT void save_state();

EXPORT DEPRECATED std::string number(unsigned n);
EXPORT DEPRECATED std::string trim(const char *str);
EXPORT QString formatDateTime(unsigned long t);
EXPORT QString formatDate(unsigned long t);
EXPORT QString formatAddr(const Data &addr, unsigned port);
EXPORT std::string getToken(std::string &from, char c, bool bUnEscape=true);
EXPORT std::string getToken(const char *&from, char c, bool bUnEscape=true);
EXPORT QString getToken(QString &from, char c, bool bUnEsacpe=true);
EXPORT QCString getToken(QCString &from, char c, bool bUnEsacpe=true);
EXPORT QString quoteChars(const QString &from, const char *chars, bool bQuoteSlash=true);
EXPORT char fromHex(char);


// _____________________________________________________________________________________
// Message

const unsigned MessageGeneric       = 1;
const unsigned MessageSMS           = 2;
const unsigned MessageFile          = 3;
const unsigned MessageAuthRequest   = 4;
const unsigned MessageAuthGranted   = 5;
const unsigned MessageAuthRefused   = 6;
const unsigned MessageAdded         = 7;
const unsigned MessageRemoved       = 8;
const unsigned MessageStatus        = 9;
const unsigned MessageTypingStart   = 10;
const unsigned MessageTypingStop    = 11;
const unsigned MessageUrl           = 12;
const unsigned MessageContacts      = 13;

// Message flags:

const unsigned  MESSAGE_RECEIVED    = 0x00000001;
const unsigned  MESSAGE_RICHTEXT    = 0x00000002;
const unsigned  MESSAGE_SECURE      = 0x00000004;
const unsigned  MESSAGE_URGENT      = 0x00000008;
const unsigned  MESSAGE_LIST        = 0x00000010;
const unsigned  MESSAGE_NOVIEW      = 0x00000020;

const unsigned  MESSAGE_SAVEMASK    = 0x0000FFFF;

const unsigned  MESSAGE_TRANSLIT    = 0x00010000;
const unsigned  MESSAGE_1ST_PART    = 0x00020000;
const unsigned  MESSAGE_NOHISTORY   = 0x00040000;
const unsigned  MESSAGE_LAST        = 0x00080000;
const unsigned  MESSAGE_MULTIPLY    = 0x00100000;
const unsigned  MESSAGE_FORWARD     = 0x00200000;
const unsigned  MESSAGE_INSERT      = 0x00400000;
const unsigned  MESSAGE_OPEN        = 0x00800000;
const unsigned  MESSAGE_NORAISE     = 0x01000000;
const unsigned  MESSAGE_TEMP        = 0x10000000;

typedef struct MessageData
{
    Data        Text;           // Message text (locale independent)
    Data        ServerText;     // Message text (locale dependent 8bit endcoding!)
    Data        Flags;          // Message flags from above, should be QFlags in Qt4
    Data        Background;
    Data        Foreground;
    Data        Time;
    Data        Font;
    Data        Error;
    Data        RetryCode;
    Data        Resource;
} MessageData;

class EXPORT Message
{
public:
    Message(unsigned type = MessageGeneric, Buffer *cfg = NULL);
    virtual ~Message();
    unsigned type() const { return m_type; }
    unsigned id() const { return m_id; }
    void setId(unsigned id) { m_id = id; }
    unsigned contact() const { return m_contact; }
    void setContact(unsigned contact) { m_contact = contact; }
    virtual std::string save();
    virtual unsigned baseType() { return m_type; }
    QString getPlainText();
    QString getRichText();
    virtual QString getText() const;
    void setText(const QString &text);
    PROP_CSTR(ServerText)
    PROP_ULONG(Flags)
    PROP_ULONG(Background)
    PROP_ULONG(Foreground)
    PROP_ULONG(Time)
    PROP_STR(Error);
    PROP_STR(Font);
    PROP_ULONG(RetryCode);
    PROP_UTF8(Resource);
    const QString &client() const { return m_client; }
    void setClient(const QString &client);
    virtual QString presentation();
    MessageData data;
protected:
    unsigned    m_id;
    unsigned    m_contact;
    unsigned    m_type;
    QString     m_client;
};

typedef struct MessageSMSData
{
    Data    Phone;
    Data    Network;
} MessageSMSData;

class EXPORT SMSMessage : public Message
{
public:
    SMSMessage(Buffer *cfg=NULL);
    ~SMSMessage();
    PROP_UTF8(Phone);
    PROP_UTF8(Network);
    virtual std::string save();
    virtual QString presentation();
protected:
    MessageSMSData data;
};

typedef struct MessageFileData
{
    Data        File;
    Data        Description;
    Data        Size;
} MessageFileData;

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

const unsigned NO_FILE  = (unsigned)(-1);

class EXPORT FileTransfer
{
public:
    FileTransfer(FileMessage *msg);
    virtual ~FileTransfer();
    void setNotify(FileTransferNotify*);
    FileTransferNotify          *notify() { return m_notify; }
    unsigned file()             { return m_nFile; }
    unsigned files()            { return m_nFiles; }
    unsigned bytes()            { return m_bytes; }
    unsigned totalBytes()       { return m_totalBytes; }
    unsigned fileSize()         { return m_fileSize; }
    unsigned totalSize()        { return m_totalSize; }
    unsigned speed()            { return m_speed; }
    unsigned transferBytes()    { return m_transferBytes; }
    virtual void setSpeed(unsigned speed);
    QString  dir()              { return m_dir; }
    void setDir(const QString &dir) { m_dir = dir; }
    enum OverwriteMode overwrite()   { return m_overwrite; }
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
    State state()   { return m_state; }
    QFile           *m_file;
    virtual void    startReceive(unsigned pos) = 0;
    virtual void    setError();
    void    addFile(const QString &file, unsigned size);
    QString filename()      { return m_name; }
    bool    isDirectory()   { return m_bDir; }
protected:
    bool    openFile();
    FileMessage         *m_msg;
    FileTransferNotify  *m_notify;
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
    QString  m_dir;
    QString  m_base;
    QString  m_name;
    State    m_state;
    bool     m_bDir;
    friend class FileMessage;
};

class EXPORT FileMessage : public Message
{
public:
    FileMessage(unsigned type=MessageFile, Buffer *cfg=NULL);
    ~FileMessage();
    PROP_UTF8(File);
    unsigned getSize();
    void     setSize(unsigned);
    virtual std::string save();
    virtual QString presentation();
    virtual QString getDescription();
    bool    setDescription(const QString&);
    void    addFile(const QString&);
    void    addFile(const QString&, unsigned size);
    class EXPORT Iterator
    {
    public:
        Iterator(const FileMessage&);
        ~Iterator();
        const QString *operator++();
        const QString *operator[](unsigned);
        void reset();
        unsigned count();
        unsigned dirs();
        unsigned size();
    protected:
        class FileMessageIteratorPrivate *p;
        friend class FileMessage;

        COPY_RESTRICTED(Iterator)
    };
    FileTransfer    *m_transfer;
protected:
    MessageFileData data;
    friend class FileTransfer;
    friend class Iterator;
};

class EXPORT AuthMessage : public Message
{
public:
    AuthMessage(unsigned type, Buffer *cfg=NULL)
: Message(type, cfg) {}
    virtual QString presentation();
};

typedef struct MessageUrlData
{
    Data    Url;
} MessageUrlData;

class EXPORT UrlMessage : public Message
{
public:
    UrlMessage(unsigned type=MessageUrl, Buffer *cfg=NULL);
    ~UrlMessage();
    virtual std::string  save();
    virtual QString presentation();
    VPROP_UTF8(Url)
protected:
    MessageUrlData data;
};

typedef struct MessageContactsData
{
    Data    Contacts;
} MessageContactsData;

class EXPORT ContactsMessage : public Message
{
public:
    ContactsMessage(unsigned type=MessageContacts, Buffer *cfg=NULL);
    ~ContactsMessage();
    virtual std::string  save();
    virtual QString presentation();
    VPROP_UTF8(Contacts);
protected:
    MessageContactsData data;
};

typedef struct MessageStatusData
{
    Data    Status;
} MessageStatusData;

class EXPORT StatusMessage : public Message
{
public:
    StatusMessage(Buffer *cfg=NULL);
    PROP_ULONG(Status);
    virtual std::string save();
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
    std::string save();
    void load(unsigned long id, const DataDef *def, Buffer *cfg);
    void *getUserData(unsigned id, bool bCreate);
    void freeUserData(unsigned id);
protected:
    class UserDataPrivate *d;

    COPY_RESTRICTED(UserData)
};

class EXPORT Client;
class ClientDataIterator;

struct clientData       // Base struct for all clientData
{
    Data    Sign;
    Data    LastSend;
};

class EXPORT ClientUserData
{
public:
    ClientUserData();
    ~ClientUserData();
    std::string save();
    void load(Client *client, Buffer *cfg);
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
    QString property(const char *name);
protected:
    class ClientUserDataPrivate *p;
    friend class ClientDataIterator;

    COPY_RESTRICTED(ClientUserData)
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
    class ClientDataIteratorPrivate *p;

    COPY_RESTRICTED(ClientDataIterator)
};

class EXPORT PacketType
{
public:
    PacketType(unsigned id, const QString &name, bool bText);
    ~PacketType();
    unsigned id() const { return m_id; }
    const QString &name() const { return m_name; }
    bool isText() const { return m_bText; }
protected:
    unsigned m_id;
    QString  m_name;
    bool     m_bText;
};

const unsigned PHONE    = 0;
const unsigned FAX      = 1;
const unsigned CELLULAR = 2;
const unsigned PAGER    = 3;

typedef struct ContactData
{
    Data            Group;      // Group ID
    Data            Name;       // Contact Display Name (UTF-8)
    Data            Ignore;     // In ignore list
    Data            LastActive;
    Data            EMails;
    Data            Phones;
    Data            PhoneStatus;
    Data            FirstName;
    Data            LastName;
    Data            Notes;
    Data            Flags;
    Data            Encoding;
} ContactData;

const unsigned CONTACT_TEMP             = 0x0001;
const unsigned CONTACT_DRAG             = 0x0002;
const unsigned CONTACT_NOREMOVE_HISTORY = 0x1000;

const unsigned CONTACT_TEMPORARY    = CONTACT_TEMP | CONTACT_DRAG;

class EXPORT Contact
{
public:
    Contact(unsigned long id = 0, Buffer *cfg = NULL);
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
    PROP_ULONG(Flags)
    PROP_STR(Encoding)
    void *getUserData(unsigned id, bool bCreate = false);
    UserData userData;
    ClientUserData clientData;
    bool setFirstName(const QString &name, const QString &client);
    bool setLastName(const QString &name, const QString &client);
    bool setEMails(const QString &mails, const QString &client);
    bool setPhones(const QString &phones, const QString &client);
    unsigned long contactInfo(unsigned &style, QString &statusIcon, QString *icons = NULL);
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
    Data        Name;       // Display name (UTF-8)
} GroupData;

class EXPORT Group
{
public:
    Group(unsigned long id = 0, Buffer *cfg = NULL);
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

const unsigned STATUS_UNKNOWN   = 0;
const unsigned STATUS_OFFLINE   = 1;
const unsigned STATUS_NA        = 10;
const unsigned STATUS_DND       = 20;
const unsigned STATUS_AWAY      = 30;
const unsigned STATUS_ONLINE    = 40;
const unsigned STATUS_FFC       = 50;

const unsigned CONTACT_UNDERLINE    = 0x0001;
const unsigned CONTACT_ITALIC       = 0x0002;
const unsigned CONTACT_STRIKEOUT    = 0x0004;

const unsigned PROTOCOL_INFO            = 0x00010000;
const unsigned PROTOCOL_SEARCH          = 0x00020000;
const unsigned PROTOCOL_AR_OFFLINE      = 0x00040000;
const unsigned PROTOCOL_INVISIBLE       = 0x00080000;
const unsigned PROTOCOL_AR              = 0x00100000;
const unsigned PROTOCOL_AR_USER         = 0x00200000;
const unsigned PROTOCOL_ANY_PORT        = 0x00800000;
const unsigned PROTOCOL_NOSMS           = 0x01000000;
const unsigned PROTOCOL_NOPROXY         = 0x02000000;
const unsigned PROTOCOL_TEMP_DATA       = 0x04000000;
const unsigned PROTOCOL_NODATA          = 0x08000000;
const unsigned PROTOCOL_NO_AUTH         = 0x10000000;

class ContactList;
class Client;

class EXPORT Protocol
{
public:
    Protocol(Plugin *plugin);
    virtual ~Protocol();
    Plugin  *plugin() { return m_plugin; }
    virtual Client  *createClient(Buffer *cfg) = 0;
    virtual const CommandDef *description() = 0;
    virtual const CommandDef *statusList() = 0;
    virtual const DataDef *userDataDef() = 0;
protected:
    Plugin *m_plugin;
};

typedef struct ClientData
{
    Data    ManualStatus;
    Data    CommonStatus;
    Data    Password;
    Data    SavePassword;
    Data    PreviousPassword;
    Data    Invisible;
    Data    LastSend;
} ClientData;

const unsigned AuthError = 1;

class EXPORT Client
{
public:
    Client(Protocol*, Buffer *cfg);
    virtual ~Client();
    enum State
    {
        Offline,
        Connecting,
        Connected,
        Error
    };
    virtual QString name() = 0;
    virtual QString dataName(void*) = 0;
    Protocol *protocol() const { return m_protocol; }
    virtual QWidget *setupWnd() = 0;
    virtual void setStatus(unsigned status, bool bCommon);
    virtual std::string getConfig();
    virtual bool compareData(void*, void*);
    virtual bool isMyData(clientData*&, Contact*&) = 0;
    virtual bool createData(clientData*&, Contact*) = 0;
    virtual void contactInfo(void *clientData, unsigned long &status, unsigned &style, QString &statusIcon, QString *icons = NULL) = 0;
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
    virtual QWidget *searchWindow(QWidget *parent) = 0;
    virtual QString resources(void *data);
    void    removeGroup(Group *grp);
    void    setState(State, const QString &text = QString::null, unsigned code = 0);
    State   getState() const { return m_state; }
    virtual unsigned getStatus() const { return m_status; }
    virtual void contactsLoaded();
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
    ClientData  data;
    Protocol    *m_protocol;
};

typedef struct UserDataDef
{
    unsigned        id;
    QString         name;
    const DataDef   *def;
} UserDataDef;

typedef struct ENCODING
{
    const char *language;
    const char *codec;
    int         mib;
    int         rtf_code;
    int         cp_code;
    bool        bMain;
} ENCODING;

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
    unsigned registerUserData(const QString &name, const DataDef *def);
    void unregisterUserData(unsigned id);
    Contact *contact(unsigned long id=0, bool isNew=false);
    Group   *group(unsigned long id=0, bool isNew=false);
    void addContact(Contact *contact);
    int  groupIndex(unsigned long id);
    int  groupCount();
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
        class GroupIteratorPrivate *p;
        friend class ContactList;

        COPY_RESTRICTED(GroupIterator)
    };
    class EXPORT ContactIterator
    {
    public:
        Contact *operator++();
        ContactIterator();
        ~ContactIterator();
        void reset();
    protected:
        class ContactIteratorPrivate *p;
        friend class ContactList;

        COPY_RESTRICTED(ContactIterator)
    };
    class EXPORT ProtocolIterator
    {
    public:
        Protocol *operator++();
        ProtocolIterator();
        ~ProtocolIterator();
        void reset();
    protected:
        class ProtocolIteratorPrivate *p;
        friend class ContactList;

        COPY_RESTRICTED(ProtocolIterator)
    };
    class EXPORT PacketIterator
    {
    public:
        PacketType *operator++();
        PacketIterator();
        ~PacketIterator();
        void reset();
    protected:
        class PacketIteratorPrivate *p;
        friend class ContactList;

        COPY_RESTRICTED(PacketIterator)
    };
    class EXPORT UserDataIterator
    {
    public:
        UserDataDef *operator++();
        UserDataIterator();
        ~UserDataIterator();
    protected:
        class UserDataIteratorPrivate *p;
        friend class ContactList;

        COPY_RESTRICTED(UserDataIterator)
    };
    void *getUserData(unsigned id);
    unsigned nClients();
    Client *getClient(unsigned n);
    void clearClients();
    void addPacketType(unsigned id, const QString &name, bool bText=false);
    void removePacketType(unsigned id);
    PacketType *getPacketType(unsigned i);
    Contact *contactByPhone(const QString &phone);
    Contact *contactByMail(const QString &_mail, const QString &_name);
    static bool cmpPhone(const QString &p1, const QString &p2);
    QString toUnicode(Contact *contact, const QCString &str, int length=-1);
    QCString fromUnicode(Contact *contact, const QString &str);
    QTextCodec *getCodec(Contact *contact);
    QTextCodec *getCodecByName(const char *encoding);
    static const ENCODING *getEncodings();
protected:
    class ContactListPrivate *p;
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

    COPY_RESTRICTED(ContactList)
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

EXPORT QString  unquoteText(const QString &text);
EXPORT QString  quoteString(const QString &str, quoteMode mode = quoteHTML, bool bQuoteSpaces = true);
EXPORT QString  unquoteString(const QString &str);
EXPORT QString  quote_nbsp(const QString &str);

// ____________________________________________________________________________________
// Log procs

const unsigned short L_ERROR        = 0x01;
const unsigned short L_WARN         = 0x02;
const unsigned short L_DEBUG        = 0x04;

const unsigned short L_PACKET_IN    = 0x10;
const unsigned short L_PACKET_OUT   = 0x20;

#if defined __GNUC__
EXPORT void log(unsigned short level, const char *fmt, ...)
__attribute__ ((__format__ (printf, 2, 3)));
#else
EXPORT void log(unsigned short level, const char *fmt, ...);
#endif      
/* QString counterpart of logging facility */
EXPORT void log(unsigned short level, const QString &msg);

EXPORT const char *level_name(unsigned short n);
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
// User interface

#ifdef WIN32
EXPORT void setWndProc(QWidget*);
#define SET_WNDPROC(A)  SIM::setWndProc(this);
#else
#ifndef QT_MACOSX_VERSION
EXPORT void setWndClass(QWidget*, const char*);
#define SET_WNDPROC(A)  SIM::setWndClass(this, A);
#else
#define SET_WNDPROC(A)
#endif
#endif

/* Raises widget w (on the KDE-Desktop desk) */
EXPORT bool raiseWindow(QWidget *w, unsigned desk = 0);
EXPORT void setButtonsPict(QWidget *w);

EXPORT QIconSet Icon(const QString &name);
EXPORT QPixmap Pict(const QString &name, const QColor &bgColor = QColor());
EXPORT QImage  Image(const QString &name);

EXPORT void setAboutData(KAboutData*);
EXPORT KAboutData *getAboutData();

EXPORT void initCombo(QComboBox *cmb, unsigned short code, const ext_info *tbl, bool bAddEmpty = true, const ext_info *tbl1 = NULL);
EXPORT unsigned short getComboValue(QComboBox *cmb, const ext_info *tbl, const ext_info *tbl1 = NULL);
EXPORT void disableWidget(QWidget *w);

EXPORT QString toTranslit(const QString&);
EXPORT bool isLatin(const QString&);
EXPORT QString getPart(QString&, unsigned size);

EXPORT unsigned screens();
EXPORT QRect screenGeometry(unsigned nScreen=~0U);

EXPORT unsigned get_random();

class EXPORT my_string
{
public:
    my_string(const char *str);
    my_string(const QString &str);
    ~my_string();
    bool operator < (const my_string &str) const;
    void operator = (const my_string &str);
    const QString &str() const { return m_str; }
protected:
    QString m_str;
};

EXPORT bool inResize();
EXPORT bool logEnabled();
EXPORT void setLogEnable(bool);

}

EXPORT QString g_i18n(const char *text, SIM::Contact *contact);

EXPORT QString get_os_version();
#endif
