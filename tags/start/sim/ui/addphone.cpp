/***************************************************************************
                          addphone.cpp  -  description
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

#include "addphone.h"
#include "icons.h"
#include "country.h"
#include "phonedetails.h"
#include "pagerdetails.h"
#include "icqclient.h"

#include <qwidgetstack.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlabel.h>

typedef struct phoneName
{
    const char *name;
    unsigned type;
    unsigned index;
} phoneName;

static phoneName names[] =
    {
        { "Home Phone", 0, 0 },
        { "Home Fax", 3, 2 },
        { "Private Cellular", 2, 1 },
        { "Work Phone", 0, 0 },
        { "Work Fax", 3, 2 },
        { "Wireless Pager", 4, 3 },
        { "", 0, 0 }
    };

AddPhone::AddPhone(QWidget *p, PhoneInfo *_info, int userCountry, bool bMyPhones)
        : AddPhoneBase(p, NULL, true)
{
    bMyInfo = bMyPhones;
    info = _info;
    setResult(0);
    setIcon(Pict("phone"));
    edtDetails->setReadOnly(true);
    cmbType->insertItem(Pict("phone"));
    cmbType->insertItem(Pict("cell"));
    cmbType->insertItem(Pict("fax"));
    cmbType->insertItem(Pict("wpager"));
    for (const phoneName *n = names; *(n->name); n++)
        cmbName->insertItem(n->name);
    cmbName->setEditable(true);
    QSize sType = cmbType->sizeHint();
    QSize sName = cmbName->minimumSizeHint();
    sName.setHeight(sType.height());
    cmbName->setMinimumSize(sName);
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
    connect(btnOK, SIGNAL(clicked()), this, SLOT(ok()));
    connect(cmbName, SIGNAL(textChanged(const QString&)), this, SLOT(nameChanged(const QString&)));
    connect(cmbType, SIGNAL(activated(int)), this, SLOT(typeChanged(int)));
    phone = new PhoneDetails(wndDetails, info, userCountry);
    connect(phone, SIGNAL(numberChanged(const QString&, bool)), this, SLOT(numberChanged(const QString&, bool)));
    wndDetails->addWidget(phone, 0);
    pager = new PagerDetails(wndDetails, info);
    wndDetails->addWidget(pager, 1);
    connect(pager, SIGNAL(numberChanged(const QString&, bool)), this, SLOT(numberChanged(const QString&, bool)));
    int phoneType = 0;
    switch (info->Type()){
    case MOBILE:
    case SMS:
        phoneType = 1;
        break;
    case FAX:
        phoneType = 2;
        break;
    case PAGER:
        phoneType = 3;
        break;
    }
    typeChanged(phoneType);
    cmbName->lineEdit()->setText(info->Name.c_str());
    nameChanged(info->Name.c_str());
}

void AddPhone::ok()
{
    info->Name = cmbName->lineEdit()->text().local8Bit();
    switch (cmbType->currentItem()){
    case 0:
        info->Type = PHONE;
        break;
    case 1:
        info->Type = MOBILE;
        break;
    case 2:
        info->Type = FAX;
        break;
    case 3:
        info->Type = PAGER;
        break;
    }
    if (cmbType->currentItem() == 3){
        pager->fillInfo(info);
    }else{
        phone->fillInfo(info);
    }
    info->MyInfo = true;
    done(1);
}

void AddPhone::nameChanged(const QString &string)
{
    for (const phoneName *n = names; *(n->name); n++){
        if (string == n->name){
            cmbType->setCurrentItem(n->index);
            typeChanged(n->index);
            cmbType->setEnabled(false);
            phone->setPublishShow(bMyInfo);
            return;
        }
    }
    cmbType->setEnabled(true);
    phone->setPublishShow(false);
}

void AddPhone::typeChanged(int)
{
    switch (cmbType->currentItem()){
    case 0:
        wndDetails->raiseWidget(phone);
        phone->getNumber();
        phone->setExtensionShow(true);
        phone->setSMSShow(false);
        break;
    case 1:
        wndDetails->raiseWidget(phone);
        phone->getNumber();
        phone->setExtensionShow(false);
        phone->setSMSShow(true);
        break;
    case 2:
        wndDetails->raiseWidget(phone);
        phone->getNumber();
        phone->setExtensionShow(false);
        phone->setSMSShow(false);
        break;
    case 3:
        wndDetails->raiseWidget(pager);
        pager->getNumber();
        break;
    }
}

void AddPhone::numberChanged(const QString &number, bool bOK)
{
    QString name;
    if (cmbName->lineEdit()->text().length() > 0) name = cmbName->lineEdit()->text() + ": ";
    edtDetails->setText(name + number);
    btnOK->setEnabled(bOK && (cmbName->lineEdit()->text().length() > 0));
}

#ifndef _WINDOWS
#include "addphone.moc"
#endif

