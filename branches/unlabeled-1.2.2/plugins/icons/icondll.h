/***************************************************************************
                          icondll.h  -  description
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

#ifndef _ICONDLL_H
#define _ICONDLL_H

#include "simapi.h"
#include <qiconset.h>
#include <map>
using namespace std;

class IconsMap : public map<unsigned, QIconSet>
{
public:
    IconsMap() {};
    const QIconSet *get(unsigned id);
};

class IconDLLBase
{
public:
    IconDLLBase();
    ~IconDLLBase();
    bool load(const char *file);
    string name;
    IconsMap  *icon_map;
};

class IconDLL : public IconDLLBase, public EventReceiver
{
public:
    IconDLL(const char *prefix);
protected:
    virtual void *processEvent(Event*);
    string m_prefix;
};

#endif

