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

typedef struct OSDUserData
{
    Data	EnableMessage;
    Data    EnableMessageShowContent;
    Data	ContentLines;
    Data	EnableAlert;
    Data	EnableTyping;
    Data	Position;
    Data	Offset;
    Data	Color;
    Data	Font;
    Data	Timeout;
    Data	Shadow;
    Data	Background;
    Data	BgColor;
    Data	Screen;
} OSDUserData;

enum OSDType
{
    OSD_NONE,
    OSD_ALERT,
    OSD_TYPING,
    OSD_MESSAGE
};

typedef struct OSDRequest
{
    unsigned long	contact;
    OSDType			type;
} OSDRequest;

class QTimer;
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

class OSDPlugin : public QObject, public Plugin, public EventReceiver
{
    Q_OBJECT
public:
    OSDPlugin(unsigned);
    virtual ~OSDPlugin();
    unsigned user_data_id;
protected slots:
    void timeout();
    void dblClick();
protected:
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual void *processEvent(Event*);
    void processQueue();
    OSDRequest			m_request;
    list<OSDRequest>	queue;
    list<unsigned>		typing;
    CorePlugin	*core;
    QWidget		*m_osd;
    QTimer		*m_timer;
};

#endif

