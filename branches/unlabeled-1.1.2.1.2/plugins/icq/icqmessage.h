/***************************************************************************
                          icqmessage.h  -  description
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

#ifndef _ICQMESSAGE_H
#define _ICQMESSAGE_H

#include "simapi.h"

#include <qlineedit.h>

const unsigned MessageICQ				= 0x100;
const unsigned MessageURL				= 0x101;
const unsigned MessageContact			= 0x102;
const unsigned MessageContactRequest	= 0x103;
const unsigned MessageICQAuthRequest	= 0x104;
const unsigned MessageICQAuthGranted	= 0x105;
const unsigned MessageICQAuthRefused	= 0x106;
const unsigned MessageWebPanel			= 0x107;
const unsigned MessageEmailPager		= 0x108;
const unsigned MessageOpenSecure		= 0x109;
const unsigned MessageCloseSecure		= 0x110;

class ListView;

typedef struct ICQMessageData
{
    char	*ServerText;
} ICQMessageData;

class ICQMessage : public Message
{
public:
    ICQMessage(unsigned type=MessageICQ, const char *cfg=NULL);
    ~ICQMessage();
    PROP_STR(ServerText);
    virtual QString getText();
    virtual string  save();
protected:
    ICQMessageData data;
};

typedef struct UrlMessageData
{
    char	*Url;
    char	*ServerUrl;
    char	*ServerText;
} UrlMessageData;

class URLMessage : public Message
{
public:
    URLMessage(const char *cfg=NULL);
    ~URLMessage();
    QString getUrl();
    bool setUrl(const QString &url);
    PROP_STR(ServerUrl);
    PROP_STR(ServerText);
    virtual QString getText();
    virtual string  save();
    virtual QString presentation();
protected:
    UrlMessageData data;
};

typedef struct ContactMessageData
{
    char	*Contacts;
    char	*ServerText;
} ContactMessageData;

class ContactMessage : public Message
{
public:
    ContactMessage(const char *cfg=NULL);
    ~ContactMessage();
    QString getContacts();
    bool setContacts(const QString&);
    PROP_STR(ServerText);
    virtual string save();
    virtual QString presentation();
protected:
    ContactMessageData data;
};

typedef struct ICQAuthMessageData
{
    char	*ServerText;
    char	*Charset;
} ICQAuthMessageData;

class ICQAuthMessage : public AuthMessage
{
public:
    ICQAuthMessage(unsigned type, const char *cfg=NULL);
    ~ICQAuthMessage();
    PROP_STR(ServerText);
    PROP_STR(Charset);
    virtual QString getText();
    virtual string save();
protected:
    ICQAuthMessageData data;
};

class QToolButton;
class CToolCustom;
class MsgEdit;
class QListViewItem;

class MsgUrl : public QLineEdit, public EventReceiver
{
    Q_OBJECT
public:
    MsgUrl(CToolCustom *btn, Message *msg);
protected slots:
    void init();
    void urlChanged(const QString&);
protected:
    virtual void *processEvent(Event*);
    QToolButton *btnSend;
    MsgEdit		*m_edit;
    string		m_client;
};

class MsgContacts : public QObject, public EventReceiver
{
    Q_OBJECT
public:
    MsgContacts(CToolCustom *btn, Message *msg, Protocol*);
    ~MsgContacts();
protected slots:
    void init();
    void contactsDestroyed();
    void contactsDragEnter(QMimeSource*);
    void contactsDrop(QMimeSource*);
    void deleteItem(QListViewItem*);
protected:
    virtual void *processEvent(Event*);
    void  changed();
    Protocol	*m_protocol;
    ListView	*m_contacts;
    MsgEdit		*m_edit;
    Message		*m_msg;
    QToolButton	*btnSend;
    string		m_client;
};

#endif

