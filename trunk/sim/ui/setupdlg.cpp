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
#include "ui/keysetup.h"
#include "ui/wndcancel.h"
#include "ui/smssetup.h"
#include "ui/forwardsetup.h"

#include "ui/enable.h"

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
#include <qlayout.h>

#define PAGE(A)	  static QWidget *p_##A(QWidget *p, unsigned) { return new A(p); }

PAGE(MainInfo)
PAGE(HomeInfo)
PAGE(WorkInfo)
PAGE(MoreInfo)
PAGE(AboutInfo)
PAGE(InterestsInfo)
PAGE(PastInfo)
PAGE(PhoneBookDlg)
PAGE(StatusSetup)
PAGE(NetworkSetup)
PAGE(ThemeSetup)
PAGE(FontSetup)
PAGE(KeySetup)
PAGE(SoundSetup)
PAGE(XOSDSetup)
PAGE(AlertDialog)
PAGE(AcceptDialog)
PAGE(SMSSetup)
PAGE(ForwardSetup)
#ifndef WIN32
PAGE(MiscSetup)
#endif
#ifdef USE_SPELL
PAGE(SpellSetup)
#endif
PAGE(GeneralSecurity)
PAGE(ChangePasswd)
PAGE(IgnoreListSetup)
PAGE(InvisibleListSetup)
PAGE(VisibleListSetup)

static QWidget *p_MsgDialog(QWidget *p, unsigned param) { return new MsgDialog(p, param); }

SetupDialog::SetupDialog(QWidget*, int nWin)
        : SetupDialogBase(NULL, "setup", false, WStyle_Minimize | WDestructiveClose )
{
    SET_WNDPROC

    setCaption(caption());

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
	lstBars->setRootIsDecorated(true);
    connect(lstBars, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));

    itemMain = new QListViewItem(lstBars, i18n("My details"), QString::number(SETUP_DETAILS));
    itemMain->setOpen(true);

    addPage(p_MainInfo, SETUP_MAININFO, i18n("Main info"), "main");
    addPage(p_HomeInfo, SETUP_HOMEINFO, i18n("Home info"), "home");
    addPage(p_WorkInfo, SETUP_WORKINFO, i18n("Work info"), "work");
    addPage(p_MoreInfo, SETUP_MOREINFO, i18n("More info"), "more");
    addPage(p_AboutInfo, SETUP_ABOUT, i18n("About info"), "info");
    addPage(p_InterestsInfo, SETUP_INTERESTS, i18n("Interests"), "interest");
    addPage(p_PastInfo, SETUP_PAST, i18n("Group/Past"), "past");
    addPage(p_PhoneBookDlg, SETUP_PHONE, i18n("Phone book"), "phone");

    itemMain = new QListViewItem(lstBars, i18n("Preferences"), QString::number(SETUP_PREFERENCES));
    itemMain->setOpen(true);

    addPage(p_StatusSetup, SETUP_STATUS, i18n("Status mode"), "status");
    addPage(p_NetworkSetup, SETUP_CONNECTION, i18n("Connection"), "network");
    addPage(p_ThemeSetup, SETUP_STYLE, i18n("Style"), "style");
    addPage(p_FontSetup, SETUP_INTERFACE, i18n("Interface"), "text");
    addPage(p_KeySetup, SETUP_KEYS, i18n("Key shortcuts"), "key_bindings");
    addPage(p_SoundSetup, SETUP_SOUND, i18n("Sound"), "sound");
    addPage(p_XOSDSetup, SETUP_XOSD, i18n("On Screen notification"), "screen");
    addPage(p_AlertDialog, SETUP_ALERT, i18n("Alert"), "alert");
    addPage(p_AcceptDialog, SETUP_ACCEPT, i18n("Accept file"), "file");
    addPage(p_SMSSetup, SETUP_SMS, i18n("SMS"), "sms");
    addPage(p_ForwardSetup, SETUP_FORWARD, i18n("Forward"), "mail_forward");

#ifndef WIN32
    addPage(p_MiscSetup, SETUP_MISC, i18n("Miscellaneous"), "misc");
#endif

#ifdef USE_SPELL
    addPage(p_SpellSetup, SETUP_SPELL, i18n("Spell check"), "spellcheck");
#endif

    itemMain = new QListViewItem(lstBars, i18n("Auto reply"), QString::number(SETUP_AUTOREPLY));
    itemMain->setOpen(true);

    addPage(p_MsgDialog, SETUP_AR_AWAY, Client::getStatusText(ICQ_STATUS_AWAY), "away", ICQ_STATUS_AWAY);
    addPage(p_MsgDialog, SETUP_AR_NA, Client::getStatusText(ICQ_STATUS_NA), "na", ICQ_STATUS_NA);
    addPage(p_MsgDialog, SETUP_AR_OCCUPIED, Client::getStatusText(ICQ_STATUS_OCCUPIED), "occupied", ICQ_STATUS_OCCUPIED);
    addPage(p_MsgDialog, SETUP_AR_DND, Client::getStatusText(ICQ_STATUS_DND), "dnd", ICQ_STATUS_DND);
    addPage(p_MsgDialog, SETUP_AR_FREEFORCHAT, Client::getStatusText(ICQ_STATUS_FREEFORCHAT), "ffc", ICQ_STATUS_FREEFORCHAT);

    itemMain = new QListViewItem(lstBars, i18n("Security"), QString::number(SETUP_SECURITY));
    itemMain->setOpen(true);

    addPage(p_GeneralSecurity, SETUP_GENERAL_SEC, i18n("General"), "webaware");
    addPage(p_ChangePasswd, SETUP_PASSWD, i18n("Password"), "password");
    addPage(p_IgnoreListSetup, SETUP_IGNORE_LIST, i18n("Ignore list"), "ignorelist");
    addPage(p_InvisibleListSetup, SETUP_INVISIBLE_LIST, i18n("Invisible list"), "invisiblelist");
    addPage(p_VisibleListSetup, SETUP_VISIBLE_LIST, i18n("Visible list"), "visiblelist");

    raiseWidget(nWin ? nWin : SETUP_MAININFO);
    connect(pMain, SIGNAL(iconChanged()), this, SLOT(iconChanged()));

    transparent = new TransparentTop(this, pMain->UseTransparentContainer, pMain->TransparentContainer);
};

SetupDialog::~SetupDialog()
{
    emit closed();
    transparent = NULL;
}

void SetupDialog::showPage(int nWin)
{
    raiseWidget(nWin);
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

void SetupDialog::addPage(PAGEPROC *page, int id, const QString &name, const char *icon, unsigned param)
{
    QListViewItem *item = new QListViewItem(itemMain, name, QString::number(id));
    item->setPixmap(0, Pict(icon));
    item->setText(2, icon);
    item->setText(3, QString::number((unsigned)page));
    item->setText(4, QString::number(param));
}

void SetupDialog::raiseWidget(int id)
{
    for (QListViewItem *item = lstBars->firstChild(); item != NULL; item = item->nextSibling())
        if (raiseWidget(item, id)) break;
}

bool SetupDialog::raiseWidget(QListViewItem *i, unsigned id)
{
    if (id == i->text(1).toULong()){
        lstBars->setCurrentItem(i);
        return true;
    }
    for (QListViewItem *item = i->firstChild(); item != NULL; item = item->nextSibling())
        if (raiseWidget(item, id)) return true;
    return false;
}

void SetupDialog::selectionChanged()
{
    QListViewItem *item = lstBars->selectedItem();
    if (item == NULL) return;
    unsigned id = item->text(1).toULong();
    if (id == 0) return;
    QWidget *w = tabBars->widget(id);
    if (w == NULL){
        PAGEPROC *p = (PAGEPROC*)(item->text(3).toUInt());
        if (p == NULL) return;
        QWidget *page = p(tabBars, item->text(4).toUInt());
        tabBars->addWidget(page, id);
        connect(this, SIGNAL(applyChanges(ICQUser*)), page, SLOT(apply(ICQUser*)));
		tabBars->setMinimumSize(tabBars->minimumSizeHint());
    }
    tabBars->raiseWidget(id);
}

void SetupDialog::update()
{
    pClient->addInfoRequest(pClient->owner->Uin, true);
}

void SetupDialog::apply()
{
    ICQUser u;
    emit applyChanges(&u);
    pClient->setInfo(&u);
    pMain->saveState();
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

