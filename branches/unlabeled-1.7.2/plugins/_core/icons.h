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
#include "stl.h"

#include <qiconset.h>

using namespace std;

typedef struct PictDef
{
    QIconSet	iconSet;
	const char	**xpm;
	const char	*system;
	unsigned	flags;
} PictDef;

typedef map<my_string, PictDef> PIXMAP_MAP;

class QMimeSourceFactory;

class Icons : public QObject, public EventReceiver
{
    Q_OBJECT
public:
    Icons();
    ~Icons();
protected slots:
    void iconChanged(int);
protected:
    PIXMAP_MAP icons;
    PIXMAP_MAP bigIcons;
    void *processEvent(Event*);
    PictDef *addIcon(const char *name, const char **xpm, const char *system, unsigned flags);
    void remove(const char *name);
    void fill(list<string> *names);
    const QIconSet *getIcon(const char *name);
    const QIconSet *getBigIcon(const char *name);
	QMimeSourceFactory	*my_factory;
};

#endif

