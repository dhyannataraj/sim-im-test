/***************************************************************************
                          moreinfo.h  -  description
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

#ifndef _MOREINFO_H
#define _MOREINFO_H

#include "ui_moreinfobase.h"
#include "contacts/client.h"
#include "../icqcontact.h"

#include "event.h"
#include <QDate>

class ICQClient;

class MoreInfo : public QWidget
{
    Q_OBJECT
public:
    MoreInfo(QWidget* parent, const ICQContactPtr& contact, ICQClient* client);

public slots:
    void contactMoreInfoUpdated(const QString& contactScreen);
    void apply();
//    void apply(SIM::Client*, void*);
//    void applyContact(const SIM::ClientPtr& client, SIM::IMContact* contact);
    void goUrl();
    void urlChanged(const QString& newUrl);
    void birthDayChanged();

protected:
//    void updateData(ICQUserData* data);
//    virtual bool processEvent(SIM::Event *e);

    void fill();

    virtual QDate currentDate() const;

    Ui::MoreInfo* ui() const;

    bool changed() const;

    int currentLanguage(int langnum) const;

private:
    ICQContactPtr m_contact;
    ICQClient* m_client;
    Ui::MoreInfo* m_ui;
};

#endif

