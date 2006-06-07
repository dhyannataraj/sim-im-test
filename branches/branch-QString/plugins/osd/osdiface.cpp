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

using namespace SIM;

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
    cmbPos->setCurrentItem(data->Position.asULong());
    spnOffs->setMinValue(0);
    spnOffs->setMaxValue(500);
    spnOffs->setValue(data->Offset.asULong());
    spnTimeout->setMinValue(1);
    spnTimeout->setMaxValue(60);
    spnTimeout->setValue(data->Timeout.asULong());
    btnColor->setColor(data->Color.asULong());
    edtFont->setFont(data->Font.str());
    chkShadow->setChecked(data->Shadow.asBool());
    if (data->Background.asBool()){
        chkBackground->setChecked(true);
        btnBgColor->setColor(data->BgColor.asULong());
    }else{
        chkBackground->setChecked(false);
    }
    bgToggled(data->Background.asBool());
    connect(chkBackground, SIGNAL(toggled(bool)), this, SLOT(bgToggled(bool)));
    unsigned nScreens = screens();
    if (nScreens <= 1){
        lblScreen->hide();
        cmbScreen->hide();
    }else{
        for (unsigned i = 0; i < nScreens; i++)
            cmbScreen->insertItem(QString::number(i));
        unsigned curScreen = data->Screen.asULong();
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
    data->Position.asULong() = cmbPos->currentItem();
    data->Offset.asULong() = atol(spnOffs->text().latin1());
    data->Timeout.asULong() = atol(spnTimeout->text().latin1());
    data->Color.asULong() = btnColor->color().rgb();
    QString f = edtFont->getFont();
    QString base = FontEdit::font2str(font(), false);
    if (f == base)
        f = "";
    data->Font.str() = f;
    data->Shadow.asBool() = chkShadow->isChecked();
    data->Background.asBool() = chkBackground->isChecked();
    if (data->Background.asBool()){
        data->BgColor.asULong() = btnBgColor->color().rgb();
    }else{
        data->BgColor.asULong() = 0;
    }
    unsigned nScreens = screens();
    if (nScreens <= 1){
        data->Screen.asULong() = 0;
    }else{
        data->Screen.asULong() = cmbScreen->currentItem();
    }
}

#ifndef _MSC_VER
#include "osdiface.moc"
#endif

