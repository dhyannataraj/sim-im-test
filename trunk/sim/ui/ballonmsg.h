/***************************************************************************
                          ballonmsg.h  -  description
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

#ifndef _BALLONMSG_H
#define _BALLONMSG_H

#include "defs.h"
#include <qstring.h>
#include <qdialog.h>
#include <qbitmap.h>
#include <qpushbutton.h>

class QStringList;

class BalloonMsg : public QDialog
{
    Q_OBJECT
public:
    BalloonMsg(const QString &text, const QRect&, QStringList&, QWidget *p, bool bModal=false);
    static void message(const QString &text, QWidget *parent, bool bModal=false);
signals:
    void action(int);
protected:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent*);
    QString text;
    QRect textRect;
    QBitmap mask;
};

class BalloonButton : public QPushButton
{
    Q_OBJECT
public:
    BalloonButton(QString, QWidget*, int);
signals:
    void action(int);
protected slots:
    void click();
protected:
    int id;
};


#endif

