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
    chkDays->setChecked(data->CutDays != 0);
    chkSize->setChecked(data->CutSize != 0);
    edtDays->setValue(data->Days);
    edtSize->setValue(data->MaxSize);
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
    data->CutDays = chkDays->isChecked();
    data->CutSize = chkSize->isChecked();
    data->Days    = atol(edtDays->text());
    data->MaxSize = atol(edtSize->text());
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

