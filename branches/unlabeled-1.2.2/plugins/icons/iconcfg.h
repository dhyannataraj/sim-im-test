/***************************************************************************
                          iconcfg.h  -  description
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

#ifndef _ICONCFG_H
#define _ICONCFG_H

#include "simapi.h"
#include "iconcfgbase.h"

#include <list>
using namespace std;

class IconPreview;

typedef struct IconsDef
{
    string	protocol;
    string	icon;
    int		index;
} IconsDef;

class IconsPlugin;
class SmileCfg;

class IconCfg : public IconCfgBase
{
    Q_OBJECT
public:
    IconCfg(QWidget *parent, IconsPlugin *plugin);
public slots:
    void apply();
    void goIcons();
    void protocolChanged(int);
    void textChanged(const QString &text);
protected:
    list<IconsDef> defs;
    IconsPlugin *m_plugin;
	SmileCfg	*m_smiles;
};

#endif

