/***************************************************************************
                          arcfg.cpp  -  description
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

#include "arcfg.h"
#include "core.h"
#include "ballonmsg.h"
#include "editfile.h"

#include <qtabwidget.h>
#include <qcheckbox.h>

using namespace SIM;

ARConfig::ARConfig(QWidget *p, unsigned status, const QString &name, Contact *contact)
        : ARConfigBase(p)
{
    m_status  = status;
    m_contact = contact;
    setButtonsPict(this);
    tabAR->changeTab(tab, name);
    ARUserData *ar;
    QString text;
    QString noShow = CorePlugin::m_plugin->getNoShowAutoReply(m_status);
    if (m_contact){
        chkNoShow->hide();
        connect(chkOverride, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
        ar = (ARUserData*)(m_contact->userData.getUserData(CorePlugin::m_plugin->ar_data_id, false));
        if (ar)
            text = get_str(ar->AutoReply, m_status);
        if (!text.isEmpty()){
            chkOverride->setChecked(true);
        }else{
            ar = NULL;
            Group *grp = getContacts()->group(m_contact->getGroup());
            if (grp)
                ar = (ARUserData*)(m_contact->userData.getUserData(CorePlugin::m_plugin->ar_data_id, false));
            if (ar)
                text = get_str(ar->AutoReply, m_status);
        }
        toggled(chkOverride->isChecked());
    }else{
        chkOverride->hide();
    }
    if (text.isEmpty()){
        ar = (ARUserData*)(getContacts()->getUserData(CorePlugin::m_plugin->ar_data_id));
        if (!noShow.isEmpty())
            chkNoShow->setChecked(true);
        text = get_str(ar->AutoReply, m_status);
        if (text.isEmpty())
            text = get_str(ar->AutoReply, STATUS_AWAY);
    }
    edtAutoReply->setText(text);
    EventTmplHelpList e;
    e.process();
    edtAutoReply->helpList = e.helpList();
    connect(btnHelp, SIGNAL(clicked()), this, SLOT(help()));
}

void ARConfig::apply()
{
    if (m_contact){
        if (chkOverride->isChecked()){
            ARUserData *ar = (ARUserData*)(m_contact->userData.getUserData(CorePlugin::m_plugin->ar_data_id, true));
            set_str(&ar->AutoReply, m_status, edtAutoReply->text());
        }else{
            ARUserData *ar = (ARUserData*)(m_contact->userData.getUserData(CorePlugin::m_plugin->ar_data_id, false));
            if (ar)
                set_str(&ar->AutoReply, m_status, QString::null);
        }
    }else{
        ARUserData *ar = (ARUserData*)(getContacts()->getUserData(CorePlugin::m_plugin->ar_data_id));
        set_str(&ar->AutoReply, m_status, edtAutoReply->text());
        CorePlugin::m_plugin->setNoShowAutoReply(m_status, chkNoShow->isChecked() ? "1" : "");
    }
}

void ARConfig::toggled(bool bState)
{
    edtAutoReply->setEnabled(bState);
}

void ARConfig::help()
{
    QString helpString = i18n("In text you can use:") + '\n';
    EventTmplHelp e(helpString);
    e.process();
    BalloonMsg::message(e.help(), btnHelp, false, 400);
}

#ifndef NO_MOC_INCLUDES
#include "arcfg.moc"
#endif

