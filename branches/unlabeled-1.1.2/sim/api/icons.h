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

class QMimeSourceFactory;

namespace SIM
{

typedef struct PictDef
{
    QPixmap		*icon;
    string		file;
#ifdef USE_KDE
    string		system;
#endif
    unsigned	flags;
} PictDef;

typedef map<my_string, PictDef> PIXMAP_MAP;

class IconSet
{
public:
	IconSet();
	virtual ~IconSet();
    virtual const QPixmap *getPict(const char *name, unsigned &flags) = 0;
	virtual void clear() = 0;
protected:
    PIXMAP_MAP m_icons;
};

class Icons : public QObject, public EventReceiver
{
    Q_OBJECT
public:
    Icons();
    ~Icons();
    const QPixmap *getPict(const char *name, unsigned &flags);
	static unsigned nSmile;
protected slots:
    void iconChanged(int);
protected:
    void *processEvent(Event*);
	list<IconSet*>	m_sets;
};

};

#endif

