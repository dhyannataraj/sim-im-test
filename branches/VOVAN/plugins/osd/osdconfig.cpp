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

OSDConfig::OSDConfig(QWidget *parent, void *d, OSDPlugin *plugin)
        : OSDConfigBase(parent)
{
    m_plugin = plugin;
    OSDUserData *data = (OSDUserData*)d;
    chkMessage->setChecked(data->EnableMessage.bValue);
    chkMessageContent->setChecked(data->EnableMessageShowContent.bValue);
    chkStatus->setChecked(data->EnableAlert.bValue);
    chkStatusOnline->setChecked(data->EnableAlertOnline.bValue);
    chkStatusAway->setChecked(data->EnableAlertAway.bValue);
    chkStatusNA->setChecked(data->EnableAlertNA.bValue);
    chkStatusDND->setChecked(data->EnableAlertDND.bValue);
    chkStatusOccupied->setChecked(data->EnableAlertOccupied.bValue);
    chkStatusFFC->setChecked(data->EnableAlertFFC.bValue);
    chkStatusOffline->setChecked(data->EnableAlertOffline.bValue);
    chkTyping->setChecked(data->EnableTyping.bValue);
    for (QObject *p = parent; p != NULL; p = p->parent()){
        if (!p->inherits("QTabWidget"))
            continue;
        QTabWidget *tab = static_cast<QTabWidget*>(p);
        void *data = getContacts()->getUserData(plugin->user_data_id);
        m_iface = new OSDIface(tab, data, plugin);
        tab->addTab(m_iface, i18n("&Interface"));
        break;
    }
    edtLines->setValue(data->ContentLines.value);
    connect(chkStatus, SIGNAL(toggled(bool)), this, SLOT(statusToggled(bool)));
    connect(chkMessage, SIGNAL(toggled(bool)), this, SLOT(showMessageToggled(bool)));
    connect(chkMessageContent, SIGNAL(toggled(bool)), this, SLOT(contentToggled(bool)));
    showMessageToggled(chkMessage->isChecked());
    contentToggled(chkMessageContent->isChecked());
    statusToggled(data->EnableAlert.bValue);
}

void OSDConfig::apply()
{
    apply(getContacts()->getUserData(m_plugin->user_data_id));
}

void OSDConfig::apply(void *d)
{
    OSDUserData *data = (OSDUserData*)d;
    data->EnableMessage.bValue = chkMessage->isChecked();
    data->EnableMessageShowContent.bValue = chkMessageContent->isChecked();
    data->EnableAlert.bValue = chkStatus->isChecked();
    data->EnableAlertOnline.bValue = chkStatusOnline->isChecked();
    data->EnableAlertAway.bValue = chkStatusAway->isChecked();
    data->EnableAlertNA.bValue = chkStatusNA->isChecked();
    data->EnableAlertDND.bValue = chkStatusDND->isChecked();
    data->EnableAlertOccupied.bValue = chkStatusOccupied->isChecked();
    data->EnableAlertFFC.bValue = chkStatusFFC->isChecked();
    data->EnableAlertOffline.bValue = chkStatusOffline->isChecked();
    data->EnableTyping.bValue = chkTyping->isChecked();
    data->ContentLines.value = atol(edtLines->text());
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

#ifndef WIN32
#include "osdconfig.moc"
#endif

