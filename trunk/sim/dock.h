/***************************************************************************
                          dock.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
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

class unread_msg
{
public:
    unread_msg(ICQMessage *msg);
    unsigned short type() { return m_type; }
    unsigned long uin() { return m_uin; }
    bool operator == (const unread_msg &msg) const { return (m_uin == msg.m_uin) && (m_id == msg.m_id); }
protected:
    unsigned long  m_uin;
    unsigned long  m_id;
    unsigned short m_type;
};

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
signals:
    void showPopup(QPoint);
    void toggleWin();
    void doubleClicked();
protected slots:
    void processEvent(ICQEvent*);
    void messageRead(ICQMessage*);
    void messageReceived(ICQMessage*);
    void timer();
    void toggle();
    void reset();
protected:
    list<unread_msg> messages;
    enum ShowIcon
    {
        State,
        Blinked,
        Message,
        Unknown
    };
    ShowIcon showIcon;
    unsigned m_state;
    bool   needToggle;
    QPixmap drawIcon;
    virtual void paintEvent( QPaintEvent *e);
    virtual void mousePressEvent( QMouseEvent *e);
    virtual void mouseDoubleClickEvent( QMouseEvent *e);
    void setIcon(const QPixmap&);
    void setTip(const QString&);
    void loadUnread();
#if !defined(WIN32) 
    WharfIcon *wharfIcon;
#endif
    friend class WharfIcon;
};

#endif

