/***************************************************************************
                          msnsearch.cpp  -  description
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

#include "msnsearch.h"
#include "msnclient.h"
#include "intedit.h"

#include <qcombobox.h>
#include <qlineedit.h>
#include <qpushbutton.h>

class MSNClient;

MSNSearch::MSNSearch(MSNClient *client, QWidget *parent)
        : MSNSearchBase(parent)
{
    m_client = client;
    connect(this, SIGNAL(setAdd(bool)), topLevelWidget(), SLOT(setAdd(bool)));
    edtMail->setValidator(new EMailValidator(edtMail));
    initCombo(cmbCountry, 0, getCountries(), true, getCountryCodes());
    m_btnMail = new GroupRadioButton(i18n("&E-Mail address"), grpMail);
    m_btnInfo = new GroupRadioButton(i18n("&Hotmail directory"), grpHotmail);
    connect(m_btnInfo, SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
    connect(m_btnMail, SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
}

void MSNSearch::radioToggled(bool)
{
    emit setAdd(m_btnMail->isChecked());
}

void MSNSearch::showEvent(QShowEvent *e)
{
    MSNSearchBase::showEvent(e);
    emit setAdd(m_btnMail->isChecked());
}

#ifndef WIN32
#include "msnsearch.moc"
#endif

