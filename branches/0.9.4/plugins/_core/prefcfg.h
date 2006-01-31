/***************************************************************************
                          prefcfg.h  -  description
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

#ifndef _PREFCFG_H
#define _PREFCFG_H

#include "simapi.h"
#include "prefcfgbase.h"

class PrefConfig : public PrefConfigBase
{
    Q_OBJECT
public:
    PrefConfig(QWidget *parent, CommandDef *cmd, Contact *contact, Group *group);
signals:
    void apply(void*);
public slots:
    void apply();
protected slots:
    void overrideToggled(bool);
protected:
    CommandDef *m_cmd;
    Contact *m_contact;
    Group   *m_group;
};

#endif

