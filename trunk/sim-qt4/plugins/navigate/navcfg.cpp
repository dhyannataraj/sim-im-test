/***************************************************************************
                          navcfg.cpp  -  description
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

#include "navcfg.h"
#include "navigate.h"

#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>

NavCfg::NavCfg(QWidget *parent, NavigatePlugin *plugin)
        : QWidget( parent)
{
    setupUi( this);
    m_plugin = plugin;
#ifdef WIN32
    chkNew->setChecked(plugin->getNewWindow());
    edtBrowser->hide();
    edtMailer->hide();
    lblBrowser->hide();
    lblMailer->hide();
#else
    edtBrowser->setText(QString::fromLocal8Bit(plugin->getBrowser()));
    edtMailer->setText(QString::fromLocal8Bit(plugin->getMailer()));
    chkNew->hide();
#endif
}

void NavCfg::apply()
{
#ifdef WIN32
    m_plugin->setNewWindow(chkNew->isChecked());
#else
    m_plugin->setBrowser(edtBrowser->text().toLocal8Bit());
    m_plugin->setMailer(edtMailer->text().toLocal8Bit());
#endif
}

#ifndef WIN32
#include "navcfg.moc"
#endif

