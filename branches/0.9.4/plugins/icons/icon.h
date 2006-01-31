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

typedef struct IconsData
{
    Data	Icon;
    Data	NIcons;
    Data	Default;
} IconsData;

class IconsPlugin : public Plugin
{
public:
    IconsPlugin(unsigned, Buffer*);
    virtual ~IconsPlugin();
    PROP_STRLIST(Icon);
    PROP_ULONG(NIcons);
    PROP_BOOL(Default);
    void setIcons(bool bForce);
protected:
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual string getConfig();
    IconsData data;
};

#endif

