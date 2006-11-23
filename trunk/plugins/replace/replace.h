/***************************************************************************
                          replace.h  -  description
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

#ifndef _REPLACE_H
#define _REPLACE_H

#include <qobject.h>

#include "plugins.h"

struct ReplaceData
{
    SIM::Data	Keys;
    SIM::Data	Key;
    SIM::Data	Value;
};

class ReplacePlugin : public QObject, public SIM::Plugin
{
    Q_OBJECT
public:
    ReplacePlugin(unsigned, Buffer *cfg);
    virtual ~ReplacePlugin();
    PROP_ULONG(Keys)
    PROP_UTFLIST(Key)
    PROP_UTFLIST(Value)
protected:
    virtual std::string getConfig();
    virtual QWidget *createConfigWindow(QWidget *parent);
    bool eventFilter(QObject *o, QEvent *e);
    ReplaceData data;
};

#endif

