/***************************************************************************
                          windock.cpp  -  description
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

#include "windock.h"
#include "simapi.h"

#include <qapplication.h>
#include <qwidgetlist.h>

#include <windows.h>

const unsigned short ABE_FLOAT   = (unsigned short)(-1);

Plugin *createWinDockPlugin(unsigned base, bool, const char *config)
{
    Plugin *plugin = new WinDockPlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Dock"),
        I18N_NOOP("Plugin provides dock main window to left or right side of screen"),
        VERSION,
        createWinDockPlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static UINT WM_APPBAR   = 0;
static WNDPROC oldProc	= NULL;
static bool bAutoHide	= false;
static UINT state		= ABE_FLOAT;
static bool bFullScreen = false;

UINT appBarMessage(HWND hWnd, DWORD dwMessage, UINT uEdge=ABE_FLOAT, LPARAM lParam=0, QRect *rc=NULL)
{
    APPBARDATA abd;
    abd.cbSize           = sizeof(abd);
    abd.hWnd             = hWnd;
    abd.uCallbackMessage = WM_APPBAR;
    abd.uEdge            = uEdge;
    if (rc){
        abd.rc.left = rc->left();
        abd.rc.top = rc->top();
        abd.rc.right = rc->right();
        abd.rc.bottom = rc->bottom();
    }else{
        abd.rc.left = 0;
        abd.rc.top = 0;
        abd.rc.right = 0;
        abd.rc.bottom = 0;
    }
    abd.lParam           = lParam;
    UINT uRetVal         = SHAppBarMessage(dwMessage, &abd);

    if (rc != NULL)
        rc->setCoords(abd.rc.left, abd.rc.top, abd.rc.right, abd.rc.bottom);
    return uRetVal;
}

LRESULT CALLBACK dockWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_APPBAR){
        switch (wParam){
        case ABN_FULLSCREENAPP:
            bFullScreen = (lParam != 0);
            //            setBarState();
            break;
        }
    }
    return oldProc(hWnd, msg, wParam, lParam);
}

/*
typedef struct WinDockData
{
    bool AutoHide;
} WinDocData;
*/
static DataDef winDockData[] =
    {
        { "AutoHide", DATA_BOOL, 1, 0 },
        { NULL, 0, 0, 0 }
    };

WinDockPlugin::WinDockPlugin(unsigned base, const char *config)
        : Plugin(base)
{
    load_data(winDockData, &data, config);
    bAutoHide = getAutoHide();

    CmdAutoHide = registerType();

    Command cmd;
    cmd->id          = CmdAutoHide;
    cmd->text        = I18N_NOOP("AutoHide");
    cmd->menu_id     = MenuMain;
    cmd->menu_grp    = 0x7001;
    cmd->flags		= COMMAND_CHECK_STATE;

    Event eCmd(EventCommandCreate, cmd);
    eCmd.process();
    WM_APPBAR = RegisterWindowMessageA("AppBarNotify");
    QWidget *main = getMainWindow();
    if (main){
        if (IsWindowUnicode(main->winId())){
            oldProc = (WNDPROC)SetWindowLongW(main->winId(), GWL_WNDPROC, (LONG)dockWndProc);
            if (oldProc == 0)
                oldProc = (WNDPROC)SetWindowLongA(main->winId(), GWL_WNDPROC, (LONG)dockWndProc);
        }
        appBarMessage(main->winId(), ABM_NEW);
    }
}

WinDockPlugin::~WinDockPlugin()
{
    Event eCmd(EventCommandRemove, (void*)CmdAutoHide);
    eCmd.process();
    QWidget *main = getMainWindow();
    if (main && oldProc){
        appBarMessage(main->winId(), ABM_REMOVE);
        WNDPROC p;
        p = (WNDPROC)SetWindowLongW(main->winId(), GWL_WNDPROC, (LONG)oldProc);
        if (p == 0)
            p = (WNDPROC)SetWindowLongA(main->winId(), GWL_WNDPROC, (LONG)oldProc);
    }
    free_data(winDockData, &data);
}

void *WinDockPlugin::processEvent(Event *e)
{
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->id == CmdAutoHide){
            return cmd;
        }
    }
    if (e->type() == EventCheckState){
        CommandDef *cmd = (CommandDef*)(e->param());
        if ((cmd->id == CmdAutoHide) && (state != ABE_FLOAT)){
            cmd->flags &= ~COMMAND_CHECKED;
            if (bAutoHide)
                cmd->flags |= COMMAND_CHECKED;
            return cmd;
        }
    }
    return NULL;
}

string WinDockPlugin::getConfig()
{
    setAutoHide(bAutoHide);
    return save_data(winDockData, &data);
}

QWidget *WinDockPlugin::getMainWindow()
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


