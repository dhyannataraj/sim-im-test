/***************************************************************************
                          monitor.h  -  description
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

#ifndef _MONITOR_H
#define _MONITOR_H

#include "defs.h"
#include <qmainwindow.h>

class QMultiLineEdit;
class QPopupMenu;

class MonitorWindow : public QMainWindow
{
    Q_OBJECT
public:
    MonitorWindow();
    ~MonitorWindow();
    QMultiLineEdit *edit;
    void add(const char *s);
    int logLevel;
signals:
    void finished();
protected slots:
    void save();
    void exit();
    void copy();
    void erase();
    void packets();
    void debug();
    void warning();
    void error();
    void adjustFile();
    void adjustEdit();
    void adjustLog();
protected:
    QPopupMenu *menuFile;
    QPopupMenu *menuEdit;
    QPopupMenu *menuLog;
};

#endif

