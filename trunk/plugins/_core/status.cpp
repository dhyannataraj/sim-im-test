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

#include "status.h"
#include "simapi.h"
#include "core.h"
#include "socket.h"
#include "ballonmsg.h"
#include "statuswnd.h"
#include "logindlg.h"
#include "autoreply.h"

#include <qtimer.h>
#include <qapplication.h>
#include <qwidgetlist.h>

#include <map>
using namespace std;

typedef map<unsigned, unsigned> MAP_STATUS;

CommonStatus::CommonStatus()
        : EventReceiver(LowPriority + 2)
{
    m_bBlink = false;
    m_timer  = NULL;

    Event eMenu(EventMenuCreate, (void*)MenuStatus);
    eMenu.process();

    Command cmd;
    cmd->id          = CmdStatusMenu;
    cmd->text        = I18N_NOOP("Status");
    cmd->menu_id     = MenuMain;
    cmd->menu_grp    = 0x6000;
    cmd->popup_id    = MenuStatus;
    cmd->flags		 = COMMAND_IMPORTANT;

    Event eCmd(EventCommandCreate, cmd);
    eCmd.process();

    setBarStatus(true);
    rebuildStatus();
}

CommonStatus::~CommonStatus()
{
    Event eGroup(EventCommandRemove, (void*)CmdStatusBar);
    eGroup.process();
    Event eMenuRemove(EventMenuRemove, (void*)MenuStatus);
    eMenuRemove.process();
}

void CommonStatus::setBarStatus(bool bFirst)
{
    const char *text = I18N_NOOP("Inactive");
    const char *icon = "inactive";

    m_bConnected = false;
    bool bActive = getSocketFactory()->isActive();
    if (!bActive){
        for (unsigned i = 0; i < getContacts()->nClients(); i++){
            Client *client = getContacts()->getClient(i);
            if (client->getState() == Client::Connected){
                bActive = true;
                break;
            }
        }
    }

    if (bActive){
        m_bConnected = false;
        for (unsigned i = 0; i < getContacts()->nClients(); i++){
            Client *client = getContacts()->getClient(i);
            if (!client->getCommonStatus())
                continue;
            if (client->getState() == Client::Connecting){
                m_bConnected = true;
                break;
            }
        }
        if (m_bConnected){
            text = I18N_NOOP("Connecting");
            Client *client = getContacts()->getClient(0);
            Protocol *protocol = NULL;
            if (client)
                protocol = client->protocol();
            if (m_timer == NULL){
                m_timer = new QTimer(this);
                connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
                m_timer->start(1000);
                m_bBlink = false;
            }
            unsigned status;
            if (m_bBlink){
                icon = "online";
                status = CorePlugin::m_plugin->getManualStatus();
            }else{
                icon = "offline";
                status = STATUS_OFFLINE;
            }
            if (protocol){
                for (const CommandDef *d = protocol->statusList(); d->text; d++){
                    if (d->id == status){
                        icon = d->icon;
                        break;
                    }
                }
            }
        }else{
            if (m_timer){
                delete m_timer;
                m_timer = NULL;
            }
            unsigned status = STATUS_OFFLINE;
            unsigned i;
            for (i = 0; i < getContacts()->nClients(); i++){
                Client *client = getContacts()->getClient(i);
                if (!client->getCommonStatus())
                    continue;
                if (client->getState() == Client::Error){
                    icon = "error";
                    text = I18N_NOOP("Error");
                    break;
                }
                status = client->getStatus();
            }
            if (i >= getContacts()->nClients()){
                Client *client = getContacts()->getClient(0);
                if (client){
                    const CommandDef *d;
                    unsigned i = getContacts()->nClients();
                    if ((status == STATUS_ONLINE) && CorePlugin::m_plugin->getInvisible()){
                        for (i = 0; i < getContacts()->nClients(); i++){
                            Client *client = getContacts()->getClient(i);
                            if (client->protocol()->description()->flags & PROTOCOL_INVISIBLE){
                                icon = client->protocol()->description()->icon_on;
                                text = I18N_NOOP("&Invisible");
                                break;
                            }
                        }
                    }
                    if (i >= getContacts()->nClients()){
                        for (d = client->protocol()->statusList(); d->text; d++){
                            if (d->id == status){
                                icon = d->icon;
                                text = d->text;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    Command cmd;
    cmd->id          = CmdStatusBar;
    cmd->text        = text;
    cmd->icon        = icon;
    cmd->bar_id      = ToolBarMain;
    cmd->bar_grp     = 0x6000;
    cmd->popup_id    = MenuStatus;
    cmd->flags		 = BTN_PICT;

    Event eCmd(bFirst ? EventCommandCreate : EventCommandChange, cmd);
    eCmd.process();

    Event eIcon(EventSetMainIcon, (void*)icon);
    eIcon.process();

    Event eText(EventSetMainText, (void*)text);
    eText.process();
}

void CommonStatus::timeout()
{
    m_bBlink = !m_bBlink;
    setBarStatus(false);
}

void CommonStatus::rebuildStatus()
{
    MAP_STATUS status;
    unsigned nClients = getContacts()->nClients();
    if (nClients == 0)
        return;
    int nInvisible = -1;
    for (unsigned i = 0; i < nClients; i++){
        Client *client = getContacts()->getClient(i);
        for (const CommandDef *cmd = client->protocol()->statusList(); cmd->text; cmd++){
            MAP_STATUS::iterator it = status.find(cmd->id);
            if (it == status.end()){
                status.insert(MAP_STATUS::value_type(cmd->id, 1));
            }else{
                (*it).second++;
            }
        }
        if ((nInvisible == -1) && (client->protocol()->description()->flags & PROTOCOL_INVISIBLE))
            nInvisible = i;
    }
    if (nInvisible != -1){
        Client *client = getContacts()->getClient(nInvisible);
        Command cmd;
        cmd->id			= CmdInvisible;
        cmd->text		= I18N_NOOP("&Invisible");
        cmd->icon		= client->protocol()->description()->icon_on;
        cmd->menu_id	= MenuStatus;
        cmd->menu_grp	= 0x2000;
        cmd->flags		= COMMAND_CHECK_STATE;
        Event e(EventCommandCreate, &cmd);
        e.process();
    }
    Client *client = getContacts()->getClient(0);
    unsigned id = 0x1000;
    unsigned long FirstStatus = 0;
    unsigned long ManualStatus = 0;
    for (const CommandDef *cmd = client->protocol()->statusList(); cmd->text; cmd++){
        MAP_STATUS::iterator it = status.find(cmd->id);
        if (it == status.end())
            continue;
        if ((*it).second != nClients)
            continue;
        CommandDef c = *cmd;
        if (FirstStatus == 0)
            FirstStatus = cmd->id;
        if ((ManualStatus == 0) && (CorePlugin::m_plugin->getManualStatus() == cmd->id))
            ManualStatus = cmd->id;
        c.menu_id  = MenuStatus;
        c.menu_grp = id++;
        c.flags = COMMAND_CHECK_STATE;
        Event e(EventCommandCreate, &c);
        e.process();
    }
    if (ManualStatus == 0)
        ManualStatus = FirstStatus;
    CorePlugin::m_plugin->setManualStatus(ManualStatus);
    setBarStatus(false);
}

void CommonStatus::checkInvisible()
{
    bool bAllInvisible    = true;
    bool bAllNotInvisible = true;
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        if (client->protocol()->description()->flags & PROTOCOL_INVISIBLE){
            if (client->getInvisible()){
                bAllNotInvisible = false;
            }else{
                bAllInvisible = false;
            }
        }
    }
    if (!bAllNotInvisible || !bAllInvisible){
        if (bAllInvisible)
            CorePlugin::m_plugin->setInvisible(true);
        if (bAllNotInvisible)
            CorePlugin::m_plugin->setInvisible(false);
    }
}

void *CommonStatus::processEvent(Event *e)
{
    switch (e->type()){
    case EventClientChanged:
        checkInvisible();
        setBarStatus(false);
        break;
    case EventClientError:{
            clientErrorData *data = (clientErrorData*)(e->param());
            QString msg;
            if (data->err_str && *data->err_str){
                msg = i18n(data->err_str);
                if (data->args)
                    msg = msg.arg(QString::fromUtf8(data->args));
            }
            if (data->code == AuthError){
                LoginDialog *loginDlg = new LoginDialog(false, data->client, msg, NULL);
                raiseWindow(loginDlg);
            }else{
                Command cmd;
                cmd->id = CmdStatusBar;
                Event eWidget(EventCommandWidget, cmd);
                QWidget *widget = (QWidget*)(eWidget.process());
                if (widget){
                    raiseWindow(widget->topLevelWidget());
                    if (!msg.isEmpty())
                        BalloonMsg::message(msg, widget);
                }
            }
            return e->param();
        }
    case EventClientStatus:
    case EventSocketActive:
        setBarStatus(false);
        break;
    case EventClientsChanged:{
            unsigned i;
            bool bCommon		  = false;
            for (i = 0; i < getContacts()->nClients(); i++){
                Client *client = getContacts()->getClient(i);
                if (client->getCommonStatus())
                    bCommon = true;
            }
            if (!bCommon){
                Client *client = getContacts()->getClient(0);
                if (client){
                    client->setCommonStatus(true);
                    Event e(EventClientChanged, client);
                    e.process();
                }
            }
            checkInvisible();
            rebuildStatus();
            break;
        }
    case EventCheckState:{
            CommandDef *def = (CommandDef*)(e->param());
            if (def->menu_id == MenuStatus){
                if (def->id == CmdInvisible){
                    if (CorePlugin::m_plugin->getInvisible()){
                        def->flags |= COMMAND_CHECKED;
                    }else{
                        def->flags &= ~COMMAND_CHECKED;
                    }
                    return e->param();
                }
                Client *client = getContacts()->getClient(0);
                if (client == NULL)
                    return 0;
                const CommandDef *curStatus = NULL;
                const CommandDef *d;
                for (d = client->protocol()->statusList(); d->text; d++){
                    if (d->id == def->id)
                        curStatus = d;
                }
                if (curStatus == NULL)
                    return 0;
                bool bChecked = false;
                unsigned status = CorePlugin::m_plugin->getManualStatus();
                bChecked = (status == curStatus->id);

                if (bChecked){
                    def->flags |= COMMAND_CHECKED;
                }else{
                    def->flags &= ~COMMAND_CHECKED;
                }
                return e->param();
            }
            return 0;
        }
    case EventCommandExec:{
            CommandDef *def = (CommandDef*)(e->param());
            if (def->menu_id == MenuStatus){
                if (def->id == CmdInvisible){
                    CorePlugin::m_plugin->setInvisible(!CorePlugin::m_plugin->getInvisible());
                    for (unsigned i = 0; i < getContacts()->nClients(); i++){
                        getContacts()->getClient(i)->setInvisible(CorePlugin::m_plugin->getInvisible());
                    }
                    return e->param();
                }
                Client *client = getContacts()->getClient(0);
                if (client == NULL)
                    return 0;
                const CommandDef *curStatus = NULL;
                const CommandDef *d;
                for (d = client->protocol()->statusList(); d->text; d++){
                    if (d->id == def->id)
                        curStatus = d;
                }
                if (curStatus == NULL)
                    return 0;
                if ((def->id != STATUS_ONLINE) && (def->id != STATUS_OFFLINE)){
                    const char *noShow = CorePlugin::m_plugin->getNoShowAutoReply(def->id);
                    if ((noShow == NULL) || (*noShow == 0)){
                        AutoReplyDialog dlg(def->id);
                        if (!dlg.exec())
                            return e->param();
                    }
                }
                CorePlugin::m_plugin->setManualStatus(def->id);
                for (unsigned i = 0; i < getContacts()->nClients(); i++){
                    Client *client = getContacts()->getClient(i);
                    if (client->getCommonStatus())
                        client->setStatus(def->id, true);
                }
                return e->param();
            }
            break;
        }
    }
    return NULL;
}

#ifndef WIN32
#include "status.moc"
#endif

