/***************************************************************************
                          transparent.cpp  -  description
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

#include "transparent.h"
#include "transparentcfg.h"
#include "simapi.h"

#ifndef WIN32
#include "transtop.h"
#endif

#include <qapplication.h>
#include <qwidgetlist.h>
#include <qtimer.h>
#include <qpainter.h>

#ifdef WIN32
#include <windows.h>

#define SHOW_TIMEOUT	300
#define HIDE_TIMEOUT	1000

static BOOL (WINAPI *SetLayeredWindowAttributes)(
    HWND hwnd,
    COLORREF crKey,
    BYTE bAlpha,
    DWORD dwFlags) = NULL;

#define WS_EX_LAYERED           0x00080000
#define LWA_COLORKEY            0x00000001
#define LWA_ALPHA               0x00000002

#endif

Plugin *createTransparentPlugin(unsigned base, bool, const char *config)
{
#ifdef WIN32
    HINSTANCE hLib = LoadLibraryA("user32.dll");
    if (hLib != NULL)
        (DWORD&)SetLayeredWindowAttributes = (DWORD)GetProcAddress(hLib,"SetLayeredWindowAttributes");
    if (SetLayeredWindowAttributes == NULL)
        return NULL;
#endif
    Plugin *plugin = new TransparentPlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Transparent"),
#ifdef WIN32
        I18N_NOOP("Plugin provides windows transparency\n"
                  "This plugin works only on Windows 2000 or Windows XP")
#else
        I18N_NOOP("Plugin provides windows transparency")
#endif
        ,
        VERSION,
        createTransparentPlugin,
#ifdef WIN32
        PLUGIN_DEFAULT
#else
        PLUGIN_NOLOAD_DEFAULT
#endif
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

/*
typedef struct TransparentData
{
    unsigned long	Transparency;
#ifdef WIN32
    bool		IfInactive;
#endif
} TransparentData;
*/
static DataDef transparentData[] =
    {
        { "Transparency", DATA_ULONG, 1, 60 },
#ifdef WIN32
        { "IfInactive", DATA_BOOL, 1, 1 },
#endif
        { NULL, 0, 0, 0 }
    };

TransparentPlugin::TransparentPlugin(unsigned base, const char *config)
        : Plugin(base)
#ifndef WIN32
        , EventReceiver(HighPriority)
#endif
{
    load_data(transparentData, &data, config);
    if (getTransparency() >100) setTransparency(100);
#ifdef WIN32
    timer = NULL;
#else
    top = NULL;
#endif
    setState();
}

TransparentPlugin::~TransparentPlugin()
{
#ifdef WIN32
    QWidget *main = getMainWindow();
    if (main)
        SetWindowLongW(main->winId(), GWL_EXSTYLE, GetWindowLongW(main->winId(), GWL_EXSTYLE) & (~WS_EX_LAYERED));
    if (timer)
        delete timer;
#else
    if (top)
        delete top;
#endif
    free_data(transparentData, &data);
}

string TransparentPlugin::getConfig()
{
    return save_data(transparentData, &data);
}

QWidget *TransparentPlugin::getMainWindow()
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

QWidget *TransparentPlugin::createConfigWindow(QWidget *parent)
{
    return new TransparentCfg(parent, this);
}

bool TransparentPlugin::eventFilter(QObject *o, QEvent *e)
{
#ifdef WIN32
    if (getIfInactive()){
        switch (e->type()){
        case QEvent::WindowActivate:
        case QEvent::WindowDeactivate:
            setState();
            break;
        case QEvent::Show:{
                QWidget *main = getMainWindow();
                if (main){
                    setState();
                }
                break;
            }
        default:
            break;
        }
    }
#endif
    return QObject::eventFilter(o, e);
}

void TransparentPlugin::setState()
{
    QWidget *main = getMainWindow();
    if (main == NULL)
        return;
#ifdef WIN32
    if (timer == NULL){
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(tick()));
        main->installEventFilter(this);
        SetWindowLongW(main->winId(), GWL_EXSTYLE, GetWindowLongW(main->winId(), GWL_EXSTYLE) | WS_EX_LAYERED);
        SetLayeredWindowAttributes(main->winId(), main->colorGroup().background().rgb(), 0, LWA_ALPHA);
        RedrawWindow(main->winId(), NULL, NULL, RDW_UPDATENOW);
        main->setMouseTracking(true);
        m_bState = !main->isActiveWindow();
    }
    bool bNewState = main->isActiveWindow();
    if (bNewState == m_bState){
        BYTE d = bNewState ? 255 : QMIN((100 - getTransparency()) * 256 / 100, 255);
        SetLayeredWindowAttributes(main->winId(), main->colorGroup().background().rgb(), d, LWA_ALPHA);
        return;
    }
    m_bState = bNewState;
    startTime = GetTickCount();
    timer->start(10);
#else
    if (top == NULL)
        top = new TransparentTop(main, getTransparency());
    top->setTransparent(getTransparency());
#endif
}

void TransparentPlugin::tick()
{
#ifdef WIN32
    QWidget *main = getMainWindow();
    if (main == NULL){
        timer->stop();
        return;
    }
    unsigned timeout = m_bState ? SHOW_TIMEOUT : HIDE_TIMEOUT;
    unsigned time = GetTickCount() - startTime;
    if (time >= timeout){
        time = timeout;
        timer->stop();
    }
    if (m_bState)
        time = timeout - time;
    BYTE d = QMIN((100 - getTransparency() * time / timeout) * 256 / 100, 255);
    SetLayeredWindowAttributes(main->winId(), main->colorGroup().background().rgb(), d, LWA_ALPHA);
#endif
}

void *TransparentPlugin::processEvent(Event *e)
{
    if (e->type() == EventInit) {
#ifndef WIN32
        top = NULL;
#endif
        setState();
    }
#ifndef WIN32
    if (e->type() == EventPaintView){
        PaintView *pv = (PaintView*)(e->param());
        QPixmap pict = top->background(pv->win->colorGroup().background());
        if (!pict.isNull()){
            QPoint p = pv->pos;
            p = pv->win->mapToGlobal(p);
            p = pv->win->topLevelWidget()->mapFromGlobal(p);
            pv->p->drawPixmap(0, 0, pict, p.x(), p.y());
            pv->isStatic = true;
        }
    }
    if (e->type() == EventRaiseWindow){
        QWidget *w = (QWidget*)(e->param());
        if (w == getMainWindow())
            setState();
    }
#endif
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

#ifndef WIN32
#include "transparent.moc"
#endif


