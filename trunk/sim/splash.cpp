/***************************************************************************
                          splash.cpp  -  description
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

#include "splash.h"
#include "mainwin.h"
#include "cfg.h"
#include "log.h"
#include "sim.h"

#ifndef _WINDOWS
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#endif

#include <qwidget.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qfile.h>
#include <qapplication.h>

using namespace std;

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4355)
#endif
#endif

const char *app_file(const char *f);

static const char SPLASH_CONF[] = "splash.conf";

cfgParam Splash_Params[] =
    {
        { "Show", OFFSET_OF(Splash, Show), PARAM_BOOL, (unsigned)true },
        { "Picture", OFFSET_OF(Splash, Picture), PARAM_STRING, (unsigned)"pict/splash.png" },
        { "UseArts", OFFSET_OF(Splash, UseArts), PARAM_BOOL, (unsigned)true },
        { "SoundPlayer", OFFSET_OF(Splash, SoundPlayer), PARAM_STRING, 0 },
        { "StartupSound", OFFSET_OF(Splash, StartupSound), PARAM_STRING, (unsigned)"startup.wav" },
        { "SoundDisable", OFFSET_OF(Splash, SoundDisable), PARAM_BOOL, 0 },
        { "Langauge", OFFSET_OF(Splash, Language), PARAM_STRING, 0 },
        { "LastUIN", OFFSET_OF(Splash, LastUIN), PARAM_ULONG, 0 },
        { "SavePassword", OFFSET_OF(Splash, SavePassword), PARAM_BOOL, 1 },
        { "NoShowLogin", OFFSET_OF(Splash, NoShowLogin), PARAM_BOOL, 0 },
        { "", 0, 0, 0 }
    };

Splash::Splash()
{
    ::init(this, Splash_Params);
    pSplash = this;
    wnd = NULL;
    string part;
    string file = buildFileName(SPLASH_CONF);
    QFile fs(QString::fromLocal8Bit(file.c_str()));
    if (fs.open(IO_ReadOnly))
        ::load(this, Splash_Params, fs, part);
    if (Show){
        QPixmap pict(QString::fromLocal8Bit(app_file(Picture.c_str())));
        if (!pict.isNull()){
            wnd = new QWidget(NULL, "splash",
                              QWidget::WType_TopLevel | QWidget::WStyle_Customize |
                              QWidget::WStyle_NoBorderEx | QWidget::WStyle_StaysOnTop);
            wnd->resize(pict.width(), pict.height());
            QWidget *desktop = QApplication::desktop();
            wnd->move((desktop->width() - pict.width()) / 2, (desktop->height() - pict.height()) / 2);
            wnd->setBackgroundPixmap(pict);
            const QBitmap *mask = pict.mask();
            if (mask) wnd->setMask(*mask);
            wnd->show();
        }
    }
    MainWindow::playSound(StartupSound.c_str());
}

Splash::~Splash()
{
    hide();
}

void Splash::hide()
{
    if (wnd){
        delete wnd;
        wnd = NULL;
    }
}

void Splash::save()
{
    string file = buildFileName(SPLASH_CONF);
    QFile fs(QString::fromLocal8Bit(file.c_str()));
    if (fs.open(IO_WriteOnly | IO_Truncate))
        ::save(this, Splash_Params, fs);
}



