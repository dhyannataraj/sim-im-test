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
#include "ontop.h"

#include <qapplication.h>
#include <qwidgetlist.h>
#include <qmessagebox.h>

#include <windows.h>

const unsigned short ABE_FLOAT   = (unsigned short)(-1);

static WinDockPlugin *dock = NULL;

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

static QWidget *pMain   = NULL;
static UINT WM_APPBAR   = 0;
static WNDPROC oldProc	= NULL;
static bool bAutoHideVisible = true;
static bool bFullScreen = false;
static bool bMoving = false;
static bool bSizing = false;

UINT appBarMessage(DWORD dwMessage, UINT uEdge=ABE_FLOAT, LPARAM lParam=0, QRect *rc=NULL)
{
    APPBARDATA abd;
    abd.cbSize           = sizeof(abd);
    abd.hWnd             = pMain->winId();
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

static unsigned short getEdge(RECT *rcWnd = NULL)
{
    RECT rc;
    if (rcWnd == NULL){
        GetWindowRect(pMain->winId(), &rc);
        rcWnd = &rc;
    }
    if (rcWnd->left <= 0) return ABE_LEFT;
    if (rcWnd->right >= GetSystemMetrics(SM_CXSCREEN)) return ABE_RIGHT;
    return ABE_FLOAT;
}

static void getBarRect(UINT state, QRect &rc, RECT *rcWnd = NULL)
{
    RECT rcWork;
    SystemParametersInfoA(SPI_GETWORKAREA, 0, &rcWork, 0);
    rc.setCoords(0, rcWork.top, GetSystemMetrics(SM_CXSCREEN), rcWork.bottom);
    appBarMessage(ABM_QUERYPOS, state, FALSE, &rc);
    int w;
    if (rcWnd){
        w = rcWnd->right - rcWnd->left;
    }else{
        GetWindowRect(pMain->winId(), &rcWork);
        w = rcWork.right - rcWork.left + GetSystemMetrics(SM_CXBORDER) * 2;
    }
    switch (state){
    case ABE_LEFT:
        rc.setRight(rc.left() + w);
        break;
    case ABE_RIGHT:
        rc.setLeft(rc.right() - w);
        break;
    }
}

const int SLIDE_INTERVAL = 400;

void slideWindow (const QRect &rcEnd, bool bAnimate)
{
    BOOL fFullDragOn;

    // Only slide the window if the user has FullDrag turned on
    SystemParametersInfoA(SPI_GETDRAGFULLWINDOWS, 0, &fFullDragOn, 0);

    // Get the current window position
    RECT rcWnd;
    GetWindowRect(pMain->winId(), &rcWnd);
    QRect rcStart;
    rcStart.setCoords(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom);

    if (bAnimate && fFullDragOn && (rcStart != rcEnd)) {

        // Get our starting and ending time.
        DWORD dwTimeStart = GetTickCount();
        DWORD dwTimeEnd = dwTimeStart + SLIDE_INTERVAL;
        DWORD dwTime;

        while ((dwTime = ::GetTickCount()) < dwTimeEnd) {
            int delta = (int)(dwTime - dwTimeStart);
            QRect rc = rcStart;
            rc.setLeft(rcStart.left() +
                       (rcEnd.left() - rcEnd.left()) * delta / SLIDE_INTERVAL);
            rc.setTop(rcStart.top() +
                      (rcEnd.top() - rcEnd.top()) * delta / SLIDE_INTERVAL);
            rc.setWidth(rcStart.width() +
                        (rcEnd.width() - rcEnd.width()) * delta / SLIDE_INTERVAL);
            rc.setHeight(rcStart.height() +
                         (rcEnd.height() - rcEnd.height()) * delta / SLIDE_INTERVAL);
            SetWindowPos(pMain->winId(), NULL,
                         rc.left(), rc.top(), rc.width(), rc.height(),
                         SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME);
            UpdateWindow(pMain->winId());
        }
    }
	Event e(EventInTaskManager, (void*)(dock->getState() == ABE_FLOAT));
	e.process();
    SetWindowPos(pMain->winId(), NULL,
                 rcEnd.left(), rcEnd.top(), rcEnd.width(), rcEnd.height(),
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME);
    UpdateWindow(pMain->winId());
}

void setBarState(bool bAnimate = false)
{
    if ((dock->getState() == ABE_FLOAT) || !pMain->isVisible()){
        appBarMessage(ABM_SETPOS, dock->getState(), FALSE);
    }else{
        if (dock->getAutoHide() && !appBarMessage(ABM_SETAUTOHIDEBAR, dock->getState(), TRUE, NULL)){
            dock->setAutoHide(false);
            QMessageBox::warning(NULL, i18n("Error"),
                                 i18n("There is already an auto hidden window on this edge.\nOnly one auto hidden window is allowed on each edge."),
                                 QMessageBox::Ok, 0);
        }
        QRect rc;
        getBarRect(dock->getState(), rc);
        if (dock->getAutoHide()){
            QRect rcAutoHide = rc;
            int w = 4 * GetSystemMetrics(SM_CXBORDER);
            if (dock->getState() == ABE_LEFT){
                rcAutoHide.setRight(rcAutoHide.left() + w);
            }else{
                rcAutoHide.setLeft(rcAutoHide.right() - w);
            }
            appBarMessage(ABM_SETPOS, dock->getState(), FALSE, &rcAutoHide);
            if (!bAutoHideVisible)
                rc = rcAutoHide;
        }else{
            appBarMessage(ABM_SETPOS, dock->getState(), FALSE, &rc);
        }
        slideWindow(rc, bAnimate);
    }
    if (pMain->isVisible()){
		Event eOnTop(EventOnTop, (void*)bFullScreen);
		eOnTop.process();
        if (!bFullScreen)
            appBarMessage(ABM_ACTIVATE);
    }
}

LRESULT CALLBACK dockWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT res;
    if (msg == WM_APPBAR){
        switch (wParam){
        case ABN_FULLSCREENAPP:
            bFullScreen = (lParam != 0);
            setBarState();
            break;
        case ABN_POSCHANGED:
            if (dock->getState() != ABE_FLOAT)
                setBarState();
            break;
        }
    }
	unsigned type;
    RECT  *prc;
	RECT  rcWnd;
	QRect rc;
	switch (msg){
    case WM_ACTIVATE:
        if ((wParam == WA_INACTIVE) && dock->getAutoHide() && bAutoHideVisible){
            bAutoHideVisible = false;
            setBarState();
        }
        appBarMessage(ABM_ACTIVATE);
        break;
    case WM_NCMOUSEMOVE:
        if ((dock->getState() != ABE_FLOAT) && dock->getAutoHide() && !bAutoHideVisible){
            bAutoHideVisible = true;
            setBarState(true);
        }
		break;
    case WM_ENTERSIZEMOVE:
        bMoving = true;
		bSizing = true;
		if (dock->getState() == ABE_FLOAT){
			bSizing = false;
			GetWindowRect(hWnd, &rcWnd);
			dock->setHeight(rcWnd.bottom - rcWnd.top);
		}
        return DefWindowProc(hWnd, msg, wParam, lParam);
    case WM_EXITSIZEMOVE:
		bMoving = false;
		dock->setState(getEdge());
        setBarState(true);
        return DefWindowProc(hWnd, msg, wParam, lParam);
    case WM_MOVING:
    case WM_SIZING:
        if (!bMoving) break;
        prc = (RECT*)lParam;
        type = getEdge(prc);
        if (type == ABE_FLOAT){
            if (bSizing){
                prc->bottom = prc->top + dock->getHeight();
                bSizing = false;
            }
        }else{
            getBarRect(type, rc, prc);
            prc->left = rc.left();
            prc->top = rc.top();
            prc->right = rc.right();
            prc->bottom = rc.bottom();
			bSizing = true;
        }
        return 1;
    case WM_WINDOWPOSCHANGED:
        res = oldProc(hWnd, msg, wParam, lParam);
        appBarMessage(ABM_WINDOWPOSCHANGED);
        return res;
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
		{ "State", DATA_ULONG, 1, ABE_FLOAT },
		{ "Height", DATA_ULONG, 1, 0 },
        { NULL, 0, 0, 0 }
    };

WinDockPlugin::WinDockPlugin(unsigned base, const char *config)
        : Plugin(base), EventReceiver(DefaultPriority - 1)
{
	dock = this;

    load_data(winDockData, &data, config);

    CmdAutoHide = registerType();

    Command cmd;
    cmd->id          = CmdAutoHide;
    cmd->text        = I18N_NOOP("AutoHide");
    cmd->menu_id     = MenuMain;
    cmd->menu_grp    = 0x7001;
    cmd->flags		= COMMAND_CHECK_STATE;

	m_bInit = false;

    Event eCmd(EventCommandCreate, cmd);
    eCmd.process();
    WM_APPBAR = RegisterWindowMessageA("AppBarNotify");
	init();
}

WinDockPlugin::~WinDockPlugin()
{
    Event eCmd(EventCommandRemove, (void*)CmdAutoHide);
    eCmd.process();
    QWidget *main = getMainWindow();
    if (main && oldProc){
        appBarMessage(ABM_REMOVE);
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
			dock->setAutoHide(cmd->flags & COMMAND_CHECKED);
			bAutoHideVisible = true;
			setBarState();
            return cmd;
        }
    }
    if (e->type() == EventCheckState){
        CommandDef *cmd = (CommandDef*)(e->param());
        if ((cmd->id == CmdAutoHide) && (dock->getState() != ABE_FLOAT)){
            cmd->flags &= ~COMMAND_CHECKED;
            if (dock->getAutoHide())
                cmd->flags |= COMMAND_CHECKED;
            return cmd;
        }
    }
	if ((e->type() == EventInit) && !m_bInit)
		init();
	if (e->type() == EventInTaskManager){
		if ((dock->getState() != ABE_FLOAT) && e->param()){
			Event eMy(EventInTaskManager, (void*)false);
			eMy.process();
			return e->param();
		}
	}
    return NULL;
}

void WinDockPlugin::init()
{
    pMain = getMainWindow();
    if (pMain){
        if (IsWindowUnicode(pMain->winId())){
            oldProc = (WNDPROC)SetWindowLongW(pMain->winId(), GWL_WNDPROC, (LONG)dockWndProc);
            if (oldProc == 0)
                oldProc = (WNDPROC)SetWindowLongA(pMain->winId(), GWL_WNDPROC, (LONG)dockWndProc);
        }
        appBarMessage(ABM_NEW);
		m_bInit = true;
		setBarState();
    }
}

string WinDockPlugin::getConfig()
{
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


