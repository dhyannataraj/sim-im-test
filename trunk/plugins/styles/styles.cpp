/***************************************************************************
                          styles.cpp  -  description
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

#include "styles.h"
#include "stylescfg.h"
#include "simapi.h"
#include "fontedit.h"

#include <qapplication.h>
#include <qpopupmenu.h>
#include <qpalette.h>
#include <qstyle.h>

#ifdef WIN32
#include <windows.h>
#endif

#if COMPAT_QT_VERSION >= 0x030000
#include <qstylefactory.h>
#else
#include <qwindowsstyle.h>
#include <qplatinumstyle.h>
#include <qmotifstyle.h>
#include <qmotifplusstyle.h>
#include <qcdestyle.h>
#include <qsgistyle.h>
#endif

Plugin *createStylesPlugin(unsigned base, bool, Buffer *config)
{
    Plugin *plugin = new StylesPlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Styles"),
        I18N_NOOP("Plugin provides styles"),
        VERSION,
        createStylesPlugin,
        PLUGIN_NOLOAD_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static DataDef stylesData[] =
    {
        { "Style", DATA_STRING, 1, 0 },
        { "SystemFonts", DATA_BOOL, 1, DATA(1) },
        { "BaseFont", DATA_STRING, 1, 0 },
        { "MenuFont", DATA_STRING, 1, 0 },
        { "MessageFont", DATA_STRING, 1, 0 },
        { "SystemColors", DATA_BOOL, 1, DATA(1) },
        { "BtnColor", DATA_ULONG, 1, 0 },
        { "BgColor", DATA_ULONG, 1, 0 },
        { NULL, 0, 0, 0 }
    };

StylesPlugin::StylesPlugin(unsigned base, Buffer *config)
        : Plugin(base)
{
    m_saveBaseFont = NULL;
    m_saveMenuFont = NULL;
    m_savePalette = new QPalette(QApplication::palette());
    load_data(stylesData, &data, config);
    setFonts();
    if (getSystemColors()){
        setBtnColor(m_savePalette->active().color(QColorGroup::Button).rgb() & 0xFFFFFF);
        setBgColor(m_savePalette->active().color(QColorGroup::Background).rgb() & 0xFFFFFF);
    }else{
        setColors();
    }
#ifdef WIN32
	QApplication::addLibraryPath(app_file("plugins"));
#else
    QString path;
    path = PLUGIN_PATH;
    path += "/";
	QApplication::addLibraryPath(path);
    log(L_DEBUG,"Path: %s",path.ascii());
#endif
    setStyles();
}

StylesPlugin::~StylesPlugin()
{
    free_data(stylesData, &data);
    if (m_saveBaseFont)
        delete m_saveBaseFont;
    if (m_saveMenuFont)
        delete m_saveMenuFont;
    if (m_savePalette)
        delete m_savePalette;
}

string StylesPlugin::getConfig()
{
    return save_data(stylesData, &data);
}

QWidget *StylesPlugin::createConfigWindow(QWidget *parent)
{
    return new StylesConfig(parent, this);
}

void StylesPlugin::setFonts()
{
    if (getSystemFonts()){
        if (m_saveBaseFont)
            QApplication::setFont(*m_saveBaseFont, true);
        if (m_saveMenuFont)
            QApplication::setFont(*m_saveMenuFont, true, "QPopupMenu");
    }else{
        setupDefaultFonts();
        QPopupMenu m;
        QApplication::setFont(FontEdit::str2font(getBaseFont(), *m_saveBaseFont), true);
        QApplication::setFont(FontEdit::str2font(getMenuFont(), *m_saveMenuFont), true, "QPopupMenu");
    }
}

void StylesPlugin::setupDefaultFonts()
{
    if (m_saveBaseFont == NULL)
        m_saveBaseFont = new QFont(QApplication::font());
    if (m_saveMenuFont == NULL){
        QPopupMenu menu;
        m_saveMenuFont = new QFont(QApplication::font(&menu));
    }
}

void StylesPlugin::setColors()
{
    if (getSystemColors()){
        QApplication::setPalette(*m_savePalette, true);
    }else{
        QApplication::setPalette(QPalette(QColor(getBtnColor()), QColor(getBgColor())), true);
    }
}

void StylesPlugin::setStyles()
{
    QStyle *style = NULL;
#if COMPAT_QT_VERSION >= 0x030000
    if (*getStyle())
        style = QStyleFactory::create(getStyle());
#else
if (*getStyle()){
    string s = getStyle();
    if (s == "windows"){
        style = new QWindowsStyle;
    }else if (s == "motif"){
        style = new QMotifStyle;
    }else if (s == "cde"){
        style = new QCDEStyle;
    }else if (s == "motifplus"){
        style = new QMotifPlusStyle;
    }else if (s == "platinum"){
        style = new QPlatinumStyle;
    }else if (s == "sgi"){
        style = new QSGIStyle;
    }
}
#endif
    if (style){
        QApplication::setStyle(style);
        if (!getSystemColors())
            setColors();
    }else{
        setStyle(NULL);
    }
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


