/***************************************************************************
                          nonim.h  -  description
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

#ifndef _NONIM_H
#define _NONIM_H

#include "simapi.h"
#include "nonimbase.h"

class NonIM : public NonIMBase
{
    Q_OBJECT
public:
    NonIM(QWidget *parent);
signals:
    void setAdd(bool);
	void showError(const QString&);
protected slots:
	void add(unsigned grp_id);
protected:
    void showEvent(QShowEvent*);
};

#endif

