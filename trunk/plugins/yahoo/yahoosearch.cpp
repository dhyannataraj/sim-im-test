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
    edtID->setValidator(new RegExpValidator("[0-9A-Za-z \\-_]+", this));
    initCombo(cmbAge, 0, ages);
    initCombo(cmbGender, 0, genders);
}

void YahooSearch::showEvent(QShowEvent *e)
{
    YahooSearchBase::showEvent(e);
    emit setAdd(false);
}

void YahooSearch::search()
{
}

void YahooSearch::searchStop()
{
}

void YahooSearch::searchMail(const QString&)
{
    emit searchDone(this);
}

void YahooSearch::searchName(const QString&, const QString&, const QString&)
{
}

bool YahooSearch::done(unsigned, Buffer&, const char*)
{
    emit searchDone(this);
    return false;
}

#ifndef WIN32
#include "yahoosearch.moc"
#endif

