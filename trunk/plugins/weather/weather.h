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

#include <libxml/parser.h>
#include <qdatetime.h>

#include "simapi.h"
#include "fetch.h"

class QToolBar;

typedef struct WeatherData
{
    Data	ID;
    Data	Location;
    Data	Time;
    Data	ForecastTime;
    Data	Forecast;
    Data	Text;
    Data	Tip;
    Data	ForecastTip;
    Data	Units;
    Data	bar[7];
    Data	Updated;
    Data	Temperature;
    Data	FeelsLike;
    Data	DewPoint;
    Data	Humidity;
    Data	Pressure;
    Data	PressureD;
    Data	Conditions;
    Data	Wind;
    Data	Wind_speed;
    Data	WindGust;
    Data	Visibility;
    Data	Sun_raise;
    Data	Sun_set;
    Data	Icon;
    Data	UT;
    Data	UP;
    Data	US;
    Data	UD;
    Data	Day;
    Data	WDay;
    Data	MinT;
    Data	MaxT;
    Data	DayIcon;
    Data	DayConditions;
} WeatherData;

class WeatherPlugin : public QObject, public Plugin, public EventReceiver, public FetchClient
{
    Q_OBJECT
public:
    WeatherPlugin(unsigned, bool, Buffer*);
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
    unsigned BarWeather;
    unsigned CmdWeather;
    string m_data;
    bool   m_bData;
    bool   m_bBar;
    bool   m_bWind;
    bool   m_bUv;
    bool   m_bForecast;
    bool   m_bCC;
    unsigned m_day;
    string getConfig();
    bool isDay();
    bool parseTime(const char *str, int &h, int &m);
    bool parseDateTime(const char *str, QDateTime &dt);
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual bool done(unsigned code, Buffer &data, const char *headers);
    void *processEvent(Event*);
    WeatherData data;
    xmlSAXHandler		m_handler;
    xmlParserCtxtPtr	m_context;
    void		element_start(const char *el, const char **attr);
    void		element_end(const char *el);
    void		char_data(const char *str, int len);
    static void p_element_start(void *data, const xmlChar *el, const xmlChar **attr);
    static void p_element_end(void *data, const xmlChar *el);
    static void p_char_data(void *data, const xmlChar *str, int len);
};

#endif

