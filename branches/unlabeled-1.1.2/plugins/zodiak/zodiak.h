/***************************************************************************
                          zodiak.h  -  description
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

#ifndef _UPDATE_H
#define _UPDATE_H

#include "simapi.h"
#include "stl.h"

#include <qframe.h>

class DatePicker;
class QLabel;
class QPushButton;

class ZodiakWnd : public QFrame
{
    Q_OBJECT
public:
    ZodiakWnd(DatePicker *parent);
protected slots:
    void changed();
    void view();
protected:
    QLabel		*m_picture;
    QLabel		*m_name;
    QPushButton	*m_button;
    DatePicker  *m_picker;
	void setEnabled(bool);
    void paintEvent(QPaintEvent *e);
    int	getSign(int day, int month);
};

typedef struct Picker
{
    DatePicker	*picker;
    ZodiakWnd	*label;
} Picker;

class ZodiakPlugin : public QObject, public Plugin, public EventReceiver
{
    Q_OBJECT
public:
    ZodiakPlugin(unsigned);
    virtual ~ZodiakPlugin();
protected:
    list<Picker>	m_pickers;
    bool eventFilter(QObject*, QEvent*);
    void createLabel(DatePicker *picker);
    void *processEvent(Event*);
};

#endif

