/***************************************************************************
                          smssetup.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "smssetup.h"
#include "icqclient.h"
#include "mainwin.h"
#include "client.h"
#include "icons.h"
#include "enable.h"

#include <qmultilineedit.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qcombobox.h>
#include <qlineedit.h>

SMSSetup::SMSSetup(QWidget *p)
        : SMSSetupBase(p)
{
    QString s;
    set(s, pMain->SMSSignTop.c_str());
    edtBefore->setText(s);
    set(s, pMain->SMSSignBottom.c_str());
    edtAfter->setText(s);
    cmbPhone->setEditable(true);
    for (PhoneBook::iterator it = pClient->owner->Phones.begin(); it != pClient->owner->Phones.end(); ++it){
        if (((*it)->Type != SMS) && ((*it)->Type != MOBILE)) continue;
        string number = (*it)->getNumber();
        cmbPhone->insertItem(QString::fromLocal8Bit(number.c_str()));
    }
    cmbPhone->lineEdit()->setText(pMain->ForwardPhone.c_str());
}

void SMSSetup::apply(ICQUser*)
{
    set(pMain->SMSSignTop, edtBefore->text());
    set(pMain->SMSSignBottom, edtAfter->text());
    set(pMain->ForwardPhone, cmbPhone->lineEdit()->text());
}

#ifndef _WINDOWS
#include "smssetup.moc"
#endif

