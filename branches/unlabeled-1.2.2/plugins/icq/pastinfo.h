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

#include "simapi.h"
#include "pastinfobase.h"

class ICQClient;

class PastInfo : public PastInfoBase, public EventReceiver
{
    Q_OBJECT
public:
    PastInfo(QWidget *parent, struct ICQUserData *data, unsigned contact, ICQClient *client);
public slots:
    void apply();
    void apply(Client*, void*);
    void cmbAfChanged(int);
    void cmbBgChanged(int);
protected:
    void *processEvent(Event*);
    QString getInfo(QComboBox *cmb, QLineEdit *edt, const ext_info*);
    void fill();
    struct ICQUserData *m_data;
	unsigned  m_contact;
    ICQClient *m_client;
};

#endif

