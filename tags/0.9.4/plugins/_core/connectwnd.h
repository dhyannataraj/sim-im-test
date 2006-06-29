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
    ConnectWnd(bool bStart);
    void setConnecting(bool bState);
    void setErr(const QString &text, const char *url);
protected slots:
    void updateMovie();
protected:
    bool m_bStart;
};

#endif

