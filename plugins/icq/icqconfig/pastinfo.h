/***************************************************************************
                          pastinfo.h  -  description
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

#ifndef _PASTINFO_H
#define _PASTINFO_H

#include "country.h"
#include "event.h"
#include "icqcontact.h"

#include "ui_pastinfobase.h"

class ICQClient;
class ICQUserData;

class ICQClient;

class PastInfo : public QWidget
{
    Q_OBJECT
public:
    PastInfo(QWidget* parent, const ICQContactPtr& contact, ICQClient* client);
    virtual ~PastInfo();

public slots:
//    void apply();
//    void apply(SIM::Client*, void*);
//    void applyContact(const SIM::ClientPtr& client, SIM::IMContact* contact);
//    void cmbAfChanged(int);
//    void cmbBgChanged(int);

protected:
    Ui::PastInfoBase* ui() const;
//    void updateData(ICQUserData* data);
//    virtual bool processEvent(SIM::Event *e);
//    QString getInfo(QComboBox *cmb, QLineEdit *edt, const SIM::ext_info*);

    void fill();
    ICQContactPtr  m_contact;
    ICQClient *m_client;
    Ui::PastInfoBase* m_ui;
};

#endif

