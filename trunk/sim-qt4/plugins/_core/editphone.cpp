/***************************************************************************
                          editphone.cpp  -  description
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

#include "editphone.h"
#include "phonedetails.h"
#include "pagerdetails.h"

#include <qlineedit.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qwidgetstack.h>
#include <qcombobox.h>
#include <qcheckbox.h>

extern ext_info phoneIcons[];
extern const char *phoneTypeNames[];

EditPhone::EditPhone(QWidget *parent, const QString &number, const QString &type, unsigned icon, bool bPublish, bool bShowPublish)
        : EditPhoneBase(parent, "editphone", true)
{
    SET_WNDPROC("editphone")
    setIcon(Pict("phone"));
    setButtonsPict(this);
    setCaption(number.isEmpty() ? i18n("Add phone number") : i18n("Edit phone number"));
    m_ok = false;
    m_phone = new PhoneDetails(wndDetails, (icon == PAGER) ? QString() : number);
    m_pager = new PagerDetails(wndDetails, (icon == PAGER) ? number : QString());
    wndDetails->addWidget(m_phone, 1);
    wndDetails->addWidget(m_pager, 2);
    connect(m_phone, SIGNAL(numberChanged(const QString&, bool)), this, SLOT(numberChanged(const QString&, bool)));
    connect(m_pager, SIGNAL(numberChanged(const QString&, bool)), this, SLOT(numberChanged(const QString&, bool)));
    edtDetails->setReadOnly(true);
    for (const ext_info *icons = phoneIcons; icons->szName; icons++){
        cmbType->insertItem(Pict(icons->szName));
    }
    for (const char **names = phoneTypeNames; *names; names++){
        cmbName->insertItem(i18n(*names));
    }
    cmbName->setEditable(true);
    cmbName->lineEdit()->setText(type);
    connect(cmbType, SIGNAL(activated(int)), this, SLOT(typeChanged(int)));
    connect(cmbName, SIGNAL(textChanged(const QString&)), this, SLOT(nameChanged(const QString&)));
    cmbType->setCurrentItem(icon);
    typeChanged(icon);
    publish = bPublish;
    if (bShowPublish){
        chkPublish->setChecked(publish);
    }else{
        chkPublish->hide();
    }
}

void EditPhone::typeChanged(int)
{
    switch (cmbType->currentItem()){
    case 0:
        wndDetails->raiseWidget(m_phone);
        m_phone->setExtensionShow(true);
        m_phone->getNumber();
        break;
    case 1:
    case 2:
        wndDetails->raiseWidget(m_phone);
        m_phone->setExtensionShow(false);
        m_phone->getNumber();
        break;
    case 3:
        wndDetails->raiseWidget(m_pager);
        m_pager->getNumber();
        break;
    }
}

void EditPhone::numberChanged(const QString &number, bool isOK)
{
    edtDetails->setText(number);
    m_ok = isOK;
    changed();
}

void EditPhone::nameChanged(const QString &name)
{
    unsigned i = 0;
    for (const char **p = phoneTypeNames; *p; p++, i++){
        if (name == i18n(*p))
            break;
    }
    switch (i){
    case 0:
    case 2:
        cmbType->setCurrentItem(0);
        typeChanged(0);
        break;
    case 1:
    case 3:
        cmbType->setCurrentItem(1);
        typeChanged(1);
        break;
    case 4:
        cmbType->setCurrentItem(2);
        typeChanged(2);
        break;
    case 5:
        cmbType->setCurrentItem(3);
        typeChanged(3);
        break;
    }
    changed();
}

void EditPhone::changed()
{
    buttonOk->setEnabled(m_ok && !cmbName->lineEdit()->text().isEmpty());
}

void EditPhone::accept()
{
    number = edtDetails->text();
    type = cmbName->lineEdit()->text();
    for (const char **names = phoneTypeNames; *names; names++){
        if (type == i18n(*names)){
            type = *names;
            break;
        }
    }
    icon = cmbType->currentItem();
    publish = chkPublish->isChecked();
    EditPhoneBase::accept();
}

#ifndef WIN32
#include "editphone.moc"
#endif

