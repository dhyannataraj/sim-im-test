/***************************************************************************
                          jabberinfo.h  -  description
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

#ifndef _JABBERINFO_H
#define _JABBERINFO_H

#include "simapi.h"
#include "jabberinfobase.h"

class JabberClient;

class JabberInfo : public JabberInfoBase, public EventReceiver
{
    Q_OBJECT
public:
    JabberInfo(QWidget *parent, struct JabberUserData *data, JabberClient *client);
public slots:
    void apply();
    void apply(Client*, void*);
    void goUrl();
    void urlChanged(const QString&);
    void resourceActivated(int);
protected:
    void *processEvent(Event *e);
    void fill();
    struct JabberUserData *m_data;
    JabberClient *m_client;
};

#endif

