/***************************************************************************
                          userinfo.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "userinfo.h"
#include "icons.h"
#include "client.h"
#include "setupdlg.h"

#include "ui/maininfo.h"
#include "ui/homeinfo.h"
#include "ui/workinfo.h"
#include "ui/moreinfo.h"
#include "ui/aboutinfo.h"
#include "ui/interestsinfo.h"
#include "ui/pastinfo.h"
#include "ui/phonebook.h"
#include "ui/alertdialog.h"
#include "ui/msgdialog.h"
#include "ui/accept.h"
#include "ui/soundsetup.h"

#include <qlistview.h>
#include <qheader.h>
#include <qpushbutton.h>
#include <qwidgetstack.h>

UserInfo::UserInfo(QWidget *parent, unsigned long uin, int page)
        : UserInfoBase(parent)
{
    inSave = false;

    m_nUin = uin;
    ICQUser *u = pClient->getUser(m_nUin);
    if (uin && (u == NULL)) return;

    lstBars->clear();
    lstBars->header()->hide();
    lstBars->setSorting(1);
    connect(lstBars, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));

    itemMain = new QListViewItem(lstBars, i18n("User info"), QString::number(SETUP_DETAILS));
    itemMain->setOpen(true);

    addWidget(new MainInfo(tabBars, true), SETUP_MAININFO, i18n("Main info"), "main");
    if (u && (u->Type == USER_TYPE_ICQ)){
        addWidget(new HomeInfo(tabBars, true), SETUP_HOMEINFO, i18n("Home info"), "home");
        addWidget(new WorkInfo(tabBars, true), SETUP_WORKINFO, i18n("Work info"), "work");
        addWidget(new MoreInfo(tabBars, true), SETUP_MOREINFO, i18n("More info"), "more");
        addWidget(new AboutInfo(tabBars, true), SETUP_ABOUT, i18n("About info"), "info");
        addWidget(new InterestsInfo(tabBars, true), SETUP_INTERESTS, i18n("Interests"), "interest");
        addWidget(new PastInfo(tabBars, true), SETUP_PAST, i18n("Group/Past"), "past");
    }
    addWidget(new PhoneBookDlg(tabBars, true), SETUP_PHONE, i18n("Phone book"), "phone");

    if (u && (u->Type == USER_TYPE_ICQ)){
        itemMain = new QListViewItem(lstBars, i18n("Preferences"), QString::number(SETUP_PREFERENCES));
        itemMain->setOpen(true);
        addWidget(new AlertDialog(tabBars, true), SETUP_ALERT, i18n("Alert"), "alert");
        addWidget(new AcceptDialog(tabBars, true), SETUP_ACCEPT, i18n("Accept file"), "file");
        addWidget(new SoundSetup(tabBars, true), SETUP_SOUND, i18n("Sound"), "sound");
        addWidget(new MsgDialog(tabBars, ICQ_STATUS_AWAY, true), SETUP_AR_AWAY,
                  Client::getStatusText(ICQ_STATUS_AWAY), Client::getStatusIcon(ICQ_STATUS_AWAY));
        addWidget(new MsgDialog(tabBars, ICQ_STATUS_NA, true), SETUP_AR_NA,
                  Client::getStatusText(ICQ_STATUS_NA), Client::getStatusIcon(ICQ_STATUS_NA));
        addWidget(new MsgDialog(tabBars, ICQ_STATUS_OCCUPIED, true), SETUP_AR_OCCUPIED,
                  Client::getStatusText(ICQ_STATUS_OCCUPIED), Client::getStatusIcon(ICQ_STATUS_OCCUPIED));
        addWidget(new MsgDialog(tabBars, ICQ_STATUS_DND, true), SETUP_AR_DND,
                  Client::getStatusText(ICQ_STATUS_DND), Client::getStatusIcon(ICQ_STATUS_DND));
        addWidget(new MsgDialog(tabBars, ICQ_STATUS_FREEFORCHAT, true), SETUP_AR_FREEFORCHAT,
                  Client::getStatusText(ICQ_STATUS_FREEFORCHAT), Client::getStatusIcon(ICQ_STATUS_FREEFORCHAT));
    }
    tabBars->raiseWidget(page ? page : SETUP_MAININFO);

    connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    connect(btnSave, SIGNAL(clicked()), this, SLOT(saveInfo()));
    connect(btnUpdate, SIGNAL(clicked()), this, SLOT(update()));

    loadInfo();
}

void UserInfo::addWidget(QWidget *info, int index, const QString &name, const char *icon)
{
    tabBars->addWidget(info, index);
    QListViewItem *item = new QListViewItem(itemMain, name, QString::number(index));
    item->setPixmap(0, Pict(icon));
    connect(this, SIGNAL(loadInfo(ICQUser*)), info, SLOT(load(ICQUser*)));
    connect(this, SIGNAL(saveInfo(ICQUser*)), info, SLOT(save(ICQUser*)));
}

void UserInfo::update()
{
    pClient->addInfoRequest(m_nUin, true);
}

void UserInfo::saveInfo()
{
    ICQUser *u = pClient->getUser(m_nUin, m_nUin == 0);
    if (u == NULL) return;
    if (m_nUin == 0) m_nUin = u->Uin();
    inSave = true;
    emit saveInfo(u);
    inSave = false;
    ICQEvent e(EVENT_INFO_CHANGED, m_nUin);
    processEvent(&e);
}

void UserInfo::processEvent(ICQEvent *e)
{
    if (inSave) return;
    btnUpdate->setEnabled(pClient->m_state == ICQClient::Logged);
    if ((e->type() != EVENT_INFO_CHANGED) || (e->Uin() != m_nUin)) return;
    loadInfo();
}

void UserInfo::loadInfo()
{
    ICQUser *u = pClient->getUser(m_nUin);
    if (u == NULL) return;
    emit loadInfo(u);
}

void UserInfo::selectionChanged()
{
    QListViewItem *item = lstBars->currentItem();
    if (item == NULL) return;
    int id = item->text(1).toLong();
    tabBars->raiseWidget(id);
}

#ifndef _WINDOWS
#include "userinfo.moc"
#endif

