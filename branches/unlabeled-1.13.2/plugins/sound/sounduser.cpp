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
#include "listview.h"
#include "editfile.h"
#include "core.h"

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qregexp.h>
#include <qpainter.h>

unsigned ONLINE_ALERT = 0x10000;

SoundUserConfig::SoundUserConfig(QWidget *parent, void *data, SoundPlugin *plugin)
        : SoundUserConfigBase(parent)
{
    m_plugin = plugin;
    lstSound->addColumn(i18n("Sound"));
    lstSound->addColumn(i18n("File"));
    lstSound->setExpandingColumn(1);

    SoundUserData *user_data = (SoundUserData*)data;
    string s;
    s = plugin->fullName(user_data->Alert.ptr);
    QListViewItem *item = new QListViewItem(lstSound, i18n("Online alert"), QFile::decodeName(s.c_str()));
    item->setText(2, QString::number(ONLINE_ALERT));
    item->setPixmap(0, makePixmap("licq"));

    CommandDef *cmd;
    CommandsMapIterator it(m_plugin->core->messageTypes);
    while ((cmd = ++it) != NULL){
        MessageDef *def = (MessageDef*)(cmd->param);
        if ((def == NULL) || (cmd->icon == NULL) ||
                (def->flags & (MESSAGE_HIDDEN | MESSAGE_SENDONLY)))
            continue;
        if ((def->singular == NULL) || (def->plural == NULL) ||
                (*def->singular == 0) || (*def->plural == 0))
            continue;
        QString type = i18n(def->singular, def->plural, 1);
        int pos = type.find("1 ");
        if (pos == 0){
            type = type.mid(2);
        }else if (pos > 0){
            type = type.left(pos);
        }
        type = type.left(1).upper() + type.mid(1);
        item = new QListViewItem(lstSound, type,
                                 QFile::decodeName(m_plugin->messageSound(cmd->id, user_data).c_str()));
        item->setText(2, QString::number(cmd->id));
        item->setPixmap(0, makePixmap(cmd->icon));
    }
    lstSound->adjustColumn();
    chkActive->setChecked(user_data->NoSoundIfActive.bValue);
    chkDisable->setChecked(user_data->Disable.bValue);
    connect(chkDisable, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
    toggled(user_data->Disable.bValue);
    m_edit = NULL;
    m_editItem = NULL;
    connect(lstSound, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(selectionChanged(QListViewItem*)));
}

QPixmap SoundUserConfig::makePixmap(const char *src)
{
    const QPixmap &source = Pict(src);
    int w = source.width();
    int h = QMAX(source.height(), 22);
    QPixmap pict(w, h);
    QPainter p(&pict);
    p.eraseRect(0, 0, w, h);
    p.drawPixmap(0, (h - source.height()) / 2, source);
    p.end();
    return pict;
}

void SoundUserConfig::apply(void *data)
{
    SoundUserData *user_data = (SoundUserData*)data;
    for (QListViewItem *item = lstSound->firstChild(); item; item = item->nextSibling()){
        unsigned id = item->text(2).toUInt();
        QString text = item->text(1);
        if (text.isEmpty())
            text = "-";
        if (id == ONLINE_ALERT){
            set_str(&user_data->Alert.ptr, QFile::encodeName(text));
        }else{
            set_str(&user_data->Receive, id, QFile::encodeName(text));
        }
    }
    user_data->NoSoundIfActive.bValue = chkActive->isChecked();
    user_data->Disable.bValue = chkDisable->isChecked();
    Event e(m_plugin->EventSoundChanged);
    e.process();
}

void SoundUserConfig::resizeEvent(QResizeEvent *e)
{
    SoundUserConfigBase::resizeEvent(e);
    lstSound->adjustColumn();
}

void SoundUserConfig::toggled(bool bState)
{
    lstSound->setEnabled(!bState);
}

void SoundUserConfig::selectionChanged(QListViewItem *item)
{
    if (m_editItem){
        m_editItem->setText(1, m_edit->text());
        delete m_edit;
        m_editItem = NULL;
        m_edit     = NULL;
    }
    if (item == NULL)
        return;
    m_editItem = item;
    m_edit = new EditSound(lstSound->viewport());
    QRect rc = lstSound->itemRect(m_editItem);
    rc.setLeft(rc.left() + lstSound->columnWidth(0) + 2);
    m_edit->setGeometry(rc);
    m_edit->setText(m_editItem->text(1));
    m_edit->show();
    m_edit->setFocus();
}

#ifndef WIN32
#include "sounduser.moc"
#endif

