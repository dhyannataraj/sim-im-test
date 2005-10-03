/***************************************************************************
                          mousecfg.cpp  -  description
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

#include "mousecfg.h"
#include "shortcuts.h"

#include <Q3ListView>
#include <QLabel>
#include <QRegExp>
#include <Q3Accel>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>

#include <QResizeEvent>

MouseConfig::MouseConfig(QWidget *parent, ShortcutsPlugin *plugin)
        : QWidget( parent)
{
    setupUi( this);
    m_plugin = plugin;
    lstCmd->setSorting(0);
    loadMenu(MenuMain);
    loadMenu(MenuGroup);
    loadMenu(MenuContact);
    adjustColumns();
    cmbButton->insertItem("");
    cmbButton->insertItem(i18n("Left click"));
    cmbButton->insertItem(i18n("Right click"));
    cmbButton->insertItem(i18n("Middle click"));
    cmbButton->insertItem(i18n("Left dblclick"));
    cmbButton->insertItem(i18n("Right dblclick"));
    cmbButton->insertItem(i18n("Middle dblclick"));
    selectionChanged();
    connect(lstCmd, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    connect(cmbButton, SIGNAL(activated(int)), this, SLOT(buttonChanged(int)));
    connect(chkAlt, SIGNAL(toggled(bool)), this, SLOT(changed(bool)));
    connect(chkCtrl, SIGNAL(toggled(bool)), this, SLOT(changed(bool)));
    connect(chkShift, SIGNAL(toggled(bool)), this, SLOT(changed(bool)));
}

MouseConfig::~MouseConfig()
{
}

void MouseConfig::apply()
{
    for (Q3ListViewItem *item = lstCmd->firstChild(); item; item = item->nextSibling()){
        m_plugin->setMouse(item->text(2).toUInt(), item->text(1).latin1());
    }
}

void MouseConfig::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    adjustColumns();
}

void MouseConfig::adjustColumns()
{
    QScrollBar *bar = lstCmd->verticalScrollBar();
    int wScroll = 0;
    if (bar && bar->isVisible())
        wScroll = bar->width();
    lstCmd->setColumnWidth(0, lstCmd->width() -
                           lstCmd->columnWidth(1) - 4 - wScroll);
}

void MouseConfig::loadMenu(unsigned id)
{
    Event eDef(EventGetMenuDef, (void*)id);
    CommandsDef *def = (CommandsDef*)(eDef.process());
    if (def){
        CommandsList list(*def, true);
        CommandDef *s;
        while ((s = ++list) != NULL){
            if ((s->id == 0) || (s->popup_id == 0))
                continue;
            QString title = i18n(s->text);
            if (title == "_")
                continue;
            Q3ListViewItem *item;
            for (item = lstCmd->firstChild(); item; item = item->nextSibling()){
                if (QString::number(s->popup_id) == item->text(3))
                    break;
            }
            if (item)
                continue;
            title = title.replace(QRegExp("&"), "");
            new Q3ListViewItem(lstCmd, title, m_plugin->getMouse(s->id), QString::number(s->id), QString::number(s->popup_id));
        }
    }
}

void MouseConfig::selectionChanged()
{
    Q3ListViewItem *item = lstCmd->currentItem();
    if (item == NULL){
        lblCmd->setText("");
        cmbButton->setCurrentItem(0);
        cmbButton->setEnabled(false);
        return;
    }
    lblCmd->setText(item->text(0));
    int n = ShortcutsPlugin::stringToButton(item->text(1).latin1());
    if (n == 0)
        chkAlt->setChecked((n & Qt::AltButton) != 0);
    chkCtrl->setChecked((n & Qt::ControlButton) != 0);
    chkShift->setChecked((n & Qt::ShiftButton) != 0);
    cmbButton->setEnabled(true);
    cmbButton->setCurrentItem(n);
    buttonChanged(0);
}

void MouseConfig::buttonChanged(int)
{
    if (cmbButton->currentItem()){
        chkAlt->setEnabled(true);
        chkCtrl->setEnabled(true);
        chkShift->setEnabled(true);
    }else{
        chkAlt->setChecked(false);
        chkAlt->setEnabled(false);
        chkCtrl->setChecked(false);
        chkCtrl->setEnabled(false);
        chkShift->setChecked(false);
        chkShift->setEnabled(false);
    }
    changed(false);
}

void MouseConfig::changed(bool)
{
    QString res;
    int n = cmbButton->currentItem();
    if (n){
        if (chkAlt->isChecked()) n |= Qt::AltButton;
        if (chkCtrl->isChecked()) n |= Qt::ControlButton;
        if (chkShift->isChecked()) n |= Qt::ShiftButton;
        res = ShortcutsPlugin::buttonToString(n).c_str();
    }
    Q3ListViewItem *item = lstCmd->currentItem();
    if (item == NULL)
        return;
    item->setText(1, res);
    adjustColumns();
}

#ifndef WIN32
#include "mousecfg.moc"
#endif

