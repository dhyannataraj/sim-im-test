/***************************************************************************
                          zodiak.cpp  -  description
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

#include "zodiak.h"
#include "datepicker.h"

#include "xpm/1.xpm"
#include "xpm/2.xpm"
#include "xpm/3.xpm"
#include "xpm/4.xpm"
#include "xpm/5.xpm"
#include "xpm/6.xpm"
#include "xpm/7.xpm"
#include "xpm/8.xpm"
#include "xpm/9.xpm"
#include "xpm/10.xpm"
#include "xpm/11.xpm"
#include "xpm/12.xpm"

#include <qapplication.h>
#include <qlayout.h>
#include <qframe.h>
#include <qwidgetlist.h>
#include <qobjectlist.h>
#include <qpushbutton.h>
#include <qpainter.h>

Plugin *createZodiakPlugin(unsigned base, bool, const char*)
{
    Plugin *plugin = new ZodiakPlugin(base);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Zodiak"),
        I18N_NOOP("Plugin provides show zodiak puctures for date edit"),
        VERSION,
        createZodiakPlugin,
        PLUGIN_NOLOAD_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

ZodiakPlugin::ZodiakPlugin(unsigned base)
        : Plugin(base)
{
    qApp->installEventFilter(this);
    QWidgetList  *list = QApplication::topLevelWidgets();
    QWidgetListIt it(*list);
    QWidget * w;
    while ((w = it.current()) != NULL){
        QObjectList * l = w->queryList("DatePicker");
        QObjectListIt it1(*l);
        QObject * obj;
        while ((obj=it1.current()) != NULL){
            ++it1;
            createLabel(static_cast<DatePicker*>(obj));
        }
        delete l;
        ++it;
    }
    delete list;
}

ZodiakPlugin::~ZodiakPlugin()
{
    for (list<Picker>::iterator it = m_pickers.begin(); it != m_pickers.end(); ++it){
        delete (*it).label;
    }
    m_pickers.clear();
}

void ZodiakPlugin::createLabel(DatePicker *picker)
{
    Picker p;
    p.picker = picker;
    p.label  = new ZodiakWnd(picker);
    m_pickers.push_back(p);
    if (p.picker->layout())
        static_cast<QBoxLayout*>(p.picker->layout())->addWidget(p.label);
    p.label->show();
}

void *ZodiakPlugin::processEvent(Event *e)
{
    if (e->type() == EventQuit)
        m_pickers.clear();
    return NULL;
}

bool ZodiakPlugin::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::ChildInserted){
        QChildEvent *ce = (QChildEvent*)e;
        if (ce->child()->inherits("DatePicker")){
            DatePicker *picker = (DatePicker*)(ce->child());
            list<Picker>::iterator it;
            for (it = m_pickers.begin(); it != m_pickers.end(); ++it){
                if ((*it).picker == picker)
                    break;
            }
            if (it == m_pickers.end())
                createLabel(picker);
        }
    }
    if (e->type() == QEvent::ChildRemoved){
        QChildEvent *ce = (QChildEvent*)e;
        if (ce->child()->inherits("DatePicker")){
            DatePicker *picker = (DatePicker*)(ce->child());
            for (list<Picker>::iterator it = m_pickers.begin(); it != m_pickers.end(); ++it){
                if ((*it).picker == picker){
                    m_pickers.erase(it);
                    break;
                }
            }
        }
    }
    return QObject::eventFilter(o, e);
}

ZodiakWnd::ZodiakWnd(DatePicker *parent)
        : QFrame(parent)
{
    m_picker = parent;
    setLineWidth(0);
    QGridLayout *lay = new QGridLayout(this, 2, 2);
    lay->setSpacing(2);
    lay->setMargin(4);
    m_picture = new QLabel(this);
    m_picture->setFixedSize(52, 52);
    m_picture->setFrameShadow(Sunken);
    m_picture->setLineWidth(1);
    lay->addMultiCellWidget(m_picture, 0, 1, 0, 0);
    m_name = new QLabel(this);
    QFont f(font());
    f.setBold(true);
    m_name->setFont(f);
    m_name->setAlignment(AlignVCenter | AlignHCenter);
    lay->addWidget(m_name, 0, 1);
    m_button = new QPushButton(this);
    m_button->setText(i18n("View horoscope"));
    lay->addWidget(m_button, 1, 1);
    changed();
    connect(parent, SIGNAL(changed()), this, SLOT(changed()));
    connect(m_button, SIGNAL(clicked()), this, SLOT(view()));
}

void ZodiakWnd::paintEvent(QPaintEvent *e)
{
    if (parentWidget() && parentWidget()->parentWidget() && parentWidget()->parentWidget()->backgroundPixmap()){
        QPoint pos = mapToParent(QPoint(0, 0));
        pos = parentWidget()->mapToParent(pos);
        QPainter p(this);
        p.drawTiledPixmap(0, 0, width(), height(), *parentWidget()->parentWidget()->backgroundPixmap(), pos.x(), pos.y());
        return;
    }
    QFrame::paintEvent(e);
}

static const char *signes[] =
    {
        I18N_NOOP("Aries"),
        I18N_NOOP("Taurus"),
        I18N_NOOP("Gemini"),
        I18N_NOOP("Cancer"),
        I18N_NOOP("Leo"),
        I18N_NOOP("Virgo"),
        I18N_NOOP("Libra"),
        I18N_NOOP("Scorpio"),
        I18N_NOOP("Saqittarius"),
        I18N_NOOP("Capricorn"),
        I18N_NOOP("Aquarius"),
        I18N_NOOP("Pisces")
    };

static const char **xpms[] =
    {
        xpm_1,
        xpm_2,
        xpm_3,
        xpm_4,
        xpm_5,
        xpm_6,
        xpm_7,
        xpm_8,
        xpm_9,
        xpm_10,
        xpm_11,
        xpm_12
    };

void ZodiakWnd::changed()
{
    int day, month, year;
    m_picker->getDate(day, month, year);
    if (day && month && year){
        int n = getSign(day, month);
        m_picture->setPixmap(QPixmap(xpms[n]));
        m_name->setText(i18n(signes[n]));
        m_button->show();
    }else{
        m_picture->setPixmap(QPixmap());
        m_name->setText("");
        m_button->hide();
    }
}

void ZodiakWnd::view()
{
    int day, month, year;
    m_picker->getDate(day, month, year);
    if (day && month && year){
        int n = getSign(day, month);
        string s;
        s = i18n("http://horoscopes.swirve.com/scope.cgi?Sign=%1").arg(signes[n]).latin1();
        Event e(EventGoURL, (void*)(s.c_str()));
        e.process();
    }
}

static int bound[] =
    {
        20, 21, 21, 22, 23, 24, 23, 23, 23, 22, 20, 19
    };

int ZodiakWnd::getSign(int day, int month)
{
    month -= 3;
    if (month < 0)
        month += 12;
    if (day >= bound[month])
        return month;
    month--;
    if (month < 0)
        month += 12;
    return month;
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
#include "zodiak.moc"
#endif


