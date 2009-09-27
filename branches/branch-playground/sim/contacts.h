/***************************************************************************
                          contacts.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#ifndef _CONTACTS_H
#define _CONTACTS_H

#include "cfg.h"
#include "plugins.h"
#include "message.h"
#include "contacts/userdata.h"
#include "contacts/clientuserdata.h"
#include "contacts/packettype.h"
#include "contacts/protocol.h"

#include <map>
#include <QImage>
#include <QString>
#include <QSet>

namespace SIM {

class Client;
class ClientDataIterator;


const unsigned PHONE    = 0;
const unsigned FAX      = 1;
const unsigned CELLULAR = 2;
const unsigned PAGER    = 3;

const unsigned STATUS_UNKNOWN   = 0;
const unsigned STATUS_OFFLINE   = 1;
const unsigned STATUS_INVISIBLE	= 2;
const unsigned STATUS_DND       = 10;
const unsigned STATUS_OCCUPIED  = 20;
const unsigned STATUS_NA        = 30;
const unsigned STATUS_AWAY      = 40;
const unsigned STATUS_ONLINE    = 50;
const unsigned STATUS_FFC       = 60;

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

struct _ClientUserData
{
    Client  *client;
    Data    *data;
};


const unsigned AuthError = 1;

/*
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
    virtual QByteArray getConfig();
    virtual bool compareData(void*, void*);
    virtual bool isMyData(clientData*&, Contact*&) = 0;
    virtual bool createData(clientData*&, Contact*) = 0;
    virtual void contactInfo(void *clientData, unsigned long &status, unsigned &style, QString &statusIcon, QSet<QString> *icons = NULL) = 0;
    virtual QImage userPicture(unsigned) {return QImage();};
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
*/

struct UserDataDef
{
    unsigned        id;
    QString         name;
    const DataDef   *def;
};

struct ENCODING
{
    const char *language;
    const char *codec;
    int         mib;
    int         rtf_code;
    int         cp_code;
    bool        bMain;
};

class EXPORT ContactList
{
public:
    ContactList();
    virtual ~ContactList();
    Contact *owner();
	QVariantMap* userdata();
    void clear();
    void load();
    void save();
    void addClient(Client*);
    unsigned registerUserData(const QString &name, const DataDef *def);
    void unregisterUserData(unsigned id);
    Contact *contact(unsigned long id=0, bool isNew=false);
    bool contactExists(unsigned long id);
    bool groupExists(unsigned long id);
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
    QString toUnicode(Contact *contact, const QByteArray &str);
    QByteArray fromUnicode(Contact *contact, const QString &str);
    QTextCodec *getCodec(Contact *contact);
    static QTextCodec *getCodecByName(const QString &encoding);
    static QTextCodec *getCodecByCodePage(const int iCP);
    static const ENCODING *getEncodings();
    const ENCODING *getEncoding(Contact *contact);
protected:

	void save_new();
	void load_new();


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

private:
	QVariantMap* m_userData;
};

EXPORT ContactList *getContacts();
typedef std::map<unsigned, PacketType*>	PACKET_MAP;

} // namespace SIM

#endif
