/***************************************************************************
                          yahoosearch.cpp  -  description
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

#include "yahoosearch.h"
#include "yahooclient.h"
#include "intedit.h"

#include <qlabel.h>
#include <qcombobox.h>

const ext_info ages[] =
    {
        { "13-18", 1 },
        { "18-25", 2 },
        { "25-35", 3 },
        { "35-50", 4 },
        { "50-70", 5 },
        { "> 70",  6 },
        { "", 0 }
    };

const ext_info genders[] =
    {
        { I18N_NOOP("Female"), 1 },
        { I18N_NOOP("Male"), 2 },
        { "", 0 }
    };

YahooSearch::YahooSearch(YahooClient *client, QWidget *parent)
: YahooSearchBase(parent)
{
    m_client = client;
	connect(this, SIGNAL(setAdd(bool)), topLevelWidget(), SLOT(setAdd(bool)));
	edtMail->setValidator(new EMailValidator(edtMail));
	edtID->setValidator(new RegExpValidator("[0-9A-Za-z \\-_]+", this));
    initCombo(cmbAge, 0, ages);
    initCombo(cmbGender, 0, genders);
	m_btnID			= new GroupRadioButton(i18n("&Yahoo! ID"), grpID);
	m_btnMail		= new GroupRadioButton(i18n("&E-Mail address"), grpMail);
	m_btnName		= new GroupRadioButton(i18n("&Name"), grpName);
	m_btnKeyword	= new GroupRadioButton(i18n("&Keywords"), grpKeyword);
	connect(m_btnID,		SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
	connect(m_btnMail,		SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
	connect(m_btnName,		SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
	connect(m_btnKeyword,	SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
}

void YahooSearch::radioToggled(bool)
{
	emit setAdd(m_btnID->isChecked());
	lblGender->setEnabled(!m_btnID->isChecked());
	lblAge->setEnabled(!m_btnID->isChecked());
	cmbGender->setEnabled(!m_btnID->isChecked());
	cmbAge->setEnabled(!m_btnID->isChecked());
}

void YahooSearch::showEvent(QShowEvent *e)
{
	YahooSearchBase::showEvent(e);
	radioToggled(false);
	emit setAdd(m_btnID->isChecked());
}

#ifndef WIN32
#include "yahoosearch.moc"
#endif

