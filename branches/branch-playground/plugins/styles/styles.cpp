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

#include <qapplication.h>
#include <qpalette.h>
#include <qstyle.h>
#include <qstylefactory.h>
#include <qmenu.h>

#include "fontedit.h"
#include "misc.h"

#include "styles.h"
#include "stylescfg.h"

using namespace SIM;

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

StylesPlugin::StylesPlugin(unsigned base, Buffer *config)
    : Plugin(base), PropertyHub("styles")
{
    m_saveBaseFont = NULL;
    m_saveMenuFont = NULL;
    m_savePalette = new QPalette(QApplication::palette());
}

StylesPlugin::~StylesPlugin()
{
    PropertyHub::save();
    if (m_saveBaseFont)
        delete m_saveBaseFont;
    if (m_saveMenuFont)
        delete m_saveMenuFont;
    if (m_savePalette)
        delete m_savePalette;
}

QByteArray StylesPlugin::getConfig()
{
    return QByteArray();
}

QWidget *StylesPlugin::createConfigWindow(QWidget *parent)
{
    return new StylesConfig(parent, this);
}

void StylesPlugin::setFonts()
{
    if (property("SystemFonts").toBool()){
        if (m_saveBaseFont)
            QApplication::setFont(*m_saveBaseFont);
        if (m_saveMenuFont)
            QApplication::setFont(*m_saveMenuFont, "Q3PopupMenu");
    }else{
        setupDefaultFonts();
        QApplication::setFont(FontEdit::str2font(property("BaseFont").toString(), *m_saveBaseFont));
        QApplication::setFont(FontEdit::str2font(property("MenuFont").toString(), *m_saveMenuFont), "Q3PopupMenu");
    }
}

void StylesPlugin::setupDefaultFonts()
{
    if (m_saveBaseFont == NULL)
        m_saveBaseFont = new QFont(QApplication::font());
    if (m_saveMenuFont == NULL){
        QMenu menu;
        m_saveMenuFont = new QFont(QApplication::font(&menu));
    }
}

void StylesPlugin::setColors()
{
    if (property("SystemColors").toBool()){
        QApplication::setPalette(*m_savePalette);
    }else{
        QApplication::setPalette(QPalette(QColor(property("BtnColor").toUInt()), QColor(property("BgColor").toUInt())));
    }
}

void StylesPlugin::setStyles()
{
    QString sStyle = property("Style").toString();
    QStyle *style = QStyleFactory::create(sStyle);
    if (style){
        QApplication::setStyle(style);
        if (!property("SystemColors").toBool())
            setColors();
    }else{
        setProperty("Style", QString());
    }
}

bool StylesPlugin::processEvent(SIM::Event *e)
{
    if(e->type() == eEventPluginLoadConfig)
    {
        PropertyHub::load();
        setFonts();
        if (property("SystemColors").toBool()){
            setProperty("BtnColor", m_savePalette->color(QPalette::Active, QPalette::Button).rgb());
            setProperty("BgColor", m_savePalette->color(QPalette::Active, QPalette::Background).rgb());
        }
        setColors();
        setStyles();
    }
    return false;
}
