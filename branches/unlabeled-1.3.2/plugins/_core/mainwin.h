/***************************************************************************
                          mainwin.h  -  description
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

#ifndef _MAINWIN_H
#define _MAINWIN_H

#include "simapi.h"
#include <qmainwindow.h>

#include <list>
using namespace std;

class QVBoxLayout;
class QHBoxLayout;
class QToolBat;
class CorePlugin;
class QSizeGrip;

class MainWindow : public QMainWindow, public EventReceiver
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();
	bool m_bNoResize;
protected slots:
    void setGrip();
protected:
    QWidget		*main;
    QToolBar	*bar;
    QVBoxLayout	*lay;
    QHBoxLayout	*h_lay;
    QSizeGrip	*m_grip;
    void *processEvent(Event*);
    void focusInEvent(QFocusEvent*);
    void setTitle();
    void closeEvent(QCloseEvent *e);
	void resizeEvent(QResizeEvent *e);
    bool eventFilter(QObject *o, QEvent *e);
    void quit();
    void addWidget(QWidget*, bool bDown);
    void addStatus(QWidget *w, bool);
    list<QWidget*> statusWidgets;
    friend class CorePlugin;
#ifdef WIN32
    QPoint p;
    QSize s;
#endif
};

#endif

