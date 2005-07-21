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
#include "styles.h"

#include <qcheckbox.h>
#include <qapplication.h>
#include <qpopupmenu.h>
#include <qcolorbutton.h>

FontConfig::FontConfig(QWidget *parent, StylesPlugin *plugin)
        : FontConfigBase(parent)
{
    m_plugin = plugin;
    connect(chkSystem, SIGNAL(toggled(bool)), this, SLOT(systemToggled(bool)));
    connect(chkColors, SIGNAL(toggled(bool)), this, SLOT(colorsToggled(bool)));
    chkSystem->setChecked(m_plugin->getSystemFonts());
    systemToggled(chkSystem->isChecked());
    if (!chkSystem->isChecked()){
        QPopupMenu m;
        QFont base = QApplication::font();
        QFont menu = QApplication::font(&m);
        base = FontEdit::str2font(m_plugin->getBaseFont(), base);
        menu = FontEdit::str2font(m_plugin->getMenuFont(), menu);
        edtFont->setFont(FontEdit::font2str(base, true));
        edtMenu->setFont(FontEdit::font2str(menu, true));
    }

    chkColors->setChecked(m_plugin->getSystemColors());
    colorsToggled(chkColors->isChecked());
}

FontConfig::~FontConfig()
{
}

void FontConfig::apply()
{
    string base;
    string menu;
    if (chkSystem->isChecked()){
        m_plugin->setSystemFonts(true);
    }else{
        m_plugin->setSystemFonts(false);
        base = edtFont->getFont();
        menu = edtMenu->getFont();
    }
    m_plugin->setBaseFont(base.c_str());
    m_plugin->setMenuFont(menu.c_str());
    m_plugin->setFonts();

    bool bChanged = false;
    if (chkColors->isChecked()){
        if (!m_plugin->getSystemColors()){
            m_plugin->setSystemColors(true);
            bChanged = true;
        }
    }else{
        if (m_plugin->getSystemColors()){
            bChanged = true;
        }else{
            bChanged = ((btnBtnColor->color().rgb() & 0xFFFFFF) != m_plugin->getBtnColor()) ||
                       ((btnBgColor->color().rgb() & 0xFFFFFF) != m_plugin->getBgColor());
        }
        m_plugin->setSystemColors(false);
        if (bChanged){
            m_plugin->setBtnColor(btnBtnColor->color().rgb() & 0xFFFFFF);
            m_plugin->setBgColor(btnBgColor->color().rgb() & 0xFFFFFF);
        }
    }
    if (bChanged)
        m_plugin->setColors();
}

void FontConfig::systemToggled(bool bState)
{
    edtFont->setEnabled(!bState);
    edtMenu->setEnabled(!bState);
    if (bState){
        m_plugin->setupDefaultFonts();
        edtFont->setFont(FontEdit::font2str(*m_plugin->m_saveBaseFont, true));
        edtMenu->setFont(FontEdit::font2str(*m_plugin->m_saveMenuFont, true));
    }
}

void FontConfig::colorsToggled(bool bState)
{
    btnBtnColor->setEnabled(!bState);
    btnBgColor->setEnabled(!bState);
    if (!bState){
        btnBtnColor->setColor(QColor(m_plugin->getBtnColor() & 0xFFFFFF));
        btnBgColor->setColor(QColor(m_plugin->getBgColor() & 0xFFFFFF));
    }
}

#ifndef WIN32
#include "fontconfig.moc"
#endif

