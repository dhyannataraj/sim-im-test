/***************************************************************************
                          historycfg.cpp  -  description
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

#include "simapi.h"
#include "historycfg.h"
#include "core.h"

#include <qcheckbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qlabel.h>

HistoryConfig::HistoryConfig(QWidget *parent)
        : HistoryConfigBase(parent)
{
    chkOwn->setChecked(CorePlugin::m_plugin->getOwnColors());
    chkSmile->setChecked(CorePlugin::m_plugin->getUseSmiles());
    btnSend->setColor(QColor(CorePlugin::m_plugin->getColorSender()));
    btnReceive->setColor(QColor(CorePlugin::m_plugin->getColorReceiver()));
    cmbPage->setEditable(true);
    cmbPage->insertItem("100");
    cmbPage->insertItem("50");
    cmbPage->insertItem("25");
    QLineEdit *edit = cmbPage->lineEdit();
    edit->setValidator(new QIntValidator(1, 500, edit));
    edit->setText(QString::number(CorePlugin::m_plugin->getHistoryPage()));
    QString str1 = i18n("Show %1 messages per page");
    QString str2;
    int n = str1.find("%1");
    if (n >= 0){
        str2 = str1.mid(n + 2);
        str1 = str1.left(n);
    }
    lblPage1->setText(str1);
    lblPage2->setText(str2);
}

HistoryConfig::~HistoryConfig()
{
}

void HistoryConfig::apply()
{
    bool bChanged = false;
    if (chkOwn->isChecked() != CorePlugin::m_plugin->getOwnColors()){
        bChanged = true;
        CorePlugin::m_plugin->setOwnColors(chkOwn->isChecked());
    }
    if (chkSmile->isChecked() != CorePlugin::m_plugin->getUseSmiles()){
        bChanged = true;
        CorePlugin::m_plugin->setUseSmiles(chkSmile->isChecked());
    }
    if (btnSend->color().rgb() != CorePlugin::m_plugin->getColorSender()){
        bChanged = true;
        CorePlugin::m_plugin->setColorSender(btnSend->color().rgb());
    }
    if (btnReceive->color().rgb() != CorePlugin::m_plugin->getColorReceiver()){
        bChanged = true;
        CorePlugin::m_plugin->setColorReceiver(btnReceive->color().rgb());
    }
    CorePlugin::m_plugin->setHistoryPage(atol(cmbPage->lineEdit()->text().latin1()));
    if (bChanged){
        Event e(EventHistoryConfig);
        e.process();
    }
}

#ifndef WIN32
#include "historycfg.moc"
#endif

