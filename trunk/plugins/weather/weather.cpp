/***************************************************************************
                          weather.cpp  -  description
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
#include "buffer.h"
#include "fetch.h"
#include "html.h"
#include "toolbtn.h"

#include "xpm/overcast.xpm"
#include "xpm/fog.xpm"
#include "xpm/storm.xpm"
#include "xpm/rain.xpm"
#include "xpm/snow.xpm"
#include "xpm/cloudy.xpm"
#include "xpm/day.xpm"
#include "xpm/night.xpm"
#include "xpm/day_cloudy.xpm"
#include "xpm/night_cloudy.xpm"

#include <time.h>

#include <qapplication.h>
#include <qwidgetlist.h>
#include <qtoolbar.h>
#include <qmainwindow.h>
#include <qtimer.h>
#include <qfile.h>
#include <qregexp.h>
#include <qtooltip.h>

const unsigned CHECK_INTERVAL = 30 * 60;

Plugin *createWeatherPlugin(unsigned base, bool bInit, const char *config)
{
    Plugin *plugin = new WeatherPlugin(base, bInit, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Weather"),
        I18N_NOOP("Plugin provides show weather"),
        VERSION,
        createWeatherPlugin,
        PLUGIN_NOLOAD_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static DataDef weatherData[] =
    {
        { "URL", DATA_STRING, 1, 0 },
        { "Time", DATA_LONG, 1, 0 },
        { "Text", DATA_UTF, 1, 0 },
        { "Tip", DATA_UTF, 1, 0 },
        { "Bar", DATA_LONG, 7, DATA(QMainWindow::Bottom) },
        { "Updated", DATA_STRING, 1, 0 },
        { "Location", DATA_STRING, 1, 0 },
        { "Temperature_F", DATA_LONG, 1, 0 },
        { "Temperature_C", DATA_LONG, 1, 0 },
        { "Humidity", DATA_LONG, 1, 0 },
        { "Pressure_In", DATA_LONG, 1, 0 },
        { "Pressure_hPa", DATA_LONG, 1, 0 },
        { "Conditions", DATA_STRING, 1, 0 },
        { "Wind", DATA_STRING, 1, 0 },
        { "Wind_speed_mph", DATA_LONG, 1, 0 },
        { "Wind_speed_km", DATA_LONG, 1, 0 },
        { "Sun_raise", DATA_STRING, 1, 0 },
        { "Sun_set", DATA_STRING, 1, 0 },
        { NULL, 0, 0, 0 }
    };

WeatherPlugin::WeatherPlugin(unsigned base, bool bInit, const char *config)
        : Plugin(base)
{
    load_data(weatherData, &data, config);
    BarWeather = registerType();
    CmdWeather = registerType();
    EventWeather = registerType();
    Event eBar(EventToolbarCreate, (void*)BarWeather);
    eBar.process();
    IconDef icon;
    icon.name = "weather";
    icon.xpm  = overcast;
    Event eIcon(EventAddIcon, &icon);
    eIcon.process();
    Command cmd;
    cmd->id = CmdWeather;
    cmd->text = I18N_NOOP("Not connected");
    cmd->icon = "weather";
    cmd->bar_id = BarWeather;
    cmd->bar_grp = 0x1000;
    cmd->flags = BTN_PICT;
    Event eCmd(EventCommandCreate, cmd);
    eCmd.process();
    m_bar = NULL;
    m_fetch_id = 0;
    if (!bInit){
        showBar();
        if (m_bar)
            m_bar->show();
    }
}

WeatherPlugin::~WeatherPlugin()
{
    if (m_bar)
        delete m_bar;
    Event eCmd(EventCommandRemove, (void*)CmdWeather);
    Event eBar(EventToolbarRemove, (void*)BarWeather);
    free_data(weatherData, &data);
}

string WeatherPlugin::getConfig()
{
    if (m_bar)
        saveToolbar(m_bar, data.bar);
    return save_data(weatherData, &data);
}

class WeatherParser : public HTMLParser
{
public:
    WeatherParser(Buffer &buf);
    string m_updated;
    string m_location;
    string m_temperature_f;
    string m_temperature_c;
    string m_humidity;
    string m_pressure_in;
    string m_pressure_hpa;
    string m_conditions;
    string m_wind;
    string m_wind_speed_mph;
    string m_wind_speed_km;
    string m_sun_raise;
    string m_sun_set;
protected:
    bool m_bWind;
    bool m_bWind1;
    bool m_bTemperature;
    bool m_bPressure;
    string *m_data;
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &options);
    virtual void tag_end(const QString &tag);
};

void WeatherPlugin::timeout()
{
    if (!getSocketFactory()->isActive() || m_fetch_id || (*getURL() == 0))
        return;
    time_t now;
    time(&now);
    if ((unsigned)now < getTime() + CHECK_INTERVAL)
        return;
    m_fetch_id = fetch(NULL, getURL());
}

WeatherParser::WeatherParser(Buffer &buf)
{
    m_data = NULL;
    m_bWind			= false;
    m_bWind1		= false;
    m_bTemperature	= false;
    m_bPressure		= false;
    parse(buf);
}

void WeatherParser::text(const QString &text)
{
    if (m_data)
        *m_data += text.latin1();
    if (text.find("Updated:") >= 0)
        m_data = &m_updated;
    if (text.find("Observed at") >= 0)
        m_data = &m_location;
    if (text == "Temperature")
        m_data = &m_temperature_f;
    if (text == "Humidity")
        m_data = &m_humidity;
    if (text == "Pressure")
        m_data = &m_pressure_in;
    if (text == "Wind")
        m_data = &m_wind;
    if ((text == "Conditions") && m_conditions.empty()){
        m_bPressure = false;
        m_data = &m_conditions;
    }
    if (text == "Actual Time")
        m_data = &m_sun_raise;
}

void WeatherParser::tag_start(const QString &tag, const list<QString>&)
{
    if (tag == "b"){
        if (m_bTemperature){
            m_data = &m_temperature_c;
            m_bWind = false;
        }
        if (m_bPressure){
            m_data = &m_pressure_hpa;
            m_bWind = false;
        }
        if (m_bWind){
            m_data = &m_wind_speed_mph;
            m_bWind = false;
        }
        if (m_bWind1){
            m_data = &m_wind_speed_km;
            m_bWind = false;
        }
    }
    if (tag == "br")
        m_data = NULL;
}

void WeatherParser::tag_end(const QString &tag)
{
    if (tag == "b"){
        m_bWind = (m_data == &m_wind);
        m_bWind1 = (m_data == &m_wind_speed_mph);
        m_bTemperature = (m_data == &m_temperature_f);
        m_bPressure = (m_data == &m_pressure_in);
        if (m_data == &m_sun_raise){
            m_data = &m_sun_set;
        }else{
            m_data = NULL;
        }
    }
}

static void setLong(const char *str, Data &d)
{
    for (; *str; str++){
        if (((*str >= '0') && (*str <= '9')) || (*str == '-')){
            d.value = atol(str);
            break;
        }
    }
}

static void setStr(const char *str, Data &d)
{
    for (; *str; str++){
        if (*str != ' '){
            set_str(&d.ptr, str);
            break;
        }
    }
}

void *WeatherPlugin::processEvent(Event *e)
{
    if (e->type() == EventLanguageChanged)
        updateButton();
    if (e->type() == EventInit)
        showBar();
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if ((cmd->id == CmdWeather) && *getURL()){
            Event eGo(EventGoURL, (void*)getURL());
            eGo.process();
            return e->param();
        }
    }
    if (e->type() == EventFetchDone){
        fetchData *d = (fetchData*)(e->param());
        if (d->req_id != m_fetch_id)
            return NULL;
        m_fetch_id = 0;
        if (d->result != 200)
            return NULL;
        WeatherParser p(*d->data);
        setStr(p.m_updated.c_str(), data.Updated);
        setStr(p.m_location.c_str(), data.Location);
        setLong(p.m_temperature_f.c_str(), data.Temperature_f);
        setLong(p.m_temperature_c.c_str(), data.Temperature_c);
        setLong(p.m_humidity.c_str(), data.Humidity);
        setLong(p.m_pressure_in.c_str(), data.Pressure_in);
        setLong(p.m_pressure_hpa.c_str(), data.Pressure_hpa);
        setStr(p.m_conditions.c_str(), data.Conditions);
        setStr(p.m_wind.c_str(), data.Wind);
        setLong(p.m_wind_speed_mph.c_str(), data.Wind_speed_mph);
        setLong(p.m_wind_speed_km.c_str(), data.Wind_speed_km);
        setStr(p.m_sun_raise.c_str(), data.Sun_raise);
        setStr(p.m_sun_set.c_str(), data.Sun_set);
        QString condition = getConditions();
        condition = condition.lower();
        if (condition.find("fog") >= 0)
            condition = "Fog";
        if (condition.find("overcast") >= 0)
            condition = "Overcast";
        if (condition.find("mist") >= 0)
            condition = "Fog";
        if (condition.find("storm") >= 0)
            condition = "Storm";
        if (condition.find("rain") >= 0)
            condition = "Rain";
        if (condition.find("snow") >= 0)
            condition = "Snow";
        if (condition.find("clear") >= 0)
            condition = "Clear";
        if (condition.find("cloudy") >= 0){
            if (condition.find("part") >= 0){
                condition = "Partial cloudy";
            }else{
                condition = "Cloudy";
            }
        }
        if (condition.find("clouds") >= 0)
            condition = "Partial cloudy";
        setConditions(condition.latin1());
        time_t now;
        time(&now);
        setTime(now);
        updateButton();
        Event eUpdate(EventWeather);
        eUpdate.process();
    }
    return NULL;
}

void WeatherPlugin::barDestroyed()
{
    m_bar = NULL;
}

void WeatherPlugin::hideBar()
{
    if (m_bar){
        delete m_bar;
        m_bar = NULL;
    }
}

bool WeatherPlugin::parseTime(const char *str, int &h, int &m)
{
    string s = str;
    h = atol(getToken(s, ':').c_str());
    m = atol(getToken(s, ' ').c_str());
    if (getToken(s, ' ') == "PM")
        h += 12;
    return true;
}

bool WeatherPlugin::isDay()
{
    int raise_h, raise_m;
    int set_h, set_m;
    if (!parseTime(getSun_raise(), raise_h, raise_m) || !parseTime(getSun_set(), set_h, set_m))
        return false;
    time_t now;
    time(&now);
    struct tm *tm = localtime(&now);
    if ((tm->tm_hour > raise_h) && (tm->tm_hour < set_h))
        return true;
    if ((tm->tm_hour == raise_h) && (tm->tm_min >= raise_m))
        return true;
    if ((tm->tm_hour == set_h) && (tm->tm_min <= set_m))
        return true;
    return false;
}

void WeatherPlugin::showBar()
{
    if (m_bar || (*getURL() == 0))
        return;
    QWidgetList  *list = QApplication::topLevelWidgets();
    QWidgetListIt it( *list );
    QWidget *w;
    while ((w=it.current()) != 0) {
        ++it;
        if (w->inherits("MainWindow"))
            break;
    }
    delete list;
    if (w == NULL)
        return;
    BarShow b;
    b.bar_id = BarWeather;
    b.parent = (QMainWindow*)w;
    Event e(EventShowBar, &b);
    m_bar = (QToolBar*)e.process();
    restoreToolbar(m_bar, data.bar);
    connect(m_bar, SIGNAL(destroyed()), this, SLOT(barDestroyed()));
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
    QTimer::singleShot(0, this, SLOT(timeout()));
    timer->start(120000);
    updateButton();
}

void WeatherPlugin::updateButton()
{
    if ((getTime() == 0) || (m_bar == NULL))
        return;
    const char **xpm = NULL;
    QString conditions(getConditions());
    if (conditions == "Overcast"){
        xpm = overcast;
    }else if (conditions == "Fog"){
        xpm = fog;
    }else if (conditions == "Storm"){
        xpm = storm;
    }else if (conditions == "Rain"){
        xpm = rain;
    }else if (conditions == "Snow"){
        xpm = snow;
    }else if (conditions == "Cloudy"){
        xpm = snow;
    }else if (conditions == "Clear"){
        xpm = isDay() ? day : night;
    }else if (conditions == "Partial cloudy"){
        xpm = isDay() ? day_cloudy : night_cloudy;
    }
    if (xpm){
        IconDef icon;
        icon.name = "weather";
        icon.xpm  = xpm;
        Event eIcon(EventAddIcon, &icon);
        eIcon.process();
    }
    QString text = unquoteText(getButtonText());
    QString tip = getTipText();
    text = replace(text);
    tip  = replace(tip);
    Command cmd;
    cmd->id		= CmdWeather;
    cmd->param	= m_bar;
    Event e(EventCommandWidget, cmd);
    CToolButton *btn = (CToolButton*)e.process();
    if (btn == NULL)
        return;
    btn->setTextLabel(text);
    btn->repaint();
    QToolTip::add(btn, tip);
}

static QString number(unsigned long n)
{
    char b[32];
    sprintf(b, "%i", n);
    return b;
}

#if 0
i18n("weather", "Overcast")
i18n("weather", "Fog")
i18n("weather", "Storm")
i18n("weather", "Rain")
i18n("weather", "Snow")
i18n("weather", "Clear")
i18n("weather", "Cloudy")
i18n("weather", "Partial cloudy")
#endif

QString WeatherPlugin::replace(const QString &text)
{
    QString res = text;
    res = res.replace(QRegExp("\\%f"), number(getTemperature_f()));
    res = res.replace(QRegExp("\\%s"), number(getTemperature_c()));
    res = res.replace(QRegExp("\\%h"), number(getHumidity()));
    res = res.replace(QRegExp("\\%w"), number(getWind_speed_mph()));
    res = res.replace(QRegExp("\\%x"), number(getWind_speed_km()));
    res = res.replace(QRegExp("\\%i"), number(getPressure_in()));
    res = res.replace(QRegExp("\\%a"), number(getPressure_hpa()));
    res = res.replace(QRegExp("\\%l"), getLocation());
    res = res.replace(QRegExp("\\%r"), getWind());
    res = res.replace(QRegExp("\\%u"), getUpdated());
    res = res.replace(QRegExp("\\%p"), getSun_raise());
    res = res.replace(QRegExp("\\%q"), getSun_set());
    res = res.replace(QRegExp("\\%c"), i18n("weather", getConditions()));
    return res;
}

QString WeatherPlugin::getButtonText()
{
    QString str = getText();
    if (str.isEmpty())
        str = i18n("%f &deg;F/ %s &deg;C");
    return str;
}


QString WeatherPlugin::getTipText()
{
    QString str = getTip();
    if (str.isEmpty())
        str = i18n("%l<br><br>\n"
                   "<img src=\"icon:weather\"> %c<br>\n"
                   "Temperature: <b>%f &deg;F</b> (<b>%s &deg;C</b>)<br>\n"
                   "Humidity: <b>%h%</b><br>\n"
                   "Pressure: <b>%i</b> in (<b>%a</a> hPa)<br>\n"
                   "Wind: <b>%r</b> <b>%w</b> mph (<b>%x</b> km/h)<br>\n"
                   "Sunrise: %p<br>\n"
                   "Sunset: %q<br>\n"
                   "<br>\n"
                   "Updated: %u");
    return str;
}

QWidget *WeatherPlugin::createConfigWindow(QWidget *parent)
{
    return new WeatherCfg(parent, this);
}

#ifdef WIN32
#include <windows.h>

/**
 * DLL's entry point
 **/
int WINAPI DllMain(HINSTANCE, DWORD, LPVOID)
{
    return TRUE;
}

/**
 * This is to prevent the CRT from loading, thus making this a smaller
 * and faster dll.
 **/
extern "C" BOOL __stdcall _DllMainCRTStartup( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return DllMain( hinstDLL, fdwReason, lpvReserved );
}

#endif

#ifndef WIN32
#include "weather.moc"
#endif


