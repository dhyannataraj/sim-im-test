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
#include "editfile.h"
#include "ballonmsg.h"

#include <qtimer.h>
#include <qtoolbar.h>
#include <qpushbutton.h>

static const char *helpList[] =
    {
        "%f",
        I18N_NOOP("Temperature &deg;F"),
        "%s",
        I18N_NOOP("Temperature &deg;C"),
        "%h",
        I18N_NOOP("Humidity"),
        "%r",
        I18N_NOOP("Wind direction"),
        "%w",
        I18N_NOOP("Wind speed mph"),
        "%x",
        I18N_NOOP("Wind speed km/h"),
        "%i",
        I18N_NOOP("Pressure in"),
        "%a",
        I18N_NOOP("Pressure hPa"),
        "%l",
        I18N_NOOP("Location"),
        "%u",
        I18N_NOOP("Updated"),
        "%p",
        I18N_NOOP("Sunraise"),
        "%q",
        I18N_NOOP("Sunset"),
        "%c",
        I18N_NOOP("Conditions"),
        NULL
    };

WeatherCfg::WeatherCfg(QWidget *parent, WeatherPlugin *plugin)
        : WeatherCfgBase(parent)
{
    m_plugin = plugin;
    setButtonsPict(this);
    edtUrl->setText(m_plugin->getURL());
    edtText->setText(unquoteText(m_plugin->getButtonText()));
    edtTip->setText(m_plugin->getTipText());
    edtText->helpList = helpList;
    edtTip->helpList = helpList;
    lnkWeather->setText(i18n("Go www.wunderground.com"));
    lnkWeather->setUrl("http://www.wunderground.com/");
    connect(btnHelp, SIGNAL(clicked()), this, SLOT(help()));
    fill();
}

void WeatherCfg::help()
{
    QString str = i18n("In text you can use:");
    str += "\n\n";
    for (const char **p = helpList; *p;){
        str += *(p++);
        str += " - ";
        str += unquoteText(i18n(*(p++)));
        str += "\n";
    }
    BalloonMsg::message(str, btnHelp, false, 400);
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

