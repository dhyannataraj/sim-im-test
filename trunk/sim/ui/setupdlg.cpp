/***************************************************************************
                          setupdlg.cpp  -  description
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

#include "setupdlg.h"
#include "icons.h"
#include "client.h"
#include "transparent.h"
#include "mainwin.h"

#include "ui/maininfo.h"
#include "ui/homeinfo.h"
#include "ui/workinfo.h"
#include "ui/moreinfo.h"
#include "ui/aboutinfo.h"
#include "ui/interestsinfo.h"
#include "ui/pastinfo.h"
#include "ui/changepasswd.h"
#include "ui/generalsec.h"
#include "ui/listsec.h"
#include "ui/themesetup.h"
#include "ui/statussetup.h"
#include "ui/phonebook.h"
#include "ui/soundsetup.h"
#include "ui/alertdialog.h"
#include "ui/msgdialog.h"
#include "ui/fontsetup.h"
#include "ui/accept.h"
#include "ui/network.h"
#include "ui/xosdsetup.h"

#ifndef WIN32
#include "ui/miscsetup.h"
#endif

#if USE_SPELL
#include "spellsetup.h"
#endif

#include <qlistview.h>
#include <qwidgetstack.h>
#include <qpushbutton.h>
#include <qheader.h>

SetupDialog::SetupDialog(QWidget *parent, int nWin)
        : SetupDialogBase(parent, "setup", false, WStyle_Minimize)
{
    new TransparentTop(this, pMain->UseTransparentContainer, pMain->TransparentContainer);

    setIcon(Pict("configure"));
    setCaption(i18n("Setup"));
    connect(btnUpdate, SIGNAL(clicked()), this, SLOT(update()));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
    connect(btnOK, SIGNAL(clicked()), this, SLOT(ok()));
    connect(btnApply, SIGNAL(clicked()), this, SLOT(apply()));
    lstBars->clear();
    lstBars->header()->hide();
    lstBars->setSorting(1);
    connect(lstBars, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));

    itemMain = new QListViewItem(lstBars, i18n("My details"), "100");
    itemMain->setOpen(true);

    addPage(new MainInfo(tabBars), 101, i18n("Main info"), "main");
    addPage(new HomeInfo(tabBars), 102, i18n("Home info"), "home");
    addPage(new WorkInfo(tabBars), 103, i18n("Work info"), "work");
    addPage(new MoreInfo(tabBars), 104, i18n("More info"), "more");
    addPage(new AboutInfo(tabBars), 105, i18n("About info"), "info");
    addPage(new InterestsInfo(tabBars), 106, i18n("Interests"), "interest");
    addPage(new PastInfo(tabBars), 107, i18n("Group/Past"), "past");
    addPage(new PhoneBookDlg(tabBars), 108, i18n("Phone book"), "phone");

    itemMain = new QListViewItem(lstBars, i18n("Preferences"), "200");
    itemMain->setOpen(true);

    addPage(new StatusSetup(tabBars), 201, i18n("Status mode"), "status");
    addPage(new NetworkSetup(tabBars), 202, i18n("Connection"), "network");
    addPage(new ThemeSetup(tabBars), 203, i18n("Style"), "style");
    addPage(new FontSetup(tabBars), 204, i18n("Interface"), "text");
    addPage(new SoundSetup(tabBars), 205, i18n("Sound"), "sound");
    addPage(new XOSDSetup(tabBars), 206, i18n("On Screen notification"), "screen");

    addPage(new AlertDialog(tabBars), 207, i18n("Alert"), "alert");
    addPage(new AcceptDialog(tabBars), 208, i18n("Accept file"), "file");

#ifndef WIN32
    addPage(new MiscSetup(tabBars), 209, i18n("Miscellaneous"), "misc");
#endif

#if USE_SPELL
    addPage(new SpellSetup(tabBars), 210, i18n("Spell check"), "spellcheck");
#endif

    itemMain = new QListViewItem(lstBars, i18n("Auto reply"), "300");
    itemMain->setOpen(true);

    addPage(new MsgDialog(tabBars, ICQ_STATUS_AWAY), 301, Client::getStatusText(ICQ_STATUS_AWAY), "away");
    addPage(new MsgDialog(tabBars, ICQ_STATUS_NA), 302, Client::getStatusText(ICQ_STATUS_NA), "na");
    addPage(new MsgDialog(tabBars, ICQ_STATUS_OCCUPIED), 303, Client::getStatusText(ICQ_STATUS_OCCUPIED), "occupied");
    addPage(new MsgDialog(tabBars, ICQ_STATUS_DND), 304, Client::getStatusText(ICQ_STATUS_DND), "dnd");
    addPage(new MsgDialog(tabBars, ICQ_STATUS_FREEFORCHAT), 305, Client::getStatusText(ICQ_STATUS_FREEFORCHAT), "ffc");

    itemMain = new QListViewItem(lstBars, i18n("Security"), "400");
    itemMain->setOpen(true);

    addPage(new GeneralSecurity(tabBars), 401, i18n("General"), "webaware");
    addPage(new ChangePasswd(tabBars), 402, i18n("Password"), "password");
    addPage(new IgnoreListSetup(tabBars), 403, i18n("Ignore list"), "ignorelist");
    addPage(new InvisibleListSetup(tabBars), 404, i18n("Invisible list"), "invisiblelist");
    addPage(new VisibleListSetup(tabBars), 405, i18n("Visible list"), "visiblelist");

    tabBars->raiseWidget(nWin ? nWin : 101);
    connect(pMain, SIGNAL(iconChanged()), this, SLOT(iconChanged()));
};

void SetupDialog::iconChanged()
{
    setIcon(Pict("configure"));
    for (QListViewItem *item = lstBars->firstChild(); item != NULL; item = item->nextSibling())
        iconChanged(item);
}

void SetupDialog::iconChanged(QListViewItem *item)
{
    QString text = item->text(2);
    if (text.length())
        item->setPixmap(0, Pict(text.latin1()));
    for (item = item->firstChild(); item != NULL; item = item->nextSibling())
        iconChanged(item);
}

void SetupDialog::addPage(QWidget *page, int id, const QString &name, const char *icon)
{
    tabBars->addWidget(page, id);
    QListViewItem *item = new QListViewItem(itemMain, name, QString::number(id));
    item->setPixmap(0, Pict(icon));
    connect(this, SIGNAL(applyChanges(ICQUser*)), page, SLOT(apply(ICQUser*)));
    item->setText(2, icon);
}

void SetupDialog::selectionChanged()
{
    QListViewItem *item = lstBars->selectedItem();
    if (item == NULL) return;
    unsigned id = item->text(1).toULong();
    if (id == 0) return;
    tabBars->raiseWidget(id);
}

void SetupDialog::update()
{
    pClient->addInfoRequest(pClient->Uin, true);
}

void SetupDialog::apply()
{
    ICQUser u;
    emit applyChanges(&u);
    pClient->setInfo(&u);
}

void SetupDialog::ok()
{
    apply();
    close();
}

#ifndef _WINDOWS
#include "setupdlg.moc"
#endif

