/***************************************************************************
                          aboutinfo.h  -  description
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

#ifndef _ABOUTINFO_H
#define _ABOUTINFO_H

#include "contacts.h"

#include "ui_aboutinfobase.h"
#include "../icqcontact.h"

class ICQClient;

class AboutInfo : public QWidget
{
    Q_OBJECT
public:
    AboutInfo(QWidget* parent, const ICQContactPtr& contact, ICQClient* client);

public slots:
    void contactAboutInfoUpdated(const QString& contactScreen);
//    void apply();
//    void apply(SIM::Client*, void*);
//    void applyContact(const SIM::ClientPtr& client, SIM::IMContact* contact);

protected:
    void fill();
    ICQContactPtr m_contact;
    ICQClient	*m_client;
    Ui::aboutInfo* m_ui;
};

#endif

