/***************************************************************************
                          jabberhomeinfo.h  -  description
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

#ifndef _JABBERHOMEINFO_H
#define _JABBERHOMEINFO_H

#include "simapi.h"
#include "jabberhomeinfobase.h"

class JabberClient;

class JabberHomeInfo : public JabberHomeInfoBase, public EventReceiver
{
    Q_OBJECT
public:
    JabberHomeInfo(QWidget *parent, struct JabberUserData *data, JabberClient *client);
public slots:
    void apply();
    void apply(Client*, void*);
protected:
    void *processEvent(Event *e);
    void fill();
    struct JabberUserData *m_data;
    JabberClient *m_client;
};

#endif

