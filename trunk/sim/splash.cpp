/***************************************************************************
                          splash.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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
        { "Show", offsetof(Splash_Data, Show), PARAM_BOOL, (unsigned)true },
        { "Picture", offsetof(Splash_Data, Picture), PARAM_CHARS, (unsigned)"pict/splash.png" },
        { "UseArts", offsetof(Splash_Data, UseArts), PARAM_BOOL, (unsigned)true },
        { "SoundPlayer", offsetof(Splash_Data, SoundPlayer), PARAM_CHARS, 0 },
        { "StartupSound", offsetof(Splash_Data, StartupSound), PARAM_CHARS, (unsigned)"startup.wav" },
        { "SoundDisable", offsetof(Splash_Data, SoundDisable), PARAM_BOOL, 0 },
        { "Langauge", offsetof(Splash_Data, Language), PARAM_CHARS, 0 },
        { "LastUIN", offsetof(Splash_Data, LastUIN), PARAM_ULONG, 0 },
        { "SavePassword", offsetof(Splash_Data, SavePassword), PARAM_BOOL, 1 },
        { "NoShowLogin", offsetof(Splash_Data, NoShowLogin), PARAM_BOOL, 0 },
        { "", 0, 0, 0 }
    };

Splash::Splash()
{
    ::init(&data, Splash_Params);
    pSplash = this;
    wnd = NULL;
    string part;
    string file = buildFileName(SPLASH_CONF);
    QFile fs(QString::fromLocal8Bit(file.c_str()));
    if (fs.open(IO_ReadOnly))
        ::load(&data, Splash_Params, fs, part);
    if (isShow()){
        QPixmap pict(QString::fromLocal8Bit(app_file(getPicture())));
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
    MainWindow::playSound(getStartupSound());
}

Splash::~Splash()
{
    hide();
    ::free(&data, Splash_Params);
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
        ::save(&data, Splash_Params, fs);
}



