/***************************************************************************
                           themesetup.cpp  -  description
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

#include "themesetup.h"
#include "mainwin.h"
#include "themes.h"
#include "icons.h"
#include "transparent.h"

#include <qlistbox.h>
#include <qlabel.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qdir.h>
#include <qregexp.h>
#include <qstringlist.h>

ThemeSetup::ThemeSetup(QWidget *parent)
        : ThemeSetupBase(parent)
{
    lblPict->setPixmap(Pict("style"));
    lstThemes->clear();
    pMain->themes->fillList(lstThemes);
    for (unsigned i = 0; i < lstThemes->count(); i++){
        if (lstThemes->text(i) != pMain->themes->getTheme()) continue;
        lstThemes->setCurrentItem(i);
        break;
    }
    connect(lstThemes, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(apply(QListBoxItem*)));
    connect(lstIcons, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(apply(QListBoxItem*)));
    if (TransparentTop::bCanTransparent){
        chkTransparent->setChecked(pMain->UseTransparent());
        sldTransparent->setMinValue(0);
        sldTransparent->setMaxValue(100);
        sldTransparent->setValue(pMain->Transparent);
        sldTransparent->setTickmarks(QSlider::Below);
        sldTransparent->setTickInterval(5);
        connect(chkTransparent, SIGNAL(toggled(bool)), this, SLOT(checkedTransparent(bool)));
        chkTransparentContainer->setChecked(pMain->UseTransparentContainer());
        sldTransparentContainer->setMinValue(0);
        sldTransparentContainer->setMaxValue(100);
        sldTransparentContainer->setValue(pMain->TransparentContainer);
        sldTransparentContainer->setTickmarks(QSlider::Below);
        sldTransparentContainer->setTickInterval(5);
        connect(chkTransparentContainer, SIGNAL(toggled(bool)), this, SLOT(checkedTransparent(bool)));
        checkedTransparent(pMain->UseTransparent());
    }else{
        chkTransparent->hide();
        lblTransparent->hide();
        sldTransparent->hide();
        chkTransparentContainer->hide();
        lblTransparentContainer->hide();
        sldTransparentContainer->hide();
    }
    int h = lstThemes->itemHeight();
    lstThemes->setMinimumSize(QSize(0, h * 3));
    lstIcons->setMinimumSize(QSize(0, h * 3));
    connect(pMain, SIGNAL(setupInit()), this, SLOT(setupInit()));
#if defined(USE_KDE) || defined(WIN32)
    chkUserWnd->setChecked(pMain->UserWindowInTaskManager());
    chkMainWnd->setChecked(pMain->MainWindowInTaskManager());
#else
    chkUserWnd->hide();
    chkMainWnd->hide();
#endif
    setupInit();
}

const char *app_file(const char *f);

void ThemeSetup::setupInit()
{
    QDir icons(QString::fromLocal8Bit(app_file("icons")));
    QStringList lst = icons.entryList("*.dll");
    unsigned i = 1;
    lstIcons->clear();
    lstIcons->insertItem(i18n("Default icons"));
    lstIcons->setCurrentItem(0);
    for (QStringList::Iterator it = lst.begin(); it != lst.end(); ++it, i++){
        QString s = *it;
        s.replace(QRegExp(".dll$"), "");
        lstIcons->insertItem(s);
        if (s == QString::fromLocal8Bit(pMain->Icons.c_str()))
            lstIcons->setCurrentItem(i);
    }
};

void ThemeSetup::checkedTransparent(bool)
{
    sldTransparent->setEnabled(chkTransparent->isChecked());
    lblTransparent->setEnabled(chkTransparent->isChecked());
    sldTransparentContainer->setEnabled(chkTransparentContainer->isChecked());
    lblTransparentContainer->setEnabled(chkTransparentContainer->isChecked());
}

void ThemeSetup::apply(QListBoxItem*)
{
    apply((ICQUser*)NULL);
}

void ThemeSetup::apply(ICQUser*)
{
    pMain->themes->setTheme(lstThemes->currentText());
    QString iconsName = lstIcons->currentItem() ? lstIcons->currentText() : QString("");
    if (QString::fromLocal8Bit(pMain->Icons.c_str()) != iconsName){
        pMain->Icons = iconsName.local8Bit();
        pMain->changeIcons(0);
    }
    if (!TransparentTop::bCanTransparent) return;
    pMain->UseTransparent = chkTransparent->isChecked();
    pMain->Transparent = sldTransparent->value();
    pMain->UseTransparentContainer = chkTransparentContainer->isChecked();
    pMain->TransparentContainer = sldTransparentContainer->value();
    pMain->changeTransparent();
#if defined(USE_KDE) || defined(WIN32)
    bool bChange = false;
    if (pMain->UserWindowInTaskManager() != chkUserWnd->isChecked()){
        pMain->UserWindowInTaskManager = chkUserWnd->isChecked();
        bChange = true;
    }
    if (pMain->MainWindowInTaskManager() != chkMainWnd->isChecked()){
        pMain->MainWindowInTaskManager = chkMainWnd->isChecked();
        bChange = true;
    }
    if (bChange) pMain->changeWm();
#endif
}


#ifndef _WINDOWS
#include "themesetup.moc"
#endif

