/***************************************************************************
                          stylescfg.cpp  -  description
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

#include "stylescfg.h"
#include "styles.h"
#include "fontconfig.h"

#include <qtabwidget.h>
#include <qlistbox.h>
#include <qstyle.h>
#include <qdir.h>

#if QT_VERSION >= 300
#include <qstylefactory.h>
#else

#ifdef WIN32
#include <windows.h>
#endif

const char *defStyles[] =
    {
        "windows",
        "motif",
        "cde",
        "motifplus",
        "platinum",
        "sgi",
        NULL
    };

#endif

StylesConfig::StylesConfig(QWidget *parent, StylesPlugin *plugin)
        : StylesConfigBase(parent)
{
    m_plugin = plugin;
    for (QObject *p = parent; p != NULL; p = p->parent()){
        if (!p->inherits("QTabWidget"))
            continue;
        QTabWidget *tab = static_cast<QTabWidget*>(p);
        font_cfg = new FontConfig(tab, m_plugin);
        tab->addTab(font_cfg, i18n("Fonts and colors"));
        break;
    }
#if QT_VERSION >= 300
    lstStyle->insertStringList(QStyleFactory::keys());
#else
    for (const char **s = defStyles; *s; s++)
        lstStyle->insertItem(*s);
#ifdef WIN32
    QDir d(app_file("plugins\\styles\\").c_str());
    QStringList styles = d.entryList("*.dll");
    for (QStringList::Iterator it = styles.begin(); it != styles.end(); ++it){
        QString name = *it;
        int n = name.findRev(".");
        if (n > 0)
            name = name.left(n);
        if (name == "xpstyle"){
            HINSTANCE hLib = LoadLibraryA("UxTheme.dll");
            if (hLib == NULL)
                continue;
            FreeLibrary(hLib);
        }
        string dll = "plugins\\styles\\";
        dll += name.latin1();
        dll += ".dll";
        HINSTANCE hLib = LoadLibraryA(app_file(dll.c_str()).c_str());
        if (hLib == NULL)
            continue;
        StyleInfo*  (*getStyleInfo)() = NULL;
        (DWORD&)getStyleInfo = (DWORD)GetProcAddress(hLib,"GetStyleInfo");
        if (getStyleInfo)
            lstStyle->insertItem(name);
        FreeLibrary(hLib);
    }
#endif
#endif
    if (*m_plugin->getStyle()){
        QListBoxItem *item = lstStyle->findItem(m_plugin->getStyle());
        if (item)
            lstStyle->setCurrentItem(item);
    }
}

StylesConfig::~StylesConfig()
{
    if (font_cfg)
        delete font_cfg;
}

void StylesConfig::apply()
{
    font_cfg->apply();
    if (m_plugin->setStyle(lstStyle->currentText().latin1()))
        m_plugin->setStyles();
}

#ifndef WIN32
#include "stylescfg.moc"
#endif

