/***************************************************************************
							pagerdetails.cpp  -  description
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

#include "pagerdetails.h"
#include "icons.h"
#include "country.h"
#include "icqclient.h"

#include <qcombobox.h>
#include <qlineedit.h>
#include <qfontmetrics.h>

PagerDetails::PagerDetails(QWidget *p, PhoneInfo *info)
        : PagerDetailsBase(p)
{
    cmbProvider->setEditable(true);
    for (const pager_provider *provider = providers; *provider->szName; provider++)
        cmbProvider->insertItem(provider->szName);
    cmbProvider->lineEdit()->setText("");
    connect(cmbProvider, SIGNAL(textChanged(const QString&)), this, SLOT(providerChanged(const QString&)));
    connect(edtNumber, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(edtGateway, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    cmbProvider->lineEdit()->setText(QString::fromLocal8Bit(info->Provider.c_str()));
    edtNumber->setText(info->Number.c_str());
    edtGateway->setText(info->Gateway.c_str());
    providerChanged(cmbProvider->lineEdit()->text());
}

void PagerDetails::getNumber()
{
    bool bOK = true;
    QString res;
    if (edtNumber->text().length() > 0){
        res = edtNumber->text();
    }else{
        bOK = false;
    }
    if (edtGateway->text().length() > 0){
        res += "@";
        res += edtGateway->text();
    }else{
        bOK = false;
    }
    emit numberChanged(res, bOK);
}

void PagerDetails::providerChanged(const QString &str)
{
    for (const pager_provider *p = providers; *p->szName; p++){
        if (str == p->szName){
            edtGateway->setText(p->szGate);
            edtGateway->setEnabled(false);
            getNumber();
            return;
        }
    }
    edtGateway->setEnabled(true);
}

void PagerDetails::textChanged(const QString&)
{
    getNumber();
}

void PagerDetails::fillInfo(PhoneInfo *info)
{
    info->Publish = false;
    info->AreaCode = "";
    info->Country = "";
    info->Extension = "";
    info->Number = edtNumber->text().local8Bit();
    info->Provider = cmbProvider->lineEdit()->text().local8Bit();
    info->Gateway = edtGateway->text().local8Bit();
}

#ifndef _WINDOWS
#include "pagerdetails.moc"
#endif

