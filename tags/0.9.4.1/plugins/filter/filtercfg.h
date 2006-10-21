/***************************************************************************
                          filtercfg.h  -  description
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

#ifndef _FILTERCFG_H
#define _FILTERCFG_H

#include "simapi.h"
#include "filtercfgbase.h"

class FilterPlugin;
class IgnoreList;

class FilterConfig : public FilterConfigBase
{
    Q_OBJECT
public:
    FilterConfig(QWidget *parent, struct FilterUserData *data, FilterPlugin *plugin, bool bMain);
    ~FilterConfig();
public slots:
    void apply();
    void apply(void *data);
protected:
    FilterUserData			*m_data;
    FilterPlugin			*m_plugin;
    IgnoreList				*m_ignore;
};

#endif

