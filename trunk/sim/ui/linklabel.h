/***************************************************************************
                          linklabel.h  -  description
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

#ifndef _LINKLABEL_H
#define _LINKLABEL_H

#include "simapi.h"
#include <qlabel.h>

class UI_EXPORT LinkLabel : public QLabel
{
    Q_OBJECT
public:
    LinkLabel(QWidget *parent = NULL, const char *name = NULL);
signals:
    void click();
protected:
    virtual void mouseReleaseEvent(QMouseEvent * e);
};

class UI_EXPORT TipLabel : public QLabel
{
    Q_OBJECT
public:
    TipLabel(const QString &text);
    ~TipLabel();
    void show(const QRect &tipRect);
    void setText(const QString &text);
signals:
    void finished();
};

#endif

