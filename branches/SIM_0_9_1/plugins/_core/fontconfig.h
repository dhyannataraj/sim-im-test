/***************************************************************************
                          fontconfig.h  -  description
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

#ifndef _FONTCONFIG_H
#define _FONTCONFIG_H

#include "simapi.h"
#include "fontconfigbase.h"

class CorePlugin;

class FontConfig : public FontConfigBase
{
    Q_OBJECT
public:
    FontConfig(QWidget *parent);
    ~FontConfig();
public slots:
    void apply();
    void systemToggled(bool);
};

#endif

