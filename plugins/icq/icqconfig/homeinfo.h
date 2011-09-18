/***************************************************************************
                          homeinfo.h  -  description
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

#ifndef _HOMEINFO_H
#define _HOMEINFO_H

#include "contacts.h"
#include "event.h"
#include "../icqcontact.h"

#include "ui_homeinfobase.h"

class ICQClient;

class HomeInfo : public QWidget
{
    Q_OBJECT
public:
    HomeInfo(QWidget* parent, const ICQContactPtr& contact, ICQClient* client);
    virtual ~HomeInfo();

public slots:
    void contactBasicInfoUpdated(const QString& screen);
    void apply();
//    void apply(SIM::Client*, void*);
//    void applyContact(const SIM::ClientPtr& client, SIM::IMContact* contact);
//    void goUrl();

protected:
//    void updateData(ICQUserData* data);
//    virtual bool processEvent(SIM::Event *e);

    void fill();
    void initCountryCombobox();
    bool changed() const;

    Ui::HomeInfo* ui();

    int selectedCountry() const;

private:
    ICQContactPtr m_contact;
    ICQClient *m_client;
    Ui::HomeInfo* m_ui;
};

#endif

