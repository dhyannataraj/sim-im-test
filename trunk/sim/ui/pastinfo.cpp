/***************************************************************************
                          pastinfo.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "pastinfo.h"
#include "client.h"
#include "icons.h"
#include "enable.h"
#include "country.h"

#include <qlabel.h>
#include <qpixmap.h>
#include <qlineedit.h>
#include <qcombobox.h>

PastInfo::PastInfo(QWidget *p, bool readOnly)
        : PastInfoBase(p)
{
    if (!readOnly){
        connect(cmbBg1, SIGNAL(activated(int)), this, SLOT(adjustEnabled(int)));
        connect(cmbBg2, SIGNAL(activated(int)), this, SLOT(adjustEnabled(int)));
        connect(cmbAf1, SIGNAL(activated(int)), this, SLOT(adjustEnabled(int)));
        connect(cmbAf2, SIGNAL(activated(int)), this, SLOT(adjustEnabled(int)));
        load(pClient->owner);
        return;
    }
    edtBg1->setReadOnly(true);
    edtBg2->setReadOnly(true);
    edtBg3->setReadOnly(true);
    edtAf1->setReadOnly(true);
    edtAf2->setReadOnly(true);
    edtAf3->setReadOnly(true);
    disableWidget(cmbBg1);
    disableWidget(cmbBg2);
    disableWidget(cmbBg3);
    disableWidget(cmbAf1);
    disableWidget(cmbAf2);
    disableWidget(cmbAf3);
}

void PastInfo::load(ICQUser *u)
{
    ExtInfoList::iterator it;
    unsigned i = 0;
    for (it = u->Backgrounds.begin(); it != u->Backgrounds.end(); ++it){
        ExtInfo *info = static_cast<ExtInfo*>(*it);
        QString bg = QString::fromLocal8Bit(info->Specific.c_str());
        switch (i){
        case 0:
            edtBg1->setText(bg);
            initCombo(cmbBg1, info->Category, pasts);
            break;
        case 1:
            edtBg2->setText(bg);
            initCombo(cmbBg2, info->Category, pasts);
            break;
        case 2:
            edtBg3->setText(bg);
            initCombo(cmbBg3, info->Category, pasts);
            break;
        }
        i++;
    }
    for (; i < 3; i++){
        switch (i){
        case 0:
            initCombo(cmbBg1, 0, pasts);
            break;
        case 1:
            initCombo(cmbBg2, 0, pasts);
            break;
        case 2:
            initCombo(cmbBg3, 0, pasts);
            break;
        }
    }
    i = 0;
    for (it = u->Affilations.begin(); it != u->Affilations.end(); ++it){
        ExtInfo *info = static_cast<ExtInfo*>(*it);
        QString bg = QString::fromLocal8Bit(info->Specific.c_str());
        switch (i){
        case 0:
            edtAf1->setText(bg);
            initCombo(cmbAf1, info->Category, affilations);
            break;
        case 1:
            edtAf2->setText(bg);
            initCombo(cmbAf2, info->Category, affilations);
            break;
        case 2:
            edtAf3->setText(bg);
            initCombo(cmbAf3, info->Category, affilations);
            break;
        }
        i++;
    }
    for (; i < 3; i++){
        switch (i){
        case 0:
            initCombo(cmbAf1, 0, affilations);
            break;
        case 1:
            initCombo(cmbAf2, 0, affilations);
            break;
        case 2:
            initCombo(cmbAf3, 0, affilations);
            break;
        }
    }
    adjustEnabled(0);
}

void PastInfo::save(ICQUser*)
{
}

void PastInfo::apply(ICQUser *u)
{
    unsigned short category;
    ExtInfoList pastsList;
    ExtInfoList affilationsList;
    category = getComboValue(cmbBg1, pasts);
    if (category){
        ExtInfo *info = new ExtInfo;
        info->Category = category;
        set(info->Specific, edtBg1->text());
        pastsList.push_back(info);
    }
    category = getComboValue(cmbBg2, pasts);
    if (category){
        ExtInfo *info = new ExtInfo;
        info->Category = category;
        set(info->Specific, edtBg2->text());
        pastsList.push_back(info);
    }
    category = getComboValue(cmbBg3, pasts);
    if (category){
        ExtInfo *info = new ExtInfo;
        info->Category = category;
        set(info->Specific, edtBg3->text());
        pastsList.push_back(info);
    }
    category = getComboValue(cmbAf1, affilations);
    if (category){
        ExtInfo *info = new ExtInfo;
        info->Category = category;
        set(info->Specific, edtAf1->text());
        affilationsList.push_back(info);
    }
    category = getComboValue(cmbAf2, affilations);
    if (category){
        ExtInfo *info = new ExtInfo;
        info->Category = category;
        set(info->Specific, edtAf2->text());
        affilationsList.push_back(info);
    }
    category = getComboValue(cmbAf3, affilations);
    if (category){
        ExtInfo *info = new ExtInfo;
        info->Category = category;
        set(info->Specific, edtAf3->text().local8Bit());
        affilationsList.push_back(info);
    }
    u->Backgrounds = pastsList;
    u->Affilations = affilationsList;
}

void PastInfo::adjustEnabled(int)
{
    if (!cmbBg1->isEnabled()) return;
    cmbBg2->setEnabled(cmbBg1->currentItem() != 0);
    edtBg2->setEnabled(cmbBg1->currentItem() != 0);
    cmbBg3->setEnabled(cmbBg2->currentItem() != 0);
    edtBg3->setEnabled(cmbBg2->currentItem() != 0);
    cmbAf2->setEnabled(cmbAf1->currentItem() != 0);
    edtAf2->setEnabled(cmbAf1->currentItem() != 0);
    cmbAf3->setEnabled(cmbAf2->currentItem() != 0);
    edtAf3->setEnabled(cmbAf2->currentItem() != 0);
}

#ifndef _WINDOWS
#include "pastinfo.moc"
#endif

