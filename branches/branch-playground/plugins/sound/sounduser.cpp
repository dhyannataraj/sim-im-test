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

#include "icons.h"
#include "sounduser.h"
#include "sound.h"
#include "listview.h"
#include "editfile.h"
#include "core.h"

#include <qcheckbox.h>
#include <qfile.h>
#include <qlabel.h>
#include <qpainter.h>
#include <QPixmap>
#include <QResizeEvent>
#include <QTableWidget>
#include <QTableWidgetItem>

using namespace SIM;

unsigned ONLINE_ALERT = 0x10000;

SoundUserConfig::SoundUserConfig(QWidget *parent, void *data, SoundPlugin *plugin)
  : QWidget(parent)
  , m_plugin(plugin)
{
    setupUi(this);

    SoundUserData *user_data = (SoundUserData*)data;
    QTableWidgetItem *item;
    EditSound *es;

    int row = 0;
    lstSound->setRowCount(row + 1);

    item = new QTableWidgetItem(Icon("SIM"), i18n("Online alert"));
    item->setData(Qt::UserRole, ONLINE_ALERT);
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    lstSound->setItem(row, 0, item);

    es = new EditSound(lstSound);
    es->setText(plugin->fullName(user_data->Alert.str()));
    lstSound->setCellWidget(row, 1, es);

    CommandDef *cmd;
    CommandsMapIterator it(m_plugin->core->messageTypes);
    while ((cmd = ++it) != NULL){
        MessageDef *def = (MessageDef*)(cmd->param);
        if ((def == NULL) || (cmd->icon.isEmpty()) ||
                (def->flags & (MESSAGE_HIDDEN | MESSAGE_SENDONLY | MESSAGE_CHILD)))
            continue;
        if ((def->singular == NULL) || (def->plural == NULL) ||
                (*def->singular == 0) || (*def->plural == 0))
            continue;
        QString type = i18n(def->singular, def->plural, 1);
        int pos = type.indexOf("1 ");
        if (pos == 0){
            type = type.mid(2);
        }else if (pos > 0){
            type = type.left(pos);
        }
        type = type.left(1).toUpper() + type.mid(1);

        row++;
        lstSound->setRowCount(row + 1);

        item = new QTableWidgetItem(Icon(cmd->icon), type);
        item->setData(Qt::UserRole, QVariant((quint64)cmd->id));
        item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        lstSound->setItem(row, 0, item);

        es = new EditSound(lstSound);
        es->setText(m_plugin->messageSound(cmd->id, user_data));
        lstSound->setCellWidget(row, 1, es);
    }
    chkActive->setChecked(user_data->NoSoundIfActive.toBool());
    chkDisable->setChecked(user_data->Disable.toBool());
    connect(chkDisable, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
    toggled(user_data->Disable.toBool());
    lstSound->resizeRowsToContents();
    lstSound->resizeColumnsToContents();
}

void SoundUserConfig::apply(void *data)
{
    SoundUserData *user_data = (SoundUserData*)data;
    for (int row = 0; row < lstSound->rowCount(); ++row) {
        QTableWidgetItem *item = lstSound->item(row, 0);
        quint64 id = item->data(Qt::UserRole).toULongLong();
        QString text = static_cast<EditSound*>(lstSound->cellWidget(row, 1))->text();
        if (text.isEmpty())
            text = "(nosound)";
        if (id == ONLINE_ALERT){
            user_data->Alert.str() = text;
        }else{
            set_str(&user_data->Receive, id, text);
        }
    }
    user_data->NoSoundIfActive.asBool() = chkActive->isChecked();
    user_data->Disable.asBool() = chkDisable->isChecked();
    Event e(m_plugin->EventSoundChanged);
    e.process();
}

void SoundUserConfig::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    lstSound->resizeRowsToContents();
    lstSound->resizeColumnsToContents();
}

void SoundUserConfig::toggled(bool bState)
{
    lstSound->setEnabled(!bState);
}
