/***************************************************************************
                          dock.cpp  -  description
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

#include "dock.h"
#include "dockcfg.h"
#include "dockwnd.h"
#include "simapi.h"
#include "core.h"
#include "mainwin.h"

#include <qapplication.h>
#include <qwidgetlist.h>
#include <qpopupmenu.h>
#include <qtimer.h>
#include <time.h>

#ifdef WIN32
#include <windows.h>
#endif

#include <list>
using namespace std;

Plugin *createDockPlugin(unsigned base, bool, const char *config)
{
    Plugin *plugin = new DockPlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
#ifdef WIN32
        I18N_NOOP("System tray"),
        I18N_NOOP("Plugin provides dock main window in system tray"),
#else
        I18N_NOOP("Dock"),
        I18N_NOOP("Plugin provides dock icon"),
#endif
        VERSION,
        createDockPlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

/*
typedef struct DockData
{
    bool			AutoHide;
    unsigned		AutoHideInterval;
    bool			ShowMain;
#ifndef WIN32 
    unsigned		DockX;
    unsigned		DockY;
#endif
} DockData;
*/
static DataDef dockData[] =
    {
        { "AutoHide", DATA_BOOL, 1, 1 },
        { "AutoHideInterval", DATA_ULONG, 1, 60 },
        { "ShowMain", DATA_BOOL, 1, 1 },
#ifndef WIN32
        { "DockPos", DATA_ULONG, 2, 0 },
#endif
        { NULL, 0, 0, 0 }
    };

DockPlugin::DockPlugin(unsigned base, const char *config)
        : Plugin(base)
{
    load_data(dockData, &data, config);
    dock = NULL;
    inactiveTime = 0;
    m_popup = NULL;

    Event ePlugin(EventGetPluginInfo, (void*)"_core");
    pluginInfo *info = (pluginInfo*)(ePlugin.process());
    core = static_cast<CorePlugin*>(info->plugin);

    DockMenu     = registerType();
    CmdTitle	 = registerType();
    CmdToggle    = registerType();
    CmdCustomize = registerType();

    Event eMenu(EventMenuCreate, (void*)DockMenu);
    eMenu.process();

    Command cmd;
    cmd->id          = CmdTitle;
    cmd->text        = I18N_NOOP("SIM");
    cmd->icon        = "licq";
    cmd->menu_id     = DockMenu;
    cmd->menu_grp    = 0x1000;
    cmd->flags       = COMMAND_TITLE;

    Event eCmd(EventCommandCreate, cmd);
    eCmd.process();

    cmd->id          = CmdCustomize;
    cmd->text        = I18N_NOOP("Customize menu");
    cmd->icon        = "configure";
    cmd->menu_id     = DockMenu;
    cmd->menu_grp    = 0x10000;
    cmd->accel       = 0;
    cmd->flags       = COMMAND_DEFAULT;

    eCmd.process();

    Event eDef(EventGetMenuDef, (void*)MenuMain);
    CommandsDef *def = (CommandsDef*)(eDef.process());
    if (def){
        CommandsList list(*def, true);
        CommandDef *s;
        while ((s = ++list) != NULL){
            if (s->id == 0)
                continue;
            cmd = *s;
            if (!(cmd->flags & COMMAND_IMPORTANT))
                cmd->menu_grp = 0;
            cmd->bar_id  = 0;
            cmd->menu_id = DockMenu;
            eCmd.process();
        }
    }

    cmd->id          = CmdToggle;
    cmd->text        = I18N_NOOP("Toggle main window");
    cmd->icon        = NULL;
    cmd->menu_id     = MenuMain;
    cmd->menu_grp    = 0;
    cmd->accel		 = "Ctrl+Shift+A";
    cmd->flags		 = COMMAND_CHECK_STATE | COMMAND_GLOBAL_ACCEL | COMMAND_IMPORTANT;
    eCmd.process();

    init();

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timer()));
    timer->start(2000);
}

DockPlugin::~DockPlugin()
{
    Event eCmd(EventCommandRemove, (void*)CmdToggle);
    eCmd.process();

    Event eMenu(EventMenuRemove, (void*)DockMenu);
    eMenu.process();
    if (dock)
        delete dock;
    free_data(dockData, &data);
}

void DockPlugin::init()
{
    if (dock)
        return;
    m_main = getMainWindow();
    if (m_main == NULL)
        return;
    m_main->installEventFilter(this);
    dock = new DockWnd(this, "inactive", I18N_NOOP("Inactive"));
    connect(dock, SIGNAL(showPopup(QPoint)), this, SLOT(showPopup(QPoint)));
    connect(dock, SIGNAL(toggleWin()), this, SLOT(toggleWin()));
    connect(dock, SIGNAL(doubleClicked()), this, SLOT(doubleClicked()));
    bQuit = false;
}

bool DockPlugin::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_popup){
        if (e->type() == QEvent::Hide){
            m_popup->removeEventFilter(this);
            m_popup = NULL;
        }
    }else{
        switch (e->type()){
        case QEvent::Close:
            if (!bQuit){
                QWidget *main = (QWidget*)o;
                setShowMain(false);
				main->hide();
                return true;
            }
            break;
        case QEvent::WindowDeactivate:
            time((time_t*)&inactiveTime);
            break;
        case QEvent::WindowActivate:
            inactiveTime = 0;
            break;
        default:
            break;
        }
    }
    return QObject::eventFilter(o, e);
}

bool DockPlugin::isMainShow()
{
    QWidget *main = getMainWindow();
    if (main && main->isVisible())
        return true;
    return false;
}

void *DockPlugin::processEvent(Event *e)
{
    CommandDef *def;
    switch (e->type()){
    case EventInit:
        init();
        break;
    case EventQuit:
        if (dock){
            delete dock;
            dock = NULL;
        }
        break;
    case EventRaiseWindow:
        if ((e->param() == getMainWindow()) && !getShowMain())
            return e->param();
        break;
    case EventCommandCreate:
        def = (CommandDef*)(e->param());
        if (def->menu_id == MenuMain){
            CommandDef d = *def;
            if (def->flags & COMMAND_IMPORTANT){
                if (d.menu_grp == 0)
                    d.menu_grp = 0x1001;
            }else{
                d.menu_grp = 0;
            }
            d.menu_id = DockMenu;
            d.bar_id  = 0;
            Event e(EventCommandCreate, &d);
            e.process();
        }
        break;
    case EventCheckState:
        def = (CommandDef*)(e->param());
        if (def->id == CmdToggle){
            def->flags &= ~COMMAND_CHECKED;
            def->text = isMainShow() ?
                        I18N_NOOP("Hide main window") :
                        I18N_NOOP("Show main window");
            return e->param();
        }
        break;
    case EventCommandExec:
        CommandDef *def = (CommandDef*)(e->param());
        if (def->id == CmdToggle){
            QWidget *main = getMainWindow();
            if (isMainShow()){
                setShowMain(false);
                main->hide();
            }else{
                setShowMain(true);
                main->show();
            }
            return e->param();
        }
        if (def->id == CmdCustomize){
            Event eCustomize(EventMenuCustomize, (void*)DockMenu);
            eCustomize.process();
            return e->param();
        }
        if (def->id == CmdQuit)
            bQuit = true;
        break;
    }
    return NULL;
}

string DockPlugin::getConfig()
{
    return save_data(dockData, &data);
}

void DockPlugin::showPopup(QPoint p)
{
    if (m_popup)
        return;
    Command cmd;
    cmd->popup_id = DockMenu;
    Event e(EventGetMenu, cmd);
    m_popup = (QPopupMenu*)e.process();
    if (m_popup){
        m_popup->installEventFilter(this);
		if (qApp->activeWindow() == NULL)
			dock->setFocus();
        m_popup->popup(p);
// #if QT_VERSION < 300
//         getMainWindow()->grabMouse(0);
// #endif
    }
}

void DockPlugin::toggleWin()
{
    if (m_popup)
        return;

    Command cmd;
    cmd->id          = CmdToggle;
    cmd->menu_id     = DockMenu;
    cmd->menu_grp    = 0x1000;
    cmd->flags       = COMMAND_CHECK_STATE;

    Event e(EventCommandExec, cmd);
    e.process();
}

void DockPlugin::doubleClicked()
{
    if (m_popup)
        return;

    Command cmd;
    cmd->id          = CmdToggle;
    cmd->menu_id     = DockMenu;
    cmd->menu_grp    = 0x1000;
    cmd->flags       = COMMAND_CHECK_STATE;

    if (core->unread.size())
        cmd->id = CmdUnread;

    Event e(EventCommandExec, cmd);
    e.process();
}

QWidget *DockPlugin::getMainWindow()
{
    QWidgetList  *list = QApplication::topLevelWidgets();
    QWidgetListIt it( *list );
    QWidget *w;
    while ( (w=it.current()) != 0 ) {
        ++it;
        if (w->inherits("MainWindow")){
            delete list;
            return w;
        }
    }
    delete list;
    return NULL;
}

const unsigned ANIMATE_TIME = 200;

QWidget *DockPlugin::createConfigWindow(QWidget *parent)
{
    return new DockCfg(parent, this);
}

void DockPlugin::timer()
{
    if (!isMainShow())  // already hidden
        return;
    if (!getAutoHide() || (inactiveTime == 0))  // no autohide
        return;
    unsigned now;
    time((time_t*)&now);
    if (m_main != getMainWindow()) {
        m_main = getMainWindow();
        m_main->installEventFilter(this);
    }
    if (now > inactiveTime + getAutoHideInterval()){
        if (m_main){
            setShowMain(false);
            m_main->hide();
        }
    }
}

#ifdef WIN32

/**
 * DLL's entry point
 **/
int WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
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
#include "dock.moc"
#endif
