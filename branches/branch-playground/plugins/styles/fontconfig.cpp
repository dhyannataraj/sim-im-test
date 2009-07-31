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
#include <qcolorbutton.h>

#include <qcheckbox.h>
#include <qapplication.h>
#include <qmenu.h>

FontConfig::FontConfig(QWidget *parent, StylesPlugin *plugin)
        : QWidget(parent)
{
    setupUi(this);
    m_plugin = plugin;
    connect(chkSystem, SIGNAL(toggled(bool)), this, SLOT(systemToggled(bool)));
    connect(chkColors, SIGNAL(toggled(bool)), this, SLOT(colorsToggled(bool)));
    chkSystem->setChecked(m_plugin->property("SystemFonts").toBool());
    systemToggled(chkSystem->isChecked());
    if (!chkSystem->isChecked()){
        QMenu m;
        QFont base = QApplication::font();
        QFont menu = QApplication::font(&m);
        base = FontEdit::str2font(m_plugin->property("BaseFont").toString(), base);
        menu = FontEdit::str2font(m_plugin->property("MenuFont").toString(), menu);
        edtFont->setFont(FontEdit::font2str(base, true));
        edtMenu->setFont(FontEdit::font2str(menu, true));
    }

    chkColors->setChecked(m_plugin->property("SystemColors").toBool());
    colorsToggled(chkColors->isChecked());
}

FontConfig::~FontConfig()
{
}

void FontConfig::apply()
{
    QString base;
    QString menu;
    if (chkSystem->isChecked()){
        m_plugin->setProperty("SystemFonts", true);
    }else{
        m_plugin->setProperty("SystemFonts", false);
        base = edtFont->getFont();
        menu = edtMenu->getFont();
    }
    m_plugin->setProperty("BaseFont", base);
    m_plugin->setProperty("MenuFont", menu);
    m_plugin->setFonts();

    bool bChanged = false;
    if (chkColors->isChecked()){
        if (!m_plugin->property("SystemColors").toBool()){
            m_plugin->setProperty("SystemColors", true);
            bChanged = true;
        }
    }else{
        if (m_plugin->property("SystemColors").toBool()){
            bChanged = true;
        }else{
            bChanged = ((btnBtnColor->color().rgb() & 0xFFFFFF) != m_plugin->property("BtnColor").toUInt()) ||
                       ((btnBgColor->color().rgb() & 0xFFFFFF) != m_plugin->property("BgColor").toUInt());
        }
        m_plugin->setProperty("SystemColors", false);
        if (bChanged){
            m_plugin->setProperty("BtnColor", btnBtnColor->color().rgb() & 0xFFFFFF);
            m_plugin->setProperty("BgColor", btnBgColor->color().rgb() & 0xFFFFFF);
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
        btnBtnColor->setColor(QColor(m_plugin->property("BtnColor").toUInt() & 0xFFFFFF));
        btnBgColor->setColor(QColor(m_plugin->property("BgColor").toUInt() & 0xFFFFFF));
    }
}

