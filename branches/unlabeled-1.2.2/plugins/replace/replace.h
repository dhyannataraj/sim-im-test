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

#include "simapi.h"

typedef struct ReplaceData
{
	Data	Keys;
	Data	Key;
	Data	Value;
} ReplaceData;

class ReplacePlugin : public QObject, public Plugin
{
	Q_OBJECT
public:
    ReplacePlugin(unsigned, const char *cfg);
    virtual ~ReplacePlugin();
	PROP_ULONG(Keys)
	PROP_UTFLIST(Key)
	PROP_UTFLIST(Value)
protected:
	virtual bool eventFilter(QObject *o, QEvent *e);
    virtual string getConfig();
    virtual QWidget *createConfigWindow(QWidget *parent);
	ReplaceData data;
};

#endif

