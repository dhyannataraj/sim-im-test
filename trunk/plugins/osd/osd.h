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

#ifndef _WINDOCK_H
#define _WINDOCK_H

#include "simapi.h"
#include "stl.h"

#include <qpixmap.h>

struct OSDUserData
{
    SIM::Data	EnableMessage;
    SIM::Data    EnableMessageShowContent;
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
    OSDType			type;
};

class QTimer;
class QPushButton;
class CorePlugin;

class OSDWidget : public QWidget
{
    Q_OBJECT
public:
    OSDWidget();
    void showOSD(const QString &text, OSDUserData *data);
signals:
    void dblClick();
    void closeClick();
protected slots:
    void slotCloseClick();
protected:
    bool isScreenSaverActive();
    void paintEvent(QPaintEvent*);
    void mouseDoubleClickEvent(QMouseEvent *e);
    QFont	baseFont;
    QPixmap bgPict;
    QPushButton	*m_button;
};

class OSDPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
    Q_OBJECT
public:
    OSDPlugin(unsigned);
    virtual ~OSDPlugin();
    unsigned long user_data_id;
protected slots:
    void timeout();
    void dblClick();
    void closeClick();
protected:
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual void *processEvent(SIM::Event*);
    void processQueue();
    OSDRequest			m_request;
    std::list<OSDRequest>	queue;
    std::list<unsigned>		typing;
    CorePlugin	*core;
    QWidget		*m_osd;
    QTimer		*m_timer;
};

#endif

