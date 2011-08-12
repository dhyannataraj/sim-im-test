/***************************************************************************
                          status.cpp  -  description
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
#include "commonstatus.h"

#include <map>
#include <list>

#include <QTimer>
#include <QApplication>
#include <QWidget>

#include "core.h"
#include "events/eventhub.h"

#include "contacts/client.h"
#include "clientmanager.h"
#include "commands/commandhub.h"
#include "contacts/protocol.h"
#include "events/commonstatusevent.h"

using namespace SIM;

CommonStatus::CommonStatus(SIM::ClientManager* manager) : QObject(), m_clientManager(manager)
{
    rebuildStatusList();

    m_currentStatusId = "offline";

    getEventHub()->registerEvent(IEventPtr(new SIM::CommonStatusEvent()));

    getEventHub()->getEvent("init")->connectTo(this, SLOT(eventInit()));
}

CommonStatus::~CommonStatus()
{

}

void CommonStatus::rebuildStatusList()
{
    m_statuses.clear();
    m_statuses << CommonStatusDescription {"online", "Online", "SIM_online"};
    m_statuses << CommonStatusDescription {"away", "Away", "SIM_away"};
    m_statuses << CommonStatusDescription {"na", "N/A", "SIM_na"};
    m_statuses << CommonStatusDescription {"dnd", "Do not disturb", "SIM_dnd"};
    m_statuses << CommonStatusDescription {"offline", "Offline", "SIM_offline"};
}

void CommonStatus::statusOnline()
{
    setCommonStatus("online");
}

void CommonStatus::statusFreeForChat()
{
    setCommonStatus("free_for_chat");
}

void CommonStatus::statusAway()
{
    setCommonStatus("away");
}

void CommonStatus::statusNa()
{
    setCommonStatus("na");
}

void CommonStatus::statusDnd()
{
    setCommonStatus("dnd");
}

void CommonStatus::statusOffline()
{
    setCommonStatus("offline");
}

void CommonStatus::setCommonStatus(const QString& id)
{
    foreach(const CommonStatusDescription& desc, m_statuses)
    {
        if(desc.id == id)
        {
            m_currentStatusId = id;
            getEventHub()->triggerEvent("common_status", CommonStatusEventData::create("online"));
            return;
        }
    }
}

QList<CommonStatus::CommonStatusDescription> CommonStatus::allCommonStatuses() const
{
    return m_statuses;
}

CommonStatus::CommonStatusDescription CommonStatus::currentCommonStatus() const
{
    foreach(const CommonStatusDescription& desc, m_statuses)
    {
        if(desc.id == m_currentStatusId)
            return desc;
    }
    return CommonStatusDescription();
}

int CommonStatus::indexOfCommonStatus(const QString& id)
{
    int i = 0;
    foreach(const CommonStatusDescription& desc, m_statuses)
    {
        if(desc.id == m_currentStatusId)
            return i;
        i++;
    }
    return -1;
}

CommonStatus::CommonStatusDescription CommonStatus::commonStatusByIndex(int index) const
{
    foreach(const CommonStatusDescription& desc, m_statuses)
    {
        if(desc.id == m_currentStatusId)
            return desc;
    }
    return CommonStatusDescription();
}

void CommonStatus::eventInit()
{
}
