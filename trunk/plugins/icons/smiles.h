/***************************************************************************
                          smiles.h  -  description
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

#ifndef _SMILES_H
#define _SMILES_H

#include "simapi.h"
#include "icon.h"

#include <qiconset.h>
#include <vector>
#include <list>
using namespace std;

typedef struct SmileDef
{
    string	exp;
    string	paste;
    string	title;
    const QIconSet *icon;
} SmileDef;

class Smiles
{
public:
    Smiles();
    ~Smiles();
    bool load(const QString &file);
    void clear();
    string name;
    ICONS_MAP icons;
    vector<SmileDef> m_smiles;
    list<QIconSet*> m_icons;
};

#endif

