/***************************************************************************
                          weathercfg.cpp  -  description
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

#include "weather.h"
#include "weathercfg.h"
#include "linklabel.h"

#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qtimer.h>
#include <qtoolbar.h>

WeatherCfg::WeatherCfg(QWidget *parent, WeatherPlugin *plugin)
        : WeatherCfgBase(parent)
{
    m_plugin = plugin;
    edtUrl->setText(m_plugin->getURL());
    edtText->setText(unquoteText(m_plugin->getButtonText()));
    edtTip->setText(m_plugin->getTipText());
    lnkWeather->setText(i18n("Go www.wunderground.com"));
    lnkWeather->setUrl("http://www.wunderground.com/");
    fill();
}

void *WeatherCfg::processEvent(Event *e)
{
    if (e->type() == m_plugin->EventWeather)
        fill();
    return NULL;
}

void WeatherCfg::fill()
{
    lblName->setText(m_plugin->getLocation());
}

void WeatherCfg::apply()
{
    if (edtText->text() == unquoteText(m_plugin->getButtonText())){
        m_plugin->setText("");
    }else{
        m_plugin->setText(edtText->text());
    }
    if (edtTip->text() == m_plugin->getTipText()){
        m_plugin->setTip("");
    }else{
        m_plugin->setTip(edtTip->text());
    }
    m_plugin->setURL(edtUrl->text());
    if (*m_plugin->getURL()){
        m_plugin->showBar();
        m_plugin->updateButton();
        if (m_plugin->m_bar)
            m_plugin->m_bar->show();
        m_plugin->setTime(0);
        QTimer::singleShot(0, m_plugin, SLOT(timeout()));
    }else{
        m_plugin->hideBar();
    }
}

#ifndef WIN32
#include "weathercfg.moc"
#endif

