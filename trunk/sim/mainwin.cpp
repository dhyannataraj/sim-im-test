/***************************************************************************
                          mainwin.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
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
#include "passwddlg.h"
#include "transparent.h"
#include "ui/searchdlg.h"
#include "ui/setupdlg.h"
#include "ui/autoreply.h"
#include "ui/userautoreply.h"
#include "ui/alertmsg.h"
#include "ui/ballonmsg.h"
#include "ui/filetransfer.h"
#include "ui/securedlg.h"

#include "ui/enable.h"
#include "chatwnd.h"
#include "about.h"
#include "splash.h"
#include "keys.h"
#include "history.h"

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
#include <qdns.h>
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

#ifndef WIN32
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#define XA_WINDOWMAKER_WM_PROTOCOLS      "_WINDOWMAKER_WM_PROTOCOLS"
#endif

#ifdef WIN32
const unsigned short ABE_FLOAT   = (unsigned short)(-1);
#endif

static char ICQ_CONF[] = "icq.conf";
static char SIM_CONF[] = "sim.conf";

static string app_file_name;
const char *app_file(const char *f)
{
    app_file_name = "";
#ifdef WIN32
    char buff[256];
    GetModuleFileNameA(NULL, buff, sizeof(buff));
    char *p = strrchr(buff, '\\');
    if (p) *p = 0;
    app_file_name = buff;
    if (app_file_name.length() && (app_file_name[app_file_name.length()-1] != '\\'))
        app_file_name += "\\";
#else
#ifdef USE_KDE
    QStringList lst = KGlobal::dirs()->findDirs("data", "sim");
    for (QStringList::Iterator it = lst.begin(); it != lst.end(); ++it){
        QFile f(*it + f);
        if (f.exists()){
            app_file_name = (const char*)f.name().local8Bit();
            return app_file_name.c_str();
        }
    }
    if (!lst.isEmpty()){
        app_file_name = (const char*)lst[0].local8Bit();
    }
#else
    app_file_name = PREFIX "/share/apps/sim/";
#endif
#endif
    app_file_name += f;
    return app_file_name.c_str();
}

const char *MainWindow::sound(const char *wav)
{
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
        { "Show", OFFSET_OF(MainWindow, Show), PARAM_BOOL, (unsigned)true },
        { "OnTop", OFFSET_OF(MainWindow, OnTop), PARAM_BOOL, (unsigned)true },
        { "ShowOffline", OFFSET_OF(MainWindow, ShowOffline), PARAM_BOOL, (unsigned)true },
        { "GroupMode", OFFSET_OF(MainWindow, GroupMode), PARAM_BOOL, (unsigned)true },
        { "Left", OFFSET_OF(MainWindow, mLeft), PARAM_SHORT, 0 },
        { "Top", OFFSET_OF(MainWindow, mTop), PARAM_SHORT, 0 },
        { "Width", OFFSET_OF(MainWindow, mWidth), PARAM_USHORT, 0 },
        { "Height", OFFSET_OF(MainWindow, mHeight), PARAM_USHORT, 0 },
        { "Style", OFFSET_OF(MainWindow, UseStyle), PARAM_STRING, 0 },
        { "AutoAwayTime", OFFSET_OF(MainWindow, AutoAwayTime), PARAM_ULONG, 300 },
        { "AutoNATime", OFFSET_OF(MainWindow, AutoNATime), PARAM_ULONG, 900 },
        { "AlertAway", OFFSET_OF(MainWindow, NoAlertAway), PARAM_BOOL, (unsigned)true },
        { "ManualStatus", OFFSET_OF(MainWindow, ManualStatus), PARAM_ULONG, ICQ_STATUS_OFFLINE },
        { "DivPos", OFFSET_OF(MainWindow, DivPos), PARAM_SHORT, 0 },
        { "SpellOnSend", OFFSET_OF(MainWindow, SpellOnSend), PARAM_BOOL, 0 },
        { "ToolbarDock", OFFSET_OF(MainWindow, ToolbarDock), PARAM_STRING, (unsigned)"Top" },
        { "ToolbarOffset", OFFSET_OF(MainWindow, ToolbarOffset), PARAM_SHORT, 0 },
        { "ToolbarY", OFFSET_OF(MainWindow, ToolbarY), PARAM_SHORT, 0 },
        { "URLViewer", OFFSET_OF(MainWindow, UrlViewer), PARAM_STRING, (unsigned)
#ifdef USE_KDE
          "konqueror"
#else
          "netscape"
#endif
        },
        { "MailClient", OFFSET_OF(MainWindow, MailClient), PARAM_STRING, (unsigned)
#ifdef USE_KDE
          "kmail"
#else
          "netscape mailto:%s"
#endif
        },
        { "TransparentMain", OFFSET_OF(MainWindow, UseTransparent), PARAM_BOOL, 0 },
        { "TransparencyMain", OFFSET_OF(MainWindow, Transparent), PARAM_ULONG, 80 },
        { "TransparentContainer", OFFSET_OF(MainWindow, UseTransparentContainer), PARAM_BOOL, 0 },
        { "TransparencyContainer", OFFSET_OF(MainWindow, TransparentContainer), PARAM_ULONG, 80 },
        { "TransparentIfInactive", OFFSET_OF(MainWindow, TransparentIfInactive), PARAM_BOOL, (unsigned)true },
        { "NoShowAway", OFFSET_OF(MainWindow, NoShowAway), PARAM_BOOL, 0 },
        { "NoShowNA", OFFSET_OF(MainWindow, NoShowNA), PARAM_BOOL, 0 },
        { "NoShowOccupied", OFFSET_OF(MainWindow, NoShowOccupied), PARAM_BOOL, 0 },
        { "NoShowDND", OFFSET_OF(MainWindow, NoShowDND), PARAM_BOOL, 0 },
        { "NoShowFFC", OFFSET_OF(MainWindow, NoShowFFC), PARAM_BOOL, 0 },
        { "UseSystemFonts", OFFSET_OF(MainWindow, UseSystemFonts), PARAM_BOOL, 0 },
        { "Font", OFFSET_OF(MainWindow, Font), PARAM_STRING, 0 },
        { "FontMenu", OFFSET_OF(MainWindow, FontMenu), PARAM_STRING, 0 },
        { "ColorSend", OFFSET_OF(MainWindow, ColorSend), PARAM_ULONG, 0x0000B0 },
        { "ColorReceive", OFFSET_OF(MainWindow, ColorReceive), PARAM_ULONG, 0xB00000 },
        { "ChatWidth", OFFSET_OF(MainWindow, ChatWidth), PARAM_USHORT, 0 },
        { "ChatHeight", OFFSET_OF(MainWindow, ChatHeight), PARAM_USHORT, 0 },
        { "UserBoxX", OFFSET_OF(MainWindow, UserBoxX), PARAM_USHORT, 0 },
        { "UserBoxY", OFFSET_OF(MainWindow, UserBoxY), PARAM_USHORT, 0 },
        { "UserBoxWidth", OFFSET_OF(MainWindow, UserBoxWidth), PARAM_SHORT, 0 },
        { "UserBoxHeight", OFFSET_OF(MainWindow, UserBoxHeight), PARAM_SHORT, 0 },
        { "UserBoxToolbarDock", OFFSET_OF(MainWindow, UserBoxToolbarDock), PARAM_STRING, (unsigned)"Top" },
        { "UserBoxToolbarOffset", OFFSET_OF(MainWindow, UserBoxToolbarOffset), PARAM_SHORT, 0 },
        { "UserBoxToolbarY", OFFSET_OF(MainWindow, UserBoxToolbarY), PARAM_SHORT, 0 },
        { "UserBoxFont", OFFSET_OF(MainWindow, UserBoxFont), PARAM_STRING, 0 },
	{ "UserBoxEditHeight", OFFSET_OF(MainWindow, UserBoxEditHeight), PARAM_USHORT, 0 },
        { "CloaseAfterSend", OFFSET_OF(MainWindow, CloseAfterSend), PARAM_BOOL, 0 },
        { "CloaseAfterFileTransfer", OFFSET_OF(MainWindow, CloseAfterFileTransfer), PARAM_BOOL, 0 },
        { "MainWindowInTaskManager", OFFSET_OF(MainWindow, MainWindowInTaskManager), PARAM_BOOL, 0 },
        { "UserWindowInTaskManager", OFFSET_OF(MainWindow, UserWindowInTaskManager), PARAM_BOOL, (unsigned)true },
        { "Icons", OFFSET_OF(MainWindow, Icons), PARAM_STRING, 0 },
        { "XOSD_on", OFFSET_OF(MainWindow, XOSD_on), PARAM_BOOL, (unsigned)true },
        { "XOSD_pos", OFFSET_OF(MainWindow, XOSD_pos), PARAM_SHORT, 0 },
        { "XOSD_offset", OFFSET_OF(MainWindow, XOSD_offset), PARAM_SHORT, 30 },
        { "XOSD_color", OFFSET_OF(MainWindow, XOSD_color), PARAM_ULONG, 0x00E000 },
        { "XOSD_Font", OFFSET_OF(MainWindow, XOSD_Font), PARAM_STRING, 0 },
        { "XOSD_timeout", OFFSET_OF(MainWindow, XOSD_timeout), PARAM_USHORT, 7 },
        { "XOSD_Shadow", OFFSET_OF(MainWindow, XOSD_Shadow), PARAM_BOOL, (unsigned)true },
        { "XOSD_Background", OFFSET_OF(MainWindow, XOSD_Background), PARAM_BOOL, 0 },
        { "XOSD_Bgcolor", OFFSET_OF(MainWindow, XOSD_BgColor), PARAM_ULONG, (unsigned)(-1) },
        { "ContainerMode", OFFSET_OF(MainWindow, ContainerMode), PARAM_SHORT, ContainerModeGroup },
        { "MessageBgColor", OFFSET_OF(MainWindow, MessageBgColor), PARAM_ULONG, 0 },
        { "MessageFgColor", OFFSET_OF(MainWindow, MessageFgColor), PARAM_ULONG, 0 },
        { "SimpleMode", OFFSET_OF(MainWindow, SimpleMode), PARAM_BOOL, 0 },
        { "UseOwnColors", OFFSET_OF(MainWindow, UseOwnColors), PARAM_BOOL, 0 },
        { "UserWndOnTop", OFFSET_OF(MainWindow, UserWndOnTop), PARAM_BOOL, 0 },
        { "KeyWindow", OFFSET_OF(MainWindow, KeyWindow), PARAM_STRING, (unsigned)"CTRL-SHIFT-A" },
        { "KeyDblClick", OFFSET_OF(MainWindow, KeyDblClick), PARAM_STRING, (unsigned)"CTRL-SHIFT-I" },
        { "KeySearch", OFFSET_OF(MainWindow, KeySearch), PARAM_STRING, (unsigned)"CTRL-SHIFT-S" },
        { "UseEmotional", OFFSET_OF(MainWindow, UseEmotional), PARAM_BOOL, (unsigned)true },
        { "AutoHideTime", OFFSET_OF(MainWindow, AutoHideTime), PARAM_ULONG, 60 },
        { "SMSSignTop", OFFSET_OF(MainWindow, SMSSignTop), PARAM_STRING, 0 },
        { "SMSSignBottom", OFFSET_OF(MainWindow, SMSSignBottom), PARAM_STRING, (unsigned)"\n&MyAlias; (ICQ# &MyUin;)" },
        { "ForwardPhone", OFFSET_OF(MainWindow, ForwardPhone), PARAM_STRING, 0 },
        { "SendEnter", OFFSET_OF(MainWindow, SendEnter), PARAM_BOOL, 0 },
        { "AlphabetSort", OFFSET_OF(MainWindow, AlphabetSort), PARAM_BOOL, 0 },
        { "UseDock", OFFSET_OF(MainWindow, UseDock), PARAM_BOOL, 1 },
        { "DockX", OFFSET_OF(MainWindow, DockX), PARAM_SHORT, 0 },
        { "DockY", OFFSET_OF(MainWindow, DockY), PARAM_SHORT, 0 },
        { "MonitorX", OFFSET_OF(MainWindow, MonitorX), PARAM_SHORT, 0 },
        { "MonitorY", OFFSET_OF(MainWindow, MonitorY), PARAM_SHORT, 0 },
        { "MonitorWidth", OFFSET_OF(MainWindow, MonitorWidth), PARAM_USHORT, 0 },
        { "MonitorHeight", OFFSET_OF(MainWindow, MonitorHeight), PARAM_USHORT, 0 },
        { "MonitorLevel", OFFSET_OF(MainWindow, MonitorLevel), PARAM_USHORT, L_PACKET | L_DEBUG | L_WARN | L_ERROR },
        { "CopyMessages", OFFSET_OF(MainWindow, CopyMessages), PARAM_USHORT, 3 },
        { "AllEncodings", OFFSET_OF(MainWindow, AllEncodings), PARAM_BOOL, 0 },
        { "HistoryDirection", OFFSET_OF(MainWindow, HistoryDirection), PARAM_BOOL, 1 },
        { "ToolBarMain", OFFSET_OF(MainWindow, ToolBarMain), PARAM_ULONGS, 0 },
        { "ToolBarMsg", OFFSET_OF(MainWindow, ToolBarMsg), PARAM_ULONGS, 0 },
        { "ToolBarHistory", OFFSET_OF(MainWindow, ToolBarHistory), PARAM_ULONGS, 0 },
        { "ToolBarUserBox", OFFSET_OF(MainWindow, ToolBarUserBox), PARAM_ULONGS, 0 },
        { "BackgroundFile", OFFSET_OF(MainWindow, BackgroundFile), PARAM_STRING, 0 },
        { "BackgroundMode", OFFSET_OF(MainWindow, BackgroundMode), PARAM_USHORT, 0 },
        { "IconMargin", OFFSET_OF(MainWindow, IconMargin), PARAM_USHORT, 2 },
        { "UseSystemColors", OFFSET_OF(MainWindow, UseSystemColors), PARAM_BOOL, 1 },
        { "OnlineColor", OFFSET_OF(MainWindow, OnlineColor), PARAM_ULONG, 0 },
        { "OfflineColor", OFFSET_OF(MainWindow, OfflineColor), PARAM_ULONG, 0 },
#ifdef WIN32
        { "BarState", OFFSET_OF(MainWindow, BarState), PARAM_USHORT, ABE_FLOAT },
        { "BarAutoHide", OFFSET_OF(MainWindow, BarAutoHide), PARAM_BOOL, 0 },
#endif
#ifdef USE_KDE
        { "AutoSync", OFFSET_OF(MainWindow, AutoSync), PARAM_BOOL, 0 },
#endif
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
static bool bFullScreen = false;
static bool bOnTop = false;
static bool bAutoHideVisible = false;

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
        w = pMain->mWidth + GetSystemMetrics(SM_CXBORDER) * 2;
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
    SetWindowPos(pMain->winId(), NULL,
                 rcEnd.left(), rcEnd.top(), rcEnd.width(), rcEnd.height(),
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME);
    UpdateWindow(pMain->winId());
}


void setBarState(bool bAnimate = false)
{
    if ((pMain->BarState == ABE_FLOAT) || !pMain->Show){
        appBarMessage(ABM_SETPOS, pMain->BarState, FALSE);
    }else{
        if (pMain->BarAutoHide && !appBarMessage(ABM_SETAUTOHIDEBAR, pMain->BarState, TRUE, NULL)){
            pMain->BarAutoHide = false;
            QMessageBox::warning(NULL, i18n("Error"),
                                 i18n("There is already an auto hidden window on this edge.\nOnly one auto hidden window is allowed on each edge."),
                                 QMessageBox::Ok, 0);
        }
        QRect rc;
        getBarRect(pMain->BarState, rc);
        if (pMain->BarAutoHide){
            QRect rcAutoHide = rc;
            int w = 4 * GetSystemMetrics(SM_CXBORDER);
            if (pMain->BarState == ABE_LEFT){
                rcAutoHide.setRight(rcAutoHide.left() + w);
            }else{
                rcAutoHide.setLeft(rcAutoHide.right() - w);
            }
            appBarMessage(ABM_SETPOS, pMain->BarState, FALSE, &rcAutoHide);
            if (!bAutoHideVisible) rc = rcAutoHide;
        }else{
            appBarMessage(ABM_SETPOS, pMain->BarState, FALSE, &rc);
        }
        slideWindow(rc, bAnimate);
    }
    if ((bOnTop != pMain->OnTop) || bFullScreen){
        bOnTop = pMain->OnTop;
        HWND hState = HWND_NOTOPMOST;
        if (pMain->OnTop) hState = HWND_TOPMOST;
        if (bFullScreen) hState = HWND_BOTTOM;
        SetWindowPos(pMain->winId(), hState, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    appBarMessage(ABM_ACTIVATE);
}

static bool bSizing = false;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    unsigned type;
    RECT  *prc;
    RECT rcWnd;
    QRect rc;

    if (msg == WM_APPBAR){
        switch (wParam){
        case ABN_FULLSCREENAPP:
            bFullScreen = (lParam != 0);
            setBarState();
            break;
        case ABN_POSCHANGED:
            if (pMain->BarState != ABE_FLOAT)
                setBarState();
            break;
        }
        return 0;
    }
    switch (msg){
    case WM_ENTERSIZEMOVE:
        bInMoving = true;
        pMain->mWidth = pMain->size().width();
        bSizing = true;
        if (pMain->BarState != ABE_FLOAT) break;
        bSizing = false;
        pMain->mLeft = pMain->pos().x();
        pMain->mTop = pMain->pos().y();
        pMain->mHeight = pMain->size().height();
        break;
    case WM_EXITSIZEMOVE:
        bInMoving = false;
        GetWindowRect(pMain->winId(), &rcWnd);
        pMain->mWidth = rcWnd.right - rcWnd.left - GetSystemMetrics(SM_CXBORDER) * 2;
        pMain->BarState = getEdge();
        setBarState(true);
        break;
    case WM_NCMOUSEMOVE:
        if ((pMain->BarState != ABE_FLOAT) && pMain->BarAutoHide && !bAutoHideVisible){
            bAutoHideVisible = true;
            setBarState(true);
        }
        break;
    case WM_MOVING:
    case WM_SIZING:
        if (!bInMoving) break;
        prc = (RECT*)lParam;
        type = getEdge(prc);
        if (type == ABE_FLOAT){
            if (bSizing){
                prc->bottom = prc->top + pMain->mHeight;
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
const int btnMonitor		= 9;

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
        { btnMonitor, "network", NULL, I18N_NOOP("Network monitor"), 0, SLOT(networkMonitor()), NULL },
        END_DEF
    };

const ToolBarDef *pToolBarMain = mainWndToolBar;

MainWindow::MainWindow(const char *name)
        : QMainWindow(NULL, name, WType_TopLevel | WStyle_Customize | WStyle_Title | WStyle_NormalBorder| WStyle_SysMenu)
{
    SET_WNDPROC
    ::init(this, MainWindow_Params);

    pMain = this;
    bQuit = false;
    dock = NULL;
    noToggle = false;
    bInLogin = false;
    lockFile = -1;
    translator = NULL;
    mAboutApp = NULL;
    time_t now;
    time(&now);
    inactiveTime = now;
    mNetMonitor = NULL;

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

    QDns resolver;
    resolver.setRecordType(QDns::A);
    resolver.setLabel("localhost");

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

    menuContainers = new QPopupMenu(this);
    connect(menuContainers, SIGNAL(activated(int)), this, SLOT(toContainer(int)));

    toolbar = new CToolBar(mainWndToolBar, &ToolBarMain, this, this);
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
    transparent = new TransparentTop(this, UseTransparent, Transparent);
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
#endif
}

void MainWindow::changeMode(bool bSimple)
{
    if (SimpleMode == bSimple) return;
    SimpleMode = bSimple;
    emit modeChanged(bSimple);
}

void MainWindow::setOnTop()
{
    if (OnTop){
        setWFlags(WStyle_StaysOnTop);
    }else{
        clearWFlags(WStyle_StaysOnTop);
    }
#ifdef WIN32
    menuFunction->setItemChecked(mnuOnTop, OnTop);
    setBarState();
#else
#ifdef USE_KDE
    if (OnTop){
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
    if (bUse == UseOwnColors) return;
    UseOwnColors = bUse;
    emit ownColorsChanged();
}

void MainWindow::toggleOnTop()
{
    OnTop = !OnTop;
    setOnTop();
}

void MainWindow::toggleAutoHide()
{
#ifdef WIN32
    BarAutoHide = !BarAutoHide;
    setShow(true);
    bAutoHideVisible = true;
    setBarState();
#endif
}

void MainWindow::adjustFucntionMenu()
{
    int n;
    int oldItems = menuMsgs.size();
    fillUnread(menuMsgs);
    int index = menuFunction->indexOf(mnuPopupStatus);
    for (n = 0; n < oldItems; n++)
        menuFunction->removeItemAt(index+1);
    int id = mnuPopupStatus;
    for (list<msgInfo>::iterator it = menuMsgs.begin(); it != menuMsgs.end(); ++it){
        CUser u((*it).uin);
        (*it).menuId = ++id;
        menuFunction->insertItem(Icon(SIMClient::getMessageIcon((*it).type)),
                                 i18n("%1 from %2")
                                 .arg(SIMClient::getMessageText((*it).type, (*it).count))
                                 .arg(u.name()),
                                 id, ++index);
    }
#ifdef WIN32
    index = menuFunction->indexOf(mnuOnTop);
    id = menuFunction->idAt(index + 1);
    if (BarState == ABE_FLOAT){
        if (id == mnuAutoHide)
            menuFunction->removeItem(mnuAutoHide);
    }else{
        if (id != mnuAutoHide)
            menuFunction->insertItem(i18n("AutoHide"), this, SLOT(toggleAutoHide()), 0, mnuAutoHide, index+1);
        menuFunction->setItemChecked(mnuAutoHide, BarAutoHide);
    }
#endif
}

void MainWindow::adjustGroupsMenu()
{
    menuGroups->setItemEnabled(mnuGrpCreate, GroupMode && pClient->isLogged());
    menuGroups->setItemEnabled(mnuGrpCollapseAll, GroupMode);
    menuGroups->setItemEnabled(mnuGrpExpandAll, GroupMode);
}

void MainWindow::showGroupPopup(QPoint p)
{
    menuGroups->popup(p);
}

MainWindow::~MainWindow()
{
    transparent = NULL;
    while (containers.size()){
        delete containers.front();
    }
    for (list<UserFloat*>::iterator it = floating.begin(); it != floating.end(); ++it)
        delete *it;
#ifndef WIN32
    if (lockFile != -1) ::close(lockFile);
#endif
    if (dock) delete dock;
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

void MainWindow::addStatusItem(int status)
{
    menuStatus->insertItem(Icon(SIMClient::getStatusIcon(status)), SIMClient::getStatusText(status), status & 0xFF);
    setStatusItem(status);
}

void MainWindow::setStatusItem(int status)
{
    menuStatus->setItemChecked(status & 0xFF, (unsigned)(pClient->owner->uStatus & 0xFF) == (unsigned)(status & 0xFF));
}

#ifdef WIN32

bool makedir(char *p)
{
    char *r = strrchr(p, '\\');
    if (r == NULL) return true;
    *r = 0;
    CreateDirectoryA(p, NULL);
    *r = '\\';
    return true;
}

#else

bool makedir(char *p)
{
    bool res = true;
    char *r = strrchr(p, '/');
    if (r == NULL) return res;
    *r = 0;
    struct stat st;
    if (stat(p, &st)){
        if (makedir(p)){
            if (mkdir(p, 0755)){
                log(L_ERROR, "Can't create %s: %s", p, strerror(errno));
                res = false;
            }
        }else{
            res = false;
        }
    }else{
        if ((st.st_mode & S_IFMT) != S_IFDIR){
            log(L_ERROR, "%s no directory", p);
            res = false;
        }
    }
    *r = '/';
    return res;
}

#endif

void MainWindow::toggleGroupMode()
{
    setGroupMode(toolbar->isOn(btnGroupMode));
}

void MainWindow::toggleShowOffline()
{
    setShowOffline(toolbar->isOn(btnShowOffline));
}

void MainWindow::setGroupMode(bool bState)
{
    toolbar->setOn(btnGroupMode, bState);
    users->setGroupMode(bState);
}

void MainWindow::setShowOffline(bool bState)
{
    toolbar->setOn(btnShowOffline, bState);
    users->setShowOffline(bState);
}

#ifdef WIN32
static BOOL (WINAPI *_SHGetSpecialFolderPath)(HWND hwndOwner, LPSTR lpszPath, int nFolder, BOOL fCreate) = NULL;
#endif

void MainWindow::buildFileName(string &s, const char *name, bool bUseKDE, bool bCreate)
{
    s = homeDir;
    if (s.length() == 0){
#ifndef _WINDOWS
        struct passwd *pwd = getpwuid(getuid());
        if (pwd){
            s = pwd->pw_dir;
        }else{
            log(L_ERROR, "Can't get pwd");
        }
        if (s[s.size() - 1] != '/') s += '/';
#ifndef USE_KDE
        bUseKDE = false;
#endif
        if (bUseKDE){
            char *kdehome = getenv("KDEHOME");
            if (kdehome){
                s = kdehome;
            }else{
                s += ".kde/";
            }
            if (s.length() == 0) s += '/';
            if (s[s.length()-1] != '/') s += '/';
            s += "share/apps/sim/";
        }else{
            s += ".sim/";
        }
#else
        char szPath[MAX_PATH];
        HINSTANCE hLib = LoadLibraryA("Shell32.dll");
        if (hLib != NULL)
            (DWORD&)_SHGetSpecialFolderPath = (DWORD)GetProcAddress(hLib,"SHGetSpecialFolderPathA");
        if (_SHGetSpecialFolderPath && _SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, true)){
            s = szPath;
            if (s.length()  == 0) s = "c:\\";
            if (s[s.length() - 1] != '\\') s += '\\';
            s += "sim\\";
        }else{
            s = app_file("");
        }
#endif
    }
#ifndef _WINDOWS
    if (s[s.length() - 1] != '/') s += '/';
#else
    if (s[s.length() - 1] != '\\') s += '\\';
#endif
    homeDir = s;
    s += name;
    if (bCreate) makedir((char*)s.c_str());
}

extern const ToolBarDef *pUserBoxToolBar;
extern const ToolBarDef *pMsgEditToolBar;
extern const ToolBarDef *pHistoryToolBar;

bool MainWindow::init()
{
#ifdef WIN32
    WM_APPBAR = RegisterWindowMessageA("AppBarNotify");
    WNDPROC p;
    p = (WNDPROC)SetWindowLongW(winId(), GWL_WNDPROC, (LONG)MainWndProc);
    if (p == 0)
        p = (WNDPROC)SetWindowLongA(winId(), GWL_WNDPROC, (LONG)MainWndProc);
    appBarMessage(ABM_NEW);
#endif

    setIcons();
    string file;
#ifndef WIN32
    buildFileName(file, "lock");
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
#endif
    string part;
    buildFileName(file, SIM_CONF);
    QFile fs(QString::fromLocal8Bit(file.c_str()));
    if (!fs.open(IO_ReadOnly)){
        setDock();
        return true;
    }
    ::load(this, MainWindow_Params, fs, part);

    if (ToolBarMain.size()) emit toolBarChanged(mainWndToolBar);
    if (ToolBarMsg.size()) emit toolBarChanged(pMsgEditToolBar);
    if (ToolBarHistory.size()) emit toolBarChanged(pHistoryToolBar);
    if (ToolBarUserBox.size()) emit toolBarChanged(pUserBoxToolBar);

    if (mLeft < 5) mLeft = 5;
    if (mTop < 5) mTop = 5;
    if (mLeft > QApplication::desktop()->width() - 5) mLeft = QApplication::desktop()->width() - 5;
    if (mTop > QApplication::desktop()->height() - 5) mTop = QApplication::desktop()->height() - 5;
    if (mWidth > QApplication::desktop()->width() - 5) mWidth = QApplication::desktop()->width() - 5;
    if (mHeight > QApplication::desktop()->height() - 5) mHeight = QApplication::desktop()->height() - 5;
    move(mLeft, mTop);
    if (mWidth && mHeight) resize(mWidth, mHeight);
    if (*Icons.c_str()) changeIcons(0);
    themes->setTheme(QString::fromLocal8Bit(UseStyle.c_str()));
    setFonts();
    {
        string file, part;
        buildFileName(file, ICQ_CONF);
        QFile fs(QString::fromLocal8Bit(file.c_str()));
        if (fs.open(IO_ReadOnly))
            pClient->load(fs, part);
    }
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
    menuStatus->setItemChecked(ICQ_STATUS_FxPRIVATE, pClient->owner->inInvisible);
    changeWm();

    bool bNeedSetup = false;
    if ((pClient->owner->Uin == 0) || (*pClient->EncryptedPassword.c_str() == 0)){
        pSplash->hide();
        bInLogin = true;
        LoginDialog dlg;
        bNeedSetup = (dlg.exec() != 0);
        bInLogin = false;
        ManualStatus = ICQ_STATUS_ONLINE;
        Show = true;
        ShowOffline = true;
        GroupMode = true;
    }
    if (pClient->owner->Uin == 0)
        return false;

    ToolBarDock tDock = Top;
    if (ToolbarDock == "Minimized"){
        tDock = Minimized;
    }else if (ToolbarDock == "Bottom"){
        tDock = Bottom;
    }else if (ToolbarDock == "Left"){
        tDock = Left;
    }else if (ToolbarDock == "Right"){
        tDock = Right;
    }else if (ToolbarDock == "TornOff"){
        tDock = TornOff;
    }
    moveToolBar(toolbar, tDock, false, 0, ToolbarOffset);
    if (tDock == TornOff){
        toolbar->move(ToolbarOffset, ToolbarY);
        toolbar->show();
    }

    loadUnread();
    xosd->init();
    transparentChanged();
    setShow(Show);
    setDock();
    setOnTop();
    setUserBoxOnTop();
    ownerChanged();

    setShowOffline(ShowOffline);
    setGroupMode(GroupMode);

    keys->unregKeys();
    keys->regKeys();

    changeBackground();

    realSetStatus();
    if (bNeedSetup) setup();
    return true;
}

void MainWindow::setKeys(const char *kWindow, const char *kDblClick, const char *kSearch)
{
    if (!strcmp(kWindow, KeyWindow.c_str()) &&
            !strcmp(kDblClick, KeyDblClick.c_str()) &&
            !strcmp(kSearch, KeySearch.c_str())) return;
    KeyWindow = kWindow;
    KeyDblClick = kDblClick;
    KeySearch = kSearch;
    keys->unregKeys();
    keys->regKeys();
}

void MainWindow::messageReceived(ICQMessage *msg)
{
    if (msg->Type() == ICQ_MSGxSTATUS) return;

    ICQUser *u = pClient->getUser(msg->getUin());
    if (u == NULL) u = pClient->owner;
    SIMUser *_u = static_cast<SIMUser*>(u);
    if (!_u->SoundOverride)
        _u = static_cast<SIMUser*>(pClient->owner);

    const char *wav;
    switch (msg->Type()){
    case ICQ_MSGxURL:
        wav = _u->IncomingURL.c_str();
        break;
    case ICQ_MSGxSMS:
        wav = _u->IncomingSMS.c_str();
        break;
    case ICQ_MSGxFILE:
        wav = _u->IncomingFile.c_str();
        break;
    case ICQ_MSGxCHAT:
        wav = _u->IncomingChat.c_str();
        break;
    case ICQ_MSGxAUTHxREQUEST:
    case ICQ_MSGxAUTHxREFUSED:
    case ICQ_MSGxAUTHxGRANTED:
    case ICQ_MSGxADDEDxTOxLIST:
        wav = _u->IncomingAuth.c_str();
        break;
    default:
        wav = _u->IncomingMessage.c_str();
    }
    if (((pClient->owner->uStatus & 0xFF) != ICQ_STATUS_AWAY) && ((pClient->owner->uStatus & 0xFF) != ICQ_STATUS_NA))
        playSound(wav);

    unread_msg m(msg);
    u = pClient->getUser(msg->getUin());

    if (u == NULL) return;

    list<unsigned long>::iterator it;
    for (it = u->unreadMsgs.begin(); it != u->unreadMsgs.end(); it++)
        if ((*it) == msg->Id) break;
    if (it == u->unreadMsgs.end()) return;
    messages.push_back(m);
    if (dock) dock->reset();

    CUser user(u);
    xosd->setMessage(i18n("%1 from %2 received")
                     .arg(pClient->getMessageText(msg->Type(), 1))
                     .arg(user.name()), u->Uin);

    _u = static_cast<SIMUser*>(u);
    if (!_u->AcceptFileOverride)
        _u = static_cast<SIMUser*>(pClient->owner);
    if (_u->AcceptMsgWindow)
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
            ManualStatus = ICQ_STATUS_OFFLINE;
            ProxyDialog d(this, i18n("Can't connect to proxy server"));
            d.exec();
            return;
        }
    case EVENT_PROXY_BAD_AUTH:{
            ManualStatus = ICQ_STATUS_OFFLINE;
            ProxyDialog d(this, pClient->factory()->ProxyAuth ?
                          i18n("Proxy server require authorization") :
                          i18n("Invalid password for proxy"));
            d.exec();
            return;
        }
    case EVENT_BAD_PASSWORD:
        if (!bInLogin){
            pClient->setStatus(ICQ_STATUS_OFFLINE);
            ManualStatus = ICQ_STATUS_OFFLINE;
            QTimer::singleShot(50, this, SLOT(badPassword()));
        }
        return;
    case EVENT_ANOTHER_LOCATION:
        ManualStatus = ICQ_STATUS_OFFLINE;
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
            ICQUser *_u = pClient->getUser(e->Uin());
            if (_u == NULL) return;
            SIMUser *u = static_cast<SIMUser*>(_u);
            SIMUser *o = static_cast<SIMUser*>(pClient->owner);
            if (!u->inIgnore && ((NoAlertAway == 0) ||
                                 ((u->uStatus & 0xFF) == ICQ_STATUS_ONLINE) ||
                                 ((u->uStatus & 0xFF) == ICQ_STATUS_FREEFORCHAT)) &&
                    ((u->prevStatus & 0xFF) != ICQ_STATUS_ONLINE) &&
                    ((u->prevStatus & 0xFF) != ICQ_STATUS_FREEFORCHAT) &&
                    (((pClient->owner->uStatus & 0xFF) == ICQ_STATUS_ONLINE) ||
                     ((pClient->owner->uStatus & 0xFF) == ICQ_STATUS_FREEFORCHAT)) &&
                    ((u->prevStatus == ICQ_STATUS_OFFLINE) || o->AlertAway) &&
                    ((u->OnlineTime > pClient->owner->OnlineTime) || (u->prevStatus  != ICQ_STATUS_OFFLINE))){
                SIMUser *__u = u;
                if (!u->AlertOverride) u = o;
                if (u->AlertSound){
                    if (!__u->SoundOverride) __u= o;
                    playSound(__u->OnlineAlert.c_str());
                }
                if (u->AlertOnScreen){
                    CUser user(e->Uin());
                    xosd->setMessage(i18n("%1 is online") .arg(user.name()), e->Uin());
                }
                if (u->AlertPopup){
                    AlertMsgDlg *dlg = new AlertMsgDlg(this, e->Uin());
                    dlg->show();
                    dlg->raise();
                }
                if (u->AlertWindow)
                    userFunction(e->Uin(), mnuAction);
            }
        }
        return;
    }
}

void MainWindow::badPassword()
{
    unsigned long saveStatus = ManualStatus;
    ManualStatus = ICQ_STATUS_OFFLINE;
    PasswdDialog dlg;
    bInLogin = true;
    if (!dlg.exec()){
        quit();
        return;
    }
    bInLogin = false;
    ManualStatus = saveStatus;
}

void MainWindow::saveState()
{
    if (m_bAutoAway || m_bAutoNA) ManualStatus = (unsigned long)m_autoStatus;
    ShowOffline = toolbar->isOn(btnShowOffline);
    GroupMode = toolbar->isOn(btnGroupMode);
    Show = isShow();
#ifdef WIN32
    if (BarState == ABE_FLOAT){
#endif
        mLeft = pos().x();
        mTop = pos().y();
        mWidth = size().width();
        mHeight = size().height();
#ifdef WIN32
    }
#endif
    ToolBarDock tDock;
    int index;
    bool nl;
    int extraOffset;
    getLocation(toolbar, tDock, index, nl, extraOffset);
    ToolbarOffset = (short)extraOffset;
    switch (tDock){
    case Minimized:
        ToolbarDock = "Minimized";
        break;
    case Bottom:
        ToolbarDock = "Bottom";
        break;
    case Left:
        ToolbarDock = "Left";
        break;
    case Right:
        ToolbarDock = "Right";
        break;
    case TornOff:
        ToolbarDock = "TornOff";
        ToolbarOffset = toolbar->geometry().left();
        ToolbarY = toolbar->geometry().top();
        break;
    default:
        ToolbarDock = "Top";
    }
    CToolBar::save(mainWndToolBar, &ToolBarMain);
    CToolBar::save(pMsgEditToolBar, &ToolBarMsg);
    CToolBar::save(pHistoryToolBar, &ToolBarHistory);
    CToolBar::save(pUserBoxToolBar, &ToolBarUserBox);

    OnTop = testWFlags(WStyle_StaysOnTop);
#ifdef USE_KDE
    OnTop = KWin::info(winId()).state & NET::StaysOnTop;
#endif
    UseStyle = "";
    if (themes->getTheme().length()) UseStyle = themes->getTheme().local8Bit();
    string file;
    buildFileName(file, SIM_CONF);
#ifndef WIN32
    struct stat st;
    if ((stat(file.c_str(), &st) >= 0) && (st.st_mode != 0600))
        unlink(file.c_str());
#endif
    QFile fs(file.c_str());
    if (fs.open(IO_WriteOnly | IO_Truncate)){
        ::save(this, MainWindow_Params, fs);
        for (list<UserFloat*>::iterator itFloat = floating.begin(); itFloat != floating.end(); itFloat++){
            writeStr(fs, "[Floaty]\n");
            (*itFloat)->save(fs);
        }
        for (list<UserBox*>::iterator itBox = containers.begin(); itBox != containers.end(); itBox++){
            writeStr(fs, "[UserBox]\n");
            (*itBox)->save(fs);
        }
    }
    saveContacts();
    pSplash->save();
}

void MainWindow::saveContacts()
{
    if (pClient->owner->Uin == 0) return;
    string file;
    buildFileName(file, ICQ_CONF);
#ifndef WIN32
    struct stat st;
    if ((stat(file.c_str(), &st) >= 0) && (st.st_mode != 0600))
        unlink(file.c_str());
#endif
    QFile fs(QString::fromLocal8Bit(file.c_str()));
    if (fs.open(IO_WriteOnly | IO_Truncate))
        pClient->save(fs);
}

bool MainWindow::isDock()
{
    return (dock != NULL);
}

void MainWindow::setDock()
{
    if (UseDock){
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
        if ((*itBox)->bHistory || (*itBox)->bUserInfo) continue;
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
    if (SimpleMode){
        for (it = containers.begin(); it != containers.end(); ++it){
            if ((*it)->bHistory || (*it)->bUserInfo) continue;
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
    switch (ContainerMode){
    case ContainerModeAll:
        for (it = containers.begin(); it != containers.end(); ++it){
            if ((*it)->bHistory || (*it)->bUserInfo) continue;
            if ((*it)->GrpId == ContainerAllUsers) break;
        }
        if (it == containers.end()){
            for (it = containers.begin(); it != containers.end(); ++it)
                if (!(*it)->bHistory && !(*it)->bUserInfo) break;
        }
        if (it != containers.end()){
            (*it)->GrpId = ContainerAllUsers;
            (*it)->showUser(uin, function, param);
            return;
        }
        box = new UserBox(ContainerAllUsers);
        break;
    case ContainerModeGroup:
        for (it = containers.begin(); it != containers.end(); ++it){
            if ((*it)->bHistory || (*it)->bUserInfo) continue;
            if ((*it)->GrpId != (unsigned long)grpId) continue;
            (*it)->showUser(uin, function, param);
            return;
        }
        box = new UserBox(grpId);
        break;
    default:
        grpId = 0x7FFFFFFF;
        for (it = containers.begin(); it != containers.end(); ++it){
            if ((*it)->bHistory || (*it)->bUserInfo) continue;
            if ((*it)->GrpId < 0x10000L) continue;
            if ((*it)->GrpId < grpId) grpId = (*it)->GrpId - 1;
        }
        box = new UserBox(uin);
    }
    containers.push_back(box);
    box->showUser(uin, function, param);
}

void MainWindow::setShow(bool bShow)
{
    Show = bShow;
#ifdef WIN32
    if (BarState != ABE_FLOAT){
        if (BarAutoHide) bShow = true;
        if (bShow){
            setBarState();
        }else{
            appBarMessage(ABM_SETPOS, ABE_FLOAT, FALSE);
        }
    }
#endif
    if (!bShow){
        hide();
        return;
    }
    show();
    showNormal();
#ifdef USE_KDE
    KWin::setOnDesktop(winId(), KWin::currentDesktop());
#endif
    setActiveWindow();
    raise();
#ifdef USE_KDE
    KWin::setActiveWindow(winId());
#endif
#ifdef WIN32
    AttachThreadInput(GetWindowThreadProcessId(GetForegroundWindow(),NULL), GetCurrentThreadId(), TRUE);
    SetForegroundWindow(winId());
    SetFocus(winId());
    AttachThreadInput(GetWindowThreadProcessId(GetForegroundWindow(),NULL), GetCurrentThreadId(), FALSE);
#endif
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
    if ((BarState != ABE_FLOAT) && BarAutoHide){
        bAutoHideVisible = !bAutoHideVisible;
        setBarState();
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
        connect(setupDlg, SIGNAL(closed()), this, SLOT(setupClosed()));
    }
    emit setupInit();
    setupDlg->show();
#ifdef USE_KDE
    KWin::setOnDesktop(setupDlg->winId(), KWin::currentDesktop());
#endif
    setupDlg->raise();
#ifdef USE_KDE
    KWin::setActiveWindow(setupDlg->winId());
#endif
}

void MainWindow::setupClosed()
{
    setupDlg = NULL;
}

void MainWindow::phonebook()
{
    if (setupDlg == NULL){
        setupDlg = new SetupDialog(this, SETUP_PHONE);
    }else{
        setupDlg->showPage(SETUP_PHONE);
    }
    setupDlg->show();
#ifdef USE_KDE
    KWin::setOnDesktop(setupDlg->winId(), KWin::currentDesktop());
#endif
    setupDlg->raise();
#ifdef USE_KDE
    KWin::setActiveWindow(setupDlg->winId());
#endif

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
    searchDlg->show();
#ifdef USE_KDE
    KWin::setOnDesktop(searchDlg->winId(), KWin::currentDesktop());
#endif
    searchDlg->setActiveWindow();
    searchDlg->raise();
#ifdef USE_KDE
    KWin::setActiveWindow(searchDlg->winId());
#endif
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
    if (isDock() && !isActiveWindow() && AutoHideTime){
        time_t now;
        time(&now);
        if (now >= (time_t)(inactiveTime + AutoHideTime))
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
    if ((m_bAutoAway && (idle_time < AutoAwayTime)) ||
            (m_bAutoNA && (idle_time < AutoNATime))){
        m_bAutoAway = m_bAutoNA = false;
        if (((ManualStatus & 0xFF) == ICQ_STATUS_NA) ||
                ((ManualStatus & 0xFF) == ICQ_STATUS_AWAY)){
            ManualStatus = (unsigned long)m_autoStatus;
            realSetStatus();
        }
        return;
    }
    if (!m_bAutoNA && AutoNATime && (idle_time > AutoNATime)){
        m_bAutoNA = true;
        if ((ManualStatus == ICQ_STATUS_OFFLINE) ||
                ((ManualStatus & 0xFF) == ICQ_STATUS_NA)) return;
        if (!m_bAutoAway){
            m_autoStatus = ManualStatus;
            m_bAutoNA = true;
        }
        ManualStatus = ICQ_STATUS_NA;
        realSetStatus();
        return;
    }
    if (!m_bAutoAway && AutoAwayTime && (idle_time > AutoAwayTime)){
        m_bAutoAway = true;
        if ((ManualStatus == ICQ_STATUS_OFFLINE) ||
                ((ManualStatus & 0xFF) == ICQ_STATUS_NA) ||
                ((ManualStatus & 0xFF) == ICQ_STATUS_AWAY)) return;
        m_autoStatus = ManualStatus;
        ManualStatus = ICQ_STATUS_AWAY;
        realSetStatus();
        return;
    }
#endif
}

void MainWindow::setStatus(int status)
{
    if ((unsigned long)status == ICQ_STATUS_FxPRIVATE){
        pClient->setInvisible(!pClient->owner->inInvisible);
        menuStatus->setItemChecked(ICQ_STATUS_FxPRIVATE, pClient->owner->inInvisible);
        return;
    }
    AutoReplyDlg *autoDlg = NULL;
    switch (status & 0xFF){
    case ICQ_STATUS_AWAY:
        if (NoShowAway) break;
        autoDlg = new AutoReplyDlg(this, ICQ_STATUS_AWAY);
        break;
    case ICQ_STATUS_NA:
        if (NoShowNA) break;
        autoDlg = new AutoReplyDlg(this, ICQ_STATUS_NA);
        break;
    case ICQ_STATUS_OCCUPIED:
        if (NoShowOccupied) break;
        autoDlg = new AutoReplyDlg(this, ICQ_STATUS_OCCUPIED);
        break;
    case ICQ_STATUS_DND:
        if (NoShowDND) break;
        autoDlg = new AutoReplyDlg(this, ICQ_STATUS_DND);
        break;
    case ICQ_STATUS_FREEFORCHAT:
        if (NoShowFFC) break;
        autoDlg = new AutoReplyDlg(this, ICQ_STATUS_FREEFORCHAT);
        break;
    }
    if (autoDlg){
        autoDlg->show();
        autoDlg->raise();
    }
    if (status == (ICQ_STATUS_OFFLINE & 0xFF)) status = ICQ_STATUS_OFFLINE;
    ManualStatus = (unsigned long)status;
    QTimer::singleShot(800, this, SLOT(realSetStatus()));
}

void MainWindow::realSetStatus()
{
    pClient->setStatus(ManualStatus);
}

void MainWindow::moveUser(int grp)
{
    ICQUser *u = pClient->getUser(m_uin);
    switch (grp){
    case mnuGroupVisible:
        if (u == NULL) break;
        pClient->setInVisible(u, !u->inVisible);
        break;
    case mnuGroupInvisible:
        if (u == NULL) break;
        pClient->setInInvisible(u, !u->inInvisible);
        break;
    case mnuGroupIgnore:
        if (u == NULL) break;
        if (!u->inIgnore){
            CUser user(u);
            QStringList btns;
            btns.append(i18n("&Yes"));
            btns.append(i18n("&No"));
            BalloonMsg *msg = new BalloonMsg(i18n("Add %1 to ignore list ?") .arg(user.name()),
                                             m_rc, btns, this);
            connect(msg, SIGNAL(action(int)), this, SLOT(ignoreUser(int)));
            msg->show();
            break;
        }
        pClient->setInIgnore(u, !u->inIgnore);
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
        toolbar->setState(btnStatus, SIMClient::getStatusIcon(bBlinkState ? ICQ_STATUS_OFFLINE : ICQ_STATUS_ONLINE), i18n("Connecting"));
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
            menuGroup->setItemChecked(mnuGroupVisible, u->inVisible);
            menuGroup->insertItem(i18n("In invisible list"), mnuGroupInvisible);
            menuGroup->setItemChecked(mnuGroupInvisible, u->inInvisible);
        }
        menuGroup->insertItem(i18n("In ignore list"), mnuGroupIgnore);
        menuGroup->setItemChecked(mnuGroupIgnore, u->inIgnore);
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
        menuUser->setItemChecked(mnuSort, AlphabetSort);
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
                if (!uFloat->setUin(m_uin)){
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
                chat->show();
                chat->showNormal();
#ifdef USE_KDE
                KWin::setOnDesktop(chat->winId(), KWin::currentDesktop());
#endif
                chat->setActiveWindow();
                chat->raise();
#ifdef USE_KDE
                KWin::setActiveWindow(chat->winId());
#endif
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
                                                 m_rc, btns, this);
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
            w->show();
#ifdef USE_KDE
            KWin::setOnDesktop(w->winId(), KWin::currentDesktop());
#endif
            w->setActiveWindow();
            w->raise();
#ifdef USE_KDE
            KWin::setActiveWindow(w->winId());
#endif
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
            w->show();
#ifdef USE_KDE
            KWin::setOnDesktop(w->winId(), KWin::currentDesktop());
#endif
            w->setActiveWindow();
            w->raise();
#ifdef USE_KDE
            KWin::setActiveWindow(w->winId());
#endif
            return;
        }
    case mnuSecureOff:{
            ICQUser *u = pClient->getUser(uin);
            if (u) pClient->closeSecureChannel(u);
            return;
        }
    case mnuSort:
        AlphabetSort = !AlphabetSort;
        users->refresh();
        return;
    case mnuAlert:
    case mnuAccept:
    case mnuSound:
    case mnuInfo:
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
        for (it = containers.begin(); it != containers.end(); ++it){
            if (!(*it)->bUserInfo) continue;
            if (!(*it)->haveUser(uin)) continue;
            (*it)->showUser(uin, mnuInfo, param);
            return;
        }
        box = new UserBox(uin);
        containers.push_back(box);
        box->bUserInfo = true;
        box->hideToolbar();
        box->showUser(uin, mnuInfo, param);
        return;
    case mnuHistoryNew:
        for (it = containers.begin(); it != containers.end(); ++it){
            if (!(*it)->bHistory) continue;
            if (!(*it)->haveUser(uin)) continue;
            (*it)->showUser(uin, mnuHistory, 0);
            return;
        }
        box = new UserBox(uin);
        containers.push_back(box);
        box->bHistory = true;
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
        if (!(*it)->bHistory) continue;
        if ((*it)->haveUser(uin)) return true;
    }
    return false;
}

bool MainWindow::isUserInfo(unsigned long uin)
{
    list<UserBox*>::iterator it;
    for (it = containers.begin(); it != containers.end(); ++it){
        if (!(*it)->bUserInfo) continue;
        if ((*it)->haveUser(uin)) return true;
    }
    return false;
}

UserFloat *MainWindow::findFloating(unsigned long uin, bool bDelete)
{
    list<UserFloat*>::iterator it;
    for (it = floating.begin(); it != floating.end(); it++){
        if ((*it)->Uin == uin){
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
        if ((*it)->bHistory || (*it)->bUserInfo) continue;
        if (!(*it)->haveUser(uinMenu)) continue;
        if ((*it)->GrpId == (unsigned long)containerId) return;
        (*it)->closeUser(uinMenu);
    }
    for (it = containers.begin(); it != containers.end(); ++it)
        if ((*it)->GrpId == (unsigned long)containerId) break;
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
            if ((*it)->bHistory || (*it)->bUserInfo) continue;
            menuContainers->insertItem((*it)->containerName(), (*it)->GrpId);
            if (!(*it)->haveUser(uinMenu)) continue;
            menuContainers->setItemChecked((*it)->GrpId, true);
            if (((*it)->GrpId >= 0x10000L) && ((*it)->count() == 1))
                canNew = false;
        }
        if (canNew){
            unsigned long newId = 0x7FFFFFFF;
            for (it = containers.begin(); it != containers.end(); ++it){
                if ((*it)->bHistory || (*it)->bUserInfo) continue;
                if ((*it)->GrpId < 0x10000L) continue;
                if ((*it)->GrpId <= newId) newId = (*it)->GrpId - 1;
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
    exec(UrlViewer.c_str(), url);
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
    exec(MailClient.c_str(), mail);
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

string MainWindow::homeDir;

void MainWindow::playSound(const char *wav)
{
    if ((wav == NULL) || (*wav == 0) || pSplash->SoundDisable) return;
    wav = sound(wav);
#ifdef WIN32
    sndPlaySoundA(wav, SND_ASYNC | SND_NODEFAULT);
#else
#ifdef USE_KDE
    if (pSplash->UseArts){
        KAudioPlayer::play(wav);
        return;
    }
#endif
    if (*(pSplash->SoundPlayer.c_str()) == 0) return;

    const char *arglist[3];
    arglist[0] = pSplash->SoundPlayer.c_str();
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
    showUser(0, mnuInfo);
}

void MainWindow::sendSMS()
{
    showUser(0, mnuSMS);
}

void MainWindow::setFonts()
{
#ifdef USE_KDE
    if (UseSystemFonts) return;
#endif
    QPopupMenu p;
    qApp->setFont(str2font(Font.c_str(), font()), true);
    qApp->setFont(str2font(FontMenu.c_str(), p.font()), true, "QPopupMenu");
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
#ifdef USE_KDE
    KWin::setOnDesktop(mNetMonitor->winId(), KWin::currentDesktop());
#endif
    mNetMonitor->show();
    mNetMonitor->setActiveWindow();
    mNetMonitor->raise();
#ifdef USE_KDE
    KWin::setActiveWindow(mNetMonitor->winId());
#endif
}

void MainWindow::monitorFinished()
{
    mNetMonitor = NULL;
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
    initIcons(Icons.c_str());
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
    menuFunction->insertItem(Icon("exit"), i18n("Quit"), this, SLOT(quit()));
}

void MainWindow::changeWm()
{
#ifdef USE_KDE
    if (MainWindowInTaskManager){
        KWin::clearState(winId(), NET::SkipTaskbar);
    }else{
        KWin::setState(winId(), NET::SkipTaskbar);
    }
#endif
#ifdef WIN32
    bool bVisible = isVisible();
    if (bVisible) hide();
    if (MainWindowInTaskManager){
        SetWindowLongW(winId(), GWL_EXSTYLE, (GetWindowLongW(winId(), GWL_EXSTYLE) | WS_EX_APPWINDOW) & (~WS_EX_TOOLWINDOW));
    }else{
        SetWindowLongW(winId(), GWL_EXSTYLE, (GetWindowLongW(winId(), GWL_EXSTYLE) & ~(WS_EX_APPWINDOW)) | WS_EX_TOOLWINDOW);
    }
    if (bVisible) show();
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
    if (dock) dock->reset();
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

void MainWindow::initTranslator()
{
    if (translator)
        qApp->removeTranslator(translator);
    translator = NULL;
    string lang = pSplash->Language;
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
    string s = "po";
#ifdef WIN32
    s += "\\";
#else
    s += "/";
#endif
    for (const char *p = lang.c_str(); *p; p++)
        s += tolower(*p);
    s += ".qm";
    QFile f(QString::fromLocal8Bit(app_file(s.c_str())));
    if (!f.exists()) return;
    translator = new QTranslator(this);
    translator->load(f.name());
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
    if ((BarState != ABE_FLOAT) && BarAutoHide && bAutoHideVisible && !noToggle){
        bAutoHideVisible = false;
        setBarState(true);
    }
#endif
}

void MainWindow::changeBackground()
{
    emit bgChanged();
}

#ifndef _WINDOWS
#include "mainwin.moc"
#endif

