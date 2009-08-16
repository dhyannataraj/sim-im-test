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

#include "simapi.h"

#include <qcheckbox.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qsound.h>

#include "editfile.h"
#include "contacts.h"
#include "misc.h"

#include "soundconfig.h"
#include "sounduser.h"
#include "sound.h"

using SIM::getContacts;

SoundConfig::SoundConfig(QWidget *parent, SoundPlugin *plugin)
  : QWidget(parent)
  , m_plugin(plugin)
  , user_cfg(NULL)
{
    setupUi(this);

	/*
#if defined( WIN32 ) || defined( __OS2__ )
    lblPlayer->hide();
    edtPlayer->hide();
#else
    if (bSound){
        lblPlayer->setText(i18n("Qt provides sound output so you just need to set a player if you don't like Qt's sound."));
    }
    edtPlayer->setText(plugin->property("Player").toString());
#endif
*/
    edtStartup->setText(plugin->property("StartUp").toString());
    edtFileDone->setText(plugin->property("FileDone").toString());
    edtSent->setText(plugin->property("MessageSent").toString());

    for (QObject *p = parent; p != NULL; p = p->parent())
    {
        QTabWidget *tab = qobject_cast<QTabWidget*>(p);
        if (!tab)
            continue;

        user_cfg = new SoundUserConfig(tab, getContacts()->userdata(), plugin);
        tab->addTab(user_cfg, i18n("Events"));
        tab->adjustSize();
        break;
    }
}

SoundConfig::~SoundConfig()
{
}

void SoundConfig::apply()
{
    if(user_cfg)
	{
		QVariantMap* data = getContacts()->userdata();
		user_cfg->apply(data, true);
    }
	m_plugin->setProperty("StartUp", edtStartup->text());
    m_plugin->setProperty("FileDone", edtFileDone->text());
    m_plugin->setProperty("MessageSent", edtSent->text());
}

void SoundConfig::artsToggled(bool)
{
}

