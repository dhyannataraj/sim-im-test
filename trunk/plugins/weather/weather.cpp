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
#include "socket.h"
#include "toolbtn.h"
#include "icons.h"

#include <time.h>

#include <qapplication.h>
#include <qwidgetlist.h>
#include <qtoolbar.h>
#include <qmainwindow.h>
#include <qtimer.h>
#include <qfile.h>
#include <qregexp.h>
#include <qtooltip.h>

using namespace SIM;

const unsigned CHECK1_INTERVAL = 30 * 60;
const unsigned CHECK2_INTERVAL = 120 * 60;

Plugin *createWeatherPlugin(unsigned base, bool bInit, Buffer *config)
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
        { "ID", DATA_STRING, 1, 0 },
        { "Location", DATA_STRING, 1, 0 },
        { "Time", DATA_ULONG, 1, 0 },
        { "ForecastTime", DATA_ULONG, 1, 0 },
        { "Forecast", DATA_ULONG, 1, DATA(2) },
        { "Text", DATA_UTF, 1, 0 },
        { "Tip", DATA_UTF, 1, 0 },
        { "ForecastTip", DATA_UTF, 1, 0 },
        { "Units", DATA_BOOL, 1, 0 },
        { "Bar", DATA_LONG, 7, DATA(QMainWindow::Bottom) },
        { "Updated", DATA_STRING, 1, 0 },
        { "Temperature", DATA_LONG, 1, 0 },
        { "FeelsLike", DATA_LONG, 1, 0 },
        { "DewPoint", DATA_LONG, 1, 0 },
        { "Humidity", DATA_LONG, 1, 0 },
        { "Precipitation", DATA_LONG, 1, 0 },
        { "Pressure", DATA_LONG, 1, 0 },
        { "PressureD", DATA_STRING, 1, 0 },
        { "Conditions", DATA_STRING, 1, 0 },
        { "Wind", DATA_STRING, 1, 0 },
        { "Wind_speed", DATA_LONG, 1, 0 },
        { "WindGust", DATA_LONG, 1, 0 },
        { "Visibiliy", DATA_STRING, 1, 0 },
        { "Sun_raise", DATA_STRING, 1, 0 },
        { "Sun_set", DATA_STRING, 1, 0 },
        { "Icon", DATA_ULONG, 1, 0 },
        { "UT", DATA_STRING, 1, 0 },
        { "US", DATA_STRING, 1, 0 },
        { "UP", DATA_STRING, 1, 0 },
        { "UD", DATA_STRING, 1, 0 },
        { "Day", DATA_STRLIST, 1, 0 },
        { "WDay", DATA_STRLIST, 1, 0 },
        { "MinT", DATA_STRLIST, 1, 0 },
        { "MaxT", DATA_STRLIST, 1, 0 },
        { "DayIcon", DATA_STRLIST, 1, 0 },
        { "DayConditions", DATA_STRLIST, 1, 0 },
        { "UV_Intensity", DATA_LONG, 1, 0 },
        { "UV_Description", DATA_STRING, 1, 0 },
        { "MoonIcon", DATA_LONG, 1, 0 },
        { "MoonPhase", DATA_STRING, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

WeatherPlugin::WeatherPlugin(unsigned base, bool bInit, Buffer *config)
        : Plugin(base)
{
    load_data(weatherData, &data, config);
    BarWeather = registerType();
    CmdWeather = registerType();
    EventWeather = registerType();
    m_icons = getIcons()->addIconSet("icons/weather.jisp", true);
    // work around a cmake 2.4.3 bug (vs only)
    if(!m_icons)
        m_icons = getIcons()->addIconSet("icons/weather_.jisp", true);
    Event eBar(EventToolbarCreate, (void*)BarWeather);
    eBar.process();
    Command cmd;
    cmd->id = CmdWeather;
    cmd->text = I18N_NOOP("Not connected");
    cmd->icon = "weather";
    cmd->bar_id = BarWeather;
    cmd->bar_grp = 0x1000;
    cmd->flags = BTN_PICT | BTN_DIV;
    Event eCmd(EventCommandCreate, cmd);
    eCmd.process();
    m_bar = NULL;
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
    getIcons()->removeIconSet(m_icons);
}

std::string WeatherPlugin::getConfig()
{
    if (m_bar)
        saveToolbar(m_bar, data.bar);
    return save_data(weatherData, &data);
}

void WeatherPlugin::timeout()
{
    if (!getSocketFactory()->isActive() || !isDone() || getID().isEmpty())
        return;
    time_t now = time(NULL);
    if ((unsigned)now < getTime() + CHECK1_INTERVAL)
        return;
    m_bForecast = false;
    if ((unsigned)now >= getForecastTime() + CHECK2_INTERVAL)
        m_bForecast = true;
    QString url = "http://xoap.weather.com/weather/local/";
    url += getID();
    url += "?cc=*&prod=xoap&par=1004517364&key=a29796f587f206b2&unit=";
    url += getUnits() ? "s" : "m";
    if (m_bForecast && getForecast()){
        url += "&dayf=";
        url += QString::number(getForecast());
    }
    fetch(url);
}

void *WeatherPlugin::processEvent(Event *e)
{
    if (e->type() == eEventLanguageChanged)
        updateButton();
    if (e->type() == eEventInit)
        showBar();
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if ((cmd->id == CmdWeather) && !getID().isEmpty()){
            QString url = "http://www.weather.com/outlook/travel/local/";
            url += getID();
            EventGoURL eGo(url);
            eGo.process();
            return e->param();
        }
    }
    return NULL;
}

bool WeatherPlugin::done(unsigned code, Buffer &data, const QString&)
{
    if (code != 200)
        return false;
    m_data  = "";
    m_day   = 0;
    m_bBar  = false;
    m_bWind = false;
    m_bUv	= false;
    m_bCC	= false;
    m_bMoon	= false;
    reset();
    if (!parse(data.data(), data.size(), false)){
        log(L_WARN, "XML parse error");
        return false;
    }
    time_t now = time(NULL);
    setTime(now);
    if (m_bForecast)
        setForecastTime(now);
    updateButton();
    Event eUpdate(EventWeather);
    eUpdate.process();
    return false;
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

bool WeatherPlugin::parseTime(const QString &str, int &h, int &m)
{
    QString s = str;
    h = getToken(s, ':').toLong();
    m = getToken(s, ' ').toLong();
    if ((getToken(s, ' ') == "PM") && (h < 12))
        h += 12;
    if (h == 24)
        h = 0;
    return true;
}

bool WeatherPlugin::parseDateTime(const QString &str, QDateTime &dt)
{
    int h, m, D, M, Y;
    QString daytime;

    QString s = str;
    /* MM/DD/YY/ hh:mm */
    M = getToken(s, '/').toLong();
    D = getToken(s, '/').toLong();
    Y = getToken(s, ' ').toLong();
    h = getToken(s, ':').toLong();
    m = getToken(s, ' ').toLong();

    if (getToken(s, ' ') == "PM"  && (h < 12))
        h += 12;
    /* 12:20 PM is 00:20 and 12:30 AM is 12:20
       but what date is 12:20 pm 4/7/04? */
    if (h == 24)
        h = 0;
    if (Y < 70)
        Y += 2000;
    dt.setDate(QDate(Y,M,D));
    dt.setTime(QTime(h,m,0,0));
    return true;
}

bool WeatherPlugin::isDay()
{
    int raise_h = 0, raise_m = 0;
    int set_h = 0, set_m = 0;
    if (!parseTime(getSun_raise(), raise_h, raise_m) || !parseTime(getSun_set(), set_h, set_m))
        return false;
    time_t now = time(NULL);
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
    if (m_bar || getID().isEmpty())
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
    Command cmd;
    cmd->id      = CmdWeather;
    cmd->text    = I18N_NOOP("Not connected");
    cmd->icon    = "weather" + QString::number(getIcon());
    cmd->bar_id  = BarWeather;
    cmd->bar_grp = 0x1000;
    cmd->flags   = BTN_PICT | BTN_DIV;
    Event eCmd(EventCommandChange, cmd);
    eCmd.process();

    QString text = unquoteText(getButtonText());
    QString tip  = getTipText();
    QString ftip = getForecastText();
    text = replace(text);
    tip  = replace(tip);
    if (getForecast())
        tip = QString("<table><tr><td>") + tip + "</td><td>";
    unsigned n = (getForecast() + 1) / 2;
    if (n < 3)
        n = getForecast();
    for (m_day = 1; m_day <= getForecast(); m_day++){
        tip += forecastReplace(ftip);
        if (--n == 0){
            tip += "</td><td>";
            n = (getForecast() + 1) / 2;
        }
    }
    if (getForecast())
        tip += "</td></tr></table>";
    tip += "<br>\n"+i18n("weather", "Weather data provided by weather.com&reg;");
    Command cmdw;
    cmdw->id	= CmdWeather;
    cmdw->param	= m_bar;
    Event e(EventCommandWidget, cmdw);
    CToolButton *btn = (CToolButton*)e.process();
    if (btn == NULL)
        return;
    btn->setTextLabel(text);
    btn->repaint();
    QToolTip::add(btn, tip);
}

#if 0
i18n("Monday")
i18n("Tuesday")
i18n("Wednesday")
i18n("Thursday")
i18n("Friday")
i18n("Saturday")
i18n("Sunday")

i18n("weather", "Squalls")
i18n("weather", "Overcast")
i18n("weather", "Fog")
i18n("weather", "Mist")
i18n("weather", "Haze")
i18n("weather", "Storm")
i18n("weather", "T-Storm")
i18n("weather", "T-Storms")
i18n("weather", "Scattered T-Storms")
i18n("weather", "Thunder")
i18n("weather", "Light Rain with Thunder")
i18n("weather", "Thunder in the Vicinity")
i18n("weather", "Rain")
i18n("weather", "Light Rain")
i18n("weather", "Heavy Rain")
i18n("weather", "Freezing Rain")
i18n("weather", "Rain to Snow")
i18n("weather", "Snow")
i18n("weather", "Light Snow")
i18n("weather", "Few Snow")
i18n("weather", "Scattered Snow")
i18n("weather", "Clear")
i18n("weather", "Clearing")
i18n("weather", "Showers")
i18n("weather", "Showers in the Vicinity")
i18n("weather", "Mostly Clear")
i18n("weather", "Sunny")
i18n("weather", "Fair")
i18n("weather", "Cloudy")
i18n("weather", "Clouds")
i18n("weather", "Mostly Cloudy")
i18n("weather", "Partly Cloudy")
i18n("weather", "Wind")
i18n("weather", "Windy")
i18n("weather", "Drizzle")
i18n("weather", "Freezing Drizzle")
i18n("weather", "Freezing Rain")
i18n("weather", "Light Drizzle")
i18n("weather", "Drifting Snow")
i18n("weather", "Snow Grains")
i18n("weather", "Scattered")
i18n("weather", "Smoke")
i18n("weather", "steady")
i18n("weather", "rising")
i18n("weather", "falling")
i18n("weather", "Unlimited")

i18n("weather", "N")
i18n("weather", "NNW")
i18n("weather", "NW")
i18n("weather", "WNW")
i18n("weather", "W")
i18n("weather", "WSW")
i18n("weather", "SW")
i18n("weather", "SSW")
i18n("weather", "S")
i18n("weather", "SSE")
i18n("weather", "SE")
i18n("weather", "ESE")
i18n("weather", "E")
i18n("weather", "ENE")
i18n("weather", "NE")
i18n("weather", "NNE")
i18n("weather", "VAR")
i18n("weather", "CALM")

i18n("weather", "km")
i18n("weather", "mi")
i18n("weather", "km/h")
i18n("weather", "mph")
i18n("weather", "mmHg");
i18n("weather", "inHg");

i18n("weather", "Low")
i18n("weather", "Moderate")
i18n("weather", "High")

i18n("moonphase", "New")
i18n("moonphase", "Waxing Crescent")
i18n("moonphase", "First Quarter")
i18n("moonphase", "Waxing Gibbous")
i18n("moonphase", "Full")
i18n("moonphase", "Waning Gibbous")
i18n("moonphase", "Last Quarter")
i18n("moonphase", "Waning Crescent")
#endif

static QString i18n_conditions(const QString &str)
{
    if (str.isEmpty())
        return "";
    int n = str.find(" / ");
    if (n >= 0)
        return i18n_conditions(str.left(n)) + " / " + i18n_conditions(str.mid(n + 3));
    n = str.find(" and ");
    if (n >= 0)
        return i18n_conditions(str.left(n)) + " " + i18n("and") + " " + i18n_conditions(str.mid(n + 5));
    n = str.find(" Early");
    if (n >= 0)
        return i18n_conditions(str.left(n)) + " " + i18n("weather", "Early");
    n = str.find(" Late");
    if (n >= 0)
        return i18n_conditions(str.left(n)) + " " + i18n("weather", "Late");
    QString s = str;
    s = s.replace(QRegExp(" Showers"), "");
    s = s.replace(QRegExp(" Shower"), "");
    return i18n("weather", s);
}

QString WeatherPlugin::replace(const QString &text)
{
    QString res = text;
    QString sun_set, sun_raise, updated;
    QTime tmp_time;
    QDateTime dt;
    int h,m;

    parseTime(getSun_set(),h,m);
    tmp_time.setHMS(h,m,0,0);
    sun_set = tmp_time.toString(Qt::LocalDate);
    sun_set = sun_set.left(sun_set.length() - 3);

    parseTime(getSun_raise(),h,m);
    tmp_time.setHMS(h,m,0,0);
    sun_raise = tmp_time.toString(Qt::LocalDate);
    sun_raise = sun_raise.left(sun_raise.length() - 3);

    parseDateTime(getUpdated(),dt);
    updated = dt.toString(Qt::LocalDate);
    updated = updated.left(updated.length() - 3);
    /* double Expressions *before* single or better RegExp ! */
    res = res.replace(QRegExp("\\%mp"), i18n("moonphase", getMoonPhase()));
    res = res.replace(QRegExp("\\%mi"), QString::number(getMoonIcon()));
    res = res.replace(QRegExp("\\%pp"), QString::number(getPrecipitation()));
    res = res.replace(QRegExp("\\%ut"), i18n("weather", getUV_Description()));
    res = res.replace(QRegExp("\\%ui"), QString::number(getUV_Intensity()));
    res = res.replace(QRegExp("\\%t"), QString::number((int)getTemperature()) + QChar((unsigned short)176) + getUT());
    res = res.replace(QRegExp("\\%f"), QString::number((int)getFeelsLike()) + QChar((unsigned short)176) + getUT());
    res = res.replace(QRegExp("\\%d"), QString::number((int)getDewPoint()) + QChar((unsigned short)176) + getUT());
    res = res.replace(QRegExp("\\%h"), QString::number(getHumidity()) + "%");
    res = res.replace(QRegExp("\\%w"), QString::number(getWind_speed()) + " " + i18n("weather",getUS()));
    res = res.replace(QRegExp("\\%x"), QString::number(getWind_speed() * 10 / 36) + " " + i18n("m/s"));
    res = res.replace(QRegExp("\\%g"), getWindGust() ? QString("(") + i18n("gust") + QString(" ") + QString::number(getWindGust()) + i18n("weather",getUS()) + QString(")") : QString(""));
    res = res.replace(QRegExp("\\%y"), getWindGust() ? QString("(") + i18n("gust") + QString(" ") + QString::number(getWindGust() * 10 / 36) + QString(" ") + i18n("m/s") + QString(")") : QString(""));
    res = res.replace(QRegExp("\\%p"), QString::number(getPressure()) + " " + i18n("weather",getUP()));
    res = res.replace(QRegExp("\\%a"), QString::number(getPressure() * 75 / 100)); // depricated!
    res = res.replace(QRegExp("\\%q"), i18n("weather", getPressureD()));
    res = res.replace(QRegExp("\\%l"), getLocation());
    res = res.replace(QRegExp("\\%b"), i18n("weather", getWind()));
    res = res.replace(QRegExp("\\%u"), updated);
    res = res.replace(QRegExp("\\%r"), sun_raise);
    res = res.replace(QRegExp("\\%s"), sun_set);
    res = res.replace(QRegExp("\\%c"), i18n_conditions(getConditions()));
    res = res.replace(QRegExp("\\%v"), i18n("weather", getVisibility()) + (atol(getVisibility()) ? QString(" ") + i18n("weather",getUD()) : QString("")));
    res = res.replace(QRegExp("\\%i"), QString::number(getIcon()));
    return res;
}

QString WeatherPlugin::forecastReplace(const QString &text)
{
    if (getDay(m_day).isEmpty())
        return QString::null;
    QString res = text;
    QString temp;
    int minT = getMinT(m_day).toInt();
    int maxT = getMaxT(m_day).toInt();
    temp += QString::number(minT);
    temp += QChar((unsigned short)176);
    temp += getUT();
    if ((strcmp(getMaxT(m_day), "N/A")) && (maxT != -255)) {
        temp += "/";
        temp += QString::number(maxT);
        temp += QChar((unsigned short)176);
        temp += getUT();
    }
    QString dd = getDay(m_day);
    QString mon = getToken(dd, ' ');
    QString day = dd;
    day += ". ";
    day += i18n(mon);
    res = res.replace(QRegExp("\\%n"), getDayIcon(m_day));
    res = res.replace(QRegExp("\\%t"), temp);
    res = res.replace(QRegExp("\\%c"), i18n_conditions(getDayConditions(m_day)));
    res = res.replace(QRegExp("\\%w"), i18n(getWDay(m_day)));
    res = res.replace(QRegExp("\\%d"), day);
    return res;
}

QString WeatherPlugin::getButtonText()
{
    QString str = getText();
    if (str.isEmpty())
        str = i18n("%t | %c");
    return str;
}

QString WeatherPlugin::getTipText()
{
    QString str = getTip();
    if (str.isEmpty())
        str =	"%l<br><br>\n"
		"<b>"+i18n("weather","Current Weather")+":</b><br>\n"
		"<img src=\"icon:weather%i\"> %c<br>\n"+
		i18n("weather","Temperature")+": <b>%t</b> ("+i18n("weather","feels like")+": <b>%f</b>)<br>\n"+
		i18n("weather","Humidity")+": <b>%h</b><br>\n"+
		i18n("weather","Chance of Precipitation")+": <b>%pp%</b><br>\n"+
		i18n("weather","Pressure")+": <b>%p</b> (%q)<br>\n"+
		i18n("weather","Wind")+": <b>%b</b> <b>%w %g</b><br>\n"+
		i18n("weather","Visibility")+": <b>%v</b><br>\n"+
		i18n("weather","Dew Point")+": <b>%d</b><br>\n"+
		i18n("weather","Sunrise")+": %r<br>\n"+
		i18n("weather","Sunset")+": %s<br>\n"+
		i18n("weather","UV-Intensity is <b>%ut</b> with value <b>%ui</b> (of 11)")+"<br>\n"
		"<b>"+i18n("weather","Moonphase")+": </b>%mp<br>\n"
		"<img src=\"icon:moon%mi\"><br>\n"
		"<br>\n"+
		i18n("weather","Updated")+": %u<br>\n";
    return str;
}

QString WeatherPlugin::getForecastText()
{
    QString str = getForecastTip();
    if (str.isEmpty())
        str = i18n("<br><br>\n"
                   "<b>Forecast for</b><br>\n"
                   "<nobr><b>%d %w</b></nobr><br>\n"
                   "<img src=\"icon:weather%n\"> %c<br>\n"
                   " Temperature: <b>%t</b><br>\n");
    return str;
}

QWidget *WeatherPlugin::createConfigWindow(QWidget *parent)
{
    return new WeatherCfg(parent, this);
}

static const char *tags[] =
    {
        "obst",
        "lsup",
        "sunr",
        "suns",
        "tmp",
        "flik",
        "ppcp",
        "hmid",
        "t",
        "icon",
        "r",
        "s",
        "d",
        "ut",
        "i",
        "us",
        "up",
        "ud",
        "gust",
        "vis",
        "dewp",
        "hi",
        "low",
        NULL,
    };

void WeatherPlugin::element_start(const char *el, const char **attr)
{
    m_bData = false;
    if (!strcmp(el, "cc")){
        m_bCC = true;
        return;
    }
    if (!strcmp(el, "bar")){
        m_bBar = true;
        return;
    }
    if (!strcmp(el, "wind")){
        m_bWind = true;
        return;
    }
    if (!strcmp(el, "uv")) {
		m_bUv = true;
        return;
    }
    if (!strcmp(el, "moon")) {
        m_bMoon = true;
        return;
    }
    if (!strcmp(el, "day")){
        m_bDayForecastIsValid = true; // Initial value
        QString wday;
        QString day;
        for (const char **p = attr; *p;){
            QCString key  = *(p++);
            QString value = *(p++);
            if (key == "d"){
                m_day = value.toLong();
                continue;
            }
            if (key == "dt"){
                day = value;
                continue;
            }
            if (key == "t"){
                wday = value;
                continue;
            }
            if (m_day > getForecast()){
                m_day = 0;
                continue;
            }
        }
        m_day++;
        setDay(m_day, day);
        setWDay(m_day, wday);
        return;
    }
    if (!strcmp(el, "part")){
	for (const char **p = attr; *p;){
	    QCString key  = *(p++);
	    QString value = *(p++);
	    if (key == "p"){
		if (value == "d") m_bDayPart = 'd';
		if (value == "n") m_bDayPart = 'n';
	    }
	}
	return;
    }
    for (const char **p = tags; *p; p++){
        if (!strcmp(*p, el)){
            m_bData = true;
            m_data = QString::null;
            return;
        }
    }
}

void WeatherPlugin::element_end(const char *el)
{
    if (!strcmp(el, "day")){
        if (getMinT(m_day).isEmpty() || getMaxT(m_day).isEmpty())
            m_day--;
        return;
    }
    if (!strcmp(el, "obst")){
        setLocation(m_data);
        m_data = QString::null;
        return;
    }
    if (!strcmp(el, "lsup")){
        setUpdated(m_data);
        m_data = QString::null;
        return;
    }
    if (!strcmp(el, "sunr") && (m_day == 0)){
        setSun_raise(m_data);
        m_data = QString::null;
        return;
    }
    if (!strcmp(el, "suns") && (m_day == 0)){
        setSun_set(m_data);
        m_data = QString::null;
        return;
    }
    if (!strcmp(el, "vis") && m_bCC){
        setVisibility(m_data);
        m_data = QString::null;
        return;
    }
    if (!strcmp(el, "tmp") && m_bCC){
        setTemperature(m_data.toLong());
        m_data = QString::null;
        return;
    }
    if (!strcmp(el, "flik") && m_bCC){
        setFeelsLike(m_data.toLong());
        m_data = QString::null;
        return;
    }
    if (!strcmp(el, "dewp") && m_bCC){
        setDewPoint(m_data.toLong());
        m_data = QString::null;
        return;
    }
    if (!strcmp(el, "ppcp") && (m_day == 1) ) {
        if (((m_bDayPart == 'd') && m_bDayForecastIsValid) || ((m_bDayPart == 'n') && ! m_bDayForecastIsValid )){
    	    setPrecipitation(m_data.toLong());
            m_data = QString::null;
    	    return;
	}
    }
    if (!strcmp(el, "hmid") && m_bCC){
        setHumidity(m_data.toLong());
        m_data = QString::null;
        return;
    }
    if (!strcmp(el, "low") && m_day){
        if (m_data == "N/A")
            m_data = QString::null;
        setMinT(m_day, m_data);
        m_data = QString::null;
        return;
    }
    if (!strcmp(el, "hi") && m_day){
        if (m_data == "N/A")
            m_data = "-255";
        setMaxT(m_day, m_data);
        m_data = QString::null;
        return;
    }
    if (!strcmp(el, "t")){
        if (!m_bBar && !m_bWind && !m_bUv && !m_bMoon){
            if (m_bCC){
                setConditions(m_data);
            }else{
                setDayConditions(m_day, m_data);
		if ((m_data == "N/A") && (m_bDayPart == 'd')) 
		    m_bDayForecastIsValid = false;
            }
        }
        if (m_bWind && m_bCC)
            setWind(m_data);
        if (m_bUv && m_bCC)
            setUV_Description(m_data);
        if (m_bMoon && m_bCC)
            setMoonPhase(m_data);

        m_data = QString::null;
        return;
    }
    if (!strcmp(el, "i")) {
        if (m_bUv && m_bCC)
            setUV_Intensity(m_data.toLong());
        m_data = QString::null;
        return;
    }
    if (!strcmp(el, "icon")){
        if (m_bMoon && m_bCC) {
            setMoonIcon(m_data.toLong());
        } else if (m_bCC){
            setIcon(m_data.toULong());
        }else{
            setDayIcon(m_day, m_data);
        }
        m_data = QString::null;
        return;
    }
    if (!strcmp(el, "ut")){
        setUT(m_data);
        m_data = QString::null;
        return;
    }
    if (!strcmp(el, "up")){
        if (m_data == "in"){
	    setUP("inHg");
	} else {
	    setUP(m_data);
	}
        m_data = QString::null;
        return;
    }
    if (!strcmp(el, "us")){
        setUS(m_data);
        m_data = QString::null;
        return;
    }
    if (!strcmp(el, "ud")){
        setUD(m_data);
        m_data = QString::null;
        return;
    }
    if (!strcmp(el, "gust") && m_bCC){
        setWindGust(m_data.toLong());
        m_data = QString::null;
        return;
    }
    if (!strcmp(el, "bar")){
        m_bBar = false;
        return;
    }
    if (!strcmp(el, "cc")){
        m_bCC = false;
        return;
    }
    if (!strcmp(el, "r") && m_bBar && m_bCC){
        unsigned long v = (unsigned long)m_data.toFloat();
	if ( QString(getUP()) == "mb" ){
	    v=v * 75 / 100;
	    setPressure(v);
	    setUP("mmHg");
	} else{
	    setPressure(v);
	}
        return;
    }
    if (!strcmp(el, "d") && m_bBar && m_bCC){
        setPressureD(m_data);
        m_data = QString::null;
        return;
    }
    if (!strcmp(el, "wind")){
        m_bWind = false;
        return;
    }
    if (!strcmp(el, "s") && m_bWind && m_bCC){
        setWind_speed(m_data.toLong());
        return;
    }
    if (!strcmp(el, "uv")){
        m_bUv = false;
        return;
    }
    if (!strcmp(el, "moon")){
        m_bMoon = false;
        return;
    }
}

void WeatherPlugin::char_data(const char *str, int len)
{
    if (m_bData)
        m_data += QString::fromLatin1(str, len);
}

#ifndef NO_MOC_INCLUDES
#include "weather.moc"
#endif
