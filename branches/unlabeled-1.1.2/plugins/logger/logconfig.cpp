/***************************************************************************
                          logconfig.cpp  -  description
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

#include "logconfig.h"
#include "logger.h"

#include <qcheckbox.h>
#include <qlayout.h>

LogConfig::LogConfig(QWidget *parent, LoggerPlugin *plugin)
        : LogConfigBase(parent)
{
    m_plugin = plugin;
    unsigned log_level = plugin->getLogLevel();
    chkError->setChecked((log_level & L_ERROR) != 0);
    chkWarn->setChecked((log_level & L_WARN) != 0);
    chkDebug->setChecked((log_level & L_DEBUG) != 0);
    fill();
}

void LogConfig::apply()
{
    unsigned log_level = 0;
    if (chkError->isChecked()) log_level |= L_ERROR;
    if (chkWarn->isChecked()) log_level |= L_WARN;
    if (chkDebug->isChecked()) log_level |= L_DEBUG;
    m_plugin->setLogLevel(log_level);
    for (BOX_MAP::iterator it = m_boxes.begin(); it != m_boxes.end(); ++it){
        m_plugin->setLogType((*it).first, (*it).second->isChecked());
    }
}

void LogConfig::fill()
{
    for (BOX_MAP::iterator it = m_boxes.begin(); it != m_boxes.end(); ++it)
        delete (*it).second;
    m_boxes.clear();
    QVBoxLayout *lay = static_cast<QVBoxLayout*>(layout());
    PacketType *packet;
    ContactList::PacketIterator it_packet;
    unsigned pos = 4;
    while ((packet = ++it_packet) != NULL){
        QCheckBox *box = new QCheckBox(i18n(packet->name()), this);
        lay->insertWidget(pos++, box);
        box->setChecked(m_plugin->isLogType(packet->id()));
        m_boxes.insert(BOX_MAP::value_type(packet->id(), box));
    }
}

void *LogConfig::processEvent(Event *e)
{
    if ((e->type() == EventPluginChanged) || (e->type() == EventLanguageChanged))
        fill();
    return NULL;
}

#ifndef WIN32
#include "logconfig.moc"
#endif

