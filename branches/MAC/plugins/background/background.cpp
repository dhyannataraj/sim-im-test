/***************************************************************************
                          background.cpp  -  description
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

#include "background.h"
#include "simapi.h"

#include "bkgndcfg.h"

#include <qpainter.h>
#include <qfile.h>

Plugin *createBackgroundPlugin(unsigned base, bool, const char *config)
{
    Plugin *plugin = new BackgroundPlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Background"),
        I18N_NOOP("Plugin provides background pictures for user list"),
        VERSION,
        createBackgroundPlugin,
        PLUGIN_NOLOAD_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

/*
typedef struct BackgroundData
{
    char			*Background;
    unsigned long	Position;
    unsigned long	Margin;
} BackgroundData;
*/
static DataDef backgroundData[] =
    {
        { "Background", DATA_STRING, 1, 0 },
        { "Position", DATA_ULONG, 1, 0 },
        { "Margin", DATA_ULONG, 1, 0 },
        { NULL, 0, 0, 0 }
    };

#ifdef WIN32
void qInitJpeg();
#endif

BackgroundPlugin::BackgroundPlugin(unsigned base, const char *config)
        : Plugin(base)
{
#ifdef WIN32
    qInitJpeg();
#endif
    load_data(backgroundData, &data, config);
    redraw();
}

BackgroundPlugin::~BackgroundPlugin()
{
    free_data(backgroundData, &data);
}

string BackgroundPlugin::getConfig()
{
    return save_data(backgroundData, &data);
}

QWidget *BackgroundPlugin::createConfigWindow(QWidget *parent)
{
    return new BkgndCfg(parent, this);
}

void *BackgroundPlugin::processEvent(Event *e)
{
    if ((e->type() == EventPaintView) && !bgImage.isNull()){
        PaintView *pv = (PaintView*)(e->param());
        unsigned w = bgImage.width();
        unsigned h = bgImage.height();
        int x = pv->pos.x();
        int y = pv->pos.y();
        bool bTiled = false;
        unsigned pos = getPosition();
        switch(pos){
        case ContactLeft:
            h = pv->height;
            bTiled = true;
            break;
        case ContactScale:
            h = pv->height;
            w = pv->win->width();
            bTiled = true;
            break;
        case WindowTop:
            break;
        case WindowBottom:
            y += (bgImage.height() - pv->win->height());
            break;
        case WindowCenter:
            y += (bgImage.height() - pv->win->height()) / 2;
            break;
        case WindowScale:
            w = pv->win->width();
            h = pv->win->height();
            break;
        }
        const QPixmap &bg = makeBackground(w, h);
        if (bTiled){
            for (int py = 0; py < pv->size.height(); py += bg.height()){
                pv->p->drawPixmap(QPoint(0, py), bgScale, QRect(x, 0, w, h));
            }
        }else{
            pv->p->drawPixmap(QPoint(0, 0), bgScale, QRect(x, y, pv->size.width(), pv->size.height()));
            pv->isStatic = true;
        }
        pv->margin = getMargin();
    }
    return NULL;
}

void BackgroundPlugin::redraw()
{
    bgImage = QImage();
    bgScale = QPixmap();
    if (*getBackground() == 0)
        return;
    bgImage = QImage(QFile::decodeName(getBackground()));
    Event e(EventRepaintView);
    e.process();
}

QPixmap &BackgroundPlugin::makeBackground(int w, int h)
{
    if (bgImage.isNull())
        return bgScale;
    if ((bgScale.width() != w) || (bgScale.height() != h)){
        if ((bgImage.width() == w) && (bgImage.height() == h)){
            bgScale.convertFromImage(bgImage);
        }else{
            QImage img = bgImage.smoothScale(w, h);
            bgScale.convertFromImage(img);
        }
    }
    return bgScale;
}

#ifdef WIN32

#include <windows.h>

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


