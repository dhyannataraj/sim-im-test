/***************************************************************************
                           keysetup.cpp  -  description
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

#include "keysetup.h"
#include "mainwin.h"
#include "icons.h"
#include "qkeybutton.h"

#include <qlabel.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qheader.h>

KeySetup::KeySetup(QWidget *parent)
        : KeySetupBase(parent)
{
    chkSendEnter->setChecked(pMain->SendEnter);
    lstActions->removeColumn(0);
    lstActions->addColumn(i18n("Action"));
    lstActions->addColumn(i18n("Keys"));
    QListViewItem *first = new QListViewItem(lstActions,
                           i18n("Show/Hide main window"),
                           pMain->KeyWindow.c_str(), QString::number(0));
    new QListViewItem(lstActions,
                      i18n("Double click on dock"),
                      pMain->KeyDblClick.c_str(), QString::number(1));
    new QListViewItem(lstActions,
                      i18n("Show search window"),
                      pMain->KeySearch.c_str(), QString::number(2));
    connect(lstActions, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(selectionChanged(QListViewItem*)));
    connect(chkEnable, SIGNAL(toggled(bool)), this, SLOT(enableToggled(bool)));
    connect(btnKey, SIGNAL(changed()), this, SLOT(keyChanged()));
    lstActions->setCurrentItem(first);
    lstActions->setSelected(first, true);
}

void KeySetup::apply(ICQUser*)
{
    pMain->SendEnter = chkSendEnter->isChecked();
    string kWindow;
    string kDblClick;
    string kSearch;
    for (QListViewItem *item = lstActions->firstChild(); item != NULL; item = item->nextSibling()){
        switch (item->text(2).toInt()){
        case 0:
            if (!item->text(1).isEmpty())
                kWindow = item->text(1).latin1();
            break;
        case 1:
            if (!item->text(1).isEmpty())
                kDblClick = item->text(1).latin1();
            break;
        case 2:
            if (!item->text(1).isEmpty())
                kSearch = item->text(1).latin1();
            break;
        }
    }
    pMain->setKeys(kWindow.c_str(), kDblClick.c_str(), kSearch.c_str());
}

void KeySetup::selectionChanged(QListViewItem *item)
{
    QString key = item->text(1);
    btnKey->setText(key);
    chkEnable->setChecked(!key.isEmpty());
}

void KeySetup::enableToggled(bool bOn)
{
    btnKey->setEnabled(bOn);
    keyChanged();
}

void KeySetup::keyChanged()
{
    QListViewItem *item = lstActions->currentItem();
    if (item == NULL) return;
    item->setText(1, chkEnable->isChecked() ? btnKey->text() : QString(""));
}


#ifndef _WINDOWS
#include "keysetup.moc"
#endif

