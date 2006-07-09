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
#include <qimage.h>

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable: 4251)
#endif
#endif

class QMimeSourceFactory;

namespace SIM
{

typedef struct PictDef
{
    QImage			image;
    std::string		file;
#ifdef USE_KDE
    std::string		system;
#endif
    unsigned		flags;
} PictDef;

typedef std::map<my_string, PictDef> PIXMAP_MAP;

typedef struct smileDef
{
    std::string	smile;
    std::string	name;
} smileDef;

class IconSet
{
public:
    IconSet();
    virtual ~IconSet();
    virtual PictDef *getPict(const char *name) = 0;
    virtual void clear() = 0;
    void parseSmiles(const QString&, unsigned &start, unsigned &size, std::string &name);
    std::list<std::string> getSmile(const char *name);
    std::string getSmileName(const char *name);
    void getSmiles(std::list<std::string> &smiles, std::list<std::string> &used);
protected:
    PIXMAP_MAP		m_icons;
    std::list<smileDef>	m_smiles;
};

class EXPORT Icons : public QObject, public EventReceiver
{
    Q_OBJECT
public:
    Icons();
    ~Icons();
    PictDef *getPict(const char *name);
    QString parseSmiles(const QString&);
    std::list<std::string> getSmile(const char *name);
    void getSmiles(std::list<std::string> &smiles);
    std::string getSmileName(const char *name);
    static unsigned nSmile;
    IconSet *addIconSet(const char *name, bool bDefault);
    void removeIconSet(IconSet*);
    std::list<IconSet*> m_customSets;
protected slots:
    void iconChanged(int);
protected:
    void *processEvent(Event*);
    std::list<IconSet*>	m_defSets;
};

EXPORT Icons *getIcons();

};

#endif

