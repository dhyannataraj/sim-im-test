/***************************************************************************
                          userviewcfg.cpp  -  description
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

#include "simapi.h"
#include "userviewcfg.h"
#include "core.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>

UserViewConfig::UserViewConfig(QWidget *parent)
        : UserViewConfigBase(parent)
{
    chkDblClick->setChecked(CorePlugin::m_plugin->getUseDblClick());
    chkSysColors->setChecked(CorePlugin::m_plugin->getUseSysColors());
    btnOnline->setColor(CorePlugin::m_plugin->getColorOnline());
    btnOffline->setColor(CorePlugin::m_plugin->getColorOffline());
    btnAway->setColor(CorePlugin::m_plugin->getColorAway());
    btnNA->setColor(CorePlugin::m_plugin->getColorNA());
    btnDND->setColor(CorePlugin::m_plugin->getColorDND());
    btnGroup->setColor(CorePlugin::m_plugin->getColorGroup());
    chkGroupSeparator->setChecked(CorePlugin::m_plugin->getGroupSeparator());
    connect(chkSysColors, SIGNAL(toggled(bool)), this, SLOT(colorsToggled(bool)));
    colorsToggled(chkSysColors->isChecked());
    fillBox(cmbSort1);
    fillBox(cmbSort2);
    fillBox(cmbSort3);
    setSortMode(CorePlugin::m_plugin->getSortMode());
    connect(cmbSort1, SIGNAL(activated(int)), this, SLOT(sortChanged(int)));
    connect(cmbSort2, SIGNAL(activated(int)), this, SLOT(sortChanged(int)));
    connect(cmbSort3, SIGNAL(activated(int)), this, SLOT(sortChanged(int)));
}

UserViewConfig::~UserViewConfig()
{
}

void UserViewConfig::apply()
{
    CorePlugin::m_plugin->setUseDblClick(chkDblClick->isChecked());
    CorePlugin::m_plugin->setUseSysColors(chkSysColors->isChecked());
    CorePlugin::m_plugin->setGroupSeparator(chkGroupSeparator->isChecked());
    CorePlugin::m_plugin->setSortMode(getSortMode());
    if (CorePlugin::m_plugin->getUseSysColors()){
        CorePlugin::m_plugin->setColorOnline(0);
        CorePlugin::m_plugin->setColorOffline(0);
        CorePlugin::m_plugin->setColorAway(0);
        CorePlugin::m_plugin->setColorNA(0);
        CorePlugin::m_plugin->setColorDND(0);
        CorePlugin::m_plugin->setColorGroup(0);
    }else{
        CorePlugin::m_plugin->setColorOnline(btnOnline->color().rgb());
        CorePlugin::m_plugin->setColorOffline(btnOffline->color().rgb());
        CorePlugin::m_plugin->setColorAway(btnAway->color().rgb());
        CorePlugin::m_plugin->setColorNA(btnNA->color().rgb());
        CorePlugin::m_plugin->setColorDND(btnDND->color().rgb());
        CorePlugin::m_plugin->setColorGroup(btnGroup->color().rgb());
    }
    Event e(EventRepaintView);
    e.process();
}

void UserViewConfig::colorsToggled(bool state)
{
    if (state){
        QColor textColor = colorGroup().text();
        QColor disabledColor = palette().disabled().text();
        btnOnline->setColor(textColor);
        btnOffline->setColor(disabledColor);
        btnAway->setColor(disabledColor);
        btnNA->setColor(disabledColor);
        btnDND->setColor(disabledColor);
        btnGroup->setColor(disabledColor);
    }
    btnOnline->setEnabled(!state);
    btnOffline->setEnabled(!state);
    btnAway->setEnabled(!state);
    btnNA->setEnabled(!state);
    btnDND->setEnabled(!state);
    btnGroup->setEnabled(!state);
    lblOnline->setEnabled(!state);
    lblOffline->setEnabled(!state);
    lblAway->setEnabled(!state);
    lblNA->setEnabled(!state);
    lblDND->setEnabled(!state);
    lblGroup->setEnabled(!state);
    lblColors->setEnabled(!state);
}

void UserViewConfig::fillBox(QComboBox *cmb)
{
    cmb->insertItem(i18n("Status"));
    cmb->insertItem(i18n("Last message time"));
    cmb->insertItem(i18n("Contact name"));
    cmb->insertItem("");
}

void UserViewConfig::setSortMode(unsigned mode)
{
    QComboBox *cmb[3] = { cmbSort1, cmbSort2, cmbSort3 };
    unsigned i;
    for (i = 0; i < 3; i++){
        cmb[i]->setEnabled(true);
        unsigned m = mode & 0xFF;
        mode = mode >> 8;
        if (m == 0){
            cmb[i++]->setCurrentItem(3);
            break;
        }
        cmb[i]->setCurrentItem(m - 1);
    }
    for (;i < 3; i++){
        cmb[i]->setCurrentItem(3);
        cmb[i]->setEnabled(false);
    }
}

void UserViewConfig::sortChanged(int)
{
    setSortMode(getSortMode());
}

unsigned UserViewConfig::getSortMode()
{
    unsigned m1 = cmbSort1->currentItem() + 1;
    if (m1 > 3)
        m1 = 0;
    unsigned m2 = cmbSort2->currentItem() + 1;
    if (m2 > 3)
        m2 = 0;
    unsigned m3 = cmbSort3->currentItem() + 1;
    if (m3 > 3)
        m3 = 0;
    if (m1){
        if (m2 == m1)
            m2 = 0;
        if (m2){
            if ((m3 == m1) || (m3 == m2))
                m3 = 0;
        }else{
            m3 = 0;
        }
    }else{
        m2 = 0;
        m3 = 0;
    }
    return (m3 << 16) + (m2 << 8) + m1;
}

#ifndef WIN32
#include "userviewcfg.moc"
#endif

