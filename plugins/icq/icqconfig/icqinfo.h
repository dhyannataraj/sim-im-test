/***************************************************************************
                          icqinfo.h  -  description
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

#ifndef _ICQINFO_H
#define _ICQINFO_H

#include "ui_icqinfobase.h"
#include "contacts/client.h"
#include "../icqcontact.h"


class ICQClient;
class ICQUserData;

class ICQInfo : public QWidget
{
    Q_OBJECT

public:
    ICQInfo(QWidget* parent, const ICQContactPtr& contact, ICQClient* client);

signals:
    void raise(QWidget*);

public slots:
//    void apply();
//    void apply(SIM::Client*, void*);
//    void applyContact(const SIM::ClientPtr& client, SIM::IMContact* contact);

protected:
    Ui::MainInfo* ui() const;
//    void updateData(ICQUserData* data);
//    virtual bool processEvent(SIM::Event *e);
    void fill();

    ICQContactPtr m_contact;
    ICQClient *m_client;
    Ui::MainInfo* m_ui;
};

#endif

