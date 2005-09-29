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

#include <QIcon>

#include <QPixmap>

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable: 4251)
#endif
#endif

using namespace std;

class Q3MimeSourceFactory;

namespace SIM
{

typedef struct PictDef
{
    QImage			*image;
#if defined(WIN32) && (COMPAT_QT_VERSION < 0x030000)
    QPixmap			*pixmap;
#endif
    string			file;
#ifdef USE_KDE
    string			system;
#endif
    unsigned		flags;
} PictDef;

typedef map<my_string, PictDef> PIXMAP_MAP;

typedef struct smileDef
{
    string	smile;
    string	name;
} smileDef;

class IconSet
{
public:
    IconSet();
    virtual ~IconSet();
    virtual PictDef *getPict(const char *name) = 0;
    virtual void clear() = 0;
    void parseSmiles(const QString&, unsigned &start, unsigned &size, string &name);
    list<string> getSmile(const char *name);
    string getSmileName(const char *name);
    void getSmiles(list<string> &smiles, list<string> &used);
protected:
    PIXMAP_MAP		m_icons;
    list<smileDef>	m_smiles;
};

#if defined(WIN32) && (COMPAT_QT_VERSION < 0x030000)

typedef map<unsigned, string>	ICONS_MAP;

#endif

class EXPORT Icons : public QObject, public EventReceiver
{
    Q_OBJECT
public:
    Icons();
    ~Icons();
    PictDef *getPict(const char *name);
    QString parseSmiles(const QString&);
    list<string> getSmile(const char *name);
    void getSmiles(list<string> &smiles);
    string getSmileName(const char *name);
    static unsigned nSmile;
    IconSet *addIconSet(const char *name, bool bDefault);
    void removeIconSet(IconSet*);
    list<IconSet*>	m_customSets;
#if defined(WIN32) && (COMPAT_QT_VERSION < 0x030000)
    PictDef			*getPict(const QPixmap &pict);
    ICONS_MAP		m_icons;
#endif
protected slots:
    void iconChanged(int);
protected:
    void *processEvent(Event*);
    list<IconSet*>	m_defSets;
};

EXPORT Icons *getIcons();

};

#endif

