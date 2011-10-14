/***************************************************************************
                          workinfo.h  -  description
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

#ifndef _WORKINFO_H
#define _WORKINFO_H

#include "ui_workinfobase.h"
#include "contacts/client.h"
#include "../icqcontact.h"

class ICQClient;

class WorkInfo : public QWidget
{
    Q_OBJECT
public:
    WorkInfo(QWidget *parent, const ICQContactPtr& contact, ICQClient *client);

public slots:
    void contactWorkInfoUpdated(const QString& contactScreen);
    void apply();
//    void apply(SIM::Client*, void*);
//    void applyContact(const SIM::ClientPtr& client, SIM::IMContact* contact);
    void goUrl();
    void urlChanged(const QString&);

protected:
    Ui::WorkInfo* ui() const;
    //void updateData(ICQUserData* data);
    //virtual bool processEvent(SIM::Event *e);

    void fill();

    bool changed() const;
    int selectedCountry() const;
    int selectedOccupation() const;

    ICQContactPtr m_contact;
    ICQClient *m_client;

    Ui::WorkInfo* m_ui;
};

#endif

