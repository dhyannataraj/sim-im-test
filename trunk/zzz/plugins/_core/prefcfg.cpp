/***************************************************************************
                          prefcfg.cpp  -  description
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

#include "prefcfg.h"
#include "simapi.h"
#include "qchildwidget.h"

#include <qlayout.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qcheckbox.h>

PrefConfig::PrefConfig(QWidget *parent, CommandDef *cmd, Contact *contact, Group *group)
        : PrefConfigBase(parent)
{
    m_cmd = cmd;
    m_contact = contact;
    m_group = group;
    void *data = NULL;
    if (m_contact){
        data = m_contact->getUserData(m_cmd->id - 1);
        if (m_contact->userData.getUserData(m_cmd->id - 1, false))
            chkOverride->setChecked(true);
    }else if (m_group){
        data = m_group->getUserData(m_cmd->id - 1);
        if (m_group->userData.getUserData(m_cmd->id - 1, false))
            chkOverride->setChecked(true);
    }
    QWidget *w = NULL;
    if (data)
        w = ((getPreferencesWindow)(cmd->param))(addWnd, data);
    if (w){
        QVBoxLayout *lay = new QVBoxLayout(addWnd);
        lay->addWidget(w);
        connect(this, SIGNAL(apply(void*)), w, SLOT(apply(void*)));
        addWnd->setMinimumSize(w->minimumSizeHint());
        setMinimumSize(sizeHint());
    }
    tabWnd->setCurrentPage(0);
    tabWnd->changeTab(tabWnd->currentPage(), i18n(m_cmd->text));
    tabWnd->adjustSize();
    connect(chkOverride, SIGNAL(toggled(bool)), this, SLOT(overrideToggled(bool)));
    overrideToggled(chkOverride->isChecked());
}

void PrefConfig::apply()
{
    if (chkOverride->isChecked()){
        void *data = NULL;
        if (m_contact){
            data = m_contact->userData.getUserData(m_cmd->id - 1, true);
        }else if (m_group){
            data = m_group->userData.getUserData(m_cmd->id - 1, true);
        }
        if (data)
            emit apply(data);
    }else{
        if (m_contact){
            m_contact->userData.freeUserData(m_cmd->id - 1);
        }else if (m_group){
            m_group->userData.freeUserData(m_cmd->id - 1);
        }
    }
}

void PrefConfig::overrideToggled(bool bState)
{
    addWnd->setEnabled(bState);
}

#ifndef WIN32
#include "prefcfg.moc"
#endif


