/***************************************************************************
                          toolsetup.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#include "toolsetup.h"
#include "mainwin.h"
#include "icons.h"

#include <qapplication.h>
#include <qwidgetlist.h>
#include <qlistbox.h>
#include <qregexp.h>

#ifdef USE_KDE
#include <kwin.h>
#endif

#include <vector>
using namespace std;

ToolBarSetup::ToolBarSetup(const ToolBarDef *_def, list<unsigned long> *_active)
        : ToolBarSetupBase(NULL, "toolbar_setup", false, WDestructiveClose)
{
    def = _def;
    m_active = _active;
    active = *_active;
    setIcon(Pict("setup"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
    connect(lstButtons, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    connect(lstActive, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    connect(btnAdd, SIGNAL(clicked()), this, SLOT(addClick()));
    connect(btnRemove, SIGNAL(clicked()), this, SLOT(removeClick()));
    connect(btnUp, SIGNAL(clicked()), this, SLOT(upClick()));
    connect(btnDown, SIGNAL(clicked()), this, SLOT(downClick()));
    connect(btnOk, SIGNAL(clicked()), this, SLOT(okClick()));
    connect(btnApply, SIGNAL(clicked()), this, SLOT(applyClick()));
    setButtons();
    lstActive->clear();
    list<unsigned long>::iterator it;
    if (active.size() == 0){
        for (const ToolBarDef *d = def; d->id != BTN_END_DEF; d++)
            active.push_back(d->id);
    }
    for (it = active.begin(); it != active.end(); ++it){
        int id = *it;
        if (id == BTN_SEPARATOR){
            lstActive->insertItem(Pict("separator"), i18n("Separator"));
            continue;
        }
        const ToolBarDef *d;
        for (d = def; d->id != BTN_END_DEF; d++)
            if (d->id == id) break;
        if (d->id == BTN_END_DEF){
            for (d++; d->id != BTN_END_DEF; d++)
                if (d->id == id) break;
            if (d->id == BTN_END_DEF) continue;
        }
        addButton(lstActive, d);
    }
    selectionChanged();
    bDirty = false;
}

void ToolBarSetup::okClick()
{
    applyClick();
    close();
}

void ToolBarSetup::applyClick()
{
    if (bDirty){
        *m_active = active;
        pMain->changeToolBar(def);
    }
}

void ToolBarSetup::addButton(QListBox *lst, const ToolBarDef *d)
{
    QString icon = d->icon;
    if (d->flags & BTN_TOGGLE_PICT) icon += "_off";
    QString name = i18n(d->text);
    name = name.replace(QRegExp("&"), "");
    lst->insertItem(Pict(icon.latin1()), name);
}

void ToolBarSetup::selectionChanged()
{
    btnAdd->setEnabled(lstButtons->currentItem() >= 0);
    btnRemove->setEnabled(lstActive->currentItem() >= 0);
    btnUp->setEnabled(lstActive->currentItem() > 0);
    btnDown->setEnabled((lstActive->currentItem() >= 0) &&
                        (lstActive->currentItem() < lstActive->count() - 1));
}

void ToolBarSetup::setButtons()
{
    lstButtons->clear();
    list<unsigned long>::iterator it;
    const ToolBarDef *d;
    bool bFirst = true;
    for (d = def;; d++){
        if (d->id == BTN_END_DEF){
            if (bFirst){
                bFirst = false;
            }else{
                break;
            }
            continue;
        }
        if (d->id == BTN_SEPARATOR) continue;
        for (it = active.begin(); it != active.end(); ++it)
            if ((*it) == d->id) break;
        if (it != active.end()) continue;
        addButton(lstButtons, d);
    }
    lstButtons->insertItem(Pict("separator"), i18n("Separator"));
}

void ToolBarSetup::addClick()
{
    int i = lstButtons->currentItem();
    if (i < 0) return;
    list<unsigned long>::iterator it;
    const ToolBarDef *d;
    bool bFirst = true;
    for (d = def;; d++){
        if (d->id == BTN_END_DEF){
            if (bFirst){
                bFirst = false;
            }else{
                break;
            }
            continue;
        }
        if (d->id == BTN_SEPARATOR) continue;
        for (it = active.begin(); it != active.end(); ++it)
            if ((*it) == d->id) break;
        if (it != active.end()) continue;
        if (i-- == 0) break;
    }
    if (d->id == BTN_END_DEF){
        active.push_back(BTN_SEPARATOR);
        lstActive->insertItem(Pict("separator"), i18n("Separator"));
    }else{
        active.push_back(d->id);
        addButton(lstActive, d);
        setButtons();
    }
    bDirty = true;
}

void ToolBarSetup::removeClick()
{
    int i = lstActive->currentItem();
    if (i < 0) return;
    vector<unsigned long> b;
    list<unsigned long>::iterator it;
    for (it = active.begin(); it != active.end(); ++it)
        b.push_back(*it);
    active.clear();
    int n;
    for (n = 0; n < i; n++)
        active.push_back(b[n]);
    for (n++; n < b.size(); n++)
        active.push_back(b[n]);
    lstActive->removeItem(i);
    setButtons();
    bDirty = true;
}

void ToolBarSetup::upClick()
{
    int i = lstActive->currentItem();
    if (i <= 0) return;
    vector<unsigned long> b;
    list<unsigned long>::iterator it;
    for (it = active.begin(); it != active.end(); ++it)
        b.push_back(*it);
    unsigned long r = b[i];
    b[i] = b[i-1];
    b[i-1] = r;
    active.clear();
    for (int n = 0; n < b.size(); n++)
        active.push_back(b[n]);
    QString s = lstActive->text(i);
    QPixmap p;
    if (lstActive->pixmap(i)) p = *lstActive->pixmap(i);
    lstActive->removeItem(i);
    lstActive->insertItem(p, s, i-1);
    lstActive->setCurrentItem(i-1);
    bDirty = true;
}

void ToolBarSetup::downClick()
{
    int i = lstActive->currentItem();
    if ((i < 0) || (i >= lstActive->count() - 1)) return;
    vector<unsigned long> b;
    list<unsigned long>::iterator it;
    for (it = active.begin(); it != active.end(); ++it)
        b.push_back(*it);
    unsigned long r = b[i];
    b[i] = b[i+1];
    b[i+1] = r;
    active.clear();
    for (int n = 0; n < b.size(); n++)
        active.push_back(b[n]);
    QString s = lstActive->text(i);
    QPixmap p;
    if (lstActive->pixmap(i)) p = *lstActive->pixmap(i);
    lstActive->removeItem(i);
    lstActive->insertItem(p, s, i+1);
    lstActive->setCurrentItem(i+1);
    bDirty = true;
}

void ToolBarSetup::show(const ToolBarDef *def, list<unsigned long> *active)
{
    QWidgetList  *list = QApplication::topLevelWidgets();
    QWidgetListIt it( *list );
    QWidget * w;
    ToolBarSetup *wnd = NULL;
    while ( (w=it.current()) != 0 ){
        ++it;
        if (!w->inherits("ToolBarSetup")) continue;
        ToolBarSetup *swnd = static_cast<ToolBarSetup*>(w);
        if (swnd->def != def) continue;
        wnd = swnd;
        break;
    }
    if (wnd == NULL) wnd= new ToolBarSetup(def, active);
    ((QWidget*)wnd)->show();
    wnd->showNormal();
#ifdef USE_KDE
    KWin::setOnDesktop(wnd->winId(), KWin::currentDesktop());
#endif
    wnd->setActiveWindow();
    wnd->raise();
#ifdef USE_KDE
    KWin::setActiveWindow(wnd->winId());
#endif
    delete list;
}

#ifndef _WINDOWS
#include "toolsetup.moc"
#endif

