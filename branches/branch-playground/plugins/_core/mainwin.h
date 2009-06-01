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
#include "event.h"

#include "cfg.h"

#include <QMainWindow>

class QToolBat;
class CorePlugin;
class QCloseEvent;
class QEvent;
class QFocusEvent;
class QHBoxLayout;
class QResizeEvent;
class QSizeGrip;
class QVBoxLayout;

// Lets do this later whole in one, because at the moment it only breaks linking of all other plugins...
/*
#ifdef MAKE__CORE_LIB
# define _CORE_EXPORTS Q_DECL_EXPORT
#else
# define _CORE_EXPORTS Q_DECL_IMPORT
#endif

class _CORE_EXPORTS MainWindow : public QMainWindow, public SIM::EventReceiver
*/
class MainWindow : public QMainWindow, public SIM::EventReceiver
{
    Q_OBJECT
public:
    MainWindow(SIM::Geometry&);
    ~MainWindow();
    bool m_bNoResize;
    void closeEvent(QCloseEvent *e);
protected slots:
    void setGrip();
protected:
    QWidget *main;
    CToolBar *m_bar;
    QVBoxLayout *lay;
    QHBoxLayout *h_lay;
    QSizeGrip *m_grip;
    void focusInEvent(QFocusEvent*);
    virtual bool processEvent(SIM::Event*);
    void setTitle();
    void resizeEvent(QResizeEvent *e);
    bool eventFilter(QObject *o, QEvent *e);
    void quit();
    void addWidget(QWidget*, bool bDown);
    void addStatus(QWidget *w, bool);
    std::list<QWidget*> statusWidgets;
    QString	m_icon;
    friend class CorePlugin;
#ifdef WIN32
    QPoint p;
    QSize s;
#endif
};

#endif

