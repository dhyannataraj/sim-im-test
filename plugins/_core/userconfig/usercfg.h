/***************************************************************************
                          usercfg.h  -  description
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

#ifndef _USERCFG_H
#define _USERCFG_H

#include <QResizeEvent>
#include <QDialog>
#include <QSharedPointer>

#include "widgethierarchy.h"

class CorePlugin;

class UserConfig;
typedef QSharedPointer<UserConfig> UserConfigPtr;
class UserConfig : public QDialog
{
    Q_OBJECT
public:
    UserConfig() {}
    virtual ~UserConfig() {}

    static UserConfigPtr create();

    virtual int exec() = 0;
    virtual void setWidgetHierarchy(SIM::WidgetHierarchy* hierarchy) = 0;

signals:
    void apply();

private:
};

#endif

