/***************************************************************************
                          connectwnd.h  -  description
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

#ifndef _CONNECTWND_H
#define _CONNECTWND_H

#include "simapi.h"
#include "connectwndbase.h"

class ConnectWnd : public ConnectWndBase
{
    Q_OBJECT
public:
    ConnectWnd();
    void setConnecting(bool bState);
    void setError(const char *text);
protected slots:
    void updateMovie();
};

#endif

