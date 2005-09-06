/***************************************************************************
                          osdiface.cpp  -  description
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

#include "osdiface.h"
#include "osd.h"
#include "qcolorbutton.h"
#include "fontedit.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qlabel.h>

OSDIface::OSDIface(QWidget *parent, void *d, OSDPlugin *plugin)
        : OSDIfaceBase(parent)
{
    m_plugin = plugin;
    OSDUserData *data = (OSDUserData*)d;
    cmbPos->insertItem(i18n("Left-bottom"));
    cmbPos->insertItem(i18n("Left-top"));
    cmbPos->insertItem(i18n("Right-bottom"));
    cmbPos->insertItem(i18n("Right-top"));
    cmbPos->insertItem(i18n("Center-bottom"));
    cmbPos->insertItem(i18n("Center-top"));
    cmbPos->insertItem(i18n("Center"));
    cmbPos->setCurrentItem(data->Position.value);
    spnOffs->setMinValue(0);
    spnOffs->setMaxValue(500);
    spnOffs->setValue(data->Offset.value);
    spnTimeout->setMinValue(1);
    spnTimeout->setMaxValue(60);
    spnTimeout->setValue(data->Timeout.value);
    btnColor->setColor(data->Color.value);
    edtFont->setFont(data->Font.ptr);
    chkShadow->setChecked(data->Shadow.bValue);
    if (data->Background.bValue){
        chkBackground->setChecked(true);
        btnBgColor->setColor(data->BgColor.value);
    }else{
        chkBackground->setChecked(false);
    }
    bgToggled(data->Background.bValue);
    connect(chkBackground, SIGNAL(toggled(bool)), this, SLOT(bgToggled(bool)));
    unsigned nScreens = screens();
    if (nScreens <= 1){
        lblScreen->hide();
        cmbScreen->hide();
    }else{
        for (unsigned i = 0; i < nScreens; i++)
            cmbScreen->insertItem(QString::number(i));
        unsigned curScreen = data->Screen.value;
        if (curScreen >= nScreens)
            curScreen = 0;
        cmbScreen->setCurrentItem(curScreen);
    }
}

void OSDIface::bgToggled(bool bState)
{
    if (bState){
        btnBgColor->setEnabled(true);
        return;
    }
    btnBgColor->setColor(colorGroup().base());
    btnBgColor->setEnabled(false);
}

void OSDIface::apply(void *d)
{
    OSDUserData *data = (OSDUserData*)d;
    data->Position.value = cmbPos->currentItem();
    data->Offset.value = atol(spnOffs->text().latin1());
    data->Timeout.value = atol(spnTimeout->text().latin1());
    data->Color.value = btnColor->color().rgb();
    string f = edtFont->getFont();
    string base = FontEdit::font2str(font(), false).latin1();
    if (f == base)
        f = "";
    set_str(&data->Font.ptr, f.c_str());
    data->Shadow.bValue = chkShadow->isChecked();
    data->Background.bValue = chkBackground->isChecked();
    if (data->Background.bValue){
        data->BgColor.value = btnBgColor->color().rgb();
    }else{
        data->BgColor.value = 0;
    }
    unsigned nScreens = screens();
    if (nScreens <= 1){
        data->Screen.value = 0;
    }else{
        data->Screen.value = cmbScreen->currentItem();
    }
}

#ifndef WIN32
#include "osdiface.moc"
#endif

