/***************************************************************************
                          wifacecfg.cpp  -  description
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
#include "wifacecfg.h"
#include "linklabel.h"
#include "editfile.h"
#include "ballonmsg.h"

static const char *helpList[] =
    {
        "%t",
        I18N_NOOP("Temperature"),
        "%f",
        I18N_NOOP("Feels like"),
        "%h",
        I18N_NOOP("Humidity"),
        "%b",
        I18N_NOOP("Wind direction"),
        "%w",
        I18N_NOOP("Wind speed"),
        "%g",
        I18N_NOOP("Wind gust"),
        "%x",
        I18N_NOOP("Wind speed (m/s)"),
        "%y",
        I18N_NOOP("Wind gust (m/s)"),
        "%p",
        I18N_NOOP("Pressure"),
        "%q",
        I18N_NOOP("Pressure state"),
        "%v",
        I18N_NOOP("Visibility"),
        "%d",
        I18N_NOOP("Dew Point"),
        "%l",
        I18N_NOOP("Location"),
        "%u",
        I18N_NOOP("Updated"),
        "%r",
        I18N_NOOP("Sunraise"),
        "%s",
        I18N_NOOP("Sunset"),
        "%c",
        I18N_NOOP("Conditions"),
        NULL
    };

static const char *helpForecastList[] =
    {
        "%t",
        I18N_NOOP("Temperature"),
        "%n",
        I18N_NOOP("Number"),
        "%w",
        I18N_NOOP("Day of week"),
        "%d",
        I18N_NOOP("Date"),
        "%c",
        I18N_NOOP("Conditions"),
        NULL
    };

WIfaceCfg::WIfaceCfg(QWidget *parent, WeatherPlugin *plugin)
        : WIfaceCfgBase(parent)
{
    m_plugin = plugin;
    setButtonsPict(this);
    edtText->setText(unquoteText(m_plugin->getButtonText()));
    edtTip->setText(m_plugin->getTipText());
    edtForecastTip->setText(m_plugin->getForecastText());
    edtText->helpList = helpList;
    edtTip->helpList = helpList;
    edtForecastTip->helpList = helpForecastList;
    connect(btnHelp, SIGNAL(clicked()), this, SLOT(help()));
}

void WIfaceCfg::apply()
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
    if (edtForecastTip->text() == m_plugin->getForecastText()){
        m_plugin->setForecastTip("");
    }else{
        m_plugin->setForecastTip(edtForecastTip->text());
    }
}

void WIfaceCfg::help()
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

#ifndef WIN32
#include "wifacecfg.moc"
#endif

