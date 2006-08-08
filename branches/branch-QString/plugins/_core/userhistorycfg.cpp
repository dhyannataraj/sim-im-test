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
    chkDays->setChecked(data->CutDays.toBool());
    chkSize->setChecked(data->CutSize.toBool());
    edtDays->setValue(data->Days.toULong());
    edtSize->setValue(data->MaxSize.toULong());
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
    data->CutDays.asBool()  = chkDays->isChecked();
    data->CutSize.asBool()  = chkSize->isChecked();
    data->Days.asULong()    = edtDays->text().toULong();
    data->MaxSize.asULong() = edtSize->text().toULong();
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

#ifndef NO_MOC_INCLUDES
#include "userhistorycfg.moc"
#endif

