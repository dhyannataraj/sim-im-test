/***************************************************************************
                          icon.h  -  description
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

#ifndef _ICON_H
#define _ICON_H

#include "simapi.h"

#include <list>
using namespace std;

typedef struct IconsData
{
    void *IconDLLs;
} IconsData;

class IconDLL;

class IconsPlugin : public Plugin
{
public:
    IconsPlugin(unsigned, const char*);
    virtual ~IconsPlugin();
    PROP_STRLIST(IconDLLs);
protected:
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual string getConfig();
    void setIcons();
    list<IconDLL*> dlls;
    IconsData data;
    friend class IconCfg;
};

#endif

