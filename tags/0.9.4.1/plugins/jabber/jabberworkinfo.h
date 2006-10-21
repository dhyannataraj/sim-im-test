/***************************************************************************
                          jabberworkinfo.h  -  description
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

#ifndef _JABBERWORKINFO_H
#define _JABBERWORKINFO_H

#include "simapi.h"
#include "jabberworkinfobase.h"

class JabberClient;

class JabberWorkInfo : public JabberWorkInfoBase, public EventReceiver
{
    Q_OBJECT
public:
    JabberWorkInfo(QWidget *parent, struct JabberUserData *data, JabberClient *client);
public slots:
    void apply();
    void apply(Client*, void*);
protected:
    void *processEvent(Event *e);
    void fill(JabberUserData *data);
    struct JabberUserData *m_data;
    JabberClient *m_client;
};

#endif

