/***************************************************************************
                         services.h  -  description
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

#ifndef _SERVICES_H
#define _SERVICES_H

#include "simapi.h"
#include "servicesbase.h"
#include "jabberclient.h"

class QListViewItem;

class Services : public ServicesBase, public EventReceiver
{
    Q_OBJECT
public:
    Services(QWidget *parent, JabberClient *client);
protected slots:
    void apply(Client*, void*);
    void apply();
    void textChanged(const QString&);
    void search();
    void regAgent();
	void unregAgent();
	void logon();
	void logoff();
    void selectAgent(int);
	void selectChanged(QListViewItem*);
protected:
    void *processEvent(Event *e);
    void statusChanged();
    bool m_bOnline;
    AGENTS_MAP   m_agents;
    JabberClient *m_client;
};

#endif

