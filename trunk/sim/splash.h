/***************************************************************************
                          splash.h  -  description
                             -------------------
    begin                : Sun Mar 17 2002
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

#ifndef _SPLASH_H
#define _SPLASH_H

#include "defs.h"
#include "cfg.h"

class QWidget;

class Splash
{
public:
    Splash();
    ~Splash();
    void hide();
    void save();
    bool			Show;
    string			Picture;
    bool			UseArts;
    string			SoundPlayer;
    string			StartupSound;
protected:
    QWidget	*wnd;
};

extern Splash *pSplash;

#endif

