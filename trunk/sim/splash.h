/***************************************************************************
                          splash.h  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan.ru
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

typedef struct Splash_Data
{
    bool            Show;
    char*           Picture;
    bool            UseArts;
    char*           SoundPlayer;
    char*           StartupSound;
    bool            SoundDisable;
    char*           Language;
    unsigned long   LastUIN;
    bool            SavePassword;
    bool            NoShowLogin;
} Splash_Data;

class Splash
{
public:
    Splash();
    ~Splash();
    void hide();
    void save();
    PROP_BOOL(Show)
    PROP_STR(Picture)
    PROP_BOOL(UseArts)
    PROP_STR(SoundPlayer)
    PROP_STR(StartupSound)
    PROP_BOOL(SoundDisable)
    PROP_STR(Language)
    PROP_ULONG(LastUIN)
    PROP_BOOL(SavePassword)
    PROP_BOOL(NoShowLogin)
protected:
    Splash_Data data;
    QWidget	*wnd;
};

extern Splash *pSplash;

#endif

