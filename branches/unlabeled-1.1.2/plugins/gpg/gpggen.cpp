/***************************************************************************
                          gpggen.cpp  -  description
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

#include "gpggen.h"

#include <qpixmap.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>

GpgGen::GpgGen()
        : GpgGenBase(NULL, NULL, true)
{
    SET_WNDPROC("genkey")
    setIcon(Pict("encrypted"));
    setButtonsPict(this);
    setCaption(caption());
    cmbMail->setEditable(true);
    connect(edtName, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(cmbMail->lineEdit(), SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    Contact *owner = getContacts()->owner();
    if (owner){
        QString name;
        name = owner->getFirstName();
        QString firstName = getToken(name, '/');
        name  = owner->getLastName();
        QString lastName  = getToken(name, '/');

        if (firstName.isEmpty() || lastName.isEmpty()){
            name = firstName + lastName;
        }else{
            name = firstName + " " + lastName;
        }
        edtName->setText(name);
        QString mails = owner->getEMails();
        while (!mails.isEmpty()){
            QString item = getToken(mails, ';');
            QString mail = getToken(item, '/');
            cmbMail->insertItem(mail);
        }
    }
}

GpgGen::~GpgGen()
{
}

void GpgGen::textChanged(const QString&)
{
    buttonOk->setEnabled(!edtName->text().isEmpty() && !cmbMail->lineEdit()->text().isEmpty());
}

#ifndef WIN32
#include "gpggen.moc"
#endif

