/***************************************************************************
                          keys.h  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#include "defs.h"
#include <qwidget.h>

#ifndef KEYS_H
#define KEYS_H	1

#ifdef USE_KDE
class KGlobalAccel;
#endif

class KeyGrab : public QWidget
{
    Q_OBJECT
public:
    KeyGrab(const char *key);
    ~KeyGrab();
signals:
    void activated();
protected:
#if !defined(WIN32) && !defined(USE_KDE)
    virtual bool x11Event(XEvent*);
#endif
    unsigned m_key;
    unsigned m_state;
};

class HotKeys : public QObject
{
    Q_OBJECT
public:
    HotKeys(QWidget *parent, const char *name = NULL);
    ~HotKeys();
    static const char *keyToString(int key);
    void regKeys();
    void unregKeys();
#ifdef WIN32
    void hotKey(int id);
#endif
signals:
    void toggleWindow();
    void dblClick();
    void showSearch();
protected slots:
    void slotToggleWindow();
    void slotDblClick();
    void slotShowSearch();
protected:
#ifdef WIN32
    QWidget *wnd;
    int keyWindow;
    int keyDblClick;
    int keySearch;
#else
#ifdef USE_KDE
KGlobalAccel *accel;
#else
KeyGrab *grabWindow;
    KeyGrab *grabDblClick;
    KeyGrab *grabSearch;
#endif
#endif
};

#endif
