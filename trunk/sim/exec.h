/***************************************************************************
                          exec.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _EXEC_H
#define _EXEC_H

#include "defs.h"
#include "buffer.h"
#include <qobject.h>

class Exec : public QObject
{
    Q_OBJECT
public:
    Exec(const char *prog, const char *input);
    ~Exec();
signals:
    void ready(int res, const char *out);
protected:
    Buffer in;
    Buffer out;
    Buffer err;
};

#endif

