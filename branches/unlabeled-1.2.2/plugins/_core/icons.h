/***************************************************************************
                          icons.h  -  description
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

#ifndef _ICONS_H
#define _ICONS_H

#include "simapi.h"
#include "simapi.h"

#include <map>

#include <qiconset.h>

using namespace std;

class my_string : public string
{
public:
    my_string(const char *str) : string(str) {}
    bool operator < (const my_string &str) const;
};

typedef struct PictDef
{
    QIconSet 	 icon;
    bool	 bSystem;
} PictDef;

typedef map<my_string, PictDef> PIXMAP_MAP;

class Icons : public QObject, public EventReceiver
{
    Q_OBJECT
public:
    Icons();
protected slots:
    void iconChanged(int);
protected:
    void *processEvent(Event*);
    void addIcon(const char *name, const char **xpm, bool isSystem);
    void addBigIcon(const char *name, const char **xpm, bool isSystem);
    void remove(const char *name);
    const QIconSet *addIcon(const char *name, const QIconSet &icon, bool isSystem);
    const QIconSet *getIcon(const char *name);
    const QIconSet *addBigIcon(const char *name, const QIconSet &icon, bool isSystem);
    const QIconSet *getBigIcon(const char *name);
    PIXMAP_MAP icons;
    PIXMAP_MAP bigIcons;
};

#endif

