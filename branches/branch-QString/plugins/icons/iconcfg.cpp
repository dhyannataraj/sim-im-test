/***************************************************************************
                          iconcfg.cpp  -  description
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

#include "iconcfg.h"
#include "icon.h"
#include "icons.h"

#include <qpushbutton.h>
#include <qlistbox.h>

#ifdef USE_KDE
#include <kfiledialog.h>
#define QFileDialog	KFileDialog
#else
#include <qfiledialog.h>
#endif

IconCfg::IconCfg(QWidget *parent, IconsPlugin *plugin)
        : IconCfgBase(parent)
{
    m_plugin = plugin;
    connect(btnUp, SIGNAL(clicked()), this, SLOT(up()));
    connect(btnDown, SIGNAL(clicked()), this, SLOT(down()));
    connect(btnAdd, SIGNAL(clicked()), this, SLOT(add()));
    connect(btnRemove, SIGNAL(clicked()), this, SLOT(remove()));
    connect(lstIcon, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    if (m_plugin->getDefault()){
		lstIcon->insertItem("icons/smiles.jisp");

    }else{
        for (unsigned i = 1; i <= m_plugin->getNIcons(); i++)
            lstIcon->insertItem(m_plugin->getIcon(i));
    }
    selectionChanged();
}

void IconCfg::apply()
{
    m_plugin->clearIcon();
    m_plugin->setDefault(false);
    for (unsigned i = 0; i < lstIcon->count(); i++)
        m_plugin->setIcon(i + 1, lstIcon->text(i));
    m_plugin->setNIcons(lstIcon->count());
    m_plugin->setIcons(true);
}

void IconCfg::up()
{
    int n = lstIcon->currentItem();
    if (n < 1)
        return;
    QString t = lstIcon->text(n);
    QListBoxItem *i = lstIcon->item(n);
    if (i == NULL)
        return;
    delete i;
    lstIcon->insertItem(t, n - 1);
    lstIcon->setCurrentItem(n - 1);
}

void IconCfg::down()
{
    int n = lstIcon->currentItem();
    if ((n < 0) || (n >= (int)(lstIcon->count() - 1)))
        return;
    QString t = lstIcon->text(n);
    QListBoxItem *i = lstIcon->item(n);
    if (i == NULL)
        return;
    delete i;
    lstIcon->insertItem(t, n + 1);
    lstIcon->setCurrentItem(n + 1);
}

void IconCfg::add()
{
#ifdef USE_KDE
    QString filter = i18n("*.jisp|Icon set");
#else
    QString filter = i18n("Icon set(*.jisp)");
#endif
    QString jisp = QFileDialog::getOpenFileName(SIM::app_file("icons/"), filter, topLevelWidget(), i18n("Select icon set"));
    if (!jisp.isEmpty())
        lstIcon->insertItem(jisp);
}

void IconCfg::remove()
{
    QListBoxItem *i = lstIcon->item(lstIcon->currentItem());
    if (i == NULL)
        return;
    delete i;
    selectionChanged();
}

void IconCfg::selectionChanged()
{
    int n = lstIcon->currentItem();
    if (n < 0){
        btnUp->setEnabled(false);
        btnDown->setEnabled(false);
        btnRemove->setEnabled(false);
        return;
    }
    btnRemove->setEnabled(true);
    btnUp->setEnabled(n > 0);
    btnDown->setEnabled(n < (int)(lstIcon->count() - 1));
}

#ifndef NO_MOC_INCLUDES
#include "iconcfg.moc"
#endif

