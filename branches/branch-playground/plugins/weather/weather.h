/***************************************************************************
                          weather.h  -  description
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

#ifndef _WEATHER_H
#define _WEATHER_H

#include "cfg.h"
#include "event.h"
#include "fetch.h"
#include "plugins.h"
#include "propertyhub.h"

#include <QDomDocument>

class QByteArray;
class QDateTime;
class QToolBar;
class QXmlStreamAttributes;

namespace SIM
{
class IconSet;
};

class WeatherPlugin : virtual public QObject, public SIM::Plugin, public SIM::EventReceiver, public FetchClient, public SIM::PropertyHub
{
    Q_OBJECT
public:
    WeatherPlugin(unsigned, bool, Buffer*);
    virtual ~WeatherPlugin();
    QString getButtonText();
    QString getTipText();
    QString getForecastText();
    void updateButton();
    void showBar();
    void hideBar();
    SIM::SIMEvent EventWeather;
    QToolBar *m_bar;
protected slots:
    void timeout();
    void barDestroyed();
protected:
    QString replace(const QString&);
    QString forecastReplace(const QString&);
    unsigned long BarWeather;
    unsigned long CmdWeather;
    QString m_data;
    bool   m_bData;
    bool   m_bBar;
    bool   m_bWind;
    bool   m_bUv;
    bool   m_bMoon;
    bool   m_bForecast;
    bool   m_bCC;
    char   m_bDayPart;
    bool   m_bDayForecastIsValid;
    unsigned m_day;
    virtual QByteArray getConfig();
    bool isDay();
    bool parseTime(const QString &str, int &h, int &m);
    bool parseDateTime(const QString &str, QDateTime &dt);
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual bool done(unsigned code, Buffer &data, const QString &headers);
    virtual bool processEvent(SIM::Event *e);
    SIM::IconSet *m_icons;

    bool parse(QDomDocument document);
};

#endif

