/***************************************************************************
                          control.h  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CONTROL_H
#define _CONTROL_H

#include "defs.h"
#include <qsocketdevice.h>
#include <string>
#include <list>
using namespace std;

class QSocketNotifier;
class ControlSocket;
class ICQEvent;

class ControlListener : public QObject
{
    Q_OBJECT
public:
    ControlListener(QObject *parent);
    ~ControlListener();
    bool bind(const char *addr);
protected slots:
    void activated(int);
    void finished(ControlSocket*);
    void finished();
protected:
    list<ControlSocket*> deleted;
    bool setOptions(int nPort);
    QSocketNotifier *n;
    int s;
};

class ControlSocket : public QObject
{
    Q_OBJECT
public:
    ControlSocket(int s, QObject *parent);
    ~ControlSocket();
signals:
    void finished(ControlSocket*);
protected slots:
    void read_ready(int);
    void processEvent(ICQEvent*);
    void reset();
protected:
    void ownerChanged();
    int  s;
    bool bEscape;
    bool bNotify;
    string read_line;
    QString status;
    QString msg;
    void write(const char*);
    ICQEvent *sendEvent;
};

#endif

