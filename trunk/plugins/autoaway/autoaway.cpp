/***************************************************************************
                          autoaway.cpp  -  description
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
 ***************************************************************************

Detect idle time for MAC:
Copyright (C) 2003  Tarkvara Design Inc.

*/

#include "autoaway.h"
#include "autoawaycfg.h"
#include "simapi.h"
#include "core.h"

#include <qtimer.h>
#include <qapplication.h>
#include <qwidgetlist.h>
#include <time.h>

#ifdef WIN32
#include <windows.h>

typedef struct tagLASTINPUTINFO {
    UINT cbSize;
    DWORD dwTime;
} LASTINPUTINFO, * PLASTINPUTINFO;

static BOOL (WINAPI * _GetLastInputInfo)(PLASTINPUTINFO);
static DWORD (__stdcall *_IdleUIGetLastInputTime)(void);

static HMODULE hLibUI = NULL;

#else
#ifdef HAVE_CARBON_CARBON_H
#include <Carbon/Carbon.h>
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/scrnsaver.h>
#endif
#endif

const unsigned AUTOAWAY_TIME	= 10000;

Plugin *createAutoAwayPlugin(unsigned base, bool, const char *config)
{
    Plugin *plugin = new AutoAwayPlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("AutoAway"),
        I18N_NOOP("Plugin provides set away and N/A status after some idle time"),
        VERSION,
        createAutoAwayPlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

#ifdef HAVE_CARBON_CARBON_H

static unsigned mSecondsIdle = 0;
static EventLoopTimerRef mTimerRef;

static OSStatus LoadFrameworkBundle(CFStringRef framework, CFBundleRef *bundlePtr) {
    OSStatus  err;
    FSRef   frameworksFolderRef;
    CFURLRef baseURL;
    CFURLRef bundleURL;

    if ( bundlePtr == nil ) return( -1 );

    *bundlePtr = nil;

    baseURL = nil;
    bundleURL = nil;

    err = FSFindFolder(kOnAppropriateDisk, kFrameworksFolderType, true, &frameworksFolderRef);
    if (err == noErr) {
        baseURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &frameworksFolderRef);
        if (baseURL == nil) {
            err = coreFoundationUnknownErr;
        }
    }
    if (err == noErr) {
        bundleURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorSystemDefault, baseURL, framework, false);
        if (bundleURL == nil) {
            err = coreFoundationUnknownErr;
        }
    }
    if (err == noErr) {
        *bundlePtr = CFBundleCreate(kCFAllocatorSystemDefault, bundleURL);
        if (*bundlePtr == nil) {
            err = coreFoundationUnknownErr;
        }
    }
    if (err == noErr) {
        if ( ! CFBundleLoadExecutable( *bundlePtr ) ) {
            err = coreFoundationUnknownErr;
        }
    }

    // Clean up.
    if (err != noErr && *bundlePtr != nil) {
        CFRelease(*bundlePtr);
        *bundlePtr = nil;
    }
    if (bundleURL != nil) {
        CFRelease(bundleURL);
    }
    if (baseURL != nil) {
        CFRelease(baseURL);
    }
    return err;
}

pascal void IdleTimerAction(EventLoopTimerRef, EventLoopIdleTimerMessage inState, void* inUserData)
{
    switch (inState) {
    case kEventLoopIdleTimerStarted:
    case kEventLoopIdleTimerStopped:
        // Get invoked with this constant at the start of the idle period,
        // or whenever user activity cancels the idle.
        mSecondsIdle = 0;
        break;
    case kEventLoopIdleTimerIdling:
        // Called every time the timer fires (i.e. every second).
        mSecondsIdle++;
        break;
    }
}

typedef OSStatus (*InstallEventLoopIdleTimerPtr)(EventLoopRef inEventLoop,
        EventTimerInterval   inFireDelay,
        EventTimerInterval   inInterval,
        EventLoopIdleTimerUPP    inTimerProc,
        void *               inTimerData,
        EventLoopTimerRef *  outTimer);

#endif

static DataDef autoAwayData[] =
    {
        { "AwayTime", DATA_ULONG, 1, DATA(3) },
        { "EnableAway", DATA_BOOL, 1, DATA(1) },
        { "NATime", DATA_ULONG, 1, DATA(10) },
        { "EnableNA", DATA_BOOL, 1, DATA(1) },
        { "OffTime", DATA_ULONG, 1, DATA(10) },
        { "EnableOff", DATA_BOOL, 1, 0 },
        { "DisableAlert", DATA_BOOL, 1, DATA(1) },
        { NULL, 0, 0, 0 }
    };

AutoAwayPlugin::AutoAwayPlugin(unsigned base, const char *config)
        : Plugin(base), EventReceiver(HighPriority)
{
    load_data(autoAwayData, &data, config);
#ifdef WIN32
    HINSTANCE hLib = GetModuleHandleA("user32");
    if (hLib != NULL){
        (DWORD&)_GetLastInputInfo = (DWORD)GetProcAddress(hLib,"GetLastInputInfo");
    }else{
        hLibUI = LoadLibraryA("idleui.dll");
        if (hLibUI != NULL)
            (DWORD&)_IdleUIGetLastInputTime = (DWORD)GetProcAddress(hLibUI, "IdleUIGetLastInputTime");
    }
#else
#ifdef HAVE_CARBON_CARBNON_H
    CFBundleRef carbonBundle;
    if (LoadFrameworkBundle( CFSTR("Carbon.framework"), &carbonBundle ) == noErr) {
        InstallEventLoopIdleTimerPtr myInstallEventLoopIdleTimer = (InstallEventLoopIdleTimerPtr)CFBundleGetFunctionPointerForName(carbonBundle, CFSTR("InstallEventLoopIdleTimer"));
        if (myInstallEventLoopIdleTimer){
            EventLoopIdleTimerUPP timerUPP = NewEventLoopIdleTimerUPP(Private::IdleTimerAction);
            (*myInstallEventLoopIdleTimer)(GetMainEventLoop(), kEventDurationSecond, kEventDurationSecond, timerUPP, 0, &mTimerRef);
        }
    }
#endif
#endif
    Event ePlugin(EventGetPluginInfo, (void*)"_core");
    pluginInfo *info = (pluginInfo*)(ePlugin.process());
    core = static_cast<CorePlugin*>(info->plugin);
    bAway = false;
    bNA   = false;
    bOff  = false;
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    m_timer->start(AUTOAWAY_TIME);
}

AutoAwayPlugin::~AutoAwayPlugin()
{
#ifdef WIN32
    _IdleUIGetLastInputTime = NULL;
    if (hLibUI)
        FreeLibrary(hLibUI);
#endif
#ifdef HAVE_CARBON_CARBNON_H
    RemoveEventLoopTimer(mTimerRef);
#endif
    free_data(autoAwayData, &data);
}

string AutoAwayPlugin::getConfig()
{
    return save_data(autoAwayData, &data);
}

QWidget *AutoAwayPlugin::createConfigWindow(QWidget *parent)
{
    return new AutoAwayConfig(parent, this);
}

void AutoAwayPlugin::timeout()
{
    unsigned long newStatus = core->getManualStatus();
    unsigned idle_time = getIdleTime() * 60;
    if ((bAway && getEnableAway() && (idle_time < getAwayTime() * 60000)) ||
            (bNA && getEnableNA() && (idle_time < getNATime() * 60000)) ||
            (bOff && getEnableOff() && (idle_time < getOffTime() * 60000))){
        bAway = false;
        bNA   = false;
        bOff  = false;
        newStatus = oldStatus;
    }else if (!bAway && !bNA && !bOff && getEnableAway() && (idle_time > getAwayTime() * 60000)){
        unsigned long status = core->getManualStatus();
        if ((status == STATUS_AWAY) || (status == STATUS_NA) || (status == STATUS_OFFLINE))
            return;
        oldStatus = status;
        newStatus = STATUS_AWAY;
        bAway = true;
    }else  if (!bNA && !bOff && getEnableNA() && (idle_time > getNATime() * 60000)){
        unsigned long status = core->getManualStatus();
        if ((status == STATUS_NA) || (status == STATUS_OFFLINE))
            return;
        if (!bAway)
            oldStatus = status;
        bNA = true;
        newStatus = STATUS_NA;
    }else if (!bOff && getEnableOff() && (idle_time > getOffTime() * 60000)){
        unsigned long status = core->getManualStatus();
        if (status == STATUS_OFFLINE)
            return;
        if (!bNA)
            oldStatus = status;
        bOff = true;
        newStatus = STATUS_OFFLINE;
    }
    if (newStatus == core->getManualStatus())
        return;
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        if (!client->getCommonStatus())
            continue;
        client->setStatus(newStatus, true);
    }
    if (core->getManualStatus() == newStatus)
        return;
    time_t now;
    time(&now);
    core->data.StatusTime.value = now;
    core->data.ManualStatus.value = newStatus;
    Event e(EventClientStatus);
    e.process();
}

void *AutoAwayPlugin::processEvent(Event *e)
{
    if ((e->type() == EventContactOnline) && getDisableAlert()){
        unsigned commonStatus = STATUS_UNKNOWN;
        for (unsigned i = 0; i < getContacts()->nClients(); i++){
            Client *client = getContacts()->getClient(i);
            if (!client->getCommonStatus())
                continue;
            commonStatus = client->getManualStatus();
            break;
        }
        if ((commonStatus == STATUS_ONLINE) || (commonStatus == STATUS_OFFLINE))
            return NULL;
        return (void*)commonStatus;
    }
    return NULL;
}

unsigned AutoAwayPlugin::getIdleTime()
{
#ifdef WIN32
    if (_GetLastInputInfo){
        LASTINPUTINFO lii;
        ZeroMemory(&lii,sizeof(lii));
        lii.cbSize=sizeof(lii);
        _GetLastInputInfo(&lii);
        return (GetTickCount()-lii.dwTime) / 1000;
    }
    if (_IdleUIGetLastInputTime)
        return _IdleUIGetLastInputTime() / 1000;
    return 0;
#else
#ifdef HAVE_CARBON_CARBON_H
    return mSecondsIdle;
#else
    QWidgetList *list = QApplication::topLevelWidgets();
    QWidgetListIt it(*list);
    QWidget *w = it.current();
    delete list;
    if (w == NULL)
        return 0;

    static XScreenSaverInfo *mit_info = NULL;
    if (mit_info == NULL) {
        int event_base, error_base;
        if(XScreenSaverQueryExtension(w->x11Display(), &event_base, &error_base)) {
            mit_info = XScreenSaverAllocInfo ();
        }
    }
    if (mit_info == NULL){
        log(L_WARN, "No XScreenSaver extension found on current XServer, disabling auto-away.");
        m_timer->stop();
        return 0;
    }
    if (!XScreenSaverQueryInfo(w->x11Display(), qt_xrootwin(), mit_info)) {
        log(L_WARN, "XScreenSaverQueryInfo failed, disabling auto-away.");
        m_timer->stop();
        return 0;
    }
    return (mit_info->idle / 1000);
#endif
#endif
}

#ifdef WIN32

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
#include "autoaway.moc"
#endif

