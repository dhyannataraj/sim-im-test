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
    cmbPos->setCurrentItem(data->Position);
    spnOffs->setMinValue(0);
    spnOffs->setMaxValue(500);
    spnOffs->setValue(data->Offset);
    spnTimeout->setMinValue(1);
    spnTimeout->setMaxValue(60);
    spnTimeout->setValue(data->Timeout);
    btnColor->setColor(data->Color);
    edtFont->setFont(data->Font);
    chkShadow->setChecked(data->Shadow != 0);
    if (data->Background){
        chkBackground->setChecked(true);
        btnBgColor->setColor(data->BgColor);
    }else{
        chkBackground->setChecked(false);
    }
    bgToggled(data->Background != 0);
    connect(chkBackground, SIGNAL(toggled(bool)), this, SLOT(bgToggled(bool)));
    unsigned nScreens = screens();
    if (nScreens <= 1){
        lblScreen->hide();
        cmbScreen->hide();
    }else{
        for (unsigned i = 0; i < nScreens; i++)
            cmbScreen->insertItem(QString::number(i));
        unsigned curScreen = data->Screen;
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
    data->Position = cmbPos->currentItem();
    data->Offset = atol(spnOffs->text().latin1());
    data->Timeout = atol(spnTimeout->text().latin1());
    data->Color = btnColor->color().rgb();
    set_str(&data->Font, edtFont->getFont().c_str());
    data->Shadow = chkShadow->isChecked();
    data->Background = chkBackground->isChecked();
    if (data->Background){
        data->BgColor = btnBgColor->color().rgb();
    }else{
        data->BgColor = 0;
    }
    unsigned nScreens = screens();
    if (nScreens <= 1){
        data->Screen = 0;
    }else{
        data->Screen = cmbScreen->currentItem();
    }
}

#ifndef WIN32
#include "osdiface.moc"
#endif

