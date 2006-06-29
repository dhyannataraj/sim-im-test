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
#include <qtoolbutton.h>

const unsigned long MessageICQ				= 0x100;
const unsigned long MessageICQUrl			= 0x101;
const unsigned long MessageICQContacts		= 0x102;
const unsigned long MessageContactRequest	= 0x103;
const unsigned long MessageICQAuthRequest	= 0x104;
const unsigned long MessageICQAuthGranted	= 0x105;
const unsigned long MessageICQAuthRefused	= 0x106;
const unsigned long MessageWebPanel			= 0x107;
const unsigned long MessageEmailPager		= 0x108;
const unsigned long MessageOpenSecure		= 0x109;
const unsigned long MessageCloseSecure		= 0x110;
//const unsigned long MessageCheckInvisible	= 0x111;
const unsigned long MessageICQFile			= 0x112;
const unsigned long MessageWarning			= 0x113;

class ListView;

class IcqContactsMessage : public ContactsMessage
{
public:
    IcqContactsMessage(Buffer *cfg=NULL);
    ~IcqContactsMessage();
    QString getContacts() const;
    virtual unsigned baseType() { return MessageContacts; }
};

typedef struct ICQAuthMessageData
{
    Data	Charset;
} ICQAuthMessageData;

class ICQAuthMessage : public AuthMessage
{
public:
    ICQAuthMessage(unsigned type, unsigned base_type, Buffer *cfg=NULL);
    ~ICQAuthMessage();
    PROP_STR(Charset);
    virtual QString getText() const;
    virtual string save();
    virtual unsigned baseType() { return m_baseType; }
protected:
    unsigned m_baseType;
    ICQAuthMessageData data;
};

typedef struct ICQFileMessageData
{
    Data	ServerDescr;
    Data	IP;
    Data	Port;
    Data	ID_L;
    Data	ID_H;
    Data	Cookie;
    Data	Extended;
} ICQFileMessageData;

class ICQFileMessage : public FileMessage
{
public:
    ICQFileMessage(Buffer *cfg=NULL);
    ~ICQFileMessage();
    PROP_STR(ServerDescr);
    PROP_ULONG(IP);
    PROP_USHORT(Port);
    PROP_ULONG(ID_L);
    PROP_ULONG(ID_H);
    PROP_ULONG(Cookie);
    PROP_ULONG(Extended);
    virtual QString getDescription();
    virtual string  save();
    virtual unsigned baseType() { return MessageFile; }
protected:
    ICQFileMessageData data;
};

typedef struct AIMFileMessageData
{
    Data	Port;
    Data	ID_L;
    Data	ID_H;
} AIMFileMessageData;

class AIMFileMessage : public FileMessage
{
public:
    AIMFileMessage(Buffer *cfg=NULL);
    ~AIMFileMessage();
    PROP_USHORT(Port);
    PROP_ULONG(ID_L);
    PROP_ULONG(ID_H);
    virtual unsigned baseType() { return MessageFile; }
protected:
    AIMFileMessageData data;
};

typedef struct MessageWarningData
{
    Data	Anonymous;
    Data	OldLevel;
    Data	NewLevel;
} MessageWarningData;

class WarningMessage : public AuthMessage
{
public:
    WarningMessage(Buffer *cfg=NULL);
    PROP_BOOL(Anonymous);
    PROP_USHORT(OldLevel);
    PROP_USHORT(NewLevel);
    virtual string  save();
    QString presentation();
protected:
    MessageWarningData data;
};

#endif

