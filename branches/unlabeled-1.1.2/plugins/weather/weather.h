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
#include <libxml/parser.h>

class QToolBar;

typedef struct WeatherData
{
    Data	ID;
	Data	Location;
    Data	Time;
    Data	Text;
    Data	Tip;
	Data	Units;
    Data	bar[7];
    Data	Updated;
    Data	Temperature;
    Data	Humidity;
    Data	Pressure;
    Data	Conditions;
    Data	Wind;
    Data	Wind_speed;
    Data	Sun_raise;
    Data	Sun_set;
	Data	Icon;
	Data	UT;
	Data	UP;
	Data	US;
} WeatherData;

class WeatherPlugin : public QObject, public Plugin, public EventReceiver
{
    Q_OBJECT
public:
    WeatherPlugin(unsigned, bool, const char*);
    virtual ~WeatherPlugin();
    PROP_STR(ID);
	PROP_STR(Location);
    PROP_ULONG(Time);
    PROP_UTF8(Text);
    PROP_UTF8(Tip);
	PROP_BOOL(Units);
    PROP_STR(Updated);
    PROP_LONG(Temperature);
    PROP_LONG(Humidity);
    PROP_LONG(Pressure);
    PROP_STR(Conditions);
    PROP_STR(Wind);
    PROP_LONG(Wind_speed);
    PROP_STR(Sun_raise);
    PROP_STR(Sun_set);
	PROP_ULONG(Icon);
	PROP_STR(UT);
	PROP_STR(US);
	PROP_STR(UP);
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
	string m_data;
	bool   m_bData;
	bool   m_bBar;
	bool   m_bWind;
	bool   m_bUv;
    string getConfig();
    bool isDay();
    bool parseTime(const char *str, int &h, int &m);
    virtual QWidget *createConfigWindow(QWidget *parent);
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

