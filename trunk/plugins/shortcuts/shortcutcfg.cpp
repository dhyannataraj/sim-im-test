/***************************************************************************
                          shortcutcfg.cpp  -  description
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

#include "shortcutcfg.h"
#include "shortcuts.h"
#include "qkeybutton.h"
#include "mousecfg.h"
#include "core.h"

#include <qlistview.h>
#include <qlabel.h>
#include <qregexp.h>
#include <qaccel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qtabwidget.h>

ShortcutsConfig::ShortcutsConfig(QWidget *parent, ShortcutsPlugin *plugin)
        : ShortcutsConfigBase(parent)
{
    m_plugin = plugin;
    lstKeys->setSorting(0);
    loadMenu(MenuMain, true);
    loadMenu(MenuGroup, false);
    loadMenu(MenuContact, false);
    loadMenu(MenuStatus, true);
    adjustColumns();
    selectionChanged();
    connect(lstKeys, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    connect(edtKey, SIGNAL(changed()), this, SLOT(keyChanged()));
    connect(btnClear, SIGNAL(clicked()), this, SLOT(keyClear()));
    connect(chkGlobal, SIGNAL(toggled(bool)), this, SLOT(globalChanged(bool)));
    for (QObject *p = parent; p != NULL; p = p->parent()){
        if (!p->inherits("QTabWidget"))
            continue;
        QTabWidget *tab = static_cast<QTabWidget*>(p);
        mouse_cfg = new MouseConfig(tab, plugin);
        tab->addTab(mouse_cfg, i18n("Mouse"));
        break;
    }
}

ShortcutsConfig::~ShortcutsConfig()
{
}

void ShortcutsConfig::loadMenu(unsigned id, bool bCanGlobal)
{
    Event eDef(EventGetMenuDef, (void*)id);
    CommandsDef *def = (CommandsDef*)(eDef.process());
    if (def){
        CommandsList list(*def, true);
        CommandDef *s;
        while ((s = ++list) != NULL){
            if ((s->id == 0) || s->popup_id || (s->flags & COMMAND_TITLE))
                continue;
            QString title = i18n(s->text);
            title = title.replace(QRegExp("&"), "");
            QString accel;
            int key = 0;
            const char *cfg_accel = m_plugin->getKey(s->id);
            if (cfg_accel)
                key = QAccel::stringToKey(cfg_accel);
            if ((key == 0) && s->accel)
                key = QAccel::stringToKey(i18n(s->accel));
            if (key)
                accel = QAccel::keyToString(key);
            QString global;
            bool bGlobal = m_plugin->getOldGlobal(s);
            const char *cfg_global = m_plugin->getGlobal(s->id);
            if (cfg_global && *cfg_global)
                bGlobal = !bGlobal;
            if (bGlobal)
                global = i18n("Global");
            QListViewItem *item;
            for (item = lstKeys->firstChild(); item; item = item->nextSibling()){
                if (item->text(3).toUInt() == s->id)
                    break;
            }
            if (item == NULL)
                new QListViewItem(lstKeys,
                                  title, accel, global,
                                  QString::number(s->id), bCanGlobal ? "1" : "");
        }
    }
}

void ShortcutsConfig::apply()
{
    mouse_cfg->apply();
    saveMenu(MenuMain);
    saveMenu(MenuGroup);
    saveMenu(MenuContact);
    saveMenu(MenuStatus);
    m_plugin->releaseKeys();
    m_plugin->applyKeys();
}

void ShortcutsConfig::saveMenu(unsigned id)
{
    Event eDef(EventGetMenuDef, (void*)id);
    CommandsDef *def = (CommandsDef*)(eDef.process());
    if (def){
        CommandsList list(*def, true);
        CommandDef *s;
        while ((s = ++list) != NULL){
            if ((s->id == 0) || s->popup_id)
                continue;
            for (QListViewItem *item = lstKeys->firstChild(); item; item = item->nextSibling()){
                if (item->text(3).toUInt() != s->id) continue;
                int key = QAccel::stringToKey(item->text(1));
                const char *cfg_key = m_plugin->getOldKey(s);
                if (key == QAccel::stringToKey(cfg_key)){
                    m_plugin->setKey(s->id, NULL);
                }else{
                    QString t = item->text(1);
                    if (t.isEmpty())
                        t = "-";
                    m_plugin->setKey(s->id, t);
                }
                bool bGlobal = !item->text(2).isEmpty();
                bool bCfgGlobal = m_plugin->getOldGlobal(s);
                if (item->text(1).isEmpty()){
                    bGlobal = false;
                    bCfgGlobal = false;
                }
                if (bGlobal == bCfgGlobal){
                    m_plugin->setGlobal(s->id, NULL);
                }else{
                    m_plugin->setGlobal(s->id, bGlobal ? "1" : "-1");
                }
            }
        }
    }
}

void ShortcutsConfig::resizeEvent(QResizeEvent *e)
{
    ShortcutsConfigBase::resizeEvent(e);
    adjustColumns();
}

void ShortcutsConfig::adjustColumns()
{
    QScrollBar *bar = lstKeys->verticalScrollBar();
    int wScroll = 0;
    if (bar && bar->isVisible())
        wScroll = bar->width();
    lstKeys->setColumnWidth(0, lstKeys->width() -
                            lstKeys->columnWidth(2) - lstKeys->columnWidth(1) - 4 - wScroll);
}

void ShortcutsConfig::selectionChanged()
{
    QListViewItem *item = lstKeys->currentItem();
    if (item == NULL){
        lblKey->setText("");
        edtKey->setEnabled(false);
        btnClear->setEnabled(false);
        chkGlobal->setEnabled(false);
        return;
    }
    lblKey->setText(item->text(0));
    edtKey->setEnabled(true);
    btnClear->setEnabled(true);
    edtKey->setText(item->text(1));
    if (!item->text(1).isEmpty() && !item->text(4).isEmpty()){
        chkGlobal->setEnabled(true);
        chkGlobal->setChecked(!item->text(2).isEmpty());
    }else{
        chkGlobal->setEnabled(false);
        chkGlobal->setChecked(false);
    }
}

void ShortcutsConfig::keyClear()
{
    QListViewItem *item = lstKeys->currentItem();
    if (item == NULL)
        return;
    item->setText(1, "");
    edtKey->setText("");
    edtKey->clearFocus();
}

void ShortcutsConfig::keyChanged()
{
    QListViewItem *item = lstKeys->currentItem();
    if (item == NULL)
        return;
    QString key = edtKey->text();
    if (key.isEmpty() || item->text(4).isEmpty()){
        chkGlobal->setChecked(false);
        chkGlobal->setEnabled(false);
    }else{
        chkGlobal->setEnabled(true);
    }
    item->setText(1, key);
    edtKey->clearFocus();
}

void ShortcutsConfig::globalChanged(bool)
{
    QListViewItem *item = lstKeys->currentItem();
    if ((item == NULL) || item->text(4).isEmpty())
        return;
    item->setText(2, chkGlobal->isChecked() ? i18n("Global") : QString(""));
}

#ifndef WIN32
#include "shortcutcfg.moc"
#endif

