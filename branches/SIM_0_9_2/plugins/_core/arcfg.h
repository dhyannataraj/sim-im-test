/***************************************************************************
                          arcfg.h  -  description
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

#ifndef _ARCFG_H
#define _ARCFG_H

#include "simapi.h"
#include "arcfgbase.h"

class ARConfig : public ARConfigBase
{
    Q_OBJECT
public:
    ARConfig(QWidget *parent, unsigned status, const QString &name, Contact *contact);
public slots:
    void apply();
    void toggled(bool);
	void help();
protected:
    unsigned m_status;
    Contact	 *m_contact;
};

#endif

