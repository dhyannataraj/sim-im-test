/***************************************************************************
                          interestsinfo.h  -  description
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

#ifndef _INTERESTSINFO_H
#define _INTERESTSINFO_H

#include "event.h"
#include "contacts/client.h"
#include "ui_interestsinfobase.h"
#include "../icqcontact.h"

class ICQClient;

class InterestsInfo : public QWidget
{
    Q_OBJECT
public:
    InterestsInfo(QWidget *parent, const ICQContactPtr& contact, ICQClient* client);

public slots:
    void contactInterestsInfoUpdated(const QString& contactScreen);
//    void apply();
//    void apply(SIM::Client*, void*);
//    void applyContact(const SIM::ClientPtr& client, SIM::IMContact* contact);

protected:
//    void updateData(ICQUserData* data);
//    virtual bool processEvent(SIM::Event *e);
    void fill();
    Ui::InterestsInfoBase* ui() const;

private:
    ICQContactPtr m_contact;
    ICQClient *m_client;
    Ui::InterestsInfoBase* m_ui;
};

#endif

