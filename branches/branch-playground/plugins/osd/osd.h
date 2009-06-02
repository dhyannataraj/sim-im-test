/***************************************************************************
                          osd.h  -  description
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

#ifndef _OSD_H
#define _OSD_H

#include <simapi.h>
#include <qfont.h>
#include <qpixmap.h>
#include <qwidget.h>
#include <qthread.h>
#include <QTimer>

#include "cfg.h"
#include "event.h"
#include "plugins.h"

#ifdef WIN32
	#include <windows.h>
#else  // assume POSIX
	#include <unistd.h>
#endif

inline void sleepTime(int i) {
#ifdef WIN32
  Sleep(i);
#else
  sleep(i);
#endif
}

class QPushButton;

struct OSDUserData
{
    SIM::Data	EnableMessage;
    SIM::Data	EnableMessageShowContent;
    SIM::Data	EnableCapsLockFlash;
    SIM::Data	ContentLines;
    SIM::Data	EnableAlert;
    SIM::Data	EnableAlertOnline;
    SIM::Data	EnableAlertAway;
    SIM::Data	EnableAlertNA;
    SIM::Data	EnableAlertDND;
    SIM::Data	EnableAlertOccupied;
    SIM::Data	EnableAlertFFC;
    SIM::Data	EnableAlertOffline;
    SIM::Data	EnableTyping;
    SIM::Data	Position;
    SIM::Data	Offset;
    SIM::Data	Color;
    SIM::Data	Font;
    SIM::Data	Timeout;
    SIM::Data	Shadow;
    SIM::Data	Fading;
    SIM::Data	Background;
    SIM::Data	BgColor;
    SIM::Data	Screen;
};

enum OSDType
{
    OSD_NONE,
    OSD_ALERTONLINE,
    OSD_ALERTAWAY,
    OSD_ALERTNA,
    OSD_ALERTDND,
    OSD_ALERTOCCUPIED,
    OSD_ALERTFFC,
    OSD_ALERTOFFLINE,
    OSD_TYPING,
    OSD_MESSAGE
};

struct OSDRequest
{
    unsigned long	contact;
    OSDType		type;
};

class QTimer;
class QPushButton;
class CorePlugin;
class OSDPlugin;

class OSDWidget : public QWidget
{
    Q_OBJECT
public:
    OSDWidget(OSDPlugin* plugin);
    void showOSD(const QString &text, OSDUserData *data);

    virtual QSize sizeHint () const;

signals:
    void dblClick();
    void closeClick();
protected slots:
    void slotCloseClick();
    void slotTimerFadeInTimeout();
public slots:
    void hide();

protected:
    bool isScreenSaverActive();
    virtual void paintEvent(QPaintEvent*);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void mousePressEvent(QMouseEvent *event);
    QRect recalcGeometry();
    void draw(QPainter &p);

    QFont	baseFont;
    QPixmap bgPict;
    OSDPlugin	*m_plugin;
    QTimer      m_transTimer;
    int transCounter;
    int transCounterDelta;
    bool m_bBackground;
    bool m_bShadow;
    int m_text_y;
    QString m_sText;
    bool m_bFading;
    QRect m_Rect;
    OSDUserData currentData;
    QImage m_imageButton;
    QImage m_image;
    QRect m_rectButton;
};

class OSDPlugin : public QThread, public SIM::Plugin, public SIM::EventReceiver
{
    Q_OBJECT
public:
    OSDPlugin(unsigned);
    QFont getBaseFont(QFont font);
    virtual ~OSDPlugin();
    unsigned long user_data_id;
protected slots:
    void timeout();
    void dblClick();
    void closeClick();
protected:
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual bool processEvent(SIM::Event *e);
    virtual void run();
    void processQueue();
    void flashCapsLockLED(bool);
    OSDRequest			m_request;
    std::list<OSDRequest>	queue;
    std::list<unsigned>		typing;
    CorePlugin	*core;
    OSDWidget		*m_osd;
    QTimer		*m_timer;
    bool bCapsState;
    bool bHaveUnreadMessages; // Should use this flag in OSDPlugin::run instead of core->unread.size()
                              // see pacth #2304 for more info.
};

#endif

