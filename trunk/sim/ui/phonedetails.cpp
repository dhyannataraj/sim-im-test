/***************************************************************************
							phonedetails.cpp  -  description
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

#include "phonedetails.h"
#include "icons.h"
#include "enable.h"
#include "icqclient.h"

#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qvalidator.h>

PhoneDetails::PhoneDetails(QWidget *p, PhoneInfo *info, unsigned userCountry)
        : PhoneDetailsBase(p)
{
    initCombo(cmbCountry, userCountry, countries);
    QFontMetrics fm(font());
    unsigned wChar = fm.width("0");
    QSize s(wChar*10, 0);
    edtNumber->setMinimumSize(s);
    s = edtAreaCode->size();
    s.setWidth(wChar*5);
    QSize sLabel = lblAreaCode->sizeHint();
    sLabel.setHeight(0);
    s = s.expandedTo(sLabel);
    edtAreaCode->setMaximumSize(s);
    s.setWidth(wChar*5);
    sLabel = lblExtension->sizeHint();
    sLabel.setHeight(0);
    s = s.expandedTo(sLabel);
    edtExtension->setMaximumSize(s);
    connect(cmbCountry, SIGNAL(activated(int)), this, SLOT(countryChanged(int)));
    connect(edtAreaCode, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(edtNumber, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(edtExtension, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    edtAreaCode->setValidator(new QIntValidator(edtAreaCode));
    edtNumber->setValidator(new QIntValidator(edtNumber));
    edtExtension->setValidator(new QIntValidator(edtExtension));
    chkPublish->setChecked(info->Publish);
    chkSMS->setChecked(info->Type == SMS);
    edtAreaCode->setText(info->AreaCode.c_str());
    edtNumber->setText(info->Number.c_str());
    edtExtension->setText(info->Extension.c_str());
    int i = 1;
    for (const ext_info *c = countries; *c->szName; c++, i++){
        if (info->Country == c->szName){
            cmbCountry->setCurrentItem(i);
            break;
        }
    }
}

void PhoneDetails::setPublishShow(bool bShow)
{
    if (bShow){
        chkPublish->show();
    }else{
        chkPublish->hide();
    }
}

void PhoneDetails::setExtensionShow(bool bShow)
{
    if (bShow){
        lblExtension->show();
        edtExtension->show();
        lblDivExtension->show();
    }else{
        lblExtension->hide();
        edtExtension->hide();
        lblDivExtension->hide();
    }
}

void PhoneDetails::setSMSShow(bool bShow)
{
    if (bShow){
        chkSMS->show();
    }else{
        chkSMS->hide();
    }
}

void PhoneDetails::getNumber()
{
    QString res;
    bool bOK = true;
    if (cmbCountry->currentItem() > 0){
        res = "+";
        int i = cmbCountry->currentItem() - 1;
        const ext_info *c;
        for (c = countries; i && *c->szName; c++, i--);
        res += QString::number(c->nCode);
        res += " ";
    }else{
        bOK = false;
    }
    if (edtAreaCode->text().length() > 0){
        res += "(";
        res += edtAreaCode->text();
        res += ") ";
    }else{
        bOK = false;
    }
    if (edtNumber->text().length() > 0){
        res += edtNumber->text();
    }else{
        bOK = false;
    }
    if (edtExtension->isVisible() && (edtExtension->text().length() > 0)){
        res += " - ";
        res += edtExtension->text();
    }
    emit numberChanged(res, bOK);
}

void PhoneDetails::countryChanged(int)
{
    getNumber();
}

void PhoneDetails::textChanged(const QString&)
{
    getNumber();
}

void PhoneDetails::fillInfo(PhoneInfo *info)
{
    if (chkSMS->isVisible())
        info->Type = chkSMS->isChecked() ? SMS : MOBILE;
    if (chkPublish->isVisible()){
        info->Publish = chkPublish->isChecked();
    }else{
        info->Publish = false;
    }
    info->AreaCode = edtAreaCode->text().local8Bit();
    info->Country = cmbCountry->currentText().local8Bit();
    if (edtExtension->isVisible()){
        info->Extension = edtExtension->text().local8Bit();
    }else{
        info->Extension = 0;
    }
    info->Gateway = "";
    info->Number = edtNumber->text().local8Bit();
    info->Provider = "";
}

#ifndef _WINDOWS
#include "phonedetails.moc"
#endif

