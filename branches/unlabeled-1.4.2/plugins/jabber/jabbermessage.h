/***************************************************************************
                          jabbermessage.h  -  description
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

#ifndef _JABBERMESSAGE_H
#define _JABBERMESSAGE_H

#include "simapi.h"

const unsigned MessageJabber		= 0x201;
const unsigned MessageJabberOnline	= 0x202;
const unsigned MessageJabberOffline	= 0x203;
const unsigned MessageJabberError	= 0x204;

typedef struct JabberMessageData
{
    char	*Subject;
} JabberMessageData;

class JabberMessage : public Message
{
public:
    JabberMessage(const char *cfg = NULL);
    ~JabberMessage();
    PROP_UTF8(Subject);
    virtual string save();
    virtual QString presentation();
protected:
    JabberMessageData	data;
};

typedef struct JabberMessageErrorData
{
    char		*Error;
    unsigned	Code;
} JabberMessageErrorData;

class JabberMessageError : public Message
{
public:
    JabberMessageError(const char *cfg = NULL);
    ~JabberMessageError();
    PROP_UTF8(Error);
    PROP_ULONG(Code);
    virtual string save();
    virtual QString presentation();
protected:
    JabberMessageErrorData	data;
};

typedef struct JabberMessageFileData
{
	char		*Host;
	unsigned	Port;
} JabberMessageFileData;

class JabberFileMessage : public FileMessage
{
public:
	JabberFileMessage(const char *cfg = NULL);
	~JabberFileMessage();
	PROP_STR(Host);
	PROP_USHORT(Port);
protected:
	JabberMessageFileData	data;
};

#endif

