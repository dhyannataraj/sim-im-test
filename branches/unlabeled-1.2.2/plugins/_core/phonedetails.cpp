/***************************************************************************
							phonedetails.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#include "phonedetails.h"

#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qvalidator.h>

PhoneDetails::PhoneDetails(QWidget *p, const QString &oldNumber)
        : PhoneDetailsBase(p)
{
    QString number = oldNumber;
    QString areaCode;
    QString extension;
    int countryCode = 0;
    if (number.find('(') >= 0){
        QCString country = trim(getToken(number, '(')).latin1();
        const char *p;
        for (p = country; *p; p++){
            if ((*p >= '0') && (*p <= '9'))
                break;
        }
        countryCode = atol(p);
        areaCode = trim(getToken(number, ')'));
    }
    if (number.find(" - ") >= 0){
        int pos = number.find(" - ");
        extension = trim(number.mid(pos + 3));
        number = number.mid(0, pos);
    }
    number = trim(number);
    initCombo(cmbCountry, (unsigned short)countryCode, getCountries());

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
    edtAreaCode->setText(areaCode);
    edtNumber->setText(number);
    edtExtension->setText(extension);
    m_bExt = false;
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
    m_bExt = bShow;
}

void PhoneDetails::getNumber()
{
    QString res;
    bool bOK = true;
    if (cmbCountry->currentItem() > 0){
        res = "+";
        res += QString::number(getComboValue(cmbCountry, getCountries()));
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
    if (m_bExt && (edtExtension->text().length() > 0)){
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

#ifndef _WINDOWS
#include "phonedetails.moc"
#endif

