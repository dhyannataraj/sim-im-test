/***************************************************************************
                          toolsetup.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#include "toolsetup.h"
#include "mainwin.h"
#include "icons.h"
#include "log.h"
#include "enable.h"

#include <qapplication.h>
#include <qwidgetlist.h>
#include <qlistbox.h>
#include <qregexp.h>

#ifdef USE_KDE
#include <kwin.h>
#endif

#include <vector>
using namespace std;

ToolBarSetup::ToolBarSetup(const ToolBarDef *_def, unsigned long **_active)
        : ToolBarSetupBase(NULL, "toolbar_setup", false, WDestructiveClose)
{
    setButtonsPict(this);
    def = _def;
    m_active = _active;
    const ToolBarDef *d;
    unsigned i, n;
    unsigned long *p;

    if (*m_active){
        n = **m_active;
        active = (unsigned long*)malloc((n + 1) * sizeof(unsigned long));
        memmove(active, *m_active, (n + 1) * sizeof(unsigned long));
    }else{
        n = 0;
        for (d = def; d->id != BTN_END_DEF; d++)
            n++;
        active = (unsigned long*)malloc((n + 1) * sizeof(unsigned long));
        p = active;
        *(p++) = n;
        for (d = def; d->id != BTN_END_DEF; d++)
            *(p++) = d->id;
    }

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

    p = active;
    n = *(p++);

    for (i = 0; i < n; i++, p++){
        unsigned long id = *p;
        if (id == BTN_SEPARATOR){
            lstActive->insertItem(Pict("separator"), i18n("Separator"));
            continue;
        }
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

ToolBarSetup::~ToolBarSetup()
{
    if (active) free(active);
}

void ToolBarSetup::okClick()
{
    applyClick();
    close();
}

void ToolBarSetup::applyClick()
{
    if (bDirty){
        const ToolBarDef *d;
        unsigned long n = *active;
        unsigned long *n_active = (unsigned long*)malloc((n + 1) * sizeof(unsigned long));
        memcpy(n_active, active, (n + 1) * sizeof(unsigned long));
        n = 0;
        for (d = def; d->id != BTN_END_DEF; d++)
            n++;
        if (n == *active){
            unsigned long *d_active = (unsigned long*)malloc((n + 1) * sizeof(unsigned long));
            unsigned long *p = d_active;
            *(p++) = n;
            for (d = def; d->id != BTN_END_DEF; d++)
                *(p++) = d->id;
            if (!memcpy(d_active, n_active, (n + 1) * sizeof(unsigned long))){
                free(n_active);
                n_active = NULL;
            }
            free(d_active);
        }
        if (*m_active) free(*m_active);
        *m_active = n_active;
        pMain->changeToolBar(def);
        bDirty = false;
    }
}

void ToolBarSetup::addButton(QListBox *lst, const ToolBarDef *d)
{
    QString icon = d->icon;
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
                        (lstActive->currentItem() < (int)(lstActive->count() - 1)));
}

void ToolBarSetup::setButtons()
{
    lstButtons->clear();
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
        unsigned long *p = active;
        unsigned n = *(p++);
        unsigned i;
        for (i = 0; i < n; i++, p++)
            if (*p == d->id) break;
        if (i < n) continue;
        addButton(lstButtons, d);
    }
    lstButtons->insertItem(Pict("separator"), i18n("Separator"));
}

void ToolBarSetup::addClick()
{
    int i = lstButtons->currentItem();
    if (i < 0) return;
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
        unsigned long *p = active;
        unsigned long n = *(p++);
        unsigned long j;
        for (j = 0; j < n; j++, p++)
            if (*p == d->id) break;
        if (j < n) continue;
        if (i-- == 0) break;
    }
    unsigned long n = *active;
    n++;
    unsigned long *n_active = (unsigned long*)malloc((n + 1) * sizeof(unsigned long));
    memcpy(n_active, active, n * sizeof(unsigned long));
    *n_active = n;
    free(active);
    active = n_active;

    if (d->id == BTN_END_DEF){
        active[n] = BTN_SEPARATOR;
        lstActive->insertItem(Pict("separator"), i18n("Separator"));
    }else{
        active[n] = d->id;
        addButton(lstActive, d);
        setButtons();
    }
    bDirty = true;
}

void ToolBarSetup::removeClick()
{
    int i = lstActive->currentItem();
    if (i < 0) return;
    (*active)--;
    unsigned n = *active;
    unsigned long *p = active + (i + 1);
    if (i < (int)n)
        memmove(p, p + 1, (n - i) * sizeof(unsigned long));
    lstActive->removeItem(i);
    setButtons();
    bDirty = true;
}

void ToolBarSetup::upClick()
{
    int i = lstActive->currentItem();
    if (i <= 0) return;

    unsigned long *pp = active + (i + 1);
    unsigned long r = *pp;
    *pp = pp[-1];
    pp[-1] = r;

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
    if ((i < 0) || (i >= (int)(lstActive->count() - 1))) return;

    unsigned long *pp = active + (i + 1);
    unsigned long r = *pp;
    *pp = pp[1];
    pp[1] = r;

    QString s = lstActive->text(i);
    QPixmap p;
    if (lstActive->pixmap(i)) p = *lstActive->pixmap(i);
    lstActive->removeItem(i);
    lstActive->insertItem(p, s, i+1);
    lstActive->setCurrentItem(i+1);
    bDirty = true;
}

void ToolBarSetup::show(const ToolBarDef *def, unsigned long **active)
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

