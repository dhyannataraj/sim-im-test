/***************************************************************************
                          xosd.h  -  description
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

#ifndef _XOSD_H
#define _XOSD_H

#include "defs.h"

#include <qwidget.h>
#include <qpixmap.h>
#include <qstringlist.h>

class XOSD : public QWidget
{
    Q_OBJECT
public:
    XOSD(QWidget *parent);
public slots:
    void init();
    void setMessage(const QString&);
protected slots:
    void set(const QString&);
    void timeout();
protected:
    void paintEvent(QPaintEvent*);
    QPixmap bgPict;
    QStringList msg;
};

#endif

