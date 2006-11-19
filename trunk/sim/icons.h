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
#include "event.h"

#include <qiconset.h>
#include <qimage.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qptrlist.h>
#include <qvaluelist.h>

namespace SIM
{

struct PictDef
{
    QImage      image;
    QString     file;
#ifdef USE_KDE
    QString     system;
#endif
    unsigned    flags;
};

typedef QMap<QString, PictDef> PIXMAP_MAP;

struct smileDef
{
    QString smile;
    QString name;
};

class IconSet
{
public:
    IconSet();
    virtual ~IconSet();
    virtual PictDef *getPict(const QString &name) = 0;
    virtual void clear() = 0;
    void parseSmiles(const QString&, unsigned &start, unsigned &size, QString &name);
    QStringList getSmile(const QString &name);
    QString getSmileName(const QString &name);
    void getSmiles(QStringList &smiles, QStringList &used);
protected:
    PIXMAP_MAP      m_icons;
    QValueList<smileDef>    m_smiles;
};

class EXPORT Icons : public QObject, public EventReceiver
{
    Q_OBJECT
public:
    Icons();
    ~Icons();
    PictDef *getPict(const QString &name);
    QString parseSmiles(const QString&);
    QStringList getSmile(const QString &ame);
    void getSmiles(QStringList &smiles);
    QString getSmileName(const QString &name);
    static unsigned nSmile;
    IconSet *addIconSet(const QString &name, bool bDefault);
    void removeIconSet(IconSet*);
    QValueList<IconSet*> m_customSets;
protected slots:
    void iconChanged(int);
protected:
    void *processEvent(Event*);
    QValueList<IconSet*>    m_defSets;
};

EXPORT Icons *getIcons();

};

#endif

