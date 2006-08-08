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

#include <qstylefactory.h>

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
    lstStyle->insertStringList(QStyleFactory::keys());
    if (!m_plugin->getStyle().isEmpty()){
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

#ifndef NO_MOC_INCLUDES
#include "stylescfg.moc"
#endif

