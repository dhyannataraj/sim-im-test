/***************************************************************************
                          mainwin.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mainwin.h"
#include "client.h"
#include "cuser.h"
#include "toolbtn.h"
#include "userview.h"
#include "icons.h"
#include "dock.h"
#include "log.h"
#include "userbox.h"
#include "themes.h"
#include "xosd.h"
#include "monitor.h"
#include "ui/logindlg.h"
#include "ui/proxydlg.h"
#include "transparent.h"
#include "ui/searchdlg.h"
#include "ui/setupdlg.h"
#include "ui/autoreply.h"
#include "ui/userautoreply.h"
#include "ui/alertmsg.h"
#include "ui/ballonmsg.h"
#include "ui/filetransfer.h"
#include "ui/userinfo.h"
#include "ui/securedlg.h"
#include "ui/randomchat.h"

#include "ui/enable.h"
#include "chatwnd.h"
#include "about.h"
#include "splash.h"
#include "keys.h"
#include "history.h"
#include "sim.h"

#ifndef _WINDOWS
#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#endif

#ifdef WIN32
#include <winuser.h>
#include <windowsx.h>
#include <shellapi.h>
#include <mmsystem.h>
#include <shlobj.h>
#include <winreg.h>

typedef struct tagLASTINPUTINFO {
    UINT cbSize;
    DWORD dwTime;
} LASTINPUTINFO, * PLASTINPUTINFO;

static BOOL (WINAPI * _GetLastInputInfo)(PLASTINPUTINFO);

#endif

#include <errno.h>
#include <fcntl.h>
#include <time.h>

#include <qframe.h>
#include <qlayout.h>
#include <qtoolbar.h>
#include <qapplication.h>
#include <qfile.h>
#include <qstyle.h>
#include <qwidgetlist.h>
#include <qobjectlist.h>
#include <qtranslator.h>
#include <qregexp.h>
#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qaccel.h>
#include <qfile.h>
#include <qmessagebox.h>

#ifdef USE_KDE
#include <kwin.h>
#include <kwinmodule.h>
#include <kpopupmenu.h>
#include <kaudioplayer.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kapp.h>
#include <kipc.h>
#include <kaboutapplication.h>
#include <kaboutkde.h>
#include <kabcsync.h>
#else
#include "ui/kpopup.h"
#endif

#ifdef USE_SCRNSAVER
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/scrnsaver.h>
#endif

#ifdef WIN32
const unsigned short ABE_FLOAT   = (unsigned short)(-1);
#endif

const char *MainWindow::sound(const char *wav)
{
    if (wav == NULL) return "";
    if (*wav == 0) return wav;
#ifdef WIN32
    if ((strlen(wav) > 3) &&
            (((wav[1] == ':') && (wav[2] == '\\')) ||
             ((wav[0] == '\\') && (wav[1] == '\\'))))
        return wav;
#else
    if (*wav == '/') return wav;
#endif
    string s = "sounds";
#ifdef WIN32
    s += "\\";
#else
    s += "/";
#endif
    s += wav;
    return app_file(s.c_str());
}

cfgParam MainWindow_Params[] =
    {
        { "Show", offsetof(MainWindow_Data, ShowState), PARAM_BOOL, (unsigned)true },
        { "OnTop", offsetof(MainWindow_Data, OnTop), PARAM_BOOL, (unsigned)true },
        { "ShowOffline", offsetof(MainWindow_Data, ShowOffline), PARAM_BOOL, (unsigned)true },
        { "GroupMode", offsetof(MainWindow_Data, GroupMode), PARAM_BOOL, (unsigned)true },
        { "Left", offsetof(MainWindow_Data, Left), PARAM_SHORT, 0 },
        { "Top", offsetof(MainWindow_Data, Top), PARAM_SHORT, 0 },
        { "Width", offsetof(MainWindow_Data, Width), PARAM_USHORT, 0 },
        { "Height", offsetof(MainWindow_Data, Height), PARAM_USHORT, 0 },
        { "Style", offsetof(MainWindow_Data, UseStyle), PARAM_CHARS, 0 },
        { "AutoAwayTime", offsetof(MainWindow_Data, AutoAwayTime), PARAM_ULONG, 300 },
        { "AutoNATime", offsetof(MainWindow_Data, AutoNATime), PARAM_ULONG, 900 },
        { "AlertAway", offsetof(MainWindow_Data, NoAlertAway), PARAM_BOOL, (unsigned)true },
        { "ManualStatus", offsetof(MainWindow_Data, ManualStatus), PARAM_ULONG, ICQ_STATUS_OFFLINE },
        { "DivPos", offsetof(MainWindow_Data, DivPos), PARAM_SHORT, 0 },
        { "SpellOnSend", offsetof(MainWindow_Data, SpellOnSend), PARAM_BOOL, 0 },
        { "ToolbarDock", offsetof(MainWindow_Data, ToolbarDock), PARAM_CHARS, (unsigned)"Top" },
        { "ToolbarOffset", offsetof(MainWindow_Data, ToolbarOffset), PARAM_SHORT, 0 },
        { "ToolbarY", offsetof(MainWindow_Data, ToolbarY), PARAM_SHORT, 0 },
        { "URLViewer", offsetof(MainWindow_Data, UrlViewer), PARAM_CHARS, (unsigned)
#ifdef USE_KDE
          "konqueror"
#else
          "netscape"
#endif
        },
        { "MailClient", offsetof(MainWindow_Data, MailClient), PARAM_CHARS, (unsigned)
#ifdef USE_KDE
          "kmail"
#else
          "netscape mailto:%s"
#endif
        },
        { "TransparentMain", offsetof(MainWindow_Data, UseTransparent), PARAM_BOOL, 0 },
        { "TransparencyMain", offsetof(MainWindow_Data, Transparent), PARAM_ULONG, 80 },
        { "TransparentContainer", offsetof(MainWindow_Data, UseTransparentContainer), PARAM_BOOL, 0 },
        { "TransparencyContainer", offsetof(MainWindow_Data, TransparentContainer), PARAM_ULONG, 80 },
        { "TransparentIfInactive", offsetof(MainWindow_Data, TransparentIfInactive), PARAM_BOOL, (unsigned)true },
        { "NoShowAway", offsetof(MainWindow_Data, NoShowAway), PARAM_BOOL, 0 },
        { "NoShowNA", offsetof(MainWindow_Data, NoShowNA), PARAM_BOOL, 0 },
        { "NoShowOccupied", offsetof(MainWindow_Data, NoShowOccupied), PARAM_BOOL, 0 },
        { "NoShowDND", offsetof(MainWindow_Data, NoShowDND), PARAM_BOOL, 0 },
        { "NoShowFFC", offsetof(MainWindow_Data, NoShowFFC), PARAM_BOOL, 0 },
        { "UseSystemFonts", offsetof(MainWindow_Data, UseSystemFonts), PARAM_BOOL, 0 },
        { "Font", offsetof(MainWindow_Data, Font), PARAM_CHARS, 0 },
        { "FontMenu", offsetof(MainWindow_Data, FontMenu), PARAM_CHARS, 0 },
        { "ColorSend", offsetof(MainWindow_Data, ColorSend), PARAM_ULONG, 0x0000B0 },
        { "ColorReceive", offsetof(MainWindow_Data, ColorReceive), PARAM_ULONG, 0xB00000 },
        { "ChatWidth", offsetof(MainWindow_Data, ChatWidth), PARAM_USHORT, 0 },
        { "ChatHeight", offsetof(MainWindow_Data, ChatHeight), PARAM_USHORT, 0 },
        { "UserBoxX", offsetof(MainWindow_Data, UserBoxX), PARAM_USHORT, 0 },
        { "UserBoxY", offsetof(MainWindow_Data, UserBoxY), PARAM_USHORT, 0 },
        { "UserBoxWidth", offsetof(MainWindow_Data, UserBoxWidth), PARAM_SHORT, 0 },
        { "UserBoxHeight", offsetof(MainWindow_Data, UserBoxHeight), PARAM_SHORT, 0 },
        { "UserBoxToolbarDock", offsetof(MainWindow_Data, UserBoxToolbarDock), PARAM_CHARS, (unsigned)"Top" },
        { "UserBoxToolbarOffset", offsetof(MainWindow_Data, UserBoxToolbarOffset), PARAM_SHORT, 0 },
        { "UserBoxToolbarY", offsetof(MainWindow_Data, UserBoxToolbarY), PARAM_SHORT, 0 },
        { "UserBoxFont", offsetof(MainWindow_Data, UserBoxFont), PARAM_CHARS, 0 },
        { "UserBoxEditHeight", offsetof(MainWindow_Data, UserBoxEditHeight), PARAM_USHORT, 0 },
        { "CloaseAfterSend", offsetof(MainWindow_Data, CloseAfterSend), PARAM_BOOL, 0 },
        { "CloaseAfterFileTransfer", offsetof(MainWindow_Data, CloseAfterFileTransfer), PARAM_BOOL, 0 },
        { "MainWindowInTaskManager", offsetof(MainWindow_Data, MainWindowInTaskManager), PARAM_BOOL, 0 },
        { "UserWindowInTaskManager", offsetof(MainWindow_Data, UserWindowInTaskManager), PARAM_BOOL, (unsigned)true },
        { "Icons", offsetof(MainWindow_Data, Icons), PARAM_CHARS, 0 },
        { "XOSD_on", offsetof(MainWindow_Data, XOSD_on), PARAM_BOOL, (unsigned)true },
        { "XOSD_pos", offsetof(MainWindow_Data, XOSD_pos), PARAM_SHORT, 0 },
        { "XOSD_offset", offsetof(MainWindow_Data, XOSD_offset), PARAM_SHORT, 30 },
        { "XOSD_color", offsetof(MainWindow_Data, XOSD_color), PARAM_ULONG, 0x00E000 },
        { "XOSD_Font", offsetof(MainWindow_Data, XOSD_Font), PARAM_CHARS, 0 },
        { "XOSD_timeout", offsetof(MainWindow_Data, XOSD_timeout), PARAM_USHORT, 7 },
        { "XOSD_Shadow", offsetof(MainWindow_Data, XOSD_Shadow), PARAM_BOOL, (unsigned)true },
        { "XOSD_Background", offsetof(MainWindow_Data, XOSD_Background), PARAM_BOOL, 0 },
        { "XOSD_Bgcolor", offsetof(MainWindow_Data, XOSD_BgColor), PARAM_ULONG, (unsigned)(-1) },
        { "ContainerMode", offsetof(MainWindow_Data, ContainerMode), PARAM_SHORT, ContainerModeGroup },
        { "MessageBgColor", offsetof(MainWindow_Data, MessageBgColor), PARAM_ULONG, 0 },
        { "MessageFgColor", offsetof(MainWindow_Data, MessageFgColor), PARAM_ULONG, 0 },
        { "SimpleMode", offsetof(MainWindow_Data, SimpleMode), PARAM_BOOL, 0 },
        { "UseOwnColors", offsetof(MainWindow_Data, UseOwnColors), PARAM_BOOL, 0 },
        { "UserWndOnTop", offsetof(MainWindow_Data, UserWndOnTop), PARAM_BOOL, 0 },
        { "KeyWindow", offsetof(MainWindow_Data, KeyWindow), PARAM_CHARS, (unsigned)"CTRL-SHIFT-A" },
        { "KeyDblClick", offsetof(MainWindow_Data, KeyDblClick), PARAM_CHARS, (unsigned)"CTRL-SHIFT-I" },
        { "KeySearch", offsetof(MainWindow_Data, KeySearch), PARAM_CHARS, (unsigned)"CTRL-SHIFT-S" },
        { "UseEmotional", offsetof(MainWindow_Data, UseEmotional), PARAM_BOOL, (unsigned)true },
        { "AutoHideTime", offsetof(MainWindow_Data, AutoHideTime), PARAM_ULONG, 60 },
        { "SMSSignTop", offsetof(MainWindow_Data, SMSSignTop), PARAM_CHARS, 0 },
        { "SMSSignBottom", offsetof(MainWindow_Data, SMSSignBottom), PARAM_CHARS, (unsigned)"\n&MyAlias; (ICQ# &MyUin;)" },
        { "ForwardPhone", offsetof(MainWindow_Data, ForwardPhone), PARAM_CHARS, 0 },
        { "SendEnter", offsetof(MainWindow_Data, SendEnter), PARAM_BOOL, 0 },
        { "AlphabetSort", offsetof(MainWindow_Data, AlphabetSort), PARAM_BOOL, 0 },
        { "UseDoubleClick", offsetof(MainWindow_Data, UseDoubleClick), PARAM_BOOL, 0 },
        { "UseDock", offsetof(MainWindow_Data, UseDock), PARAM_BOOL, 1 },
        { "DockX", offsetof(MainWindow_Data, DockX), PARAM_SHORT, 0 },
        { "DockY", offsetof(MainWindow_Data, DockY), PARAM_SHORT, 0 },
        { "MonitorX", offsetof(MainWindow_Data, MonitorX), PARAM_SHORT, 0 },
        { "MonitorY", offsetof(MainWindow_Data, MonitorY), PARAM_SHORT, 0 },
        { "MonitorWidth", offsetof(MainWindow_Data, MonitorWidth), PARAM_USHORT, 0 },
        { "MonitorHeight", offsetof(MainWindow_Data, MonitorHeight), PARAM_USHORT, 0 },
        { "MonitorLevel", offsetof(MainWindow_Data, MonitorLevel), PARAM_USHORT, L_PACKET | L_DEBUG | L_WARN | L_ERROR },
        { "CopyMessages", offsetof(MainWindow_Data, CopyMessages), PARAM_USHORT, 3 },
        { "AllEncodings", offsetof(MainWindow_Data, AllEncodings), PARAM_BOOL, 0 },
        { "HistoryDirection", offsetof(MainWindow_Data, HistoryDirection), PARAM_BOOL, 1 },
        { "ToolBarMain", offsetof(MainWindow_Data, ToolBarMain), PARAM_LIST, 0 },
        { "ToolBarMsg", offsetof(MainWindow_Data, ToolBarMsg), PARAM_LIST, 0 },
        { "ToolBarHistory", offsetof(MainWindow_Data, ToolBarHistory), PARAM_LIST, 0 },
        { "ToolBarChat", offsetof(MainWindow_Data, ToolBarChat), PARAM_LIST, 0 },
        { "ToolBarUserBox", offsetof(MainWindow_Data, ToolBarUserBox), PARAM_LIST, 0 },
        { "BackgroundFile", offsetof(MainWindow_Data, BackgroundFile), PARAM_CHARS, 0 },
        { "BackgroundMode", offsetof(MainWindow_Data, BackgroundMode), PARAM_USHORT, 0 },
        { "IconMargin", offsetof(MainWindow_Data, IconMargin), PARAM_USHORT, 2 },
        { "UseSystemColors", offsetof(MainWindow_Data, UseSystemColors), PARAM_BOOL, 1 },
        { "OnlineColor", offsetof(MainWindow_Data, OnlineColor), PARAM_ULONG, 0 },
        { "OfflineColor", offsetof(MainWindow_Data, OfflineColor), PARAM_ULONG, 0 },
#ifdef WIN32
        { "BarState", offsetof(MainWindow_Data, BarState), PARAM_USHORT, ABE_FLOAT },
        { "BarAutoHide", offsetof(MainWindow_Data, BarAutoHide), PARAM_BOOL, 0 },
#endif
#ifdef USE_KDE
        { "AutoSync", offsetof(MainWindow_Data, AutoSync), PARAM_BOOL, 0 },
#endif
        { "ChatGroup", offsetof(MainWindow_Data, ChatGroup), PARAM_USHORT, 1 },
        { "", 0, 0, 0 }
    };

#ifndef WIN32

struct sigaction oldChildAct;

static void child_proc(int sig)
{
    if (pMain)
        QTimer::singleShot(0, pMain, SLOT(checkChilds()));
    if (oldChildAct.sa_handler)
        oldChildAct.sa_handler(sig);
}

#endif

#ifdef WIN32

// Bar functions

static UINT WM_APPBAR = 0;

UINT appBarMessage(DWORD dwMessage, UINT uEdge = ABE_FLOAT, LPARAM lParam = 0, QRect *rc = NULL)
{

    // Initialize an APPBARDATA structure.
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

    // If the caller passed a rectangle, return the updated rectangle.
    if (rc != NULL)
        rc->setCoords(abd.rc.left, abd.rc.top, abd.rc.right, abd.rc.bottom);
    return uRetVal;
}

static bool bInMoving = false;
bool bFullScreen = false;
static bool bOnTop = false;
static bool bAutoHideVisible = false;
static int  lastHeight = 0;
static int  lastTop = 0;

void getBarRect(UINT state, QRect &rc, RECT *rcWnd = NULL)
{
    RECT rcWork;
    SystemParametersInfoA(SPI_GETWORKAREA, 0, &rcWork, 0);
    rc.setCoords(0, rcWork.top, GetSystemMetrics(SM_CXSCREEN), rcWork.bottom);
    appBarMessage(ABM_QUERYPOS, state, FALSE, &rc);
    int w;
    if (rcWnd){
        w = rcWnd->right - rcWnd->left;
    }else{
        w = pMain->getWidth() + GetSystemMetrics(SM_CXBORDER) * 2;
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

void getNeedBarRect(UINT state, QRect &rc)
{
    getBarRect(state, rc);
    if (pMain->isBarAutoHide() && !bAutoHideVisible){
        int w = 4 * GetSystemMetrics(SM_CXBORDER);
        if (pMain->getBarState() == ABE_LEFT){
            rc.setRight(rc.left() + w);
        }else{
            rc.setLeft(rc.right() - w);
        }
    }
}

bool bInBar = true;

void setInBar(bool bState)
{
    if (bState == bInBar) return;
    bInBar = bState;
    bool bVisible = pMain->isVisible();
    if (bVisible) pMain->hide();
    if (bState){
        SetWindowLongW(pMain->winId(), GWL_EXSTYLE, (GetWindowLongW(pMain->winId(), GWL_EXSTYLE) | WS_EX_APPWINDOW) & (~WS_EX_TOOLWINDOW));
    }else{
        SetWindowLongW(pMain->winId(), GWL_EXSTYLE, (GetWindowLongW(pMain->winId(), GWL_EXSTYLE) & ~(WS_EX_APPWINDOW)) | WS_EX_TOOLWINDOW);
    }
    if (bVisible) pMain->show();
}

const int SLIDE_INTERVAL = 400;

unsigned short getEdge(RECT *rcWnd = NULL)
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
    setInBar(pMain->isMainWindowInTaskManager() && (pMain->getBarState() == ABE_FLOAT));
    SetWindowPos(pMain->winId(), NULL,
                 rcEnd.left(), rcEnd.top(), rcEnd.width(), rcEnd.height(),
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME);
    UpdateWindow(pMain->winId());
}


void setBarState(bool bAnimate = false)
{
    if ((pMain->getBarState() == ABE_FLOAT) || !pMain->isShowState()){
        appBarMessage(ABM_SETPOS, pMain->getBarState(), FALSE);
    }else{
        if (pMain->isBarAutoHide() && !appBarMessage(ABM_SETAUTOHIDEBAR, pMain->getBarState(), TRUE, NULL)){
            pMain->setBarAutoHide(false);
            QMessageBox::warning(NULL, i18n("Error"),
                                 i18n("There is already an auto hidden window on this edge.\nOnly one auto hidden window is allowed on each edge."),
                                 QMessageBox::Ok, 0);
        }
        QRect rc;
        getBarRect(pMain->getBarState(), rc);
        if (pMain->isBarAutoHide()){
            QRect rcAutoHide = rc;
            int w = 4 * GetSystemMetrics(SM_CXBORDER);
            if (pMain->getBarState() == ABE_LEFT){
                rcAutoHide.setRight(rcAutoHide.left() + w);
            }else{
                rcAutoHide.setLeft(rcAutoHide.right() - w);
            }
            appBarMessage(ABM_SETPOS, pMain->getBarState(), FALSE, &rcAutoHide);
            if (!bAutoHideVisible)
                rc = rcAutoHide;
        }else{
            appBarMessage(ABM_SETPOS, pMain->getBarState(), FALSE, &rc);
        }
        slideWindow(rc, bAnimate);
    }
    if (pMain->isShowState()){
        if ((bOnTop != pMain->isOnTop()) || bFullScreen){
            bOnTop = pMain->isOnTop();
            HWND hState = HWND_NOTOPMOST;
            if (pMain->isOnTop()) hState = HWND_TOPMOST;
            if (bFullScreen) hState = HWND_BOTTOM;
            SetWindowPos(pMain->winId(), hState, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }
        if (!bFullScreen)
            appBarMessage(ABM_ACTIVATE);
    }
}

static bool bSizing = false;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    unsigned type;
    RECT  *prc;
    RECT rcWnd;
    LRESULT res;
    WINDOWPOS *wPos;
    QRect rc;
    int sLastHeight;
    if (msg == WM_APPBAR){
        switch (wParam){
        case ABN_FULLSCREENAPP:
            bFullScreen = (lParam != 0);
            setBarState();
            break;
        case ABN_POSCHANGED:
            if (pMain->getBarState() != ABE_FLOAT)
                setBarState();
            break;
        }
        return 0;
    }
    switch (msg){
    case WM_ACTIVATE:
        if ((wParam == WA_INACTIVE) && pMain->isBarAutoHide() && bAutoHideVisible){
            bAutoHideVisible = false;
            setBarState();
        }
        appBarMessage(ABM_ACTIVATE);
        break;
    case WM_ENTERSIZEMOVE:
        bInMoving = true;
        pMain->setWidth(pMain->size().width());
        bSizing = true;
        if (pMain->getBarState() != ABE_FLOAT) break;
        bSizing = false;
        pMain->setLeft(pMain->pos().x());
        pMain->setTop(pMain->pos().y());
        pMain->setHeight(pMain->size().height());
        return DefWindowProc(hWnd, msg, wParam, lParam);
    case WM_EXITSIZEMOVE:
        sLastHeight = lastHeight;
        res = DefWindowProc(hWnd, msg, wParam, lParam);
        bInMoving = false;
        GetWindowRect(pMain->winId(), &rcWnd);
        pMain->setWidth(rcWnd.right - rcWnd.left - GetSystemMetrics(SM_CXBORDER) * 2);
        pMain->setBarState(getEdge());
        setBarState(true);
        lastHeight = sLastHeight;
        return res;
    case WM_NCMOUSEMOVE:
        if ((pMain->getBarState() != ABE_FLOAT) && pMain->isBarAutoHide() && !bAutoHideVisible){
            bAutoHideVisible = true;
            setBarState(true);
        }
        return WndProc(hWnd, msg, wParam, lParam);
    case WM_WINDOWPOSCHANGING:
        wPos = (WINDOWPOS*)lParam;
        if (bInMoving || ((wPos->flags & SWP_NOMOVE) && (wPos->flags & SWP_NOSIZE))) break;
        switch (pMain->getBarState()){
        case ABE_LEFT:
            getNeedBarRect(pMain->getBarState(), rc);
            if (rc.left() != wPos->x){
                wPos->x = rc.left();
                wPos->y = rc.top();
                wPos->cx = rc.width();
                wPos->cy = rc.height();
                break;
            }
            break;
        case ABE_RIGHT:
            getNeedBarRect(pMain->getBarState(), rc);
            if (rc.right() != wPos->x + wPos->cx + GetSystemMetrics(SM_CXBORDER)){
                wPos->x = rc.left();
                wPos->y = rc.top();
                wPos->cx = rc.width();
                wPos->cy = rc.height();
                break;
            }
            break;
        case ABE_FLOAT:
            if (lastHeight && (wPos->cy != lastHeight)){
                wPos->y = lastTop;
                wPos->cy = lastHeight;
            }
            break;
        }
        lastHeight = 0;
        return WndProc(hWnd, msg, wParam, lParam);
    case WM_WINDOWPOSCHANGED:
        res = WndProc(hWnd, msg, wParam, lParam);
        appBarMessage(ABM_WINDOWPOSCHANGED);
        return res;
    case WM_MOVING:
    case WM_SIZING:
        if (!bInMoving) break;
        prc = (RECT*)lParam;
        type = getEdge(prc);
        if (type == ABE_FLOAT){
            if (bSizing){
                prc->bottom = prc->top + pMain->getHeight();
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
        lastTop    = prc->top;
        lastHeight = prc->bottom - prc->top;
        return 1;
    case WM_ENDSESSION:
        if (pMain) pMain->saveState();
        break;
    }
    return WndProc(hWnd, msg, wParam, lParam);
}

#endif

const int btnShowOffline	= 1;
const int btnGroupMode		= 2;
const int btnStatus			= 3;
const int btnSetup			= 4;
const int btnFind			= 5;
const int btnNoIM			= 6;
const int btnSMS			= 7;
const int btnConfigure		= 8;
const int btnRandomChat		= 9;
const int btnMonitor		= 10;

ToolBarDef mainWndToolBar[] =
    {
        { btnShowOffline, "online_on", "online_off", I18N_NOOP("Show &offline"), BTN_TOGGLE, SLOT(toggleShowOffline()), NULL },
        { btnGroupMode, "grp_off", "grp_on", I18N_NOOP("&Group mode"), BTN_TOGGLE, SLOT(toggleGroupMode()), SLOT(showGroupPopup(QPoint)) },
        SEPARATOR,
        { btnStatus, "online", NULL, I18N_NOOP("Status"), BTN_PICT, NULL, NULL },
        { btnSetup, "2downarrow", NULL, I18N_NOOP("&Menu"), 0, NULL, NULL },
        END_DEF,
        { btnFind, "find", NULL, I18N_NOOP("Find User"), 0, SLOT(search()), NULL },
        { btnNoIM, "nonim", NULL, I18N_NOOP("Add Non-IM contact"), 0, SLOT(addNonIM()), NULL },
        { btnSMS, "sms", NULL, I18N_NOOP("Send SMS"), 0, SLOT(sendSMS()), NULL },
        { btnConfigure, "configure", NULL, I18N_NOOP("Setup"), 0, SLOT(setup()), NULL },
        { btnRandomChat, "randomchat", NULL, I18N_NOOP("Random chat"), 0, SLOT(randomChat()), NULL },
        { btnMonitor, "network", NULL, I18N_NOOP("Network monitor"), 0, SLOT(networkMonitor()), NULL },
        END_DEF
    };

const ToolBarDef *pToolBarMain = mainWndToolBar;

MainWindow::MainWindow(const char *name)
        : QMainWindow(NULL, name, WType_TopLevel | WStyle_Customize | WStyle_Title | WStyle_NormalBorder| WStyle_SysMenu)
{
    SET_WNDPROC("mainwnd")
    ::init(&data, MainWindow_Params);

    bLocked = false;
    pMain = this;
    bQuit = false;
    dock = NULL;
    noToggle = false;
    lockFile = -1;
    translator = NULL;
    mAboutApp = NULL;
    time_t now;
    time(&now);
    inactiveTime = now;
    mNetMonitor = NULL;
    mRandomChat = NULL;

#ifdef HAVE_UMASK
    umask(0077);
#endif

    initTranslator();

#ifdef USE_KDE
    mAboutKDE = NULL;
#endif

    int tz;
#ifndef HAVE_TM_GMTOFF
    tz = - _timezone;
#else
    struct tm *tm = localtime(&now);
    tz = tm->tm_gmtoff;
    if (tm->tm_isdst) tz -= (60 * 60);
#endif
    realTZ = - tz / (30 * 60);

    menuUser = new KPopupMenu(this);
    menuGroup = new QPopupMenu(this);
    menuGroup->setCheckable(true);
    connect(menuGroup, SIGNAL(activated(int)), this, SLOT(moveUser(int)));
    connect(menuUser, SIGNAL(activated(int)), this, SLOT(userFunction(int)));

    m_autoStatus = ICQ_STATUS_OFFLINE;
    m_bAutoAway = m_bAutoNA = false;
    autoAwayTimer = new QTimer(this);
    connect(autoAwayTimer, SIGNAL(timeout()), this, SLOT(autoAway()));
    autoAwayTimer->start(10000);

    pClient = new SIMClient(this);
    connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    connect(pClient, SIGNAL(messageReceived(ICQMessage*)), this, SLOT(messageReceived(ICQMessage*)));
    connect(pClient, SIGNAL(messageRead(ICQMessage*)), this, SLOT(messageRead(ICQMessage*)));

#ifdef USE_KDE
#ifdef HAVE_KABC
    pSyncher=new KabcSync;
    connect(pClient,SIGNAL(event(ICQEvent*)),pSyncher,SLOT(processEvent(ICQEvent*)));
#endif
#endif

    menuStatus = new QPopupMenu(this);
    menuStatus->setCheckable(true);
    connect(menuStatus, SIGNAL(activated(int)), this, SLOT(setStatus(int)));

    themes = new Themes(this);
    xosd = new XOSD(this);

    menuPhoneLocation = new QPopupMenu(this);
    connect(menuPhoneLocation, SIGNAL(activated(int)), this, SLOT(setPhoneLocation(int)));
    menuPhoneLocation->setCheckable(true);
    menuPhoneStatus = new QPopupMenu(this);
    connect(menuPhoneStatus, SIGNAL(activated(int)), this, SLOT(setPhoneStatus(int)));
    menuPhoneStatus->setCheckable(true);

    menuPhone = new QPopupMenu(this);
    menuPhone->insertItem(i18n("Current location"), menuPhoneLocation);
    menuPhone->insertItem(i18n("Phone status"), menuPhoneStatus);
    menuPhone->insertSeparator();
    menuPhone->insertItem(i18n("Phone Book"), this, SLOT(phonebook()));

    menuGroups = new QPopupMenu(this);
    connect(menuGroups, SIGNAL(aboutToShow()), this, SLOT(adjustGroupsMenu()));

    menuFunction = new KPopupMenu(this);
    menuFunction->setCheckable(true);
    connect(menuFunction, SIGNAL(aboutToShow()), this, SLOT(adjustFucntionMenu()));
    connect(menuFunction, SIGNAL(activated(int)), this, SLOT(showUser(int)));

    menuFunctionDock = new KPopupMenu(this);
    menuFunctionDock->setCheckable(true);
    connect(menuFunctionDock, SIGNAL(aboutToShow()), this, SLOT(adjustDockMenu()));
    connect(menuFunctionDock, SIGNAL(activated(int)), this, SLOT(showUser(int)));

    menuContainers = new QPopupMenu(this);
    connect(menuContainers, SIGNAL(activated(int)), this, SLOT(toContainer(int)));

    toolbar = new CToolBar(mainWndToolBar, _ToolBarMain(), this, this);
    toolbar->setState(btnStatus, pClient->getStatusIcon(), pClient->getStatusText());
    toolbar->setPopup(btnStatus, menuStatus);
    toolbar->setPopup(btnSetup, menuFunction);

    users = new UserView(this);
    setCentralWidget(users);

    connect(menuGroups, SIGNAL(activated(int)), users, SLOT(grpFunction(int)));

    searchDlg = NULL;
    setupDlg = NULL;

#ifndef WIN32
    struct sigaction act;
    act.sa_handler = child_proc;
    sigemptyset( &(act.sa_mask) );
    sigaddset( &(act.sa_mask), SIGCHLD );
    act.sa_flags = SA_NOCLDSTOP;
#if defined(SA_RESTART)
    act.sa_flags |= SA_RESTART;
#endif
    if (sigaction( SIGCHLD, &act, &oldChildAct ))
        log(L_WARN,  "Error installing SIGCHLD handler: %s", strerror(errno));

    QTimer *childTimer = new QTimer(this);
    connect(childTimer, SIGNAL(timeout()), this, SLOT(checkChilds()));
    childTimer->start(1000);
#endif

    bBlinkState = false;
    blinkTimer = new QTimer(this);
    connect(blinkTimer, SIGNAL(timeout()), this, SLOT(blink()));
    blinkTimer->start(800);
#ifdef WIN32
    HINSTANCE hLib = GetModuleHandleA("user32");
    if (hLib != NULL)
        (DWORD&)_GetLastInputInfo = (DWORD)GetProcAddress(hLib,"GetLastInputInfo");
#endif
#ifdef USE_KDE
    connect(kapp, SIGNAL(iconChanged(int)), this, SLOT(changeIcons(int)));
    kapp->addKipcEventMask(KIPC::IconChanged);
#endif
    transparent = new TransparentTop(this, _UseTransparent(), _Transparent());
    setOnTop();
    loadMenu();
    keys = new HotKeys(this);
    connect(keys, SIGNAL(toggleWindow()), this, SLOT(toggleWindow()));
    connect(keys, SIGNAL(dblClick()), this, SLOT(dockDblClicked()));
    connect(keys, SIGNAL(showSearch()), this, SLOT(search()));
    setIcons();
#ifdef WIN32
    QTimer *autoHideTimer = new QTimer(this);
    connect(autoHideTimer, SIGNAL(timeout()), this, SLOT(autoHide()));
    autoHideTimer->start(1000);
    WM_APPBAR = RegisterWindowMessageA("AppBarNotify");
    WNDPROC p;
    p = (WNDPROC)SetWindowLongW(winId(), GWL_WNDPROC, (LONG)MainWndProc);
    if (p == 0)
        p = (WNDPROC)SetWindowLongA(winId(), GWL_WNDPROC, (LONG)MainWndProc);
    appBarMessage(ABM_NEW);
#endif
    setIcons();
}

void MainWindow::moveEvent(QMoveEvent *e)
{
    QMainWindow::moveEvent(e);
}

void MainWindow::changeMode(bool bSimple)
{
    if (isSimpleMode() == bSimple) return;
    setSimpleMode(bSimple);
    emit modeChanged(bSimple);
}

void MainWindow::setOnTop()
{
    if (isOnTop()){
        setWFlags(WStyle_StaysOnTop);
    }else{
        clearWFlags(WStyle_StaysOnTop);
    }
#ifdef WIN32
    menuFunction->setItemChecked(mnuOnTop, isOnTop());
    ::setBarState();
#else
#ifdef USE_KDE
    if (isOnTop()){
        KWin::setState(winId(), NET::StaysOnTop);
    }else{
        KWin::clearState(winId(), NET::StaysOnTop);
    }
    KWinModule *kwin = new KWinModule(this);
    connect(kwin, SIGNAL(currentDesktopChanged(int)),
            this, SLOT(currentDesktopChanged(int)));
#endif
#endif
}

void MainWindow::setBackgroundPixmap(const QPixmap &pm)
{
    if (transparent) transparent->updateBackground(pm);
}

void MainWindow::changeColors()
{
    emit colorsChanged();
}

void MainWindow::changeOwnColors(bool bUse)
{
    if (bUse == isUseOwnColors()) return;
    setUseOwnColors(bUse);
    emit ownColorsChanged();
}

void MainWindow::toggleOnTop()
{
    setOnTop(!isOnTop());
    setOnTop();
}

void MainWindow::toggleAutoHide()
{
#ifdef WIN32
    setBarAutoHide(!isBarAutoHide());
    setShow(true);
    bAutoHideVisible = true;
    ::setBarState();
#endif
}

void MainWindow::adjustFucntionMenu()
{
    addUnread2Menu(menuFunction);
#ifdef WIN32
    int index = menuFunction->indexOf(mnuOnTop);
    int id = menuFunction->idAt(index + 1);
    if (getBarState() == ABE_FLOAT){
        if (id == mnuAutoHide)
            menuFunction->removeItem(mnuAutoHide);
    }else{
        if (id != mnuAutoHide)
            menuFunction->insertItem(i18n("AutoHide"), this, SLOT(toggleAutoHide()), 0, mnuAutoHide, index+1);
        menuFunction->setItemChecked(mnuAutoHide, isBarAutoHide());
    }
#endif
}

void MainWindow::adjustDockMenu()
{
    menuFunctionDock->changeItem(mnuToggleWnd,
                                 isShowState() ?
                                 i18n("Close main window") :
                                 i18n("Open main window"));
    addUnread2Menu(menuFunctionDock);
}

void MainWindow::addUnread2Menu(QPopupMenu *mnu)
{
    int n;
    int oldItems = menuMsgs.size();
    fillUnread(menuMsgs);
    int index = mnu->indexOf(mnuPopupStatus);
    for (n = 0; n < oldItems; n++)
        mnu->removeItemAt(index+1);
    int id = mnuPopupStatus;
    for (list<msgInfo>::iterator it = menuMsgs.begin(); it != menuMsgs.end(); ++it){
        CUser u((*it).uin);
        (*it).menuId = ++id;
        mnu->insertItem(Icon(SIMClient::getMessageIcon((*it).type)),
                        i18n("%1 from %2")
                        .arg(SIMClient::getMessageText((*it).type, (*it).count))
                        .arg(u.name()),
                        id, ++index);
    }
}

void MainWindow::adjustGroupsMenu()
{
    menuGroups->setItemEnabled(mnuGrpCreate, isGroupMode() && pClient->isLogged());
    menuGroups->setItemEnabled(mnuGrpCollapseAll, isGroupMode());
    menuGroups->setItemEnabled(mnuGrpExpandAll, isGroupMode());
}

void MainWindow::showGroupPopup(QPoint p)
{
    menuGroups->popup(p);
}

void MainWindow::deleteChilds()
{
    if (setupDlg){
        delete setupDlg;
        setupDlg = NULL;
    }
    if (searchDlg){
        delete searchDlg;
        searchDlg = NULL;
    }
    if (mNetMonitor){
        delete mNetMonitor;
        mNetMonitor = NULL;
    }
    if (transparent){
        delete transparent;
        transparent = NULL;
    }
    while (containers.size()){
        delete containers.front();
    }
    for (list<UserFloat*>::iterator it = floating.begin(); it != floating.end(); ++it)
        delete *it;
    floating.clear();
    if (dock){
        delete dock;
        dock = NULL;
    }
#ifndef WIN32
    if (lockFile != -1){
        ::close(lockFile);
        lockFile = -1;
    }
#else
    if (lockFile != -1){
        ReleaseMutex((HANDLE)lockFile);
        lockFile = -1;
    }
#endif
    bLocked = false;
    ::free(&data, MainWindow_Params);
}

MainWindow::~MainWindow()
{
    deleteChilds();
    pMain = NULL;
}

void MainWindow::changeTransparent()
{
    emit transparentChanged();
}

void MainWindow::setPhoneLocation(int location)
{
    unsigned n;
    for (n = 0; n < menuPhoneLocation->count(); n++){
        if (menuPhoneLocation->idAt(n) == location)
            break;
    }
    for (PhoneBook::iterator it = pClient->owner->Phones.begin(); it != pClient->owner->Phones.end(); it++){
        PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
        phone->Active = false;
        if ((phone->Type == PHONE) || (phone->Type == MOBILE) || (phone->Type == SMS)){
            if (n == 0) phone->Active = true;
            n--;
        }
    }
    pClient->updatePhoneBook();
}

void MainWindow::setPhoneStatus(int status)
{
    pClient->owner->PhoneState = (unsigned short)status;
    pClient->updatePhoneStatus();
}

void MainWindow::showPopup(QPoint p)
{
    if (menuFunction->isVisible()) return;
    menuFunction->popup(p);
}

void MainWindow::showDockPopup(QPoint p)
{
    if (menuFunctionDock->isVisible()) return;
    menuFunctionDock->popup(p);
}

void MainWindow::addStatusItem(int status)
{
    menuStatus->insertItem(Icon(SIMClient::getStatusIcon(status)), SIMClient::getStatusText(status), status & 0xFF);
    setStatusItem(status);
}

void MainWindow::setStatusItem(int status)
{
    menuStatus->setItemChecked(status & 0xFF, (unsigned)(pClient->owner->uStatus & 0xFF) == (unsigned)(status & 0xFF));
}

void MainWindow::toggleGroupMode()
{
    setupGroupMode(toolbar->isOn(btnGroupMode));
}

void MainWindow::toggleShowOffline()
{
    setupShowOffline(toolbar->isOn(btnShowOffline));
}

void MainWindow::setupGroupMode(bool bState)
{
    toolbar->setOn(btnGroupMode, bState);
    users->setGroupMode(bState);
}

void MainWindow::setupShowOffline(bool bState)
{
    toolbar->setOn(btnShowOffline, bState);
    users->setShowOffline(bState);
}

extern const ToolBarDef *pUserBoxToolBar;
extern const ToolBarDef *pMsgEditToolBar;
extern const ToolBarDef *pHistoryToolBar;
extern const ToolBarDef *pChatToolBar;

extern char ICQ_CONF[];
extern char SIM_CONF[];

bool MainWindow::init(bool bNoApply)
{
    if (pClient->owner->Uin == 0)
        return false;
    string file;
    if (!bLocked){
#ifndef WIN32
        file = getFullPath("lock");
        if ((lockFile = ::open(file.c_str(), O_RDWR | O_CREAT, 0600)) == -1){
            log(L_ERROR, "Can't open %s: %s", file.c_str(), strerror(errno));
            return false;
        }
        struct flock fl;
        fl.l_type   = F_WRLCK;
        fl.l_whence = SEEK_SET;
        fl.l_len    = 1;
        fl.l_start  = 0;
        fl.l_pid    = getpid();
        if ((fcntl(lockFile, F_SETLK, &fl) == -1) && (errno != ENOLCK)){
            log(L_ERROR, "Can't lock %s: %s", file.c_str(), strerror(errno));
            return false;
        }
#else
        char mutex_name[32];
        snprintf(mutex_name, sizeof(mutex_name), "sim_%lu", pClient->owner->Uin);
        HANDLE mutex;
        mutex = OpenMutexA(NULL, FALSE, mutex_name);
        if (mutex){
            CloseHandle(mutex);
            return false;
        }
        mutex = CreateMutexA(NULL, TRUE, mutex_name);
        if (mutex == NULL) return false;
        lockFile = (int)mutex;
#endif
        bLocked = true;
    }
    string part;
    file = getFullPath(SIM_CONF);
    QFile fs(QString::fromLocal8Bit(file.c_str()));
    if (fs.open(IO_ReadOnly)){
        ::load(&data, MainWindow_Params, fs, part);
    }

    if (bNoApply) return true;

    if (*_ToolBarMain()) emit toolBarChanged(mainWndToolBar);
    if (*_ToolBarMsg()) emit toolBarChanged(pMsgEditToolBar);
    if (*_ToolBarHistory()) emit toolBarChanged(pHistoryToolBar);
    if (*_ToolBarChat()) emit toolBarChanged(pChatToolBar);
    if (*_ToolBarUserBox()) emit toolBarChanged(pUserBoxToolBar);

    if ((getWidth() == 0) || (getHeight() == 0)){
        setHeight(QApplication::desktop()->height() * 3 / 2);
        setWidth(getHeight() / 3);
        if ((getLeft() == 0) && (getTop() == 0)){
            setLeft(QApplication::desktop()->width() - 5 - getWidth());
            setTop(5);
        }
    }
    if (getLeft() < 5)
        setLeft(5);
    if (getTop() < 5)
        setTop(5);
    if (getLeft() > QApplication::desktop()->width() - 5)
        setLeft(QApplication::desktop()->width() - 5);
    if (getTop() > QApplication::desktop()->height() - 5)
        setTop(QApplication::desktop()->height() - 5);
    if (getWidth() > QApplication::desktop()->width() - 5)
        setWidth(QApplication::desktop()->width() - 5);
    if (getHeight() > QApplication::desktop()->height() - 5)
        setHeight(QApplication::desktop()->height() - 5);
    move(getLeft(), getTop());
    if (getWidth() && getHeight())
        resize(getWidth(), getHeight());
    if (*getIcons())
        changeIcons(0);
    themes->setTheme(QString::fromLocal8Bit(getUseStyle()));
    setFonts();
    for (;;){
        if (part.size() == 0) break;
        if (!strcmp(part.c_str(), "[Floaty]")){
            UserFloat *uFloat = new UserFloat;
            if (!uFloat->load(fs, part)){
                delete uFloat;
                continue;
            }
            floating.push_back(uFloat);
            uFloat->show();
#ifdef USE_KDE
            KWin::setOnAllDesktops(uFloat->winId(), true);
            KWin::setState(uFloat->winId(),  NET::SkipTaskbar | NET::StaysOnTop);
#endif
            continue;
        }
        if (!strcmp(part.c_str(), "[UserBox]")){
            UserBox *box = new UserBox;
            if (!box->load(fs, part)){
                delete box;
                continue;
            }
            containers.push_back(box);
            continue;
        }
        break;
    }
    menuStatus->setItemChecked(ICQ_STATUS_FxPRIVATE, pClient->owner->InvisibleId);
    changeWm();

    bool bNeedSetup = false;
    if ((pClient->owner->Uin == 0) || (*pClient->EncryptedPassword.c_str() == 0))
        return false;

    ToolBarDock tDock = Top;
    if (!strcmp(getToolbarDock(), "Minimized")){
        tDock = Minimized;
    }else if (!strcmp(getToolbarDock(), "Bottom")){
        tDock = Bottom;
    }else if (!strcmp(getToolbarDock(), "Left")){
        tDock = Left;
    }else if (!strcmp(getToolbarDock(), "Right")){
        tDock = Right;
    }else if (!strcmp(getToolbarDock(), "TornOff")){
        tDock = TornOff;
    }
    moveToolBar(toolbar, tDock, false, 0, getToolbarOffset());
    if (tDock == TornOff){
        toolbar->move(getToolbarOffset(), getToolbarY());
        toolbar->show();
    }

    loadUnread();
    xosd->init();
    transparentChanged();
    setShow(isShowState());
    setDock();
    setOnTop();
    setUserBoxOnTop();
    ownerChanged();

    setupShowOffline(isShowOffline());
    setupGroupMode(isGroupMode());

    keys->unregKeys();
    keys->regKeys();

    changeBackground();

    QTimer::singleShot(0, this, SLOT(realSetStatus()));
    if (bNeedSetup) setup();
    return true;
}

void MainWindow::setKeys(const char *kWindow, const char *kDblClick, const char *kSearch)
{
    if (!strcmp(kWindow, getKeyWindow()) &&
            !strcmp(kDblClick, getKeyDblClick()) &&
            !strcmp(kSearch, getKeySearch())) return;
    setKeyWindow(kWindow);
    setKeyDblClick(kDblClick);
    setKeySearch(kSearch);
    keys->unregKeys();
    keys->regKeys();
}

void MainWindow::messageReceived(ICQMessage *msg)
{
    if (msg->Type() == ICQ_MSGxSTATUS) return;
    ICQUser *u = pClient->getUser(msg->getUin());
    UserSettings *settings = pClient->getSettings(u, offsetof(UserSettings, SoundOverride));
    const char *wav;
    switch (msg->Type()){
    case ICQ_MSGxURL:
        wav = settings->IncomingURL;
        break;
    case ICQ_MSGxSMS:
        wav = settings->IncomingSMS;
        break;
    case ICQ_MSGxFILE:
        wav = settings->IncomingFile;
        break;
    case ICQ_MSGxCHAT:
        wav = settings->IncomingChat;
        break;
    case ICQ_MSGxAUTHxREQUEST:
    case ICQ_MSGxAUTHxREFUSED:
    case ICQ_MSGxAUTHxGRANTED:
    case ICQ_MSGxADDEDxTOxLIST:
        wav = settings->IncomingAuth;
        break;
    default:
        wav = settings->IncomingMessage;
    }
    if (wav && ((pClient->owner->uStatus & 0xFF) != ICQ_STATUS_AWAY) && ((pClient->owner->uStatus & 0xFF) != ICQ_STATUS_NA))
        playSound(wav);
    unread_msg m(msg);

    CUser user(u);
    xosd->setMessage(i18n("%1 from %2 received")
                     .arg(pClient->getMessageText(msg->Type(), 1))
                     .arg(user.name()), u->Uin);

    if (u == NULL) return;
    list<unsigned long>::iterator it;
    for (it = u->unreadMsgs.begin(); it != u->unreadMsgs.end(); it++)
        if ((*it) == msg->Id) break;
    if (it == u->unreadMsgs.end()) return;
    messages.push_back(m);
    emit msgChanged();

    settings = pClient->getSettings(u, offsetof(UserSettings, ProgOverride));
    if (settings->AcceptMsgWindow)
        userFunction(msg->getUin(), mnuActionAuto);
}

void MainWindow::showUser(int id)
{
    for (list<msgInfo>::iterator it = menuMsgs.begin(); it != menuMsgs.end(); ++it){
        if ((*it).menuId != id) continue;
        userFunction((*it).uin, mnuAction, 0);
        return;
    }
}

extern LoginDialog *pLoginDlg;

void MainWindow::processEvent(ICQEvent *e)
{
    switch (e->type()){
    case EVENT_AUTH_REQUIRED:
        if (!pClient->BypassAuth)
            userFunction(e->Uin(), mnuAuth, 0);
        return;
    case EVENT_USER_DELETED:{
            loadUnread();
            UserFloat *floaty = findFloating(e->Uin(), true);
            if (floaty == NULL) return;
            delete floaty;
            saveContacts();
            return;
        }
    case EVENT_PROXY_ERROR:{
            log(L_DEBUG, "Set manual to offline (proxy error)");
            setManualStatus(ICQ_STATUS_OFFLINE);
            ProxyDialog d(this, i18n("Can't connect to proxy server"));
            d.exec();
            return;
        }
    case EVENT_PROXY_BAD_AUTH:{
            log(L_DEBUG, "Set manual to offline (proxy bad auth)");
            setManualStatus(ICQ_STATUS_OFFLINE);
            ProxyDialog d(this, pClient->factory()->ProxyAuth ?
                          i18n("Proxy server require authorization") :
                          i18n("Invalid password for proxy"));
            d.exec();
            return;
        }
    case EVENT_BAD_PASSWORD:
        if (pLoginDlg) return;
        pClient->EncryptedPassword = "";
        QTimer::singleShot(50, this, SLOT(shangeUIN()));
        return;
    case EVENT_ANOTHER_LOCATION:
        setManualStatus(ICQ_STATUS_OFFLINE);
        log(L_DEBUG, "Set manual to offline (another location)");
        pClient->setStatus(ICQ_STATUS_OFFLINE);
        setShow(true);
        BalloonMsg::message(i18n("Your UIN used from another location"), toolbar->getWidget(btnStatus));
        break;
    case EVENT_INFO_CHANGED:
        saveContacts();
        if ((e->Uin() == pClient->owner->Uin) || (e->Uin() == 0)){
            if ((realTZ != pClient->owner->TimeZone) && (e->subType() == EVENT_SUBTYPE_FULLINFO)){
                ICQUser u;
                u = (*pClient->owner);
                u.TimeZone = realTZ;
                pClient->setInfo(&u);
            }
            menuPhoneLocation->clear();
            unsigned long n = 0;
            for (PhoneBook::iterator it = pClient->owner->Phones.begin(); it != pClient->owner->Phones.end(); it++){
                PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
                if ((phone->Type == PHONE) || (phone->Type == MOBILE) || (phone->Type == SMS)){
                    menuPhoneLocation->insertItem(phone->getNumber().c_str());
                    if (phone->Active)
                        menuPhoneLocation->setItemChecked(menuPhoneLocation->idAt(n), true);
                    n++;
                }
            }
            menuPhone->setItemEnabled(menuPhone->idAt(0), n != 0);
            menuPhoneStatus->setItemChecked(0, pClient->owner->PhoneState == 0);
            menuPhoneStatus->setItemChecked(1, pClient->owner->PhoneState == 1);
            menuPhoneStatus->setItemChecked(2, pClient->owner->PhoneState == 2);
        }
    case EVENT_STATUS_CHANGED:
        if ((e->Uin() == pClient->owner->Uin) || (e->Uin() == 0)){
            ownerChanged();
            if (realTZ != pClient->owner->TimeZone)
                pClient->addInfoRequest(pClient->owner->Uin, true);
        }else{
            ICQUser *u = pClient->getUser(e->Uin());
            if (u == NULL) return;
            UserSettings *settings = pClient->getSettings(u, offsetof(UserSettings, AlertOverride));
            if ((u->IgnoreId == 0) &&
                    (!isNoAlertAway() ||
                     ((u->uStatus & 0xFF) == ICQ_STATUS_ONLINE) ||
                     ((u->uStatus & 0xFF) == ICQ_STATUS_FREEFORCHAT)) &&
                    ((u->prevStatus & 0xFF) != ICQ_STATUS_ONLINE) &&
                    ((u->prevStatus & 0xFF) != ICQ_STATUS_FREEFORCHAT) &&
                    (((pClient->owner->uStatus & 0xFF) == ICQ_STATUS_ONLINE) ||
                     ((pClient->owner->uStatus & 0xFF) == ICQ_STATUS_FREEFORCHAT)) &&
                    ((u->prevStatus == ICQ_STATUS_OFFLINE) || settings->AlertAway) &&
                    ((u->OnlineTime > pClient->owner->OnlineTime) || (u->prevStatus  != ICQ_STATUS_OFFLINE))){
                if (settings->AlertSound){
                    UserSettings *soundSettings = pClient->getSettings(u, offsetof(UserSettings, SoundOverride));
                    playSound(soundSettings->OnlineAlert);
                }
                if (settings->AlertOnScreen){
                    CUser user(e->Uin());
                    xosd->setMessage(i18n("%1 is online") .arg(user.name()), e->Uin());
                }
                if (settings->AlertPopup){
                    AlertMsgDlg *dlg = new AlertMsgDlg(this, e->Uin());
                    raiseWindow(dlg);
                }
                if (settings->AlertWindow)
                    userFunction(e->Uin(), mnuAction);
            }
        }
        return;
    }
}

void MainWindow::changeUIN()
{
    if (pLoginDlg) return;
    scanUIN();
    pLoginDlg = new LoginDialog();
    pLoginDlg->show();
}

bool MainWindow::isLoad()
{
    return bLocked;
}

void MainWindow::reset()
{
    if (!bLocked) return;
    pClient->setStatus(ICQ_STATUS_OFFLINE);
    saveState();
    hide();
    deleteChilds();
    ::init(&data, MainWindow_Params);
    pClient->init();
}

void MainWindow::saveState()
{
    if ((pClient->owner->Uin == 0) || !bLocked) return;
    if (m_bAutoAway || m_bAutoNA){
        log(L_DEBUG, "Set manual to %X (saveState)", m_autoStatus);
        setManualStatus((unsigned long)m_autoStatus);
    }
    setShowOffline(toolbar->isOn(btnShowOffline));
    setGroupMode(toolbar->isOn(btnGroupMode));
    setShowState(isShow());
#ifdef WIN32
    if (getBarState() == ABE_FLOAT){
#endif
        setLeft(pos().x());
        setTop(pos().y());
        setWidth(size().width());
        setHeight(size().height());
#ifdef WIN32
    }
#endif
    ToolBarDock tDock;
    int index;
    bool nl;
    int extraOffset;
    getLocation(toolbar, tDock, index, nl, extraOffset);
    setToolbarOffset((short)extraOffset);
    switch (tDock){
    case Minimized:
        setToolbarDock("Minimized");
        break;
    case Bottom:
        setToolbarDock("Bottom");
        break;
    case Left:
        setToolbarDock("Left");
        break;
    case Right:
        setToolbarDock("Right");
        break;
    case TornOff:
        setToolbarDock("TornOff");
        setToolbarOffset(toolbar->geometry().left());
        setToolbarY(toolbar->geometry().top());
        break;
    default:
        setToolbarDock("Top");
    }
    setOnTop(testWFlags(WStyle_StaysOnTop));
#ifdef USE_KDE
    setOnTop(KWin::info(winId()).state & NET::StaysOnTop);
#endif
    setUseStyle("");
    if (themes->getTheme().length()) setUseStyle(themes->getTheme().local8Bit());
    string file = getFullPath(SIM_CONF);
#ifndef WIN32
    struct stat st;
    if ((stat(file.c_str(), &st) >= 0) && (st.st_mode != 0600))
        unlink(file.c_str());
#endif
    QFile fs(file.c_str());
    if (fs.open(IO_WriteOnly | IO_Truncate)){
        ::save(&data, MainWindow_Params, fs);
        for (list<UserFloat*>::iterator itFloat = floating.begin(); itFloat != floating.end(); itFloat++){
            writeStr(fs, "[Floaty]\n");
            (*itFloat)->save(fs);
        }
        for (list<UserBox*>::iterator itBox = containers.begin(); itBox != containers.end(); itBox++){
            writeStr(fs, "[UserBox]\n");
            (*itBox)->save(fs);
        }
    }
    fs.flush();
    saveContacts();
    pSplash->save();
}

void MainWindow::saveContacts()
{
    if (pClient->owner->Uin == 0) return;
    string file = getFullPath(ICQ_CONF);
#ifndef WIN32
    struct stat st;
    if ((stat(file.c_str(), &st) >= 0) && (st.st_mode != 0600))
        unlink(file.c_str());
#endif
    pClient->save();
}

bool MainWindow::isDock()
{
    return (dock != NULL);
}

extern bool bNoDock;

void MainWindow::setDock()
{
    bool bDock = isUseDock();
    if (bNoDock){
        bDock = false;
        bNoDock = false;
    }
    if (bDock){
        if (dock) return;
        dock = new DockWnd(this);
        return;
    }
    if (dock){
        delete dock;
        dock = NULL;
    }
    setShow(true);
}

void MainWindow::dockDblClicked()
{
    if (menuFunction && menuFunction->isVisible()) return;
    for (list<UserBox*>::iterator itBox = containers.begin(); itBox != containers.end(); ++itBox){
        if ((*itBox)->isHistory()) continue;
        if (!(*itBox)->isActiveWindow()) continue;
        ICQUser *u = pClient->getUser((*itBox)->currentUser());
        if (u && u->unreadMsgs.size()){
            (*itBox)->showUser(u->Uin, mnuAction, 0);
            return;
        }
    }
    list<ICQUser*>::iterator it;
    for (it = pClient->contacts.users.begin(); it != pClient->contacts.users.end(); ++it){
        if ((*it)->unreadMsgs.size()){
            showUser((*it)->Uin, mnuAction, 0);
            return;
        }
    }
    setShow(true);
}

void MainWindow::destroyBox(UserBox *box)
{
    containers.remove(box);
}

void MainWindow::closeUser(unsigned long uin)
{
    for (list<UserBox*>::iterator it = containers.begin(); it != containers.end(); ++it)
        if ((*it)->closeUser(uin)) return;
}

void MainWindow::showUser(unsigned long uin, int function, unsigned long param)
{
    ICQUser *u = pClient->getUser(uin);
    if (uin && (u == NULL)) return;
    list<UserBox*>::iterator it;
    if (isSimpleMode()){
        for (it = containers.begin(); it != containers.end(); ++it){
            if ((*it)->isHistory()) continue;
            if (!(*it)->haveUser(uin)) continue;
            ICQMessage *msg = (*it)->currentMessage();
            bool bOK = false;
            switch (function){
            case mnuActionAuto:
                if (u->unreadMsgs.size()){
                    if (msg && msg->Received) return;
                }
            case mnuGo:
                bOK = msg && msg->Received;
                break;
            case mnuAction:
            case mnuActionInt:
                if (u->unreadMsgs.size()){
                    bOK = msg && msg->Received;
                    break;
                }
            case mnuMessage:
                bOK = msg && (msg->Type() == ICQ_MSGxMSG) && !msg->Received;
                break;
            case mnuURL:
                bOK = msg && (msg->Type() == ICQ_MSGxURL) && !msg->Received;
                break;
            case mnuSMS:
                bOK = msg && (msg->Type() == ICQ_MSGxSMS) && !msg->Received;
                break;
            case mnuFile:
                bOK = msg && (msg->Type() == ICQ_MSGxFILE) && !msg->Received;
                break;
            case mnuContacts:
                bOK = msg && (msg->Type() == ICQ_MSGxCONTACTxLIST) && !msg->Received;
                break;
            case mnuAuth:
                bOK = msg && (msg->Type() == ICQ_MSGxAUTHxREQUEST) && !msg->Received;
                break;
            case mnuChat:
                bOK = msg && (msg->Type() == ICQ_MSGxCHAT) && !msg->Received;
                break;
            default:
                bOK = true;
            }
            if (bOK){
                (*it)->showUser(uin, function, param);
                return;
            }
        }
        UserBox *box = new UserBox(uin);
        containers.push_back(box);
        box->showUser(uin, function, param);
        return;
    }
    if (uin){
        for (it = containers.begin(); it != containers.end(); ++it){
            if (!(*it)->haveUser(uin)) continue;
            (*it)->showUser(uin, function, param);
            return;
        }
    }
    UserBox *box;
    unsigned long grpId = u ? u->GrpId : 0;
    switch (getContainerMode()){
    case ContainerModeAll:
        for (it = containers.begin(); it != containers.end(); ++it){
            if ((*it)->isHistory()) continue;
            if ((*it)->getGrpId() == ContainerAllUsers) break;
        }
        if (it == containers.end()){
            for (it = containers.begin(); it != containers.end(); ++it)
                if (!(*it)->isHistory()) break;
        }
        if (it != containers.end()){
            (*it)->setGrpId(ContainerAllUsers);
            (*it)->showUser(uin, function, param);
            return;
        }
        box = new UserBox(ContainerAllUsers);
        break;
    case ContainerModeGroup:
        for (it = containers.begin(); it != containers.end(); ++it){
            if ((*it)->isHistory()) continue;
            if ((*it)->getGrpId() != (unsigned long)grpId) continue;
            (*it)->showUser(uin, function, param);
            return;
        }
        box = new UserBox(grpId);
        break;
    default:
        grpId = 0x7FFFFFFF;
        for (it = containers.begin(); it != containers.end(); ++it){
            if ((*it)->isHistory()) continue;
            if ((*it)->getGrpId() < 0x10000L) continue;
            if ((*it)->getGrpId() < grpId) grpId = (*it)->getGrpId() - 1;
        }
        box = new UserBox(uin);
    }
    containers.push_back(box);
    box->showUser(uin, function, param);
}

void MainWindow::setShow(bool bShow)
{
    setShowState(bShow);
#ifdef WIN32
    if (getBarState() != ABE_FLOAT){
        if (isBarAutoHide()) bShow = true;
        if (bShow){
            ::setBarState();
        }else{
            appBarMessage(ABM_SETPOS, ABE_FLOAT, FALSE);
        }
    }
#endif
    if (!bShow){
        hide();
        return;
    }
    raiseWindow(this);
}

bool MainWindow::isShow()
{
    if (!isVisible()) return false;
#ifdef USE_KDE
    return (KWin::info(winId()).desktop == KWin::currentDesktop())
           || KWin::info(winId()).onAllDesktops;
#else
    return true;
#endif
}

void MainWindow::toggleWindow()
{
    setShow(!isShow());
}

void MainWindow::toggleShow()
{
    if (menuFunction && menuFunction->isVisible()) return;
    if (noToggle) return;
#ifdef WIN32
    if ((getBarState() != ABE_FLOAT) && isBarAutoHide()){
        bAutoHideVisible = !bAutoHideVisible;
        ::setBarState();
        if (!bAutoHideVisible) setShow(true);
        noToggle = true;
        QTimer::singleShot(1000, this, SLOT(setToggle()));
        return;
    }
#endif
    bool bIsActive = isActiveWindow();
    if (!bIsActive && dock) bIsActive = dock->topLevelWidget()->isActiveWindow();
    if (isShow() && !bIsActive){
        time_t now;
        time(&now);
        if ((unsigned)now <= inactiveTime + 2)
            bIsActive = true;
    }
    if (!isShow() || !bIsActive){
        setShow(true);
    }else{
        setShow(false);
    }
}

void MainWindow::setToggle()
{
    noToggle = false;
}

void MainWindow::setup()
{
    if (setupDlg == NULL){
        setupDlg = new SetupDialog(this, 0);
    }
    emit setupInit();
    raiseWindow(setupDlg);
}

void MainWindow::setupClosed()
{
    if (setupDlg)
        delete setupDlg;
    setupDlg = NULL;
}

void MainWindow::phonebook()
{
    if (setupDlg == NULL){
        setupDlg = new SetupDialog(this, SETUP_PHONE);
    }else{
        setupDlg->showPage(SETUP_PHONE);
    }
    raiseWindow(setupDlg);
}

void MainWindow::quit()
{
    bQuit = true;
    saveState();
    close();
}

void MainWindow::search()
{
    if (searchDlg == NULL) searchDlg = new SearchDialog(NULL);
    raiseWindow(searchDlg);
}

void MainWindow::showSearch(bool bSearch)
{
    if (bSearch){
        search();
        return;
    }
    if (searchDlg) searchDlg->close();
}

bool MainWindow::isSearch()
{
    return (searchDlg != NULL) && searchDlg->isVisible();
}

bool MainWindow::event(QEvent *e)
{
    if (e->type() == QEvent::WindowDeactivate){
        time_t now;
        time(&now);
        inactiveTime = now;
    }
    return QMainWindow::event(e);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (!bQuit && (dock != NULL)){
        e->ignore();
        setShow(false);
        return;
    }
    pClient->setStatus(ICQ_STATUS_OFFLINE);
    saveState();
    if (dock){
        delete dock;
        dock = NULL;
    }
#ifdef WIN32
    appBarMessage(ABM_REMOVE);
#endif
    QMainWindow::closeEvent(e);
}

void MainWindow::autoAway()
{
    if (isDock() && !isActiveWindow() && getAutoHideTime()){
        time_t now;
        time(&now);
        if (now >= (time_t)(inactiveTime + getAutoHideTime()))
            setShow(false);
    }
#ifdef WIN32
    unsigned long idle_time = 0;
    if (_GetLastInputInfo == NULL){
        POINT p;
        GetCursorPos(&p);
        time_t now;
        time(&now);
        if ((p.x != oldX) || (p.y != oldY)){
            oldX = p.x;
            oldY = p.y;
            lastTime = now;
        }
        idle_time = now - lastTime;
    }else{
        LASTINPUTINFO lii;
        ZeroMemory(&lii,sizeof(lii));
        lii.cbSize=sizeof(lii);
        _GetLastInputInfo(&lii);
        idle_time = (GetTickCount()-lii.dwTime) / 1000;
    }
#endif
#ifdef USE_SCRNSAVER
    static XScreenSaverInfo *mit_info = NULL;
    if (mit_info == NULL) {
        int event_base, error_base;
        if(XScreenSaverQueryExtension(x11Display(), &event_base, &error_base)) {
            mit_info = XScreenSaverAllocInfo ();
        }
    }
    if (mit_info == NULL){
        log(L_WARN, "No XScreenSaver extension found on current XServer, disabling auto-away.");
        autoAwayTimer->stop();
        return;
    }
    if (!XScreenSaverQueryInfo(x11Display(), qt_xrootwin(), mit_info)) {
        log(L_WARN, "XScreenSaverQueryInfo failed, disabling auto-away.");
        autoAwayTimer->stop();
        return;
    }
    unsigned long idle_time = (mit_info->idle / 1000);
#endif
#if defined(USE_SCRNSAVER) || defined(WIN32)
    if ((m_bAutoAway && (idle_time < getAutoAwayTime())) ||
            (m_bAutoNA && (idle_time < getAutoNATime()))){
        m_bAutoAway = m_bAutoNA = false;
        if (((getManualStatus() & 0xFF) == ICQ_STATUS_NA) ||
                ((getManualStatus() & 0xFF) == ICQ_STATUS_AWAY)){
            setManualStatus((unsigned long)m_autoStatus);
            log(L_DEBUG, "Set manual to %X (from auto away)", getManualStatus());
            realSetStatus();
        }
        return;
    }
    if (!m_bAutoNA && getAutoNATime() && (idle_time > getAutoNATime())){
        m_bAutoNA = true;
        if ((getManualStatus() == ICQ_STATUS_OFFLINE) ||
                ((getManualStatus() & 0xFF) == ICQ_STATUS_NA)) return;
        if (!m_bAutoAway){
            m_autoStatus = getManualStatus();
            m_bAutoNA = true;
        }
        log(L_DEBUG, "Set manual to %X (auto na)", getManualStatus());
        setManualStatus(ICQ_STATUS_NA);
        realSetStatus();
        return;
    }
    if (!m_bAutoNA && !m_bAutoAway && getAutoAwayTime() && (idle_time > getAutoAwayTime())){
        m_bAutoAway = true;
        if ((getManualStatus() == ICQ_STATUS_OFFLINE) ||
                ((getManualStatus() & 0xFF) == ICQ_STATUS_NA) ||
                ((getManualStatus() & 0xFF) == ICQ_STATUS_AWAY)) return;
        m_autoStatus = getManualStatus();
        log(L_DEBUG, "Set manual to %X (auto away)", getManualStatus());
        setManualStatus(ICQ_STATUS_AWAY);
        realSetStatus();
        return;
    }
#endif
}

void MainWindow::setStatus(int status)
{
    if ((unsigned long)status == ICQ_STATUS_FxPRIVATE){
        pClient->setInvisible(pClient->owner->InvisibleId == 0);
        menuStatus->setItemChecked(ICQ_STATUS_FxPRIVATE, pClient->owner->InvisibleId != 0);
        return;
    }
    AutoReplyDlg *autoDlg = NULL;
    switch (status & 0xFF){
    case ICQ_STATUS_AWAY:
        if (isNoShowAway()) break;
        autoDlg = new AutoReplyDlg(this, ICQ_STATUS_AWAY);
        break;
    case ICQ_STATUS_NA:
        if (isNoShowNA()) break;
        autoDlg = new AutoReplyDlg(this, ICQ_STATUS_NA);
        break;
    case ICQ_STATUS_OCCUPIED:
        if (isNoShowOccupied()) break;
        autoDlg = new AutoReplyDlg(this, ICQ_STATUS_OCCUPIED);
        break;
    case ICQ_STATUS_DND:
        if (isNoShowDND()) break;
        autoDlg = new AutoReplyDlg(this, ICQ_STATUS_DND);
        break;
    case ICQ_STATUS_FREEFORCHAT:
        if (isNoShowFFC()) break;
        autoDlg = new AutoReplyDlg(this, ICQ_STATUS_FREEFORCHAT);
        break;
    }
    if (autoDlg)
        raiseWindow(autoDlg);
    if (status == (ICQ_STATUS_OFFLINE & 0xFF)) status = ICQ_STATUS_OFFLINE;
    setManualStatus((unsigned long)status);
    QTimer::singleShot(800, this, SLOT(realSetStatus()));
}

void MainWindow::realSetStatus()
{
    log(L_DEBUG, "Real set status %X", getManualStatus());
    pClient->setStatus(getManualStatus());
}

void MainWindow::moveUser(int grp)
{
    ICQUser *u = pClient->getUser(m_uin);
    switch (grp){
    case mnuGroupVisible:
        if (u == NULL) break;
        pClient->setInVisible(u, u->VisibleId == 0);
        break;
    case mnuGroupInvisible:
        if (u == NULL) break;
        pClient->setInInvisible(u, u->InvisibleId == 0);
        break;
    case mnuGroupIgnore:
        if (u == NULL) break;
        if (u->IgnoreId == 0){
            CUser user(u);
            QStringList btns;
            btns.append(i18n("&Yes"));
            btns.append(i18n("&No"));
            BalloonMsg *msg = new BalloonMsg(i18n("Add %1 to ignore list ?") .arg(user.name()),
                                             btns, this, &m_rc);
            connect(msg, SIGNAL(action(int)), this, SLOT(ignoreUser(int)));
            msg->show();
            break;
        }
        pClient->setInIgnore(u, u->IgnoreId == 0);
        break;
    default:
        if (u == NULL) u = pClient->getUser(m_uin, true);
        if (u == NULL) return;
        ICQGroup *g = pClient->getGroup(grp);
        if (grp) pClient->moveUser(u, g);
    }
}

void MainWindow::setIcons()
{
    QPixmap icon = Pict(pClient->getStatusIcon());
    if (!pClient->isConnecting()){
        toolbar->setState(btnStatus, pClient->getStatusIcon(), pClient->getStatusText());
    }else{
        toolbar->setState(btnStatus, SIMClient::getStatusIcon(bBlinkState ? ICQ_STATUS_OFFLINE : ICQ_STATUS_ONLINE), pClient->getStatusText());
        icon = Pict(SIMClient::getStatusIcon(bBlinkState ? ICQ_STATUS_OFFLINE : ICQ_STATUS_ONLINE));
    }
    setIcon(icon);
}

void MainWindow::blink()
{
    if (!pClient->isConnecting()) return;
    bBlinkState = !bBlinkState;
    setIcons();
}

void MainWindow::ownerChanged()
{
    setIcons();
    setStatusItem(ICQ_STATUS_ONLINE);
    setStatusItem(ICQ_STATUS_AWAY);
    setStatusItem(ICQ_STATUS_NA);
    setStatusItem(ICQ_STATUS_DND);
    setStatusItem(ICQ_STATUS_OCCUPIED);
    setStatusItem(ICQ_STATUS_FREEFORCHAT);
    setStatusItem(ICQ_STATUS_OFFLINE);
    CUser owner(pClient->owner);
    setCaption(owner.name());

    menuFunction->changeTitle(1, owner.name());
}

void MainWindow::adjustGroupMenu(QPopupMenu *menuGroup, unsigned long uin)
{
    menuGroup->clear();
    ICQUser *u = pClient->getUser(uin);
    vector<ICQGroup*>::iterator it;
    for (it = pClient->contacts.groups.begin(); it != pClient->contacts.groups.end(); it++){
        CGroup grp(*it);
        menuGroup->insertItem(grp.name(), (*it)->Id);
        if (u && ((*it)->Id == u->GrpId)) menuGroup->setItemChecked((*it)->Id, true);
    }
    if (u){
        menuGroup->insertSeparator();
        if (u->Type == USER_TYPE_ICQ){
            menuGroup->insertItem(i18n("In visible list"), mnuGroupVisible);
            menuGroup->setItemChecked(mnuGroupVisible, u->VisibleId != 0);
            menuGroup->insertItem(i18n("In invisible list"), mnuGroupInvisible);
            menuGroup->setItemChecked(mnuGroupInvisible, u->InvisibleId != 0);
        }
        menuGroup->insertItem(i18n("In ignore list"), mnuGroupIgnore);
        menuGroup->setItemChecked(mnuGroupIgnore, u->IgnoreId != 0);
    }
}

void MainWindow::showUserPopup(unsigned long uin, QPoint p, QPopupMenu *popup, const QRect &rc)
{
    m_rc  = rc;
    m_uin = uin;
    ICQUser *u = pClient->getUser(uin);
    if (u == NULL) return;
    adjustGroupMenu(menuGroup, uin);
    menuUser->clear();
    CUser user(u);
    menuUser->insertTitle(user.name());
    menuUser->insertSeparator();
    menuUser->insertItem(i18n("Groups"), menuGroup, mnuGroups);
    menuUser->insertItem(Pict("remove"), i18n("Delete"), mnuDelete);
    menuUser->setAccel(QAccel::stringToKey(i18n("Del", "Delete")), mnuDelete);
    menuUser->setItemEnabled(mnuGroups, pClient->isLogged() || (u->Type != USER_TYPE_ICQ));
    menuUser->setItemEnabled(mnuDelete, pClient->isLogged() || (u->Type != USER_TYPE_ICQ) || (u->GrpId == 0));
    menuUser->insertSeparator();
    menuUser->insertItem(Icon("info"), i18n("&User info"), mnuInfo);
    menuUser->setAccel(QAccel::stringToKey(i18n("Ctrl+U", "UserInfo")), mnuInfo);
    menuUser->insertItem(Icon("history"), i18n("&History"), mnuHistory);
    menuUser->setAccel(QAccel::stringToKey(i18n("Ctrl+H", "History")), mnuHistory);
    menuUser->insertSeparator();
    menuUser->insertItem(Icon("alert"), i18n("Alert"), mnuAlert);
    menuUser->setAccel(QAccel::stringToKey(i18n("Ctrl+L", "Alert")), mnuAlert);
    menuUser->insertItem(Icon("file"), i18n("Accept mode"), mnuAccept);
    menuUser->setAccel(QAccel::stringToKey(i18n("Ctrl+A", "Accept")), mnuAccept);
    menuUser->insertItem(Icon("sound"), i18n("Sound"), mnuSound);
    menuUser->setAccel(QAccel::stringToKey(i18n("Ctrl+S", "Sound")), mnuSound);
    menuUser->insertSeparator();
    menuUser->insertItem(Icon("floating"),
                         findFloating(uin) ? i18n("Floating off") : i18n("Floating on"), mnuFloating);
    menuUser->setAccel(QAccel::stringToKey(i18n("Ctrl+T", "Floating")), mnuFloating);
    if (popup){
        menuUser->insertSeparator();
        menuUser->insertItem(i18n("Users"), popup);
        menuUser->insertItem(Icon("exit"), i18n("&Close"), mnuClose);
        menuUser->setAccel(QAccel::stringToKey(i18n("Ctrl+C", "Close")), mnuClose);
    }else{
        menuUser->insertItem(i18n("Alphabetically sort"), mnuSort);
        menuUser->setAccel(QAccel::stringToKey(i18n("Ctrl+B", "Alphabetically sort")), mnuSort);
        menuUser->setItemChecked(mnuSort, isAlphabetSort());
    }
    adjustUserMenu(menuUser, u, true, false);
    menuUser->installEventFilter(this);
    menuUser->popup(p);
}

void MainWindow::addMessageType(QPopupMenu *menuUser, int type, int id,
                                bool bAdd, bool bHaveTitle, unsigned long uin)
{
    addMenuItem(menuUser, SIMClient::getMessageIcon(type),
                SIMClient::getMessageText(type, 1),
                id, bAdd && canUserFunction(uin, id), bHaveTitle, SIMClient::getMessageAccel(type));
}

void MainWindow::addMenuItem(QPopupMenu *menuUser, const char *icon,
                             const QString &n, int id, bool bAdd, bool bHaveTitle, const QString &accel)
{
    if (bAdd){
        int pos = 0;
        if (bHaveTitle) pos = 1;
        if (menuUser->findItem(id)){
            if (menuUser->indexOf(id) == pos) return;
            menuUser->removeItem(id);
        }
        menuUser->insertItem(Icon(icon), n,
                             id, pos);
        if (!accel.isEmpty())
            menuUser->setAccel(QAccel::stringToKey(accel), id);
        return;
    }
    if (menuUser->findItem(id) == NULL) return;
    menuUser->removeItem(id);
}


void MainWindow::userFunction(int function)
{
    userFunction(m_uin, function);
}

void MainWindow::deleteUser(int n)
{
    if (n) return;
    ICQUser *u = pClient->getUser(m_uin);
    if (u) pClient->deleteUser(u);
}

void MainWindow::ignoreUser(int n)
{
    if (n) return;
    ICQUser *u = pClient->getUser(m_uin);
    if (u) pClient->setInIgnore(u, true);
}

void MainWindow::currentDesktopChanged(int)
{
#ifdef USE_KDE
    list<UserFloat*>::iterator it;
    for (it = floating.begin(); it != floating.end(); it++){
        KWin::setOnAllDesktops((*it)->winId(), true);
        KWin::setState((*it)->winId(),
                       /* NET::SkipTaskbar | */ NET::StaysOnTop);
    }
#endif
}

bool MainWindow::canUserFunction(unsigned long uin, int function)
{
    ICQUser *u;
    switch (function){
    case mnuFloating:
    case mnuClose:
    case mnuActionAuto:
    case mnuAction:
    case mnuGo:
    case mnuHistory:
    case mnuDelete:
    case mnuSort:
    case mnuInfo:
    case mnuHistoryNew:
        return true;
    case mnuSecureOn:
        u = pClient->getUser(uin);
        return u && (u->Type == USER_TYPE_ICQ) && (u->uStatus != ICQ_STATUS_OFFLINE) &&
               !u->isSecure();
    case mnuSecureOff:
        u = pClient->getUser(uin);
        return u && u->isSecure();
    case mnuMail:
        u = pClient->getUser(uin);
        if (u){
            for (EMailList::iterator it = u->EMails.begin(); it != u->EMails.end(); ++it){
                EMailInfo *mailInfo = static_cast<EMailInfo*>(*it);
                if (*mailInfo->Email.c_str()) return true;
            }
        }
        break;
    case mnuAuth:
        u = pClient->getUser(uin);
        return u && u->WaitAuth;
    case mnuFile:
    case mnuChat:
        u = pClient->getUser(uin);
        return u && (u->Type == USER_TYPE_ICQ) && u->isOnline();
    case mnuContacts:
        u = pClient->getUser(uin);
        return u && (u->Type == USER_TYPE_ICQ);
    case mnuSMS:
        u = pClient->getUser(uin);
        if (u == NULL) break;
        if (u->Type != USER_TYPE_ICQ){
            for (PhoneBook::iterator it = u->Phones.begin(); it != u->Phones.end(); ++it){
                PhoneInfo *info = static_cast<PhoneInfo*>(*it);
                if (info->Type == SMS) return true;
            }
        }
        return true;
    case mnuMessage:
    case mnuURL:
    case mnuAlert:
    case mnuAccept:
    case mnuSound:
    case mnuAutoResponse:
        u = pClient->getUser(uin);
        return u && (u->Type == USER_TYPE_ICQ);
    default:
        break;
    }
    return false;
}

void MainWindow::userFunction(unsigned long uin, int function, unsigned long param)
{
    if (!canUserFunction(uin, function)) return;
    UserBox *box;
    list<UserBox*>::iterator it;
    switch (function){
    case mnuFloating:
        {
            UserFloat *uFloat = findFloating(uin, true);
            if (uFloat){
                delete uFloat;
            }else{
                uFloat = new UserFloat;
                if (!uFloat->setUIN(m_uin)){
                    delete uFloat;
                    return;
                }
                floating.push_back(uFloat);
                uFloat->show();
#ifdef USE_KDE
                KWin::setOnAllDesktops(uFloat->winId(), true);
                KWin::setState(uFloat->winId(),
                               NET::SkipTaskbar | NET::StaysOnTop | NET::Sticky);
#endif
            }
            return;
        }
    case mnuClose:
        closeUser(uin);
        return;
    case mnuActionAuto:
    case mnuAction: {
            ICQUser *u = pClient->getUser(uin);
            if (u && (u->unreadMsgs.size() == 0) && (u->Type == USER_TYPE_EXT)){
                PhoneBook::iterator it;
                for (it = u->Phones.begin(); it != u->Phones.end(); ++it){
                    PhoneInfo *info = static_cast<PhoneInfo*>(*it);
                    if (info->Type == SMS) break;
                }
                if (it == u->Phones.end()){
                    if (u->EMails.size())
                        pMain->sendMail(uin);
                    return;
                }
            }
            showUser(uin, function, param);
            return;
        }
    case mnuChat:{
            QWidget *chat = chatWindow(uin);
            if (chat){
                raiseWindow(chat);
                return;
            }
            showUser(uin, function, param);
            return;
        }
    case mnuGo:
    case mnuMessage:
    case mnuURL:
    case mnuSMS:
    case mnuFile:
    case mnuAuth:
    case mnuContacts:
    case mnuHistory:
        showUser(uin, function, param);
        return;
    case mnuDelete:
        {
            ICQUser *u = pClient->getUser(uin);
            if (u){
                CUser user(u);
                m_uin = uin;
                QStringList btns;
                btns.append(i18n("&Yes"));
                btns.append(i18n("&No"));
                BalloonMsg *msg = new BalloonMsg(i18n("Delete %1?") .arg(user.name()),
                                                 btns, this, &m_rc);
                connect(msg, SIGNAL(action(int)), this, SLOT(deleteUser(int)));
                msg->show();
            }
            return;
        }
    case mnuAutoResponse:{
            QWidgetList  *list = QApplication::topLevelWidgets();
            QWidgetListIt it( *list );
            QWidget * w;
            while ( (w=it.current()) != 0 ) {   // for each top level widget...
                ++it;
                if (!w->inherits("UserAutoReplyDlg")) continue;
                UserAutoReplyDlg *dlg = static_cast<UserAutoReplyDlg*>(w);
                if (dlg->uin == uin) break;
            }
            if (w == NULL)
                w = new UserAutoReplyDlg(uin);
            raiseWindow(w);
            delete list;
            return;
        }
    case mnuMail:
        sendMail(uin);
        return;
    case mnuSecureOn:{
            QWidget *w = secureWindow(uin);
            if (w == NULL)
                w = new SecureDlg(this, uin);
            raiseWindow(w);
            return;
        }
    case mnuSecureOff:{
            ICQUser *u = pClient->getUser(uin);
            if (u) pClient->closeSecureChannel(u);
            return;
        }
    case mnuSort:
        setAlphabetSort(!isAlphabetSort());
        users->refresh();
        return;
    case mnuAlert:
    case mnuAccept:
    case mnuSound:
    case mnuInfo:{
            param = 0;
            switch (function){
            case mnuAlert:
                param = SETUP_ALERT;
                break;
            case mnuAccept:
                param = SETUP_ACCEPT;
                break;
            case mnuSound:
                param = SETUP_SOUND;
                break;
            }
            UserInfo *info = NULL;
            QWidgetList *list = QApplication::topLevelWidgets();
            QWidgetListIt it(*list);
            QWidget *w;
            while ( (w=it.current()) != NULL) {
                ++it;
                if (!w->inherits("UserInfo"))
                    continue;
                info = static_cast<UserInfo*>(w);
                if (info->Uin() == uin)
                    break;
                info = NULL;
            }
            delete list;
            if (info){
                raiseWindow(info);
                if (param)
                    info->raiseWidget(param);
            }else{
                info = new UserInfo(uin, 0, param);
                raiseWindow(info);
            }
            return;
        }
    case mnuHistoryNew:
        for (it = containers.begin(); it != containers.end(); ++it){
            if (!(*it)->isHistory()) continue;
            if (!(*it)->haveUser(uin)) continue;
            (*it)->showUser(uin, mnuHistory, 0);
            return;
        }
        box = new UserBox(uin);
        containers.push_back(box);
        box->setHistory(true);
        box->hideToolbar();
        box->showUser(uin, mnuHistory, 0);
        return;
    default:
        log(L_WARN, "No user function %u for %lu", function, uin);
    }
}

bool MainWindow::isHistory(unsigned long uin)
{
    list<UserBox*>::iterator it;
    for (it = containers.begin(); it != containers.end(); ++it){
        if (!(*it)->isHistory()) continue;
        if ((*it)->haveUser(uin)) return true;
    }
    return false;
}

UserFloat *MainWindow::findFloating(unsigned long uin, bool bDelete)
{
    list<UserFloat*>::iterator it;
    for (it = floating.begin(); it != floating.end(); it++){
        if ((*it)->getUin() == uin){
            UserFloat *res = *it;
            if (bDelete) floating.remove(res);
            return res;
        }
    }
    return NULL;
}

void MainWindow::toContainer(int containerId)
{
    list<UserBox*>::iterator it;
    for (it = containers.begin(); it != containers.end(); ++it){
        if ((*it)->isHistory()) continue;
        if (!(*it)->haveUser(uinMenu)) continue;
        if ((*it)->getGrpId() == (unsigned long)containerId) return;
        (*it)->closeUser(uinMenu);
    }
    for (it = containers.begin(); it != containers.end(); ++it)
        if ((*it)->getGrpId() == (unsigned long)containerId) break;
    UserBox *box = NULL;
    if (it != containers.end()) box = *it;
    if (box == NULL){
        box = new UserBox(containerId);
        containers.push_back(box);
    }
    box->showUser(uinMenu, mnuAction, 0);
}

void MainWindow::adjustUserMenu(QPopupMenu *menu, ICQUser *u, bool haveTitle, bool bShort)
{
    if (haveTitle){
        uinMenu = u->Uin;
        menuContainers->clear();
        bool canNew = true;
        list<UserBox*>::iterator it;
        for (it = containers.begin(); it != containers.end(); ++it){
            if ((*it)->isHistory()) continue;
            menuContainers->insertItem((*it)->containerName(), (*it)->getGrpId());
            if (!(*it)->haveUser(uinMenu)) continue;
            menuContainers->setItemChecked((*it)->getGrpId(), true);
            if (((*it)->getGrpId() >= 0x10000L) && ((*it)->count() == 1))
                canNew = false;
        }
        if (canNew){
            unsigned long newId = 0x7FFFFFFF;
            for (it = containers.begin(); it != containers.end(); ++it){
                if ((*it)->isHistory()) continue;
                if ((*it)->getGrpId() < 0x10000L) continue;
                if ((*it)->getGrpId() <= newId) newId = (*it)->getGrpId() - 1;
            }
            menuContainers->insertItem(i18n("New"), newId);
        }
        if (menu->findItem(mnuContainers) == NULL)
            menu->insertItem(i18n("To container"), menuContainers, mnuContainers);
    }
    addMessageType(menu, ICQ_MSGxSECURExOPEN,  mnuSecureOn, !bShort, haveTitle, u->Uin);
    addMessageType(menu, ICQ_MSGxSECURExCLOSE, mnuSecureOff, !bShort, haveTitle, u->Uin);
    addMenuItem(menu, SIMClient::getStatusIcon(u->uStatus),
                i18n("Read %1 message") .arg(SIMClient::getStatusText(u->uStatus)),
                mnuAutoResponse,
                (u->uStatus != ICQ_STATUS_OFFLINE) && ((u->uStatus & 0xFF) != ICQ_STATUS_ONLINE), haveTitle);
    addMessageType(menu, ICQ_MSGxMAIL, mnuMail, true, haveTitle, u->Uin);
    addMessageType(menu, ICQ_MSGxAUTHxREQUEST, mnuAuth, true, haveTitle, u->Uin);
    addMessageType(menu, ICQ_MSGxCHAT, mnuChat, true, haveTitle, u->Uin);
    addMessageType(menu, ICQ_MSGxFILE, mnuFile, true, haveTitle, u->Uin);
    addMessageType(menu, ICQ_MSGxCONTACTxLIST, mnuContacts, true, haveTitle, u->Uin);
    addMessageType(menu, ICQ_MSGxSMS, mnuSMS, true, haveTitle, u->Uin);
    addMessageType(menu, ICQ_MSGxURL, mnuURL, true, haveTitle, u->Uin);
    addMessageType(menu, ICQ_MSGxMSG, mnuMessage, true, haveTitle, u->Uin);
    menu->setItemEnabled(mnuAlert, u->Type == USER_TYPE_ICQ);
    menu->setItemEnabled(mnuAccept, u->Type == USER_TYPE_ICQ);
    menu->setItemEnabled(mnuSound, u->Type == USER_TYPE_ICQ);
}

void MainWindow::goURL(const char *url)
{
#ifdef WIN32
    ShellExecuteA(winId(), NULL, url, NULL, NULL, SW_SHOWNORMAL);
#else
    exec(getUrlViewer(), url);
#endif
}

void MainWindow::sendMail(unsigned long uin)
{
    const char *mail = NULL;
    ICQUser *u = pClient->getUser(uin);
    if (u){
        for (EMailList::iterator it = u->EMails.begin(); it != u->EMails.end(); ++it){
            EMailInfo *mailInfo = static_cast<EMailInfo*>(*it);
            if (*mailInfo->Email.c_str()){
                mail = mailInfo->Email.c_str();
                break;
            }
        }
    }
    sendMail(mail);
}

void MainWindow::sendMail(const char *mail)
{
    if (mail == NULL) return;

#ifdef WIN32
    string s = "mailto:";
    s += mail;
    ShellExecuteA(winId(), NULL, s.c_str(), NULL, NULL, SW_SHOWNORMAL);
#else
    exec(getMailClient(), mail);
#endif
}

void MainWindow::setUserBoxOnTop()
{
    emit onTopChanged();
}

void MainWindow::exec(const char *prg, const char *arg)
{
#ifndef WIN32
    if (*prg == 0) return;
    QString p = QString::fromLocal8Bit(prg);
    if (p.find("%s") >= 0){
        p.replace(QRegExp("%s"), arg);
    }else{
        p += " ";
        p += QString::fromLocal8Bit(arg);
    }
    QStringList s = QStringList::split(" ", p);
    char **arglist = new char*[s.count()+1];
    unsigned i = 0;
    for ( QStringList::Iterator it = s.begin(); it != s.end(); ++it, i++ ) {
        string arg;
        arg = (*it).local8Bit();
        arglist[i] = strdup(arg.c_str());
    }
    arglist[i] = NULL;
    if(!fork()) {
        if (execvp(arglist[0], arglist))
            log(L_WARN, "can't execute %s: %s", arglist[0], strerror(errno));
        _exit(-1);
    }
    for (char **p = arglist; *p != NULL; p++)
        free(*p);
    delete[] arglist;
#endif
}

void MainWindow::playSound(const char *wav)
{
    if ((wav == NULL) || (*wav == 0) || pSplash->isSoundDisable()) return;
    wav = sound(wav);
#ifdef WIN32
    sndPlaySoundA(wav, SND_ASYNC | SND_NODEFAULT);
#else
#ifdef USE_KDE
    if (pSplash->isUseArts()){
        KAudioPlayer::play(wav);
        return;
    }
#endif
    if (*(pSplash->getSoundPlayer()) == 0) return;

    const char *arglist[3];
    arglist[0] = pSplash->getSoundPlayer();
    arglist[1] = wav;
    arglist[2] = NULL;

    if(!fork()) {
        execvp(arglist[0], (char**)arglist);
        _exit(-1);
    }
#endif
}

void MainWindow::addNonIM()
{
    ICQUser *u = pClient->getUser(0, true, true);
    u->Type = USER_TYPE_EXT;
    userFunction(u->Uin, mnuInfo);
}

void MainWindow::sendSMS()
{
    ICQUser *u = pClient->getUser(0, true, true);
    u->Type = USER_TYPE_EXT;
    showUser(u->Uin, mnuSMS);
}

void MainWindow::setFonts()
{
#ifdef USE_KDE
    if (isUseSystemFonts()) return;
#endif
    QPopupMenu p;
    qApp->setFont(str2font(getFont(), font()), true);
    qApp->setFont(str2font(getFontMenu(), p.font()), true, "QPopupMenu");
}

extern KAboutData *appAboutData;

void MainWindow::bug_report()
{
    goURL("http://sourceforge.net/tracker/?group_id=56866");
}

void MainWindow::networkMonitor()
{
    if (mNetMonitor == NULL)
    {
        mNetMonitor = new MonitorWindow();
        connect(mNetMonitor, SIGNAL(finished()), this, SLOT(monitorFinished()));
    }
    raiseWindow(mNetMonitor);
}

void MainWindow::monitorFinished()
{
    mNetMonitor = NULL;
}

void MainWindow::randomChat()
{
    if (mRandomChat == NULL)
    {
        mRandomChat = new RandomChat();
        connect(mRandomChat, SIGNAL(finished()), this, SLOT(randomChatFinished()));
    }
    raiseWindow(mRandomChat);
}

void MainWindow::randomChatFinished()
{
    mRandomChat = NULL;
}

void MainWindow::about()
{
    if( mAboutApp == 0 )
    {
        mAboutApp = new KAboutApplication( appAboutData, this, "about", false );
        connect( mAboutApp, SIGNAL(finished()), this, SLOT( dialogFinished()) );
    }
    mAboutApp->show();
}

void MainWindow::about_kde()
{
#ifdef USE_KDE
    if( mAboutKDE == 0 )
    {
        mAboutKDE = new KAboutKDE( this, "aboutkde", false );
        connect( mAboutKDE, SIGNAL(finished()), this, SLOT( dialogFinished()) );
    }
    mAboutKDE->show();
#endif
}

void MainWindow::dialogFinished()
{
    QTimer::singleShot( 0, this, SLOT(timerExpired()) );
}


void MainWindow::timerExpired()
{
#ifdef USE_KDE
    if( mAboutKDE != 0 && mAboutKDE->isVisible() == false )
    {
        delete mAboutKDE; mAboutKDE = NULL;
    }
#endif
    if( mAboutApp != 0 && mAboutApp->isVisible() == false )
    {
        delete mAboutApp; mAboutApp = NULL;
    }
}

void MainWindow::changeIcons(int)
{
    initIcons(getIcons());
    loadMenu();
    setIcons();
    emit iconChanged();
}

void MainWindow::loadMenu()
{
    menuStatus->clear();
    addStatusItem(ICQ_STATUS_ONLINE);
    addStatusItem(ICQ_STATUS_AWAY);
    addStatusItem(ICQ_STATUS_NA);
    addStatusItem(ICQ_STATUS_DND);
    addStatusItem(ICQ_STATUS_OCCUPIED);
    addStatusItem(ICQ_STATUS_FREEFORCHAT);
    addStatusItem(ICQ_STATUS_OFFLINE);
    menuStatus->insertSeparator();
    menuStatus->insertItem(Icon("invisible"), i18n("Invisible"), ICQ_STATUS_FxPRIVATE);

    menuPhoneStatus->clear();
    menuPhoneStatus->insertItem(i18n("Don't show"), 0);
    menuPhoneStatus->insertItem(Icon("phone"), i18n("Available"), 1);
    menuPhoneStatus->insertItem(Icon("nophone"), i18n("Busy"), 2);

    menuGroups->clear();
    menuGroups->insertItem(Pict("grp_create"), i18n("Create group"), mnuGrpCreate);
    menuGroups->insertSeparator();
    menuGroups->insertItem(Pict("grp_expand"), i18n("Expand all"), mnuGrpExpandAll);
    menuGroups->insertItem(Pict("grp_collapse"), i18n("Collapse all"), mnuGrpCollapseAll);
    menuGroups->insertSeparator();
    menuGroups->insertItem(i18n("Customize toolbar..."), mnuToolBar);

    menuFunction->clear();
    menuFunction->insertTitle(i18n("ICQ"), 1);
    menuFunction->setCheckable(true);
    menuFunction->insertItem(Icon("find"), i18n("Find User"), this, SLOT(search()));
    menuFunction->insertItem(Icon("nonim"), i18n("Add Non-IM contact"), this, SLOT(addNonIM()));
    menuFunction->insertItem(Icon("sms"), i18n("Send SMS"), this, SLOT(sendSMS()));
    menuFunction->insertItem(Icon("randomchat"), i18n("Random chat"), this, SLOT(randomChat()));
    menuFunction->insertSeparator();
    menuFunction->insertItem(i18n("Status"), menuStatus, mnuPopupStatus);
    menuFunction->insertSeparator();
    menuFunction->insertItem(Icon("configure"), i18n("Setup"), this, SLOT(setup()));
    menuFunction->insertItem(Icon("grp_on"), i18n("Groups"), menuGroups);
    menuFunction->insertItem(Icon("phone"), i18n("Phone \"Follow Me\""), menuPhone);
#ifdef DEBUG
    menuFunction->insertItem(i18n("Shared files"), this, SLOT(sharedFiles()), 0, mnuShare);
#endif
#ifdef WIN32
    menuFunction->insertSeparator();
    menuFunction->insertItem(i18n("Always on top"), this, SLOT(toggleOnTop()), 0, mnuOnTop);
#endif
    menuFunction->insertSeparator();
    menuFunction->insertItem(i18n("Network monitor"), this, SLOT(networkMonitor()));
    menuFunction->insertItem(i18n("&Bug report"), this, SLOT(bug_report()));
    menuFunction->insertItem(Icon("licq"), i18n("&About SIM"), this, SLOT(about()));
#ifdef USE_KDE
    menuFunction->insertItem(Icon("about_kde"), i18n("About &KDE"), this, SLOT(about_kde()));
#endif
    menuFunction->insertSeparator();
    menuFunction->insertItem(i18n("Change UIN"), this, SLOT(changeUIN()));
    menuFunction->insertItem(Icon("exit"), i18n("Quit"), this, SLOT(quit()));

    menuFunctionDock->clear();
    menuFunctionDock->setCheckable(true);
    menuFunctionDock->insertItem("", this, SLOT(toggleShow()), 0, mnuToggleWnd);
    menuFunctionDock->insertItem(i18n("Status"), menuStatus, mnuPopupStatus);
    menuFunctionDock->insertSeparator();
    menuFunctionDock->insertItem(Icon("exit"), i18n("Quit"), this, SLOT(quit()));
}

void MainWindow::changeWm()
{
#ifdef USE_KDE
    if (isMainWindowInTaskManager()){
        KWin::clearState(winId(), NET::SkipTaskbar);
    }else{
        KWin::setState(winId(), NET::SkipTaskbar);
    }
#endif
#ifdef WIN32
    setInBar(isMainWindowInTaskManager() && (getBarState() == ABE_FLOAT));
#endif
    emit wmChanged();
}

QWidget *MainWindow::chatWindow(unsigned long uin)
{
    QWidget *res = NULL;
    QWidgetList *list = QApplication::topLevelWidgets();
    QWidgetListIt it(*list);
    QWidget *w;
    while ( (w=it.current()) != NULL) {
        ++it;
        if (w->inherits("ChatWindow")){
            ChatWindow *chat = static_cast<ChatWindow*>(w);
            if (chat->chat && (chat->chat->getUin() == uin))
                res = chat;
        }else{
            QObjectList *l = w->queryList("ChatWindow");
            QObjectListIt it(*l);
            QObject *obj;
            while ((obj=it.current()) != NULL){
                ++it;
                ChatWindow *chat = static_cast<ChatWindow*>(obj);
                if (chat->chat && (chat->chat->getUin() == uin))
                    res = chat;
            }
            delete l;
        }
    }
    delete list;
    return res;
}

void MainWindow::chatClose()
{
    emit chatChanged();
}

QWidget *MainWindow::secureWindow(unsigned long uin)
{
    QWidget *res = NULL;
    QWidgetList *list = QApplication::topLevelWidgets();
    QWidgetListIt it(*list);
    QWidget *w;
    while ( (w=it.current()) != NULL) {
        ++it;
        if (w->inherits("SecureDlg")){
            SecureDlg *s = static_cast<SecureDlg*>(w);
            if (s->Uin == uin)
                res = s;
        }else{
            QObjectList *l = w->queryList("SecureDlg");
            QObjectListIt it(*l);
            QObject *obj;
            while ((obj=it.current()) != NULL){
                ++it;
                SecureDlg *s = static_cast<SecureDlg*>(obj);
                if (s->Uin == uin)
                    res = s;
            }
            delete l;
        }
    }
    delete list;
    return res;
}

QWidget *MainWindow::ftWindow(unsigned long uin, const string &fileName)
{
    QWidget *res = NULL;
    QWidgetList *list = QApplication::topLevelWidgets();
    QWidgetListIt it(*list);
    QWidget *w;
    while ( (w=it.current()) != NULL) {
        ++it;
        if (w->inherits("FileTransferDlg")){
            FileTransferDlg *ft = static_cast<FileTransferDlg*>(w);
            if (ft->file && (ft->file->getUin() == uin) && (ft->file->shortName() == fileName))
                res = ft;
        }else{
            QObjectList *l = w->queryList("FileTransferDlg");
            QObjectListIt it(*l);
            QObject *obj;
            while ((obj=it.current()) != NULL){
                ++it;
                FileTransferDlg *ft = static_cast<FileTransferDlg*>(obj);
                if (ft->file && (ft->file->getUin() == uin) && (ft->file->shortName() == fileName))
                    res = ft;
            }
            delete l;
        }
    }
    delete list;
    return res;
}

void MainWindow::ftClose()
{
    emit ftChanged();
}

void MainWindow::loadUnread()
{
    messages.clear();
    for (list<ICQUser*>::iterator it = pClient->contacts.users.begin(); it != pClient->contacts.users.end(); it++){
        if ((*it)->unreadMsgs.size() == 0) continue;
        History h((*it)->Uin);
        for (list<unsigned long>::iterator msgs = (*it)->unreadMsgs.begin(); msgs != (*it)->unreadMsgs.end(); msgs++){
            ICQMessage *msg = h.getMessage(*msgs);
            if (msg == NULL) continue;
            unread_msg m(msg);
            messages.push_back(m);
            if (msg->Id < MSG_PROCESS_ID)
                delete msg;
        }
    }
}

void MainWindow::sharedFiles()
{
    pClient->setShare(!pClient->ShareOn);
}

void MainWindow::messageRead(ICQMessage *msg)
{
    unread_msg m(msg);
    messages.remove(m);
    emit msgChanged();
}

bool msgInfo::operator < (const msgInfo &m) const
{
    if (uin < m.uin) return true;
    if (uin > m.uin) return false;
    return type < m.type;
}

void MainWindow::fillUnread(list<msgInfo> &msgs)
{
    msgs.clear();
    for (list<unread_msg>::iterator it = messages.begin(); it != messages.end(); ++it){
        list<msgInfo>::iterator it_msg;
        for (it_msg = msgs.begin(); it_msg != msgs.end(); ++it_msg)
            if (((*it_msg).uin == (*it).uin()) && ((*it_msg).type == (*it).type()))
                break;
        if (it_msg != msgs.end()){
            (*it_msg).count++;
            continue;
        }
        msgInfo info;
        info.uin = (*it).uin();
        info.type = (*it).type();
        info.count = 1;
        msgs.push_back(info);
    }
    msgs.sort();
}

#if !defined(USE_KDE) || (QT_VERSION < 300)
void resetPlural();
#endif

#if !defined(USE_KDE) && (QT_VERSION >= 300)

struct loaded_domain;

struct loaded_l10nfile
{
    const char *filename;
    int decided;
    const void *data;
    loaded_l10nfile() : filename(0), decided(0), data(0) {}
};


void k_nl_load_domain (struct loaded_l10nfile *domain_file);
void k_nl_unload_domain (struct loaded_domain *domain);
char *k_nl_find_msg (struct loaded_l10nfile *domain_file, const char *msgid);

class SIMTranslator : public QTranslator
{
public:
    SIMTranslator(QObject *parent, const QString & filename);
    ~SIMTranslator();
    virtual QTranslatorMessage findMessage(const char* context,
                                           const char *sourceText,
                                           const char* message) const;
    void load ( const QString & filename);
protected:
    string fName;
    loaded_l10nfile domain;
};

SIMTranslator::SIMTranslator(QObject *parent, const QString & filename)
        : QTranslator(parent, "simtranslator")
{
    load(filename);
}

SIMTranslator::~SIMTranslator()
{
    if (domain.data)
        k_nl_unload_domain((struct loaded_domain *)domain.data);
}

void SIMTranslator::load ( const QString & filename)
{
    fName = filename.local8Bit();
    domain.filename = (char*)(fName.c_str());
    k_nl_load_domain(&domain);
}

QTranslatorMessage SIMTranslator::findMessage(const char* context,
        const char *sourceText,
        const char* message) const
{
    QTranslatorMessage res;
    char *text = NULL;
    if ((sourceText == NULL) || (*sourceText == 0))
        return res;
    if (message && *message){
        string s;
        s = "_: ";
        s += message;
        s += "\n";
        s += sourceText;
        text = k_nl_find_msg((struct loaded_l10nfile*)&domain, s.c_str());
    }
    if ((text == NULL) && context && *context && message && *message){
        string s;
        s = "_: ";
        s += context;
        s += "\n";
        s += message;
        text = k_nl_find_msg((struct loaded_l10nfile*)&domain, s.c_str());
    }
    if (text == NULL)
        text = k_nl_find_msg((struct loaded_l10nfile*)&domain, sourceText);
    if (text)
        res.setTranslation(QString::fromUtf8(text));
    return res;
}

#endif

void MainWindow::initTranslator()
{
    if (translator)
        qApp->removeTranslator(translator);
    translator = NULL;
    string lang = pSplash->getLanguage();
    if (*lang.c_str() == '-') return;
    if (lang.size() == 0){
#ifdef WIN32
        char buff[256];
        int res = GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SABBREVLANGNAME, buff, sizeof(buff));
        if (res){
            lang += tolower(buff[0]);
            lang += tolower(buff[1]);
        }
#else
        char *p = getenv("LANG");
        if (p){
            for (; *p; p++){
                if (*p == '.') break;
                lang += *p;
            }
        }
#endif
    }
    if (lang.size() == 0) return;
#ifdef WIN32
    string s = "po";
    s += "\\";
    for (const char *p = lang.c_str(); *p; p++)
        s += tolower(*p);
    s += ".qm";
    s = app_file(s.c_str());
    QFile f(QString::fromLocal8Bit(s.c_str()));
    if (!f.exists()) return;
#else
    string s = PREFIX "/share/locale/";
    char *p = (char*)(lang.c_str());
    char *r = strchr(p, '.');
    if (r) *r = 0;
    s += lang.c_str();
    s += "/LC_MESSAGES/sim.mo";
    QFile f(QString::fromLocal8Bit(s.c_str()));
    if (!f.exists()){
        r = strchr(p, '_');
        if (r) *r = 0;
        s = PREFIX "/share/locale/";
        s += lang.c_str();
        s += "/LC_MESSAGES/sim.mo";
        f.setName(QString::fromLocal8Bit(s.c_str()));
        if (!f.exists()) return;
    }
#endif
#if !defined(USE_KDE) && (QT_VERSION >= 300)
    translator = new SIMTranslator(this, f.name());
#else
    translator = new QTranslator(this);
    translator->load(f.name());
#endif
    qApp->installTranslator(translator);
    appAboutData->setTranslator(I18N_NOOP("_: NAME OF TRANSLATORS\nYour names"),
                                I18N_NOOP("_: EMAIL OF TRANSLATORS\nYour emails"));
#if !defined(USE_KDE) || (QT_VERSION < 300)
    resetPlural();
#endif
}

static QString s_tr(const char *s, bool use_tr)
{
    if (use_tr) return i18n(s);
    return s;
}

QString MainWindow::font2str(const QFont &f, bool use_tr)
{
    QString fontName = f.family();
    fontName += ", ";
    if (f.pointSize() > 0){
        fontName += QString::number(f.pointSize());
        fontName += " pt.";
    }else{
        fontName += QString::number(f.pixelSize());
        fontName += " pix.";
    }
    switch (f.weight()){
    case QFont::Light:
        fontName += ", ";
        fontName += s_tr(I18N_NOOP("light"), use_tr);
        break;
    case QFont::DemiBold:
        fontName += ", ";
        fontName += s_tr(I18N_NOOP("demibold"), use_tr);
        break;
    case QFont::Bold:
        fontName += ", ";
        fontName += s_tr(I18N_NOOP("bold"), use_tr);
        break;
    case QFont::Black:
        fontName += ", ";
        fontName += s_tr(I18N_NOOP("black"), use_tr);
        break;
    default:
        break;
    }
    if (f.italic()){
        fontName += ", ";
        fontName += s_tr(I18N_NOOP("italic"), use_tr);
    }
    return fontName;
}

QFont MainWindow::str2font(const char *str, const QFont &def)
{
    QFont f(def);
    QStringList l = QStringList::split(QRegExp(" *, *"), QString::fromLocal8Bit(str));
    if (l.count() == 0) return f;
    int weight = QFont::Normal;
    bool italic = false;
    f.setFamily(l[0]);
    for (unsigned i = 1; i < l.count(); i++){
        QString s = l[i];
        if (s == "italic"){
            italic = true;
            continue;
        }
        if (s == "light"){
            weight = QFont::Light;
            continue;
        }
        if (s == "demibold"){
            weight = QFont::DemiBold;
            continue;
        }
        if (s == "bold"){
            weight = QFont::Bold;
            continue;
        }
        if (s == "black"){
            weight = QFont::Black;
            continue;
        }
        int p = s.find(QRegExp("[0-9]+ *pt"));
        if (p >= 0){
            s = s.mid(p);
            int size = atol(s.latin1());
            if (size > 0)
                f.setPointSize(size);
            continue;
        }
        p = s.find(QRegExp("[0-9]+ *pix"));
        if (p >= 0){
            s = s.mid(p);
            int size = atol(s.latin1());
            if (size > 0)
                f.setPixelSize(size);
            continue;
        }
    }
    f.setItalic(italic);
    f.setWeight(weight);
    return f;
}

void MainWindow::addSearch(const QString &t)
{
    QStringList::Iterator it;
    for (it = searches.begin(); it != searches.end(); ++it)
        if ((*it) == t) break;
    if (it != searches.end())
        searches.remove(it);
    searches.prepend(t);
    emit searchChanged();
}

bool MainWindow::eventFilter(QObject *o, QEvent *e)
{
    if ((o == menuUser) && (e->type() == QEvent::Hide))
        QTimer::singleShot(0, this, SLOT(clearUserMenu()));
    return QMainWindow::eventFilter(o, e);
}

void MainWindow::clearUserMenu()
{
    menuUser->clear();
}

void MainWindow::checkChilds()
{
#ifndef WIN32
    for (;;){
        int status;
        pid_t child = waitpid(0, &status, WNOHANG);
        if ((child == 0) || (child == -1)) break;
        if (!WIFEXITED(status)) continue;
        emit childExited(child, WEXITSTATUS(status));
    }
#endif
}

void MainWindow::changeToolBar(const ToolBarDef *d)
{
    emit toolBarChanged(d);
}

void MainWindow::autoHide()
{
#ifdef WIN32
    if (isActiveWindow()) return;
    DWORD pos = GetMessagePos();
    int x = GET_X_LPARAM(pos);
    int y = GET_Y_LPARAM(pos);
    RECT rc;
    GetWindowRect(winId(), &rc);
    rc.left -= GetSystemMetrics(SM_CXDOUBLECLK) * 2;
    rc.right += GetSystemMetrics(SM_CXDOUBLECLK) * 2;
    if ((x >= rc.left) && (x <= rc.right) && (y >= rc.top) && (y <= rc.bottom)) return;
    if ((getBarState() != ABE_FLOAT) && isBarAutoHide() && bAutoHideVisible && !noToggle){
        bAutoHideVisible = false;
        setBarState(true);
    }
#endif
}

void MainWindow::changeBackground()
{
    emit bgChanged();
}

string MainWindow::getFullPath(const char *path, bool bIsDir)
{
    if (pClient->owner->Uin == 0)
        return "";
    char b[32];
    snprintf(b, sizeof(b), "%lu", pClient->owner->Uin);
    string p;
    p = b;
#ifdef WIN32
    p += "\\";
#else
    p += "/";
#endif
    p += path;
    if (bIsDir){
#ifdef WIN32
        p += "\\";
#else
        p += "/";
#endif
    }
    return buildFileName(p.c_str());
}

void MainWindow::deleteLogin()
{
    if (pLoginDlg == NULL) return;
    delete pLoginDlg;
    pLoginDlg = NULL;
}

#ifndef _WINDOWS
#include "mainwin.moc"
#endif

