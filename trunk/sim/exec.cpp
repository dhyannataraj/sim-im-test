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

Exec::Exec(QObject *parent)
	: QObject(parent)
{
}

Exec::~Exec()
{
}

void Exec::execute(const char *prg, const char *input)
{
    bIn.init(0);
    bOut.init(0);
    bErr.init(0);
    if (input)
        bIn.pack(input, strlen(input));
    emit ready(-1, prg);
}

#ifndef _WINDOWS
#include "exec.moc"
#endif

