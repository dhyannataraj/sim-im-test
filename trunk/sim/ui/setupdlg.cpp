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

#ifdef USE_SPELL
#include "spellsetup.h"
#endif

#include <qlistview.h>
#include <qwidgetstack.h>
#include <qpushbutton.h>
#include <qheader.h>

SetupDialog::SetupDialog(QWidget*, int nWin)
        : SetupDialogBase(NULL, "setup", false, WStyle_Minimize)
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

    itemMain = new QListViewItem(lstBars, i18n("My details"), QString::number(SETUP_DETAILS));
    itemMain->setOpen(true);

    addPage(new MainInfo(tabBars), SETUP_MAININFO, i18n("Main info"), "main");
    addPage(new HomeInfo(tabBars), SETUP_HOMEINFO, i18n("Home info"), "home");
    addPage(new WorkInfo(tabBars), SETUP_WORKINFO, i18n("Work info"), "work");
    addPage(new MoreInfo(tabBars), SETUP_MOREINFO, i18n("More info"), "more");
    addPage(new AboutInfo(tabBars), SETUP_ABOUT, i18n("About info"), "info");
    addPage(new InterestsInfo(tabBars), SETUP_INTERESTS, i18n("Interests"), "interest");
    addPage(new PastInfo(tabBars), SETUP_PAST, i18n("Group/Past"), "past");
    addPage(new PhoneBookDlg(tabBars), SETUP_PHONE, i18n("Phone book"), "phone");

    itemMain = new QListViewItem(lstBars, i18n("Preferences"), QString::number(SETUP_PREFERENCES));
    itemMain->setOpen(true);

    addPage(new StatusSetup(tabBars), SETUP_STATUS, i18n("Status mode"), "status");
    addPage(new NetworkSetup(tabBars), SETUP_CONNECTION, i18n("Connection"), "network");
    addPage(new ThemeSetup(tabBars), SETUP_STYLE, i18n("Style"), "style");
    addPage(new FontSetup(tabBars), SETUP_INTERFACE, i18n("Interface"), "text");
    addPage(new SoundSetup(tabBars), SETUP_SOUND, i18n("Sound"), "sound");
    addPage(new XOSDSetup(tabBars), SETUP_XOSD, i18n("On Screen notification"), "screen");
    addPage(new AlertDialog(tabBars), SETUP_ALERT, i18n("Alert"), "alert");
    addPage(new AcceptDialog(tabBars), SETUP_ACCEPT, i18n("Accept file"), "file");

#ifndef WIN32
    addPage(new MiscSetup(tabBars), SETUP_MISC, i18n("Miscellaneous"), "misc");
#endif

#ifdef USE_SPELL
    addPage(new SpellSetup(tabBars), SETUP_SPELL, i18n("Spell check"), "spellcheck");
#endif

    itemMain = new QListViewItem(lstBars, i18n("Auto reply"), QString::number(SETUP_AUTOREPLY));
    itemMain->setOpen(true);

    addPage(new MsgDialog(tabBars, ICQ_STATUS_AWAY), SETUP_AR_AWAY, Client::getStatusText(ICQ_STATUS_AWAY), "away");
    addPage(new MsgDialog(tabBars, ICQ_STATUS_NA), SETUP_AR_NA, Client::getStatusText(ICQ_STATUS_NA), "na");
    addPage(new MsgDialog(tabBars, ICQ_STATUS_OCCUPIED), SETUP_AR_OCCUPIED, Client::getStatusText(ICQ_STATUS_OCCUPIED), "occupied");
    addPage(new MsgDialog(tabBars, ICQ_STATUS_DND), SETUP_AR_DND, Client::getStatusText(ICQ_STATUS_DND), "dnd");
    addPage(new MsgDialog(tabBars, ICQ_STATUS_FREEFORCHAT), SETUP_AR_FREEFORCHAT, Client::getStatusText(ICQ_STATUS_FREEFORCHAT), "ffc");

    itemMain = new QListViewItem(lstBars, i18n("Security"), QString::number(SETUP_SECURITY));
    itemMain->setOpen(true);

    addPage(new GeneralSecurity(tabBars), SETUP_GENERAL_SEC, i18n("General"), "webaware");
    addPage(new ChangePasswd(tabBars), SETUP_PASSWD, i18n("Password"), "password");
    addPage(new IgnoreListSetup(tabBars), SETUP_IGNORE_LIST, i18n("Ignore list"), "ignorelist");
    addPage(new InvisibleListSetup(tabBars), SETUP_INVISIBLE_LIST, i18n("Invisible list"), "invisiblelist");
    addPage(new VisibleListSetup(tabBars), SETUP_VISIBLE_LIST, i18n("Visible list"), "visiblelist");

    tabBars->raiseWidget(nWin ? nWin : SETUP_MAININFO);
    connect(pMain, SIGNAL(iconChanged()), this, SLOT(iconChanged()));

    transparent = new TransparentTop(this, pMain->UseTransparentContainer, pMain->TransparentContainer);
};

SetupDialog::~SetupDialog()
{
    transparent = NULL;
}

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

void SetupDialog::setBackgroundPixmap(const QPixmap &pm)
{
    if (transparent) transparent->updateBackground(pm);
}

#ifndef _WINDOWS
#include "setupdlg.moc"
#endif

