/***************************************************************************
                          transparent.cpp  -  description
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

#include <QApplication>
#include <QWidgetlist>
#include <qtimer.h>
#include <qcursor.h>
#include <qpainter.h>

#include "log.h"
#include "misc.h"

#include "transparent.h"
#include "mainwin.h"
#include "transparentcfg.h"
#include "../floaty/floatywnd.h" //Handle Floatings

#ifndef WIN32
#include "transtop.h"
#else
// GetTickCount() - TODO: replace with QTime
#include <windows.h>
#endif
#define SHOW_TIMEOUT	300
#define HIDE_TIMEOUT	1000

using namespace SIM;

Plugin *createTransparentPlugin(unsigned base, bool, Buffer *config)
{
    return new TransparentPlugin(base, config);
}

static PluginInfo info =
    {
        I18N_NOOP("Transparent"),
#ifdef WIN32
        I18N_NOOP("Plugin provides windows transparency\n"
                  "This plugin works only on Windows 2000 or Windows XP")
#else
        I18N_NOOP("Plugin provides windows transparency")
#endif
        ,
        VERSION,
        createTransparentPlugin,
#ifdef WIN32
        PLUGIN_DEFAULT
#else
        PLUGIN_NOLOAD_DEFAULT
#endif
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static DataDef transparentData[] =
    {
        { "Transparency", DATA_ULONG, 1, DATA(60) },
#ifdef WIN32
        { "IfInactive",   DATA_BOOL, 1, DATA(1) },
        { "IfMainWindow", DATA_BOOL, 1, DATA(1) },
        { "IfFloatings",  DATA_BOOL, 1, DATA(1) },
#endif
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

TransparentPlugin::TransparentPlugin(unsigned base, Buffer *config)
        : Plugin(base)
#ifndef WIN32
        , EventReceiver(HighPriority)
#endif
{
    load_data(transparentData, &data, config);
    if (getTransparency() >100)
        setTransparency(100);
#ifdef WIN32
    timer = NULL;
    m_bHaveMouse = false;
    m_bActive    = false;
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(tickMouse()));
    timer->start(1000);
#else
    top = NULL;
#endif
    setState();
}

void TransparentPlugin::topDestroyed()
{
#ifndef WIN32
    top = NULL;
#endif
}

TransparentPlugin::~TransparentPlugin()
{
#ifdef WIN32
    delete timer;

    // reset opacity for all toplevel widgets
    QWidgetList list = QApplication::topLevelWidgets();
    foreach(QWidget *w,list) {
        w->setWindowOpacity(1.0);
    }

#else
    delete top;
#endif
    free_data(transparentData, &data);
}

Q3CString TransparentPlugin::getConfig()
{
    return save_data(transparentData, &data);
}

QWidget *TransparentPlugin::getMainWindow()
{
    QWidgetList list = QApplication::topLevelWidgets();
    foreach(QWidget *w,list) {
         QWidget *mw = qobject_cast<MainWindow*>(w);
         if (mw)
             return mw;
    }
    return NULL;
}

QWidget *TransparentPlugin::createConfigWindow(QWidget *parent)
{
    return new TransparentCfg(parent, this);
}

void TransparentPlugin::tickMouse()
{
#ifdef WIN32
    QPoint p = QCursor::pos();
    bool bMouse = false;
    QWidget *main = getMainWindow();
    if (main && main->isVisible()){
        if (main->frameGeometry().contains(p))
            bMouse = true;
    }
		
    //Handle Floatings//
    QWidgetList list = QApplication::topLevelWidgets();
    foreach (QWidget *w,list) {
        if (FloatyWnd *flt = dynamic_cast<FloatyWnd *>(w))
            bMouse= flt->frameGeometry().contains(p) ? true : false;
    }
    //Handle Floatings//

    if (bMouse != m_bHaveMouse){
        m_bHaveMouse = bMouse;
        setState();
    }
#endif
}

bool TransparentPlugin::eventFilter(QObject *o, QEvent *e)
{
#ifdef WIN32
    if (getIfInactive()){
        switch (e->type()){
        case QEvent::WindowActivate:
            m_bActive = true;
            setState();
            break;
        case QEvent::WindowDeactivate:
            m_bActive = false;
            setState();
            break;
        case QEvent::Show:{
                QWidget *main = getMainWindow();
                if (main){
                    setState();
                }
                break;
            }
        default:
            break;
        }
    }
#endif
    return QObject::eventFilter(o, e);
}

void TransparentPlugin::setState()
{
    QWidget *main = getMainWindow();
    QWidgetList list = QApplication::topLevelWidgets();
        
    if (main == NULL)
        return;
#ifdef WIN32
    if (timer == NULL){
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(tick()));
        main->installEventFilter(this);
	main->setMouseTracking(true);
        m_bActive = main->isActiveWindow();
        m_bState  = !m_bActive;
    }
    bool bNewState = m_bActive || m_bHaveMouse;
    if (bNewState == m_bState){
        qreal transparency = (100 - getTransparency()) / 100.;
        if(getIfMainWindow())
            main->setWindowOpacity(transparency);
        else
            main->setWindowOpacity(1.0);

        QWidgetList list = QApplication::topLevelWidgets();
        foreach (QWidget *w,list) {
            if (FloatyWnd *refwnd = dynamic_cast<FloatyWnd *>(w)){
                refwnd->installEventFilter(this);
                if (getIfFloatings())
                    refwnd->setWindowOpacity(transparency);
                else
                    refwnd->setWindowOpacity(1.0);
            }
        }
        return;
    }
    m_bState = bNewState;
    startTime = GetTickCount();
    timer->start(10);
#else
    if (!top) {
        top = new TransparentTop(main, getTransparency());
        connect(top,SIGNAL(destroyed()),this,SLOT(topDestroyed()));
    }
    top->setTransparent(getTransparency());
#endif
}

void TransparentPlugin::tick()
{
#ifdef WIN32
    QWidget *main = getMainWindow();
    if (main == NULL){
        timer->stop();
        return;
    }
    qreal timeout = m_bActive ? SHOW_TIMEOUT : HIDE_TIMEOUT;
    qreal time = GetTickCount() - startTime;
    if (time >= timeout){
        time = timeout;
        timer->stop();
    }
    if (m_bState)
        time = timeout - time;

    qreal transparency = (100 - getTransparency()) / 100.;
    qreal diff_to_opaque = 1. - transparency;
    transparency = transparency + diff_to_opaque * (1 - time / timeout);

    //log(L_DEBUG, "transparency: %f, diff_to_opaque %f, time %d, timeout %d",
    //             transparency, diff_to_opaque, time, timeout);
    if (getIfMainWindow())
        main->setWindowOpacity(transparency);
    else
        main->setWindowOpacity(1.0);

    //Handle Floatings
    QWidgetList list = QApplication::topLevelWidgets();
    foreach (QWidget *w,list) {
        if (FloatyWnd *refwnd = dynamic_cast<FloatyWnd *>(w)){
            refwnd->installEventFilter(this);
            if (getIfFloatings()) 
                refwnd->setWindowOpacity(transparency);
            else
                refwnd->setWindowOpacity(1.0);
        }
    }
    //Handle Floatings//
	
#endif
}

bool TransparentPlugin::processEvent(Event *e)
{
    if (e->type() == eEventInit) {
#ifndef WIN32
        top = NULL;
#endif
        setState();
    }
#ifndef WIN32
    if (e->type() == eEventPaintView){
        if (top == NULL)
            return false;
        EventPaintView *ev = static_cast<EventPaintView*>(e);
        EventPaintView::PaintView *pv = ev->paintView();
        QPixmap pict = top->background(pv->win->colorGroup().background());
        if (!pict.isNull()){
            QPoint p = pv->pos;
            p = pv->win->mapToGlobal(p);
            p = pv->win->topLevelWidget()->mapFromGlobal(p);
            pv->p->drawPixmap(p, pict);
            pv->isStatic = true;
        }
    }
    if (e->type() == eEventRaiseWindow){
        EventRaiseWindow *w = static_cast<EventRaiseWindow*>(e);
        if (w->widget() == getMainWindow())
            setState();
    }
#endif
    return false;
}

