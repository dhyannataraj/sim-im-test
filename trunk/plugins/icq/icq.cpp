/***************************************************************************
                          icq.cpp  -  description
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

#include "icq.h"
#include "icqconfig.h"
#include "core.h"

using namespace SIM;

Plugin *createICQPlugin(unsigned base, bool, Buffer*)
{
    Plugin *plugin = new ICQPlugin(base);
    return plugin;
}

static PluginInfo info =
    {
        NULL,
        NULL,
        VERSION,
        createICQPlugin,
        PLUGIN_PROTOCOL
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

ICQProtocol::ICQProtocol(Plugin *plugin)
        : Protocol(plugin)
{
}

ICQProtocol::~ICQProtocol()
{
}

Client *ICQProtocol::createClient(Buffer *cfg)
{
    return new ICQClient(this, cfg, false);
}

static CommandDef icq_descr =
    CommandDef (
        0,
        I18N_NOOP("ICQ"),
        "ICQ_online",
        "ICQ_invisible",
        "http://www.icq.com/password/",
        0,
        0,
        0,
        0,
        0,
        PROTOCOL_INFO | PROTOCOL_SEARCH | PROTOCOL_INVISIBLE | PROTOCOL_AR_USER | PROTOCOL_ANY_PORT | PROTOCOL_NODATA,
        NULL,
        QString::null
    );

const CommandDef *ICQProtocol::description()
{
    return &icq_descr;
}

static CommandDef icq_status_list[] =
    {
        CommandDef (
            STATUS_ONLINE,
            I18N_NOOP("Online"),
            "ICQ_online",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            STATUS_AWAY,
            I18N_NOOP("Away"),
            "ICQ_away",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            STATUS_NA,
            I18N_NOOP("N/A"),
            "ICQ_na",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            STATUS_DND,
            I18N_NOOP("Do not Disturb"),
            "ICQ_dnd",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            STATUS_OCCUPIED,
            I18N_NOOP("Occupied"),
            "ICQ_occupied",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            STATUS_FFC,
            I18N_NOOP("Free for chat"),
            "ICQ_ffc",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            STATUS_OFFLINE,
            I18N_NOOP("Offline"),
            "ICQ_offline",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef ()
    };

const CommandDef *ICQProtocol::statusList()
{
    return icq_status_list;
}

const CommandDef *ICQProtocol::_statusList()
{
    return icq_status_list;
}

AIMProtocol::AIMProtocol(Plugin *plugin)
        : Protocol(plugin)
{
}

AIMProtocol::~AIMProtocol()
{
}

Client *AIMProtocol::createClient(Buffer *cfg)
{
    return new ICQClient(this, cfg, true);
}

static CommandDef aim_descr =
    CommandDef (
        0,
        I18N_NOOP("AIM"),
        "AIM_online",
        QString::null,
        "http://www.aim.com/help_faq/forgot_password/password.adp",
        0,
        0,
        0,
        0,
        0,
        PROTOCOL_INFO | PROTOCOL_AR | PROTOCOL_ANY_PORT,
        NULL,
        QString::null
    );

const CommandDef *AIMProtocol::description()
{
    return &aim_descr;
}

static CommandDef aim_status_list[] =
    {
        CommandDef (
            STATUS_ONLINE,
            I18N_NOOP("Online"),
            "AIM_online",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            STATUS_AWAY,
            I18N_NOOP("Away"),
            "AIM_away",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            STATUS_OFFLINE,
            I18N_NOOP("Offline"),
            "AIM_offline",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef ()
    };

const CommandDef *AIMProtocol::statusList()
{
    return aim_status_list;
}

Protocol *ICQPlugin::m_icq = NULL;
Protocol *ICQPlugin::m_aim = NULL;

CorePlugin *ICQPlugin::core = NULL;

ICQPlugin *ICQPlugin::icq_plugin = NULL;

ICQPlugin::ICQPlugin(unsigned base)
        : Plugin(base)
{
    QString pluginName("_core");
    Event ePlugin(EventGetPluginInfo, &pluginName);
    pluginInfo *info = (pluginInfo*)(ePlugin.process());
    core = static_cast<CorePlugin*>(info->plugin);

    icq_plugin = this;

    OscarPacket = registerType();
    getContacts()->addPacketType(OscarPacket, "Oscar");
    ICQDirectPacket = registerType();
    getContacts()->addPacketType(ICQDirectPacket, "ICQ.Direct");
    AIMDirectPacket = registerType();
    getContacts()->addPacketType(AIMDirectPacket, "AIM.Direct");

    m_icq = new ICQProtocol(this);
    m_aim = new AIMProtocol(this);

    Event eMenuSearch(EventMenuCreate, (void*)MenuSearchResult);
    eMenuSearch.process();
    Event eMenuGroups(EventMenuCreate, (void*)MenuIcqGroups);
    eMenuGroups.process();
    /*
    Event eMenuCheckInvisible(EventMenuCreate, (void*)MenuCheckInvisible);
    eMenuCheckInvisible.process();
    */

    Command cmd;
    cmd->id          = CmdVisibleList;
    cmd->text        = I18N_NOOP("Visible list");
    cmd->menu_id     = MenuContactGroup;
    cmd->menu_grp    = 0x8010;
    cmd->flags		 = COMMAND_CHECK_STATE;

    Event eCmd(EventCommandCreate, cmd);
    eCmd.process();

    cmd->id			 = CmdInvisibleList;
    cmd->text		 = I18N_NOOP("Invisible list");
    cmd->menu_grp	 = 0x8011;
    eCmd.process();

    cmd->id			 = CmdIcqSendMessage;
    cmd->text		 = I18N_NOOP("&Message");
    cmd->icon		 = "message";
    cmd->menu_id	 = MenuSearchResult;
    cmd->menu_grp	 = 0x1000;
    cmd->bar_id		 = 0;
    cmd->popup_id	 = 0;
    cmd->flags		 = COMMAND_DEFAULT;
    eCmd.process();

    cmd->id			 = CmdInfo;
    cmd->text		 = I18N_NOOP("User &info");
    cmd->icon		 = "info";
    cmd->menu_grp	 = 0x1001;
    eCmd.process();

    cmd->id			 = CmdGroups;
    cmd->text		 = I18N_NOOP("&Add to group");
    cmd->icon		 = QString::null;
    cmd->menu_grp	 = 0x1002;
    cmd->popup_id	 = MenuIcqGroups;
    eCmd.process();

    cmd->id			 = CmdGroups;
    cmd->text		 = "_";
    cmd->menu_id	 = MenuIcqGroups;
    cmd->flags		 = COMMAND_CHECK_STATE;
    eCmd.process();

    registerMessages();

    RetrySendDND = registerType();
    RetrySendOccupied = registerType();
}

ICQPlugin::~ICQPlugin()
{
    unregisterMessages();

    delete m_icq;
    delete m_aim;

    getContacts()->removePacketType(OscarPacket);
    getContacts()->removePacketType(ICQDirectPacket);
    getContacts()->removePacketType(AIMDirectPacket);

    Event eVisible(EventCommandRemove, (void*)CmdVisibleList);
    eVisible.process();

    Event eInvisible(EventCommandRemove, (void*)CmdInvisibleList);
    eInvisible.process();

    Event eMenuSearch(EventMenuRemove, (void*)MenuSearchResult);
    eMenuSearch.process();

    Event eMenuGroups(EventMenuRemove, (void*)MenuIcqGroups);
    eMenuGroups.process();
}
