/***************************************************************************
                          dock.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DOCK_H
#define _DOCK_H

#include "defs.h"

#include <list>

using namespace std;

#include <qwidget.h>
#include <qtooltip.h>
#include <qpixmap.h>

class KPopupMenu;
class ICQEvent;
class ICQMessage;

class WharfIcon;

class DockWnd : public QWidget
{
    Q_OBJECT
public:
    DockWnd(QWidget *parent);
    ~DockWnd();
#ifdef WIN32
    void callProc(unsigned long);
#endif
    void mouseEvent( QMouseEvent *e);
    virtual void mouseDoubleClickEvent( QMouseEvent *e);
signals:
    void showPopup(QPoint);
    void toggleWin();
    void doubleClicked();
public slots:
    void reset();
protected slots:
    void processEvent(ICQEvent*);
    void timer();
    void toggle();
protected:
    enum ShowIcon
    {
        State,
        Blinked,
        Message,
        Unknown
    };
    ShowIcon showIcon;
    unsigned m_state;
    bool    bNoToggle;
    QPixmap drawIcon;
    virtual void enterEvent( QEvent *e);
    virtual void paintEvent( QPaintEvent *e);
    virtual void mousePressEvent( QMouseEvent *e);
    virtual void mouseReleaseEvent( QMouseEvent *e);
    virtual void mouseMoveEvent( QMouseEvent *e);
    void setIcon(const QPixmap&);
    void setTip(const QString&);
#ifndef WIN32
    QPoint mousePos;
    WharfIcon *wharfIcon;
    bool x11Event(XEvent*);
    bool inTray;
    bool inNetTray;
    bool bInit;
#endif
};

#endif

