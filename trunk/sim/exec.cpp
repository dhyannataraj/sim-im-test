/***************************************************************************
                          exec.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#include "exec.h"

Exec::Exec(const char *prog, const char *input)
{
    in.pack(input, strlen(input));
}

Exec::~Exec()
{
}

#ifndef _WINDOWS
#include "exec.moc"
#endif

