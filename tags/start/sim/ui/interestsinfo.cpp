/***************************************************************************
                          interestsinfo.cpp  -  description
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

#include "interestsinfo.h"
#include "client.h"
#include "icons.h"
#include "enable.h"

#include <qlabel.h>
#include <qpixmap.h>
#include <qlineedit.h>
#include <qcombobox.h>

InterestsInfo::InterestsInfo(QWidget *p, bool readOnly)
        : InterestsInfoBase(p)
{
    lblPict->setPixmap(Pict("interest"));
    if (!readOnly){
        connect(cmbBg1, SIGNAL(activated(int)), this, SLOT(adjustEnabled(int)));
        connect(cmbBg2, SIGNAL(activated(int)), this, SLOT(adjustEnabled(int)));
        connect(cmbBg3, SIGNAL(activated(int)), this, SLOT(adjustEnabled(int)));
        load(pClient);
        return;
    }
    edtBg1->setReadOnly(true);
    edtBg2->setReadOnly(true);
    edtBg3->setReadOnly(true);
    edtBg4->setReadOnly(true);
    disableWidget(cmbBg1);
    disableWidget(cmbBg2);
    disableWidget(cmbBg3);
    disableWidget(cmbBg4);
}

void InterestsInfo::load(ICQUser *u)
{
    unsigned i = 0;
    for (ExtInfoList::iterator it = u->Interests.begin(); it != u->Interests.end(); ++it){
        ExtInfo *info = static_cast<ExtInfo*>(*it);
        QString bg = QString::fromLocal8Bit(info->Specific.c_str());
        switch (i){
        case 0:
            edtBg1->setText(bg);
            initCombo(cmbBg1, info->Category, interests);
            break;
        case 1:
            edtBg2->setText(bg);
            initCombo(cmbBg2, info->Category, interests);
            break;
        case 2:
            edtBg3->setText(bg);
            initCombo(cmbBg3, info->Category, interests);
            break;
        case 3:
            edtBg4->setText(bg);
            initCombo(cmbBg4, info->Category, interests);
            break;
        }
        i++;
    }
    for (; i < 4; i++){
        switch (i){
        case 0:
            initCombo(cmbBg1, 0, interests);
            break;
        case 1:
            initCombo(cmbBg2, 0, interests);
            break;
        case 2:
            initCombo(cmbBg3, 0, interests);
            break;
        case 3:
            initCombo(cmbBg4, 0, interests);
            break;
        }
    }
    adjustEnabled(0);
}

void InterestsInfo::adjustEnabled(int)
{
    if (!cmbBg1->isEnabled()) return;
    cmbBg2->setEnabled(cmbBg1->currentItem() != 0);
    edtBg2->setEnabled(cmbBg1->currentItem() != 0);
    cmbBg3->setEnabled(cmbBg2->currentItem() != 0);
    edtBg3->setEnabled(cmbBg2->currentItem() != 0);
    cmbBg4->setEnabled(cmbBg3->currentItem() != 0);
    edtBg4->setEnabled(cmbBg3->currentItem() != 0);
}

void InterestsInfo::save(ICQUser*)
{
}

void InterestsInfo::apply(ICQUser *u)
{
    unsigned short category;
    ExtInfoPtrList interestsList;
    category = getComboValue(cmbBg1, interests);
    if (category){
        ExtInfo *info = new ExtInfo;
        info->Category = category;
        info->Specific = edtBg1->text().local8Bit();
        interestsList.push_back(info);
    }
    category = getComboValue(cmbBg2, interests);
    if (category){
        ExtInfo *info = new ExtInfo;
        info->Category = category;
        info->Specific = edtBg2->text().local8Bit();
        interestsList.push_back(info);
    }
    category = getComboValue(cmbBg3, interests);
    if (category){
        ExtInfo *info = new ExtInfo;
        info->Category = category;
        info->Specific = edtBg3->text().local8Bit();
        interestsList.push_back(info);
    }
    category = getComboValue(cmbBg4, interests);
    if (category){
        ExtInfo *info = new ExtInfo;
        info->Category = category;
        info->Specific = edtBg4->text().local8Bit();
        interestsList.push_back(info);
    }
    u->Interests = interestsList;
}

#ifndef _WINDOWS
#include "interestsinfo.moc"
#endif

