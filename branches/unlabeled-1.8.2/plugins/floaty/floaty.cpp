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
#include "simapi.h"

#include "core.h"

#include "xpm/floating.xpm"

#include <qpopupmenu.h>
#include <qapplication.h>
#include <qwidgetlist.h>
#include <qtimer.h>

Plugin *createFloatyPlugin(unsigned base, bool, const char*)
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

/*
typedef struct FloatyUserData
{
	unsigned	X;
	unsigned	Y;
} FloatyUserData;
*/

static DataDef floatyUserData[] =
    {
        { "FloatyPosition", DATA_ULONG, 2, 0 },
        { NULL, 0, 0, 0 }
    };

FloatyPlugin::FloatyPlugin(unsigned base)
        : Plugin(base)
{
    CmdFloaty = registerType();
    user_data_id = getContacts()->registerUserData(info.title, floatyUserData);

    IconDef icon;
    icon.name = "floating";
    icon.xpm = floating;

    m_bBlink = false;
    unreadTimer = new QTimer(this);
    connect(unreadTimer, SIGNAL(timeout()), this, SLOT(unreadBlink()));

    Event eIcon(EventAddIcon, &icon);
    eIcon.process();

    Command cmd;
    cmd->id		  = CmdFloaty;
    cmd->text	  = I18N_NOOP("Floating on");
    cmd->icon	  = "floating";
    cmd->menu_id  = MenuContact;
    cmd->menu_grp = 0xB000;
    cmd->flags	  = COMMAND_CHECK_STATE;
    Event e(EventCommandCreate, cmd);
    e.process();

    Event ePlugin(EventGetPluginInfo, (void*)"_core");
    pluginInfo *info = (pluginInfo*)(ePlugin.process());
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
    Event e(EventCommandRemove, (void*)CmdFloaty);
    e.process();
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

void *FloatyPlugin::processEvent(Event *e)
{
    switch (e->type()){
    case EventInit:{
            Contact *contact;
            ContactList::ContactIterator it;
            while ((contact = ++it) != NULL){
                FloatyUserData *data = (FloatyUserData*)(contact->userData.getUserData(user_data_id, false));
                if (data == NULL)
                    continue;
                FloatyWnd *wnd = new FloatyWnd(this, contact->id());
                wnd->move(data->X.value, data->Y.value);
                wnd->show();
            }
            break;
        }
    case EventCheckState:{
            CommandDef *cmd = (CommandDef*)(e->param());
            if (cmd->id == CmdFloaty){
                Contact *contact = getContacts()->contact((unsigned)(cmd->param));
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
                return e->param();
            }
            break;
        }
    case EventCommandExec:{
            CommandDef *cmd = (CommandDef*)(e->param());
            if (cmd->id == CmdFloaty){
                Contact *contact = getContacts()->contact((unsigned)(cmd->param));
                if (contact){
                    FloatyUserData *data = (FloatyUserData*)(contact->userData.getUserData(user_data_id, false));
                    if (data){
                        FloatyWnd *wnd = findFloaty(contact->id());
                        if (wnd)
                            delete wnd;
                        contact->userData.freeUserData(user_data_id);
                    }else{
                        data = (FloatyUserData*)(contact->userData.getUserData(user_data_id, true));
                        FloatyWnd *wnd = new FloatyWnd(this, (unsigned)(cmd->param));
                        wnd->move(0, 0);
                        wnd->show();
                    }
                }
                return e->param();
            }
            break;
        }
    case EventMessageDeleted:
    case EventMessageRead:
    case EventMessageReceived:{
            Message *msg = (Message*)(e->param());
            FloatyWnd *wnd = findFloaty(msg->contact());
            if (wnd){
                wnd->init();
                wnd->repaint();
            }
            break;
        }
    case EventContactClient:
    case EventContactStatus:
    case EventContactChanged:{
            Contact *contact = (Contact*)(e->param());
            FloatyWnd *wnd = findFloaty(contact->id());
            if (wnd){
                wnd->init();
                wnd->repaint();
            }
            break;
        }
    case EventContactDeleted:{
            Contact *contact = (Contact*)(e->param());
            FloatyWnd *wnd = findFloaty(contact->id());
            if (wnd)
                delete wnd;
            break;
        }
    case EventRepaintView:{
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
    ProcessMenuParam mp;
    mp.id    = MenuContact;
    mp.param = (void*)(popupId);
    mp.key	 = 0;
    Event eMenu(EventProcessMenu, &mp);
    QPopupMenu *menu = (QPopupMenu*)eMenu.process();
    menu->popup(popupPos);
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

#ifdef WIN32

#include <windows.h>

/**
 * DLL's entry point
 **/
int WINAPI DllMain(HINSTANCE, DWORD, LPVOID)
{
    return TRUE;
}

/**
 * This is to prevent the CRT from loading, thus making this a smaller
 * and faster dll.
 **/
extern "C" BOOL __stdcall _DllMainCRTStartup( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return DllMain( hinstDLL, fdwReason, lpvReserved );
}

#endif

#ifndef WIN32
#include "floaty.moc"
#endif

