/***************************************************************************
                          commands.cpp  -  description
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

#include "commands.h"
#include "toolbtn.h"
#include "toolsetup.h"
#include "simapi.h"
#include "core.h"
#include "cmenu.h"

#include <qapplication.h>
#include <qpopupmenu.h>
#include <qwidgetlist.h>
#include <qaccel.h>

Commands::Commands()
{
    qApp->installEventFilter(this);
}

Commands::~Commands()
{
    CMDS_MAP::iterator it;
    for (it = bars.begin(); it != bars.end(); ++it){
        delete (*it).second;
    }
    MENU_MAP::iterator itm;
    for (itm = menues.begin(); itm != menues.end(); ++itm){
		MenuDef &def = (*itm).second;
        if (def.menu)
            delete def.menu;
        delete def.def;
    }
}

CommandsDef *Commands::createBar(unsigned id)
{
    CMDS_MAP::iterator it = bars.find(id);
    if (it != bars.end())
        return (*it).second;
    CommandsDef *def = new CommandsDef(id, false);
    bars.insert(CMDS_MAP::value_type(id, def));
    return def;
}

void Commands::removeBar(unsigned id)
{
    CMDS_MAP::iterator it = bars.find(id);
    if (it == bars.end())
        return;
    delete (*it).second;
    bars.erase(it);
}

void Commands::clear()
{
    for (MENU_MAP::iterator it = menues.begin(); it != menues.end(); ++it){
        if ((*it).second.menu){
            delete (*it).second.menu;
            (*it).second.menu = NULL;
        }
    }
}

CommandsDef *Commands::createMenu(unsigned id)
{
    if (id == MenuLocation)
        id = MenuLocation;
    MENU_MAP::iterator it = menues.find(id);
    if (it != menues.end())
        return (*it).second.def;
    MenuDef def;
    def.def  = new CommandsDef(id, true);
    def.menu = NULL;
    menues.insert(MENU_MAP::value_type(id, def));
    return def.def;
}

void Commands::removeMenu(unsigned id)
{
    MENU_MAP::iterator it = menues.find(id);
    if (it == menues.end())
        return;
    if ((*it).second.menu)
        delete (*it).second.menu;
    delete (*it).second.def;
    menues.erase(it);
}

void *Commands::show(unsigned id, QMainWindow *parent)
{
    CMDS_MAP::iterator it = bars.find(id);
    if (it == bars.end())
        return NULL;
    (*it).second->setConfig(CorePlugin::m_plugin->getButtons(id));
    return new CToolBar((*it).second, parent);
}

CMenu *Commands::get(CommandDef *cmd)
{
    MENU_MAP::iterator it = menues.find(cmd->popup_id);
    if (it == menues.end())
        return NULL;
    MenuDef &d = (*it).second;
    if (d.menu && ((cmd->flags & COMMAND_NEW_POPUP) == 0)){
        d.menu->setParam(cmd->param);
        return d.menu;
    }
    d.def->setConfig(CorePlugin::m_plugin->getMenues(cmd->popup_id));
    CMenu *menu = new CMenu(d.def);
    menu->setParam(cmd->param);
    if ((cmd->flags & COMMAND_NEW_POPUP) == 0)
        d.menu = menu;
    return menu;
}

CMenu *Commands::processMenu(unsigned id, void *param, int key)
{
    MENU_MAP::iterator it = menues.find(id);
    if (it == menues.end())
        return NULL;
    MenuDef &d = (*it).second;
    if (key){
        CommandsList list(*d.def, true);
        CommandDef *cmd;
        while ((cmd = ++list) !=NULL){
            int cmdKey;
            if ((key & ALT) && ((key & ~MODIFIER_MASK) != Key_Alt)){
                if (cmd->text == NULL) continue;
                cmdKey = QAccel::shortcutKey(i18n(cmd->text));
                if ((cmdKey & ~UNICODE_ACCEL) == key){
                    cmd->param = param;
                    Event eCmd(EventCommandExec, cmd);
                    if (eCmd.process())
                        break;
                }
            }
            if (cmd->accel == NULL) continue;
            cmdKey = QAccel::stringToKey(i18n(cmd->accel));
            if (cmdKey == key){
                cmd->param = param;
                Event eCmd(EventCommandExec, cmd);
                if (eCmd.process())
                    break;
            }
        }
        if (cmd == NULL)
            return NULL;
    }
    if (d.menu){
        d.menu->setParam(param);
        return d.menu;
    }
    d.def->setConfig(CorePlugin::m_plugin->getMenues(id));
    d.menu = new CMenu(d.def);
    d.menu->setParam(param);
    return d.menu;
}

CommandsDef *Commands::getDef(unsigned id)
{
    MENU_MAP::iterator it = menues.find(id);
    if (it == menues.end())
        return NULL;
    return (*it).second.def;
}

void *Commands::processEvent(Event *e)
{
    BarShow *b;
    ProcessMenuParam *mp;
    switch (e->type()){
    case EventPluginsUnload:
        clear();
        break;
    case EventToolbarCreate:
        return (void*)createBar((unsigned)(e->param()));
    case EventToolbarRemove:
        removeBar((unsigned)(e->param()));
        break;
    case EventShowBar:
        b = (BarShow*)(e->param());
        return show(b->bar_id, b->parent);
    case EventMenuCreate:
        return (void*)createMenu((unsigned)(e->param()));
    case EventMenuRemove:
        removeMenu((unsigned)(e->param()));
        break;
    case EventGetMenu:
        return (void*)get((CommandDef*)(e->param()));
    case EventGetMenuDef:
        return (void*)getDef((unsigned)(e->param()));
    case EventProcessMenu:
        mp = (ProcessMenuParam*)(e->param());
        return (void*)processMenu(mp->id, mp->param, mp->key);
    case EventMenuCustomize:
        customizeMenu((unsigned)(e->param()));
        break;
    default:
        break;
    }
    return NULL;
}

bool Commands::eventFilter(QObject *o, QEvent *e)
{
    if ((e->type() == QEvent::Show) && o->inherits("QPopupMenu")){
        if (!o->inherits("CMenu")){
            QObject *parent = o->parent();
            if (parent){
                unsigned id = 0;
                if (parent->inherits("MainWindow")){
                    id = ToolBarMain;
                }else if (parent->inherits("CToolBar")){
                    CToolBar *bar = static_cast<CToolBar*>(parent);
                    id = bar->m_def->id();
                }
                if (id){
                    QPopupMenu *popup = static_cast<QPopupMenu*>(o);
                    popup->insertItem(i18n("Customize toolbar..."), this, SLOT(popupActivated()));
                    cur_id = id;
                }
            }
        }
    }
    return QObject::eventFilter(o, e);
}

void Commands::popupActivated()
{
    CMDS_MAP::iterator it = bars.find(cur_id);
    if (it == bars.end())
        return;
    customize((*it).second);
}

void Commands::customize(CommandsDef *def)
{
    QWidgetList  *list = QApplication::topLevelWidgets();
    QWidgetListIt it( *list );
    QWidget * w;
    ToolBarSetup *wnd = NULL;
    while ( (w=it.current()) != 0 ){
        ++it;
        if (!w->inherits("ToolBarSetup")) continue;
        ToolBarSetup *swnd = static_cast<ToolBarSetup*>(w);
        if (swnd->m_def != def) continue;
        wnd = swnd;
        break;
    }
    if (wnd == NULL) wnd= new ToolBarSetup(this, def);
    raiseWindow(wnd);
    delete list;
}

void Commands::customizeMenu(unsigned id)
{
    MENU_MAP::iterator it = menues.find(id);
    if (it == menues.end())
        return;
    MenuDef &d = (*it).second;
    d.def->setConfig(CorePlugin::m_plugin->getMenues(id));
    customize(d.def);
}

void Commands::set(CommandsDef *def, const char *str)
{
    if (def->isMenu()){
        CorePlugin::m_plugin->setMenues(def->id(), str);
    }else{
        CorePlugin::m_plugin->setButtons(def->id(), str);
        Event e(EventToolbarChanged, def);
        e.process();
    }
}

#ifndef WIN32
#include "commands.moc"
#endif

