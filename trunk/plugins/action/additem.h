/***************************************************************************
                          additem.h  -  description
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

#ifndef _ADDITEM_H
#define _ADDITEM_H

#include "additembase.h"
#include "simapi.h"

class AddItem : public AddItemBase
{
    Q_OBJECT
public:
    AddItem(QWidget *parent);
public slots:
    void changed();
    void changed(const QString&);
};

#endif

