/***************************************************************************
                          floatywnd.h  -  description
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

#ifndef _FLOATYWND_H
#define _FLOATYWND_H

#include "simapi.h"

#include <qwidget.h>

class FloatyPlugin;
class QTimer;
class TipLabel;
class QPainter;

class FloatyWnd : public QWidget
{
    Q_OBJECT
public:
    FloatyWnd(FloatyPlugin*, unsigned id);
    ~FloatyWnd();
    unsigned id() { return m_id; }
    void init();
    void startBlink();
protected slots:
    void showTip();
    void hideTip();
    void tipDestroyed();
    void startMove();
    void blink();
protected:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseDoubleClickEvent(QMouseEvent*);
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);
    void dragEnterEvent(QDragEnterEvent*);
    void dropEvent(QDropEvent*);
    void dragEvent(QDropEvent *e, bool isDrop);
    void setFont(QPainter *p);
    QPoint   mousePos;
    QPoint	 initMousePos;
    QString  m_text;
    string   m_icons;
    const char *m_statusIcon;
    unsigned m_id;
    unsigned m_style;
    unsigned m_unread;
    unsigned m_blink;
    unsigned long m_status;
    TipLabel *m_tip;
    QTimer	 *blinkTimer;
    QTimer	 *tipTimer;
    QTimer	 *moveTimer;
    FloatyPlugin *m_plugin;
};

#endif

