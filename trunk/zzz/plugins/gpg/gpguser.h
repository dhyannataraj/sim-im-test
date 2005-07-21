/***************************************************************************
                          gpguser.h  -  description
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

#ifndef _GPGUSER_H
#define _GPGUSER_H

#include "simapi.h"

#include "gpguserbase.h"

class GpgPlugin;
class Exec;

class GpgUser : public GpgUserBase
{
    Q_OBJECT
public:
    GpgUser(QWidget *parent, struct GpgUserData *data);
    ~GpgUser();
public slots:
    void apply(void *data);
    void refresh();
    void publicReady(Exec*, int, const char*);
    void clearExec();
protected:
    void fillPublic(Buffer *b);
    Exec		*m_exec;
    string		m_key;
};

#endif

