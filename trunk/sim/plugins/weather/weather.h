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

#include "sax.h"
#include "fetch.h"

#include <qdatetime.h>

class QToolBar;

namespace SIM
{
class IconSet;
};

typedef struct WeatherData
{
    SIM::Data	ID;
    SIM::Data	Location;
    SIM::Data	Time;
    SIM::Data	ForecastTime;
    SIM::Data	Forecast;
    SIM::Data	Text;
    SIM::Data	Tip;
    SIM::Data	ForecastTip;
    SIM::Data	Units;
    SIM::Data	bar[7];
    SIM::Data	Updated;
    SIM::Data	Temperature;
    SIM::Data	FeelsLike;
    SIM::Data	DewPoint;
    SIM::Data	Humidity;
    SIM::Data    Precipitance;
    SIM::Data	Pressure;
    SIM::Data	PressureD;
    SIM::Data	Conditions;
    SIM::Data	Wind;
    SIM::Data	Wind_speed;
    SIM::Data	WindGust;
    SIM::Data	Visibility;
    SIM::Data	Sun_raise;
    SIM::Data	Sun_set;
    SIM::Data	Icon;
    SIM::Data	UT;
    SIM::Data	UP;
    SIM::Data	US;
    SIM::Data	UD;
    SIM::Data	Day;
    SIM::Data	WDay;
    SIM::Data	MinT;
    SIM::Data	MaxT;
    SIM::Data	DayIcon;
    SIM::Data	DayConditions;
	SIM::Data	UV_Intensity;
	SIM::Data	UV_Description;
	SIM::Data	MoonIcon;
	SIM::Data	MoonPhase;
} WeatherData;

class WeatherPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver, public FetchClient, public SAXParser
{
    Q_OBJECT
public:
    WeatherPlugin(unsigned, bool, ConfigBuffer*);
    virtual ~WeatherPlugin();
    PROP_STR(ID);
    PROP_STR(Location);
    PROP_ULONG(Time);
    PROP_ULONG(ForecastTime);
    PROP_ULONG(Forecast);
    PROP_UTF8(Text);
    PROP_UTF8(Tip);
    PROP_UTF8(ForecastTip);
    PROP_BOOL(Units);
    PROP_STR(Updated);
    PROP_LONG(Temperature);
    PROP_LONG(FeelsLike);
    PROP_LONG(DewPoint);
    PROP_LONG(Precipitance);
    PROP_LONG(Humidity);
    PROP_LONG(Pressure);
    PROP_STR(PressureD);
    PROP_STR(Conditions);
    PROP_STR(Wind);
    PROP_LONG(Wind_speed);
    PROP_LONG(WindGust);
    PROP_STR(Visibility);
    PROP_STR(Sun_raise);
    PROP_STR(Sun_set);
    PROP_ULONG(Icon);
    PROP_STR(UT);
    PROP_STR(US);
    PROP_STR(UP);
    PROP_STR(UD);
    PROP_STRLIST(Day);
    PROP_STRLIST(WDay);
    PROP_STRLIST(MinT);
    PROP_STRLIST(MaxT);
    PROP_STRLIST(DayIcon);
    PROP_STRLIST(DayConditions);
	PROP_LONG(UV_Intensity);
	PROP_STR(UV_Description);
	PROP_LONG(MoonIcon);
	PROP_STR(MoonPhase);

    QString getButtonText();
    QString getTipText();
    QString getForecastText();
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
    QString forecastReplace(const QString&);
    unsigned long BarWeather;
    unsigned long CmdWeather;
    std::string m_data;
    bool   m_bData;
    bool   m_bBar;
    bool   m_bWind;
    bool   m_bUv;
    bool   m_bMoon;
    bool   m_bForecast;
    bool   m_bCC;
    unsigned m_day;
    QString getConfig();
    bool isDay();
    bool parseTime(const char *str, int &h, int &m);
    bool parseDateTime(const char *str, QDateTime &dt);
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual bool done(unsigned code, Buffer &data, const char *headers);
    void *processEvent(SIM::Event*);
    WeatherData data;
    SIM::IconSet *m_icons;
    void		element_start(const char *el, const char **attr);
    void		element_end(const char *el);
    void		char_data(const char *str, int len);
};

#endif

