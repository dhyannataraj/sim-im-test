/***************************************************************************
                          sounduser.cpp  -  description
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

#include "sounduser.h"
#include "sound.h"
#include "editfile.h"
#include "core.h"

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>

SoundUserConfig::SoundUserConfig(QWidget *parent, void *data, SoundPlugin *plugin)
        : SoundUserConfigBase(parent)
{
    m_plugin = plugin;
    SoundUserData *user_data = (SoundUserData*)data;
    string s;
    s = plugin->fullName(user_data->Alert);
    edtAlert->setText(QFile::decodeName(s.c_str()));
    QGridLayout *lay = static_cast<QGridLayout*>(layout());
    if (lay == NULL)
        return;
    unsigned n = 1;
    CommandDef *cmd;
    CommandsMapIterator it(m_plugin->core->messageTypes);
    while ((cmd = ++it) != NULL){
        MessageDef *def = (MessageDef*)(cmd->param);
        if ((def == NULL) || (def->base_type) || (cmd->icon == NULL) || (def->flags & MESSAGE_HIDDEN))
            continue;
        QLabel *lbl = new QLabel(this);
        lbl->setAlignment(AlignRight);
        lbl->setText(i18n(def->singular, def->plural, 1) + ":");
        lay->addWidget(lbl, n, 0);
        EditSound *snd = new EditSound(this);
        snd->setText(QFile::decodeName(m_plugin->messageSound(cmd->id, user_data).c_str()));
        lay->addWidget(snd, n, 1);
        m_sounds.insert(MAP_SOUND::value_type(cmd->id, snd));
        n++;
    }
    lay->addItem(new QSpacerItem(n, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));
    chkActive->setChecked(user_data->NoSoundIfActive);
}

void SoundUserConfig::apply(void *data)
{
    SoundUserData *user_data = (SoundUserData*)data;
    set_str(&user_data->Alert, QFile::encodeName(edtAlert->text()));
    CommandDef *cmd;
    CommandsMapIterator it(m_plugin->core->messageTypes);
    while ((cmd = ++it) != NULL){
        MessageDef *def = (MessageDef*)(cmd->param);
        if (def->base_type)
            continue;
        MAP_SOUND::iterator it = m_sounds.find(cmd->id);
        if (it == m_sounds.end())
            continue;
        EditSound *snd = (*it).second;
        QString text = snd->text();
        if (text.isEmpty())
            text = "-";
        set_str(&user_data->Receive, cmd->id, QFile::encodeName(text));
    }
    user_data->NoSoundIfActive = chkActive->isChecked();
}

#ifndef WIN32
#include "sounduser.moc"
#endif

