/***************************************************************************
                          infoproxy.h  -  description
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

#ifndef _INFOPROXY_H
#define _INFOPROXY_H

#include "simapi.h"
#include "infoproxybase.h"

class InfoProxy : public InfoProxyBase, public EventReceiver
{
    Q_OBJECT
public:
    InfoProxy(QWidget *parent, QWidget *child, const QString &title);
signals:
    void sig_apply();
    void sig_apply(Client*, void*);
public slots:
    void apply();
    void apply(Client*, void*);
};

#endif

