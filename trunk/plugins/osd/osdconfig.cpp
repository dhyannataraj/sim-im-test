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
    chkMessage->setChecked(data->EnableMessage != 0);
    chkStatus->setChecked(data->EnableAlert != 0);
    chkTyping->setChecked(data->EnableTyping != 0);
    for (QObject *p = parent; p != NULL; p = p->parent()){
        if (!p->inherits("QTabWidget"))
            continue;
        QTabWidget *tab = static_cast<QTabWidget*>(p);
        void *data = getContacts()->getUserData(plugin->user_data_id);
        m_iface = new OSDIface(tab, data, plugin);
        tab->addTab(m_iface, i18n("&Interface"));
        break;
    }
}

void OSDConfig::apply()
{
    apply(getContacts()->getUserData(m_plugin->user_data_id));
}

void OSDConfig::apply(void *d)
{
    OSDUserData *data = (OSDUserData*)d;
    data->EnableMessage = chkMessage->isChecked();
    data->EnableAlert = chkStatus->isChecked();
    data->EnableTyping = chkTyping->isChecked();
    m_iface->apply(d);
}

#ifndef WIN32
#include "osdconfig.moc"
#endif

