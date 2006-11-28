/***************************************************************************
                          floaty.cpp  -  description
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

#include "floaty.h"
#include "floatywnd.h"

#include "core.h"

#include <qpopupmenu.h>
#include <qapplication.h>
#include <qwidgetlist.h>
#include <qtimer.h>

using namespace SIM;

const unsigned BLINK_TIMEOUT	= 500;
const unsigned BLINK_COUNT		= 8;

Plugin *createFloatyPlugin(unsigned base, bool, Buffer*)
{
    FloatyPlugin *plugin = new FloatyPlugin(base);
    if (plugin->core == NULL){
        delete plugin;
        return NULL;
    }
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Floaty"),
        I18N_NOOP("Plugin provides floating windows for contacts"),
        VERSION,
        createFloatyPlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static DataDef floatyUserData[] =
    {
        { "FloatyPosition", DATA_ULONG, 2, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

FloatyPlugin::FloatyPlugin(unsigned base)
        : Plugin(base)
{
    CmdFloaty = registerType();
    user_data_id = getContacts()->registerUserData(info.title, floatyUserData);

    m_bBlink = false;
    unreadTimer = new QTimer(this);
    connect(unreadTimer, SIGNAL(timeout()), this, SLOT(unreadBlink()));

    Command cmd;
    cmd->id		  = CmdFloaty;
    cmd->text	  = I18N_NOOP("Floating on");
    cmd->icon	  = "floating";
    cmd->menu_id  = MenuContact;
    cmd->menu_grp = 0xB000;
    cmd->flags	  = COMMAND_CHECK_STATE;
    EventCommandCreate(cmd).process();

    EventGetPluginInfo ePlugin("_core");
    ePlugin.process();
    const pluginInfo *info = ePlugin.info();
    core = static_cast<CorePlugin*>(info->plugin);
}

FloatyPlugin::~FloatyPlugin()
{
    QWidgetList *list = QApplication::topLevelWidgets();
    QWidgetListIt it(*list);
    QWidget * w;
    while ( (w=it.current()) != 0 ) {
        if (w->inherits("FloatyWnd"))
            delete w;
        ++it;
    }
    delete list;
    EventCommandRemove(CmdFloaty).process();
    getContacts()->unregisterUserData(user_data_id);
}

FloatyWnd *FloatyPlugin::findFloaty(unsigned id)
{
    QWidgetList *list = QApplication::topLevelWidgets();
    QWidgetListIt it(*list);
    QWidget * w;
    while ((w = it.current()) != NULL) {
        if (w->inherits("FloatyWnd")){
            FloatyWnd *wnd = static_cast<FloatyWnd*>(w);
            if (wnd->id() == id)
                break;
        }
        ++it;
    }
    delete list;
    if (w)
        return static_cast<FloatyWnd*>(w);
    return NULL;
}

bool FloatyPlugin::processEvent(Event *e)
{
    switch (e->type()){
    case eEventInit:{
            Contact *contact;
            ContactList::ContactIterator it;
            while ((contact = ++it) != NULL){
                FloatyUserData *data = (FloatyUserData*)(contact->userData.getUserData(user_data_id, false));
                if (data == NULL)
                    continue;
                FloatyWnd *wnd = new FloatyWnd(this, contact->id());
                wnd->move(data->X.toULong(), data->Y.toULong());
                wnd->show();
            }
            break;
        }
    case eEventCheckState:{
            EventCheckState *ecs = static_cast<EventCheckState*>(e);
            CommandDef *cmd = ecs->cmd();
            if (cmd->id == CmdFloaty){
                Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
                if (contact){
                    FloatyUserData *data = (FloatyUserData*)(contact->userData.getUserData(user_data_id, false));
                    if (data){
                        cmd->text = I18N_NOOP("Floating off");
                        cmd->flags |= COMMAND_CHECKED;
                    }else{
                        cmd->text = I18N_NOOP("Floating on");
                        cmd->flags &= ~COMMAND_CHECKED;
                    }
                }
                return (void*)1;
            }
            break;
        }
    case eEventCommandExec:{
            EventCommandExec *ece = static_cast<EventCommandExec*>(e);
            CommandDef *cmd = ece->cmd();
            if (cmd->id == CmdFloaty){
                Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
                if (contact){
                    FloatyUserData *data = (FloatyUserData*)(contact->userData.getUserData(user_data_id, false));
                    if (data){
                        FloatyWnd *wnd = findFloaty(contact->id());
                        if (wnd)
                            delete wnd;
                        contact->userData.freeUserData(user_data_id);
                    }else{
                        data = (FloatyUserData*)(contact->userData.getUserData(user_data_id, true));
                        FloatyWnd *wnd = new FloatyWnd(this, (unsigned long)(cmd->param));
                        wnd->move(0, 0);
                        wnd->show();
                    }
                }
                return (void*)1;
            }
            break;
        }
    case eEventMessageDeleted:
    case eEventMessageRead:
    case eEventMessageReceived:{
            EventMessage *em = static_cast<EventMessage*>(e);
            Message *msg = em->msg();
            FloatyWnd *wnd = findFloaty(msg->contact());
            if (wnd){
                wnd->init();
                wnd->repaint();
            }
            break;
        }
    case eEventContactClient: {
            EventContactClient *ecc = static_cast<EventContactClient*>(e);
            Contact *contact = ecc->contact();
            if(!contact)
                break;
            FloatyWnd *wnd = findFloaty(contact->id());
            if (wnd){
                wnd->init();
                wnd->repaint();
            }
            break;
        }
    case eEventContact: {
            EventContact *ec = static_cast<EventContact*>(e);
            Contact *contact = ec->contact();
            FloatyWnd *wnd = findFloaty(contact->id());
            if(!wnd)
                break;
            switch(ec->action()) {
                case EventContact::eDeleted:
                    delete wnd;
                    break;
                case EventContact::eStatus:
                case EventContact::eChanged:
                    wnd->init();
                    wnd->repaint();
                    break;
                case EventContact::eOnline:
                    wnd->startBlink();
                    break;
                default:
                    break;
            }
            break;
        }
    case eEventRepaintView:{
            QWidgetList *list = QApplication::topLevelWidgets();
            QWidgetListIt it(*list);
            QWidget * w;
            while ((w = it.current()) != NULL) {
                if (w->inherits("FloatyWnd")){
                    FloatyWnd *wnd = static_cast<FloatyWnd*>(w);
                    wnd->init();
                    wnd->repaint();
                }
                ++it;
            }
            delete list;
            break;
        }
    }
    return NULL;
}

void FloatyPlugin::showPopup()
{
    EventMenuProcess eMenu(MenuContact, (void*)popupId);
    eMenu.process();
    QPopupMenu *popup = eMenu.menu();
    if(popup)
        popup->popup(popupPos);
}

void FloatyPlugin::startBlink()
{
    if (unreadTimer->isActive())
        return;
    m_bBlink = true;
    unreadTimer->start(800);
}

void FloatyPlugin::unreadBlink()
{
    m_bBlink = !m_bBlink;
    QWidgetList *list = QApplication::topLevelWidgets();
    QWidgetListIt it(*list);
    QWidget * w;
    while ((w = it.current()) != NULL) {
        if (w->inherits("FloatyWnd")){
            FloatyWnd *wnd = static_cast<FloatyWnd*>(w);
            wnd->repaint();
        }
        ++it;
    }
    delete list;
}

void FloatyWnd::startBlink()
{
    m_blink = BLINK_COUNT * 2 + 1;
    blinkTimer->start(BLINK_TIMEOUT);
    repaint();
}

#ifndef NO_MOC_INCLUDES
#include "floaty.moc"
#endif
