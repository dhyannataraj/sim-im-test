/***************************************************************************
                          splash.cpp  -  description
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

#include "splash.h"
#include "simapi.h"

#include <qwidget.h>
#include <qpixmap.h>
#include <qapplication.h>
#include <qfile.h>

Plugin *createSplashPlugin(unsigned base, bool bStart, Buffer*)
{
    Plugin *plugin = new SplashPlugin(base, bStart);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Splash"),
        I18N_NOOP("Plugin provides splash screen"),
        VERSION,
        createSplashPlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

SplashPlugin::SplashPlugin(unsigned base, bool bStart)
        : Plugin(base), EventReceiver(LowestPriority)
{
    splash = NULL;
    m_bStart = bStart;
    if (m_bStart){
        string pictPath = app_file("pict/splash.png");
        QPixmap pict(QFile::decodeName(pictPath.c_str()));
        if (!pict.isNull()){
            splash = new QWidget(NULL, "splash",
                                 QWidget::WType_TopLevel | QWidget::WStyle_Customize |
                                 QWidget::WStyle_NoBorderEx | QWidget::WStyle_StaysOnTop);
            splash->resize(pict.width(), pict.height());
            QWidget *desktop = QApplication::desktop();
            splash->move((desktop->width() - pict.width()) / 2, (desktop->height() - pict.height()) / 2);
            splash->setBackgroundPixmap(pict);
            const QBitmap *mask = pict.mask();
            if (mask) splash->setMask(*mask);
            splash->show();
        }
    }
}

SplashPlugin::~SplashPlugin()
{
    if (splash)
        delete splash;
}

void *SplashPlugin::processEvent(Event *e)
{
    switch(e->type()){
    case EventInit:
        if (splash){
            delete splash;
            splash = NULL;
        }
        break;
    }
    return NULL;
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


