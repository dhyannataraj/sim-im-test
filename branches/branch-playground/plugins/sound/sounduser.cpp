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
#include "editfile.h"
#include "core.h"
#include "log.h"

#include <QCheckBox>
#include <QFile>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>
#include <QTableWidget>
#include <QTableWidgetItem>

using namespace SIM;

unsigned ONLINE_ALERT = 0x10000;

static void addRow(QTableWidget *lstSound, int row, const QIcon &icon, const QString &text,
                   quint64 id, const QString &sound)
{
    QTableWidgetItem *item;
    lstSound->setRowCount(row+1);

    item = new QTableWidgetItem(icon, text);
    item->setData(Qt::UserRole, id);
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    lstSound->setItem(row, 0, item);

    item = new QTableWidgetItem(sound);
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsEditable);
    lstSound->setItem(row, 1, item);
}

SoundUserConfig::SoundUserConfig(QWidget *parent, QVariantMap* data, SoundPlugin *plugin)
  : QWidget(parent)
  , m_plugin(plugin)
{
    setupUi(this);

	setProperty("override", data->value("sound/override").toBool());

    int row = 0;
    addRow(lstSound, row, Icon("SIM"), i18n("Online alert"), ONLINE_ALERT, data->value("sound/Alert").toString());

    CommandDef *cmd;
    CommandsMapIterator it(m_plugin->m_core->messageTypes);
    while((cmd = ++it) != NULL)
    {
        MessageDef *def = (MessageDef*)(cmd->param);
        if ((def == NULL) || (cmd->icon.isEmpty()) ||
                (def->flags & (MESSAGE_HIDDEN | MESSAGE_SENDONLY | MESSAGE_CHILD)))
		{
            continue;
		}
        if ((def->singular == NULL) || (def->plural == NULL) ||
                (*def->singular == 0) || (*def->plural == 0))
		{
            continue;
		}
        QString type = i18n(def->singular, def->plural, 1);
        int pos = type.indexOf("1 ");
        if (pos == 0){
            type = type.mid(2);
        }else if (pos > 0){
            type = type.left(pos);
        }
        type = type.left(1).toUpper() + type.mid(1);

        row++;
        addRow(lstSound, row, Icon(cmd->icon), type, cmd->id, m_plugin->messageSound(cmd->id, data->value("id").toUInt()));
    }
    chkActive->setChecked(data->value("sound/NoSoundIfActive").toBool());
    chkDisable->setChecked(data->value("sound/Disable").toBool());
    connect(chkDisable, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
    toggled(data->value("sound/Disable").toBool());
    lstSound->resizeColumnsToContents();
    lstSound->setItemDelegate(new EditSoundDelegate(1, lstSound));
    lstSound->sortByColumn(0, Qt::AscendingOrder);
}

void SoundUserConfig::apply(QVariantMap* data, bool override)
{
    for(int row = 0; row < lstSound->rowCount(); ++row)
    {
        quint64 id = lstSound->item(row, 0)->data(Qt::UserRole).toULongLong();
        QString text = lstSound->item(row, 1)->data(Qt::EditRole).toString();
        if (text.isEmpty())
            text = "(nosound)";
        if (id == ONLINE_ALERT)
        {
            data->insert("sound/Alert", text);
        }
        else
        {
            data->insert("sound/Receive" + QString::number(id), text);
        }
    }
    data->insert("sound/NoSoundIfActive", chkActive->isChecked());
    data->insert("sound/Disable", chkDisable->isChecked());
	data->insert("sound/override", override);
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

