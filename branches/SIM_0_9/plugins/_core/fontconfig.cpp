/***************************************************************************
                          fontconfig.cpp  -  description
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

#include "fontconfig.h"
#include "fontedit.h"
#include "core.h"

#include <qcheckbox.h>
#include <qapplication.h>
#include <qpopupmenu.h>

FontConfig::FontConfig(QWidget *parent)
        : FontConfigBase(parent)
{
    connect(chkSystem, SIGNAL(toggled(bool)), this, SLOT(systemToggled(bool)));
    chkSystem->setChecked(CorePlugin::m_plugin->getSystemFonts());
    systemToggled(chkSystem->isChecked());
    if (!chkSystem->isChecked()){
        QPopupMenu m;
        QFont base = QApplication::font();
        QFont menu = QApplication::font(&m);
        base = FontEdit::str2font(CorePlugin::m_plugin->getBaseFont(), base);
        menu = FontEdit::str2font(CorePlugin::m_plugin->getMenuFont(), menu);
        edtFont->setFont(FontEdit::font2str(base, true));
        edtMenu->setFont(FontEdit::font2str(menu, true));
    }
}

FontConfig::~FontConfig()
{
}

void FontConfig::apply()
{
    string base;
    string menu;
    if (chkSystem->isChecked()){
        CorePlugin::m_plugin->setSystemFonts(true);
    }else{
        CorePlugin::m_plugin->setSystemFonts(false);
        base = edtFont->getFont();
        menu = edtMenu->getFont();
    }
    CorePlugin::m_plugin->setBaseFont(base.c_str());
    CorePlugin::m_plugin->setMenuFont(menu.c_str());
    CorePlugin::m_plugin->setFonts();
}

void FontConfig::systemToggled(bool bState)
{
    edtFont->setEnabled(!bState);
    edtMenu->setEnabled(!bState);
    if (bState){
        CorePlugin::m_plugin->setupDefaultFonts();
        edtFont->setFont(FontEdit::font2str(*CorePlugin::m_plugin->m_saveBaseFont, true));
        edtMenu->setFont(FontEdit::font2str(*CorePlugin::m_plugin->m_saveMenuFont, true));
    }
}

#ifndef WIN32
#include "fontconfig.moc"
#endif

