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

#include "simapi.h"

class QToolBar;

typedef struct WeatherData
{
    Data	URL;
    Data	Time;
    Data	Text;
    Data	Tip;
    Data	bar[7];
    Data	Updated;
    Data	Location;
    Data	Temperature_f;
    Data	Temperature_c;
    Data	Humidity;
    Data	Pressure_in;
    Data	Pressure_hpa;
    Data	Conditions;
    Data	Wind;
    Data	Wind_speed_mph;
    Data	Wind_speed_km;
    Data	Sun_raise;
    Data	Sun_set;
} WeatherData;

class WeatherPlugin : public QObject, public Plugin, public EventReceiver
{
    Q_OBJECT
public:
    WeatherPlugin(unsigned, bool, const char*);
    virtual ~WeatherPlugin();
    PROP_STR(URL);
    PROP_ULONG(Time);
    PROP_UTF8(Text);
    PROP_UTF8(Tip);
    PROP_STR(Updated);
    PROP_STR(Location);
    PROP_LONG(Temperature_f);
    PROP_LONG(Temperature_c);
    PROP_LONG(Humidity);
    PROP_LONG(Pressure_in);
    PROP_LONG(Pressure_hpa);
    PROP_STR(Conditions);
    PROP_STR(Wind);
    PROP_LONG(Wind_speed_mph);
    PROP_LONG(Wind_speed_km);
    PROP_STR(Sun_raise);
    PROP_STR(Sun_set);
    QString getButtonText();
    QString getTipText();
    void updateButton();
    void showBar();
    void hideBar();
    unsigned EventWeather;
    QToolBar *m_bar;
protected slots:
    void timeout();
    void barDestroyed();
protected:
    QString replace(const QString&);
    unsigned BarWeather;
    unsigned CmdWeather;
    unsigned m_fetch_id;
    string getConfig();
    bool isDay();
    bool parseTime(const char *str, int &h, int &m);
    virtual QWidget *createConfigWindow(QWidget *parent);
    void *processEvent(Event*);
    WeatherData data;
};

#endif

