/***************************************************************************
                          filecfg.h  -  description
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

#ifndef _FILECFG_H
#define _FILECFG_H

#include "simapi.h"
#include "filecfgbase.h"

class QCheckBox;

class FileConfig : public FileConfigBase
{
    Q_OBJECT
public:
    FileConfig(QWidget *parent, void *data);
public slots:
    void apply(void*);
protected slots:
    void acceptClicked(int);
};

#endif

