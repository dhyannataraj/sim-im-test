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

#include "sim_export.h"
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

class IconSet;

struct PictDef
{
    QImage      image;
    QString     file;
#ifdef USE_KDE
    QString     system;
#endif
    unsigned    flags;
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
protected slots:
    void iconChanged(int);
protected:
    void *processEvent(Event*);
    class IconsPrivate *d;
    COPY_RESTRICTED(Icons);
};

EXPORT Icons *getIcons();

EXPORT QIconSet Icon(const QString &name);
EXPORT QPixmap Pict(const QString &name, const QColor &bgColor = QColor());
EXPORT QImage  Image(const QString &name);

};

#endif

