/***************************************************************************
                          userhistorycfg.cpp  -  description
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

#include "userhistorycfg.h"
#include "core.h"

#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlabel.h>

UserHistoryCfg::UserHistoryCfg(QWidget *parent, void *d)
        : UserHistoryCfgBase(parent)
{
    HistoryUserData *data = (HistoryUserData*)d;
    chkDays->setChecked(data->CutDays.bValue);
    chkSize->setChecked(data->CutSize.bValue);
    edtDays->setValue(data->Days.value);
    edtSize->setValue(data->MaxSize.value);
    toggledDays(chkDays->isChecked());
    toggledSize(chkSize->isChecked());
    connect(chkDays, SIGNAL(toggled(bool)), this, SLOT(toggledDays(bool)));
    connect(chkSize, SIGNAL(toggled(bool)), this, SLOT(toggledSize(bool)));
}

UserHistoryCfg::~UserHistoryCfg()
{
}

void UserHistoryCfg::apply(void *d)
{
    HistoryUserData *data = (HistoryUserData*)d;
    data->CutDays.bValue = chkDays->isChecked();
    data->CutSize.bValue = chkSize->isChecked();
    data->Days.value     = atol(edtDays->text());
    data->MaxSize.value  = atol(edtSize->text());
}

void UserHistoryCfg::toggledDays(bool bState)
{
    lblDays->setEnabled(bState);
    lblDays1->setEnabled(bState);
    edtDays->setEnabled(bState);
}

void UserHistoryCfg::toggledSize(bool bState)
{
    lblSize->setEnabled(bState);
    lblSize1->setEnabled(bState);
    edtSize->setEnabled(bState);
}

#ifndef WIN32
#include "userhistorycfg.moc"
#endif

