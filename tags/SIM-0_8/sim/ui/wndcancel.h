/***************************************************************************
                          wndcancel.h  -  description
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

#ifndef _WNDCANCEL_H
#define _WNDCANCEL_H

#include "defs.h"
#include <qwidget.h>

class WndCancel : public QObject
{
    Q_OBJECT
public:
    WndCancel(QWidget *w);
protected:
    virtual bool eventFilter(QObject *watched, QEvent *e);
    QWidget *w;
};

#endif

