/***************************************************************************
                          osdconfig.cpp  -  description
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

#include "osdconfig.h"
#include "osdiface.h"
#include "osd.h"
#include "qcolorbutton.h"
#include "fontedit.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qlabel.h>

using SIM::getContacts;

OSDConfig::OSDConfig(QWidget *parent, void *d, OSDPlugin *plugin)
        : OSDConfigBase(parent)
{
    m_plugin = plugin;
    OSDUserData *data = (OSDUserData*)d;
    chkMessage->setChecked(data->EnableMessage.asBool());
    chkMessageContent->setChecked(data->EnableMessageShowContent.asBool());
    chkStatus->setChecked(data->EnableAlert.asBool());
    chkStatusOnline->setChecked(data->EnableAlertOnline.asBool());
    chkStatusAway->setChecked(data->EnableAlertAway.asBool());
    chkStatusNA->setChecked(data->EnableAlertNA.asBool());
    chkStatusDND->setChecked(data->EnableAlertDND.asBool());
    chkStatusOccupied->setChecked(data->EnableAlertOccupied.asBool());
    chkStatusFFC->setChecked(data->EnableAlertFFC.asBool());
    chkStatusOffline->setChecked(data->EnableAlertOffline.asBool());
    chkTyping->setChecked(data->EnableTyping.asBool());
    for (QObject *p = parent; p != NULL; p = p->parent()){
        if (!p->inherits("QTabWidget"))
            continue;
        QTabWidget *tab = static_cast<QTabWidget*>(p);
        void *data = getContacts()->getUserData(plugin->user_data_id);
        m_iface = new OSDIface(tab, data, plugin);
        tab->addTab(m_iface, i18n("&Interface"));
        break;
    }
    edtLines->setValue(data->ContentLines.asULong());
    connect(chkStatus, SIGNAL(toggled(bool)), this, SLOT(statusToggled(bool)));
    connect(chkMessage, SIGNAL(toggled(bool)), this, SLOT(showMessageToggled(bool)));
    connect(chkMessageContent, SIGNAL(toggled(bool)), this, SLOT(contentToggled(bool)));
    showMessageToggled(chkMessage->isChecked());
    contentToggled(chkMessageContent->isChecked());
    statusToggled(data->EnableAlert.asBool());
}

void OSDConfig::apply()
{
    apply(getContacts()->getUserData(m_plugin->user_data_id));
}

void OSDConfig::apply(void *d)
{
    OSDUserData *data = (OSDUserData*)d;
    data->EnableMessage.asBool() = chkMessage->isChecked();
    data->EnableMessageShowContent.asBool() = chkMessageContent->isChecked();
    data->EnableAlert.asBool() = chkStatus->isChecked();
    data->EnableAlertOnline.asBool() = chkStatusOnline->isChecked();
    data->EnableAlertAway.asBool() = chkStatusAway->isChecked();
    data->EnableAlertNA.asBool() = chkStatusNA->isChecked();
    data->EnableAlertDND.asBool() = chkStatusDND->isChecked();
    data->EnableAlertOccupied.asBool() = chkStatusOccupied->isChecked();
    data->EnableAlertFFC.asBool() = chkStatusFFC->isChecked();
    data->EnableAlertOffline.asBool() = chkStatusOffline->isChecked();
    data->EnableTyping.asBool() = chkTyping->isChecked();
    data->ContentLines.asULong() = atol(edtLines->text());
    m_iface->apply(d);
}

void OSDConfig::statusToggled(bool bState)
{
    chkStatusOnline->setEnabled(bState);
    chkStatusAway->setEnabled(bState);
    chkStatusNA->setEnabled(bState);
    chkStatusDND->setEnabled(bState);
    chkStatusOccupied->setEnabled(bState);
    chkStatusFFC->setEnabled(bState);
    chkStatusOffline->setEnabled(bState);
}

void OSDConfig::showMessageToggled(bool bState)
{
    chkMessageContent->setEnabled(bState);
    edtLines->setEnabled(bState && chkMessageContent->isChecked());
    lblLines->setEnabled(bState && chkMessageContent->isChecked());
}

void OSDConfig::contentToggled(bool bState)
{
    edtLines->setEnabled(bState && chkMessage->isChecked());
    lblLines->setEnabled(bState && chkMessage->isChecked());
}

#ifndef _MSC_VER
#include "osdconfig.moc"
#endif

