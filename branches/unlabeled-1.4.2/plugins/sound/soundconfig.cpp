/***************************************************************************
                          soundconfig.cpp  -  description
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

#include "soundconfig.h"
#include "sounduser.h"
#include "sound.h"
#include "editfile.h"

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qtabwidget.h>

SoundConfig::SoundConfig(QWidget *parent, SoundPlugin *plugin)
        : SoundConfigBase(parent)
{
    m_plugin = plugin;
    user_cfg = NULL;
#ifdef WIN32
    chkArts->hide();
    lblPlayer->hide();
    edtPlayer->hide();
#else
#ifdef USE_KDE
    connect(chkArts, SIGNAL(toggled(bool)), this, SLOT(artsToggled(bool)));
    chkArts->setChecked(plugin->getUseArts());
#else
chkArts->hide();
#endif
    edtPlayer->setText(QString::fromLocal8Bit(plugin->getPlayer()));
#endif
    string s;
    s = plugin->fullName(plugin->getStartUp());
    edtStartup->setText(QFile::decodeName(s.c_str()));
    s = plugin->fullName(plugin->getFileDone());
    edtFileDone->setText(QFile::decodeName(s.c_str()));
    s = plugin->fullName(plugin->getMessageSent());
    edtSent->setText(QFile::decodeName(s.c_str()));

    for (QObject *p = parent; p != NULL; p = p->parent()){
        if (!p->inherits("QTabWidget"))
            continue;
        QTabWidget *tab = static_cast<QTabWidget*>(p);
        void *data = getContacts()->getUserData(plugin->user_data_id);
        user_cfg = new SoundUserConfig(tab, data, plugin);
        tab->addTab(user_cfg, i18n("Events"));
        tab->adjustSize();
        break;
    }
}

SoundConfig::~SoundConfig()
{
    if (user_cfg)
        delete user_cfg;
}

void SoundConfig::apply()
{
    if (user_cfg){
        void *data = getContacts()->getUserData(m_plugin->user_data_id);
        user_cfg->apply(data);
    }
#ifndef WIN32
#ifdef USE_KDE
    m_plugin->setUseArts(chkArts->isChecked());
#endif
    m_plugin->setPlayer(edtPlayer->text().local8Bit());
#endif
    m_plugin->setStartUp(QFile::encodeName(sound(edtStartup->text(), "startup.wav")));
    m_plugin->setFileDone(QFile::encodeName(sound(edtFileDone->text(), "startup.wav")));
    m_plugin->setMessageSent(QFile::encodeName(sound(edtSent->text(), "startup.wav")));
}

QString SoundConfig::sound(QString text, const char *def)
{
    QString defFile = QFile::decodeName(m_plugin->fullName(def).c_str());
    if (defFile == text)
        text = QFile::decodeName(def);
    return text;
}

void SoundConfig::artsToggled(bool)
{
    lblPlayer->setEnabled(!chkArts->isChecked());
    edtPlayer->setEnabled(!chkArts->isChecked());
}

#ifndef WIN32
#include "soundconfig.moc"
#endif

