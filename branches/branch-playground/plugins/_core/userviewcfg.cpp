/***************************************************************************
                          userviewcfg.cpp  -  description
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

#include "icons.h"
#include "userviewcfg.h"
#include "core.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>

using namespace SIM;

UserViewConfig::UserViewConfig(QWidget *parent) : QWidget(parent)
        //: UserViewConfigBase(parent)
{
	setupUi(this);
    chkDblClick->setChecked(CorePlugin::m_plugin->property("UseDblClick").toBool());
    chkSysColors->setChecked(CorePlugin::m_plugin->property("UseSysColors").toBool());
    btnOnline->setColor(CorePlugin::m_plugin->property("ColorOnline").toUInt());
    btnOffline->setColor(CorePlugin::m_plugin->property("ColorOffline").toUInt());
    btnAway->setColor(CorePlugin::m_plugin->property("ColorAway").toUInt());
    btnNA->setColor(CorePlugin::m_plugin->property("ColorNA").toUInt());
    btnDND->setColor(CorePlugin::m_plugin->property("ColorDND").toUInt());
    btnGroup->setColor(CorePlugin::m_plugin->property("ColorGroup").toUInt());
    chkGroupSeparator->setChecked(CorePlugin::m_plugin->property("GroupSeparator").toBool());
    chkSmallFont->setChecked(CorePlugin::m_plugin->getSmallGroupFont());
    chkScroll->setChecked(CorePlugin::m_plugin->getNoScroller());
    connect(chkSysColors, SIGNAL(toggled(bool)), this, SLOT(colorsToggled(bool)));
    colorsToggled(chkSysColors->isChecked());
    fillBox(cmbSort1);
    fillBox(cmbSort2);
    fillBox(cmbSort3);
    setSortMode(CorePlugin::m_plugin->getSortMode());
    connect(cmbSort1, SIGNAL(activated(int)), this, SLOT(sortChanged(int)));
    connect(cmbSort2, SIGNAL(activated(int)), this, SLOT(sortChanged(int)));
    connect(cmbSort3, SIGNAL(activated(int)), this, SLOT(sortChanged(int)));
    btnAuth1->setIcon(Icon("text_strike"));
    btnAuth2->setIcon(Icon("text_italic"));
    btnAuth3->setIcon(Icon("text_under"));
    btnVisible1->setIcon(Icon("text_strike"));
    btnVisible2->setIcon(Icon("text_italic"));
    btnVisible3->setIcon(Icon("text_under"));
    btnInvisible1->setIcon(Icon("text_strike"));
    btnInvisible2->setIcon(Icon("text_italic"));
    btnInvisible3->setIcon(Icon("text_under"));
    btnAuth1->setChecked(true);
    btnAuth2->setChecked(true);
    btnAuth3->setChecked(true);
    btnVisible1->setChecked(true);
    btnVisible2->setChecked(true);
    btnVisible3->setChecked(true);
    btnInvisible1->setChecked(true);
    btnInvisible2->setChecked(true);
    btnInvisible3->setChecked(true);
    connect(btnAuth1, SIGNAL(toggled(bool)), this, SLOT(setFonts(bool)));
    connect(btnAuth2, SIGNAL(toggled(bool)), this, SLOT(setFonts(bool)));
    connect(btnAuth3, SIGNAL(toggled(bool)), this, SLOT(setFonts(bool)));
    connect(btnVisible1, SIGNAL(toggled(bool)), this, SLOT(setFonts(bool)));
    connect(btnVisible2, SIGNAL(toggled(bool)), this, SLOT(setFonts(bool)));
    connect(btnVisible3, SIGNAL(toggled(bool)), this, SLOT(setFonts(bool)));
    connect(btnInvisible1, SIGNAL(toggled(bool)), this, SLOT(setFonts(bool)));
    connect(btnInvisible2, SIGNAL(toggled(bool)), this, SLOT(setFonts(bool)));
    connect(btnInvisible3, SIGNAL(toggled(bool)), this, SLOT(setFonts(bool)));
    btnAuth1->setChecked((CorePlugin::m_plugin->getAuthStyle() & STYLE_STRIKE) != 0);
    btnAuth2->setChecked((CorePlugin::m_plugin->getAuthStyle() & STYLE_ITALIC) != 0);
    btnAuth3->setChecked((CorePlugin::m_plugin->getAuthStyle() & STYLE_UNDER) != 0);
    btnVisible1->setChecked((CorePlugin::m_plugin->getVisibleStyle() & STYLE_STRIKE) != 0);
    btnVisible2->setChecked((CorePlugin::m_plugin->getVisibleStyle() & STYLE_ITALIC) != 0);
    btnVisible3->setChecked((CorePlugin::m_plugin->getVisibleStyle() & STYLE_UNDER) != 0);
    btnInvisible1->setChecked((CorePlugin::m_plugin->getInvisibleStyle() & STYLE_STRIKE) != 0);
    btnInvisible2->setChecked((CorePlugin::m_plugin->getInvisibleStyle() & STYLE_ITALIC) != 0);
    btnInvisible3->setChecked((CorePlugin::m_plugin->getInvisibleStyle() & STYLE_UNDER) != 0);
    setFonts(true);
}

UserViewConfig::~UserViewConfig()
{
}

void UserViewConfig::apply()
{
    CorePlugin::m_plugin->setProperty("UseDblClick", chkDblClick->isChecked());
    CorePlugin::m_plugin->setProperty("UseSysColors", chkSysColors->isChecked());
    CorePlugin::m_plugin->setProperty("GroupSeparator", chkGroupSeparator->isChecked());
    CorePlugin::m_plugin->setSortMode(getSortMode());
    CorePlugin::m_plugin->setSmallGroupFont(chkSmallFont->isChecked());
    CorePlugin::m_plugin->setNoScroller(chkScroll->isChecked());
    if (CorePlugin::m_plugin->property("UseSysColors").toBool()){
        CorePlugin::m_plugin->setProperty("ColorOnline", 0);
        CorePlugin::m_plugin->setProperty("ColorOffline", 0);
        CorePlugin::m_plugin->setProperty("ColorAway", 0);
        CorePlugin::m_plugin->setProperty("ColorNA", 0);
        CorePlugin::m_plugin->setProperty("ColorDND", 0);
        CorePlugin::m_plugin->setProperty("ColorGroup", 0);
    }else{
        CorePlugin::m_plugin->setProperty("ColorOnline", btnOnline->color().rgb());
        CorePlugin::m_plugin->setProperty("ColorOffline", btnOffline->color().rgb());
        CorePlugin::m_plugin->setProperty("ColorAway", btnAway->color().rgb());
        CorePlugin::m_plugin->setProperty("ColorNA", btnNA->color().rgb());
        CorePlugin::m_plugin->setProperty("ColorDND", btnDND->color().rgb());
        CorePlugin::m_plugin->setProperty("ColorGroup", btnGroup->color().rgb());
    }
    unsigned style = 0;
    if (btnAuth1->isChecked()) style |= STYLE_STRIKE;
    if (btnAuth2->isChecked()) style |= STYLE_ITALIC;
    if (btnAuth3->isChecked()) style |= STYLE_UNDER;
    CorePlugin::m_plugin->setAuthStyle(style);
    style = 0;
    if (btnVisible1->isChecked()) style |= STYLE_STRIKE;
    if (btnVisible2->isChecked()) style |= STYLE_ITALIC;
    if (btnVisible3->isChecked()) style |= STYLE_UNDER;
    CorePlugin::m_plugin->setVisibleStyle(style);
    style = 0;
    if (btnInvisible1->isChecked()) style |= STYLE_STRIKE;
    if (btnInvisible2->isChecked()) style |= STYLE_ITALIC;
    if (btnInvisible3->isChecked()) style |= STYLE_UNDER;
    CorePlugin::m_plugin->setInvisibleStyle(style);
    EventRepaintView e;
    e.process();
}

void UserViewConfig::colorsToggled(bool state)
{
    if (state){
        QColor textColor = colorGroup().text();
        QColor disabledColor = palette().disabled().text();
        btnOnline->setColor(textColor);
        btnOffline->setColor(disabledColor);
        btnAway->setColor(disabledColor);
        btnNA->setColor(disabledColor);
        btnDND->setColor(disabledColor);
        btnGroup->setColor(disabledColor);
    }
    btnOnline->setEnabled(!state);
    btnOffline->setEnabled(!state);
    btnAway->setEnabled(!state);
    btnNA->setEnabled(!state);
    btnDND->setEnabled(!state);
    btnGroup->setEnabled(!state);
    lblOnline->setEnabled(!state);
    lblOffline->setEnabled(!state);
    lblAway->setEnabled(!state);
    lblNA->setEnabled(!state);
    lblDND->setEnabled(!state);
    lblGroup->setEnabled(!state);
    lblColors->setEnabled(!state);
}

void UserViewConfig::fillBox(QComboBox *cmb)
{
    cmb->insertItem(INT_MAX,i18n("Status"));
    cmb->insertItem(INT_MAX,i18n("Last message time"));
    cmb->insertItem(INT_MAX,i18n("Contact name"));
    cmb->insertItem(INT_MAX,"");
}

void UserViewConfig::setSortMode(unsigned mode)
{
    QComboBox *cmb[3] = { cmbSort1, cmbSort2, cmbSort3 };
    unsigned i;
    for (i = 0; i < 3; i++){
        cmb[i]->setEnabled(true);
        unsigned m = mode & 0xFF;
        mode = mode >> 8;
        if (m == 0){
            cmb[i++]->setCurrentIndex(3);
            break;
        }
        cmb[i]->setCurrentIndex(m - 1);
    }
    for (;i < 3; i++){
        cmb[i]->setCurrentIndex(3);
        cmb[i]->setEnabled(false);
    }
}

void UserViewConfig::sortChanged(int)
{
    setSortMode(getSortMode());
}

void UserViewConfig::setFonts(bool)
{
    QFont fAuth(font());
    fAuth.setStrikeOut(btnAuth1->isChecked());
    fAuth.setItalic(btnAuth2->isChecked());
    fAuth.setUnderline(btnAuth3->isChecked());
    lblAuth->setFont(fAuth);
    QFont fVisible(font());
    fVisible.setStrikeOut(btnVisible1->isChecked());
    fVisible.setItalic(btnVisible2->isChecked());
    fVisible.setUnderline(btnVisible3->isChecked());
    lblVisible->setFont(fVisible);
    QFont fInvisible(font());
    fInvisible.setStrikeOut(btnInvisible1->isChecked());
    fInvisible.setItalic(btnInvisible2->isChecked());
    fInvisible.setUnderline(btnInvisible3->isChecked());
    lblInvisible->setFont(fInvisible);
}

unsigned UserViewConfig::getSortMode()
{
    unsigned m1 = cmbSort1->currentIndex() + 1;
    if (m1 > 3)
        m1 = 0;
    unsigned m2 = cmbSort2->currentIndex() + 1;
    if (m2 > 3)
        m2 = 0;
    unsigned m3 = cmbSort3->currentIndex() + 1;
    if (m3 > 3)
        m3 = 0;
    if (m1){
        if (m2 == m1)
            m2 = 0;
        if (m2){
            if ((m3 == m1) || (m3 == m2))
                m3 = 0;
        }else{
            m3 = 0;
        }
    }else{
        m2 = 0;
        m3 = 0;
    }
    return (m3 << 16) + (m2 << 8) + m1;
}


/*
#ifndef NO_MOC_INCLUDES
#include "userviewcfg.moc"
#endif
*/

