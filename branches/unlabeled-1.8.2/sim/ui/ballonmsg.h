/***************************************************************************
                          ballonmsg.h  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#ifndef _BALLONMSG_H
#define _BALLONMSG_H

#include "simapi.h"
#include <qstring.h>
#include <qdialog.h>
#include <qbitmap.h>
#include <qpushbutton.h>

class QStringList;

class UI_EXPORT BalloonMsg : public QDialog
{
    Q_OBJECT
public:
    BalloonMsg(void *param, const QString &text, QStringList&, QWidget *p, const QRect *rc = NULL, bool bModal=false, bool bAutoHide=true, unsigned width=150);
    ~BalloonMsg();
    static void message(const QString &text, QWidget *parent, bool bModal=false, unsigned width=150);
    static void ask(void *param, const QString &text, QWidget *parent, const char *slotYes, const char *slotNo, const QRect *rc=NULL, QObject *receiver=NULL);
signals:
    void action(int, void*);
    void yes_action(void*);
    void no_action(void*);
    void destroyed();
protected slots:
    void action(int);
protected:
    bool eventFilter(QObject*, QEvent*);
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent*);
    QString text;
    QRect textRect;
    QBitmap mask;
    QWidget *m_parent;
    bool m_bAutoHide;
    bool m_bYes;
	unsigned m_width;
    void *m_param;
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

