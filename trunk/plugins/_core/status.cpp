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

#include <map>

#include <qtimer.h>
#include <qapplication.h>
#include <qwidgetlist.h>

#include "ballonmsg.h"
#include "simapi.h"
#include "unquot.h"

#include "status.h"
#include "core.h"
#include "socket.h"
#include "statuswnd.h"
#include "logindlg.h"
#include "autoreply.h"

using namespace std;
using namespace SIM;

typedef map<unsigned, unsigned> MAP_STATUS;

CommonStatus::CommonStatus()
        : EventReceiver(LowPriority + 2)
{
    m_bBlink  = false;
    m_timer   = NULL;
    m_balloon = NULL;

    EventMenu(MenuStatus, EventMenu::eAdd).process();

    Command cmd;
    cmd->id          = CmdStatusMenu;
    cmd->text        = I18N_NOOP("Status");
    cmd->menu_id     = MenuMain;
    cmd->menu_grp    = 0x6000;
    cmd->popup_id    = MenuStatus;
    cmd->flags		 = COMMAND_IMPORTANT;

    EventCommandCreate(cmd).process();

    m_bInit = false;
    rebuildStatus();
    QTimer::singleShot(500, this, SLOT(setBarStatus()));
}

CommonStatus::~CommonStatus()
{
    EventCommandRemove(CmdStatusBar).process();
    EventMenu(MenuStatus, EventMenu::eRemove).process();
}

void CommonStatus::setBarStatus()
{
    const char *text = I18N_NOOP("Inactive");
    const char *icon = "SIM_inactive";

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
                icon = "SIM_online";
                status = CorePlugin::m_plugin->getManualStatus();
            }else{
                icon = "SIM_offline";
                status = STATUS_OFFLINE;
            }
        }else{
            if (m_timer){
                delete m_timer;
                m_timer = NULL;
            }
            unsigned status = CorePlugin::m_plugin->getManualStatus();
            unsigned i;
            for (i = 0; i < getContacts()->nClients(); i++){
                Client *client = getContacts()->getClient(i);
                if (!client->getCommonStatus())
                    continue;
                if (client->getState() == Client::Error){
                    icon = "SIM_error";
                    text = I18N_NOOP("Error");
                    break;
                }
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
                                icon = "SIM_invisible";
                                text = I18N_NOOP("&Invisible");
                                break;
                            }
                        }
                    }
                    if (i >= getContacts()->nClients()){
                        for (d = client->protocol()->statusList(); !d->text.isEmpty(); d++){
                             if (d->id == status){
                                 switch (status){
                                 case STATUS_ONLINE: 
                                     icon="SIM_online";
                                     break;
                                 case STATUS_AWAY:
                                     icon="SIM_away";
                                     break;
                                 case STATUS_NA:
                                     icon="SIM_na";
                                     break;
                                 case STATUS_DND:
                                     icon="SIM_dnd";
                                     break;
                                 case STATUS_FFC:
                                     icon="SIM_ffc";
                                     break;
                                 case STATUS_OFFLINE:
                                     icon="SIM_offline";
                                     break;
                                 }
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

    if(!m_bInit)
        EventCommandCreate(cmd).process();
    else
        EventCommandChange(cmd).process();

    m_bInit = true;

    EventSetMainIcon(icon).process();
    EventSetMainText(text).process();
}

void CommonStatus::timeout()
{
    m_bBlink = !m_bBlink;
    setBarStatus();
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
        for (const CommandDef *cmd = client->protocol()->statusList(); !cmd->text.isEmpty(); cmd++){
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
        Command cmd;
        cmd->id			= CmdInvisible;
        cmd->text		= I18N_NOOP("&Invisible");
        cmd->icon		= "SIM_invisible";
        cmd->menu_id	= MenuStatus;
        cmd->menu_grp	= 0x2000;
        cmd->flags		= COMMAND_CHECK_STATE;
        EventCommandCreate(cmd).process();
    }
    Client *client = getContacts()->getClient(0);
    unsigned id = 0x1000;
    unsigned long FirstStatus = 0;
    unsigned long ManualStatus = 0;
    for (const CommandDef *cmd = client->protocol()->statusList(); !cmd->text.isEmpty(); cmd++){
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
        switch (c.id){
        case STATUS_ONLINE: 
            c.icon="SIM_online";
            break;
        case STATUS_AWAY:
            c.icon="SIM_away";
            break;
        case STATUS_NA:
            c.icon="SIM_na";
            break;
        case STATUS_DND:
            c.icon="SIM_dnd";
            break;
        case STATUS_FFC:
            c.icon="SIM_ffc";
            break;
        case STATUS_OFFLINE:
            c.icon="SIM_offline";
            break;
        }
        c.menu_id  = MenuStatus;
        c.menu_grp = id++;
        c.flags = COMMAND_CHECK_STATE;
        EventCommandCreate(&c).process();
    }
    if (ManualStatus == 0)
        ManualStatus = FirstStatus;
    CorePlugin::m_plugin->setManualStatus(ManualStatus);
    setBarStatus();
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
    case eEventClientChanged:
        checkInvisible();
        setBarStatus();
        break;
    case eEventShowError:{
            EventShowError *ee = static_cast<EventShowError*>(e);
            const EventError::ClientErrorData &data = ee->data();
            for (list<BalloonItem>::iterator it = m_queue.begin(); it != m_queue.end(); ++it){
                if ((*it).id == data.id)
                    return (void*)1;
            }
            BalloonItem item;
            item.id     = data.id;
            item.client = data.client;
            item.text   = i18n(data.err_str);
            if (!data.args.isEmpty()){
                if (item.text.find("%1") >= 0)
                    item.text = item.text.arg(data.args);
            }
            QString title = "SIM";
            if (getContacts()->nClients() > 1){
                for (unsigned i = 0; i < getContacts()->nClients(); i++){
                    if (getContacts()->getClient(i) == data.client){
                        title = data.client->name();
                        int n = title.find(".");
                        if (n > 0)
                            title = title.left(n) + " " + title.mid(n + 1);
                        break;
                    }
                }
            }
            item.text	= QString("<img src=\"icon:%1\">&nbsp;<b><nobr>%2</nobr></b><br><center>")
                .arg((data.flags & EventError::ClientErrorData::E_INFO) ? "info" : "error")
                        .arg(title) + quoteString(item.text) + "</center>";
            if (data.options){
                for (const char *p = data.options; *p; p += strlen(p) + 1)
                    item.buttons.append(i18n(p));
            }else{
                item.buttons.append(i18n("OK"));
            }
            m_queue.push_back(item);
            if (m_balloon == NULL)
                showBalloon();
            break;
        }
    case eEventClientError:{
            EventClientError *ee = static_cast<EventClientError*>(e);
            const EventError::ClientErrorData &data = ee->data();
            if (data.code == AuthError){
                QString msg;
                if (!data.err_str.isEmpty())
                    msg = i18n(data.err_str).arg(data.args);
                LoginDialog *loginDlg = new LoginDialog(false, data.client, msg, NULL);
                raiseWindow(loginDlg);
            }else{
                EventShowError eShow(data);
                eShow.process();
            }
            return (void*)1;
        }
    case EventClientStatus:
    case eEventSocketActive:
    case eEventInit:
        setBarStatus();
        break;
    case eEventClientsChanged:{
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
                    EventClientChanged(client).process();
                }
            }
            checkInvisible();
            rebuildStatus();
            break;
        }
    case eEventCheckState:{
            EventCheckState *ecs = static_cast<EventCheckState*>(e);
            CommandDef *def = ecs->cmd();
            if (def->menu_id == MenuStatus){
                if (def->id == CmdInvisible){
                    if (CorePlugin::m_plugin->getInvisible()){
                        def->flags |= COMMAND_CHECKED;
                    }else{
                        def->flags &= ~COMMAND_CHECKED;
                    }
                    return (void*)1;
                }
                Client *client = getContacts()->getClient(0);
                if (client == NULL)
                    return 0;
                const CommandDef *curStatus = NULL;
                const CommandDef *d;
                for (d = client->protocol()->statusList(); !d->text.isEmpty(); d++){
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
                return (void*)1;
            }
            return 0;
        }
    case eEventCommandExec:{
            EventCommandExec *ece = static_cast<EventCommandExec*>(e);
            CommandDef *def = ece->cmd();
            if (def->menu_id == MenuStatus){
                if (def->id == CmdInvisible){
                    CorePlugin::m_plugin->setInvisible(!CorePlugin::m_plugin->getInvisible());
                    for (unsigned i = 0; i < getContacts()->nClients(); i++){
                        getContacts()->getClient(i)->setInvisible(CorePlugin::m_plugin->getInvisible());
                    }
                    return (void*)1;
                }
                Client *client = getContacts()->getClient(0);
                if (client == NULL)
                    return 0;
                const CommandDef *curStatus = NULL;
                const CommandDef *d;
                for (d = client->protocol()->statusList(); !d->text.isEmpty(); d++){
                    if (d->id == def->id)
                        curStatus = d;
                }
                if (curStatus == NULL)
                    return 0;
                unsigned i;
                bool bOfflineStatus = false;
                for (i = 0; i < getContacts()->nClients(); i++){
                    Client *client = getContacts()->getClient(i);
                    if (client->getCommonStatus() &&
                            (client->protocol()->description()->flags & PROTOCOL_AR_OFFLINE)){
                        bOfflineStatus = true;
                        break;
                    }
                }

                if (bOfflineStatus ||
                        ((def->id != STATUS_ONLINE) && (def->id != STATUS_OFFLINE))){
                    QString noShow = CorePlugin::m_plugin->getNoShowAutoReply(def->id);
                    if (noShow.isEmpty()){
                        AutoReplyDialog dlg(def->id);
                        if (!dlg.exec())
                            return (void*)1;
                    }
                }
                CorePlugin::m_plugin->setManualStatus(def->id);
                for (i = 0; i < getContacts()->nClients(); i++){
                    Client *client = getContacts()->getClient(i);
                    if (client->getCommonStatus())
                        client->setStatus(def->id, true);
                }
                return (void*)1;
            }
            break;
        }
    }
    return NULL;
}

void CommonStatus::showBalloon()
{
    if (m_balloon || m_queue.empty())
        return;
    Command cmd;
    cmd->id = CmdStatusBar;
    EventCommandWidget eWidget(cmd);
    eWidget.process();
    QWidget *widget = eWidget.widget();
    if (widget == NULL){
        m_queue.erase(m_queue.begin());
        return;
    }
    BalloonItem &item = m_queue.front();
    if (CorePlugin::m_plugin->m_statusWnd)
        m_balloon = CorePlugin::m_plugin->m_statusWnd->showError(item.text, item.buttons, item.client);
    if (m_balloon == NULL)
        m_balloon = new BalloonMsg(NULL, item.text, item.buttons, widget);
    connect(m_balloon, SIGNAL(yes_action(void*)), this, SLOT(yes_action(void*)));
    connect(m_balloon, SIGNAL(finished()), this, SLOT(finished()));
    raiseWindow(widget->topLevelWidget());
    m_balloon->show();
}

void CommonStatus::yes_action(void*)
{
    if (!m_queue.empty() && m_balloon){
        m_balloon->hide();
        BalloonItem &item = m_queue.front();
        Command cmd;
        cmd->id    = item.id;
        cmd->param = item.client;
        EventCommandExec(cmd).process();
    }
}

void CommonStatus::finished()
{
    m_balloon = NULL;
    if (!m_queue.empty())
        m_queue.erase(m_queue.begin());
    QTimer::singleShot(1000, this, SLOT(showBalloon()));
}

#ifndef NO_MOC_INCLUDES
#include "status.moc"
#endif

