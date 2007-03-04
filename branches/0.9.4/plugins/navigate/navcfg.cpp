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

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlabel.h>

NavCfg::NavCfg(QWidget *parent, NavigatePlugin *plugin)
        : NavCfgBase(parent)
{
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
#ifdef USE_KDE
    connect(chkKDE, SIGNAL(toggled(bool)), SLOT(useKDEtoggled(bool)));
    chkKDE->setChecked(plugin->getUseKDE());
#else
    chkKDE->hide();
#endif
}

void NavCfg::apply()
{
#ifdef WIN32
    m_plugin->setNewWindow(chkNew->isChecked());
#else
    m_plugin->setBrowser(edtBrowser->text().local8Bit());
    m_plugin->setMailer(edtMailer->text().local8Bit());
#endif
#ifdef USE_KDE
    m_plugin->setUseKDE(chkKDE->isChecked());
#endif
}

void NavCfg::useKDEtoggled(bool on)
{
#ifdef USE_KDE
    edtBrowser->setEnabled(!on);
    edtMailer->setEnabled(!on);
    lblBrowser->setEnabled(!on);
    lblMailer->setEnabled(!on);
#endif
}

#ifndef NO_MOC_INCLUDES
#include "navcfg.moc"
#endif

