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

const unsigned MessageJabber	= 0x201;

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

#endif

