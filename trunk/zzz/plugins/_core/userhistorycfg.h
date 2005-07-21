/***************************************************************************
                          userhistorycfg.h  -  description
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

#ifndef _USERHISTORYCFG_H
#define _USERHISTORYCFG_H

#include "simapi.h"

#include "userhistorycfgbase.h"

class UserHistoryCfg : public UserHistoryCfgBase
{
    Q_OBJECT
public:
    UserHistoryCfg(QWidget *parent, void *data);
    ~UserHistoryCfg();
public slots:
    void apply(void *data);
    void toggledDays(bool);
    void toggledSize(bool);
protected:
    void *m_data;
};

#endif

