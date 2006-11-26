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

using namespace SIM;

Plugin *createDockPlugin(unsigned base, bool, Buffer *config)
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
        { "AutoHide", DATA_BOOL, 1, DATA(1) },
        { "AutoHideInterval", DATA_ULONG, 1, DATA(60) },
        { "ShowMain", DATA_BOOL, 1, DATA(1) },
#ifndef WIN32
        { "DockPos", DATA_ULONG, 2, 0 },
#endif
        { "Desktop", DATA_ULONG, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

DockPlugin::DockPlugin(unsigned base, Buffer *config)
        : Plugin(base)
{
    load_data(dockData, &data, config);
    m_dock = NULL;
    m_inactiveTime = 0;
    m_popup = NULL;

    EventGetPluginInfo ePlugin("_core");
    ePlugin.process();
    const pluginInfo *info = ePlugin.info();
    m_core = static_cast<CorePlugin*>(info->plugin);

    DockMenu     = registerType();
    CmdTitle	 = registerType();
    CmdToggle    = registerType();
    CmdCustomize = registerType();

    EventMenu(DockMenu, EventMenu::eAdd).process();

    Command cmd;
    cmd->id          = CmdTitle;
    cmd->text        = I18N_NOOP("SIM");
    cmd->icon        = "SIM";
    cmd->menu_id     = DockMenu;
    cmd->menu_grp    = 0x1000;
    cmd->flags       = COMMAND_TITLE;
    EventCommandCreate(cmd).process();

    cmd->id          = CmdCustomize;
    cmd->text        = I18N_NOOP("Customize menu");
    cmd->icon        = "configure";
    cmd->menu_id     = DockMenu;
    cmd->menu_grp    = 0x10000;
    cmd->accel       = QString::null;
    cmd->flags       = COMMAND_DEFAULT;
    EventCommandCreate(cmd).process();

    EventMenuGetDef eMenu(MenuMain);
    eMenu.process();
    CommandsDef *def = eMenu.defs();
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
            EventCommandCreate(cmd).process();
        }
    }

    cmd->id          = CmdToggle;
    cmd->text        = I18N_NOOP("Toggle main window");
    cmd->icon        = QString::null;
    cmd->menu_id     = MenuMain;
    cmd->menu_grp    = 0;
    cmd->accel		 = "Ctrl+Shift+A";
    cmd->flags		 = COMMAND_CHECK_STATE | COMMAND_GLOBAL_ACCEL | COMMAND_IMPORTANT;
    EventCommandCreate(cmd).process();

    init();

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timer()));
    timer->start(2000);
}

DockPlugin::~DockPlugin()
{
    EventCommandRemove(CmdToggle).process();
    EventMenu(DockMenu, EventMenu::eRemove).process();
    delete m_dock;
    free_data(dockData, &data);
}

void DockPlugin::init()
{
    if (m_dock)
        return;
    m_main = getMainWindow();
    if (m_main == NULL)
        return;
    m_main->installEventFilter(this);
    m_dock = new DockWnd(this, "inactive", I18N_NOOP("Inactive"));
    connect(m_dock, SIGNAL(showPopup(QPoint)), this, SLOT(showPopup(QPoint)));
    connect(m_dock, SIGNAL(toggleWin()), this, SLOT(toggleWin()));
    connect(m_dock, SIGNAL(doubleClicked()), this, SLOT(doubleClicked()));
    m_bQuit = false;
    QApplication::syncX();
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
            if (!m_bQuit){
                QWidget *main = static_cast<QWidget*>(o);
                setShowMain(false);
                main->hide();
                return true;
            }
            break;
        case QEvent::WindowDeactivate:
            time(&m_inactiveTime);
            break;
        case QEvent::WindowActivate:
            m_inactiveTime = 0;
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
    switch (e->type()){
    case eEventInit:
        init();
        break;
    case eEventQuit:
        if (m_dock){
            delete m_dock;
            m_dock = NULL;
        }
        break;
    case eEventRaiseWindow: {
        EventRaiseWindow *w = static_cast<EventRaiseWindow*>(e);
        if (w->widget() == getMainWindow()){
            if (m_dock == NULL)
                init();
            if (!getShowMain())
                return (void*)1;
        }
        break;
    }
    case eEventCommandCreate: {
        EventCommandCreate *ecc = static_cast<EventCommandCreate*>(e);
        CommandDef *def = ecc->cmd();
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
            EventCommandCreate(&d).process();
        }
        break;
    }
    case eEventCheckState: {
        EventCheckState *ecs = static_cast<EventCheckState*>(e);
        CommandDef *def = ecs->cmd();
        if (def->id == CmdToggle){
            def->flags &= ~COMMAND_CHECKED;
            def->text = isMainShow() ?
                        I18N_NOOP("Hide main window") :
                        I18N_NOOP("Show main window");
            return (void*)1;
        }
        break;
    }
    case eEventCommandExec: {
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *def = ece->cmd();
        if (def->id == CmdToggle){
            QWidget *main = getMainWindow();
            if(!main) return NULL;
            if (isMainShow()){
                setShowMain(false);
                main->hide();
            }else{
                m_inactiveTime = 0;
                setShowMain(true);
                raiseWindow(main,getDesktop());
            }
            return (void*)1;
        }
        if (def->id == CmdCustomize){
            EventMenu(DockMenu, EventMenu::eCustomize).process();
            return (void*)1;
        }
        if (def->id == CmdQuit)
            m_bQuit = true;
        break;
    }
    default:
        break;
    }
    return NULL;
}

std::string DockPlugin::getConfig()
{
    return save_data(dockData, &data);
}

void DockPlugin::showPopup(QPoint p)
{
    if (m_popup)
        return;
    Command cmd;
    cmd->popup_id = DockMenu;
    EventMenuGet e(cmd);
    e.process();
    m_popup = e.menu();
    if (m_popup){
        m_popup->installEventFilter(this);
        m_popup->popup(p);
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

    EventCommandExec(cmd).process();
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

    if (m_core->unread.size())
        cmd->id = CmdUnread;

    EventCommandExec(cmd).process();
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
    if (!getAutoHide() || (m_inactiveTime == 0))  // no autohide
        return;
    if (m_main != getMainWindow()) {
        m_main = getMainWindow();
        m_main->installEventFilter(this);
    }
    if (time(NULL) > m_inactiveTime + (time_t)getAutoHideInterval()){
        if (m_main){
            setShowMain(false);
            m_main->hide();
        }
    }
}

#ifndef NO_MOC_INCLUDES
#include "dock.moc"
#endif
