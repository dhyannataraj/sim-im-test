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

#include <map>
using namespace std;

typedef struct IconsData
{
    void	*IconDLLs;
    char	*Smiles;
} IconsData;

class IconDLL;
class Smiles;

class my_string : public string
{
public:
    my_string(const char *str) : string(str) {}
    bool operator < (const my_string &str) const;
};

typedef map<my_string, IconDLL*> ICONS_MAP;

class IconsPlugin : public Plugin, public EventReceiver
{
public:
    IconsPlugin(unsigned, const char*);
    virtual ~IconsPlugin();
    PROP_STRLIST(IconDLLs);
    PROP_UTF8(Smiles);
    Smiles *smiles;
protected:
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual string getConfig();
    void *processEvent(Event*);
    void setIcons();
    ICONS_MAP dlls;
    IconsData data;
    friend class IconCfg;
};

#endif

