/***************************************************************************
                          connectwnd.cpp  -  description
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

#include "connectwnd.h"

#include <qlabel.h>
#include <qmovie.h>
#include <qframe.h>

ConnectWnd::ConnectWnd()
{
    setConnecting(true);
#if QT_VERSION < 300
    QMovie movie(QString::fromLocal8Bit(app_file("pict/connect.gif").c_str()));
#else
    QMovie movie(QString::fromLocal8Bit(app_file("pict/connect.mng").c_str()));
#endif
    lblMovie->setMovie(movie);
    movie.connectUpdate(this, SLOT(updateMovie()));
    setConnecting(true);
    movie.restart();
    updateMovie();
}

void ConnectWnd::updateMovie()
{
    lblMovie->repaint();
}

void ConnectWnd::setConnecting(bool bState)
{
    if (bState){
        lblConnect->show();
        lblMovie->show();
        lblComplete->hide();
        frmError->hide();
    }else{
        lblConnect->hide();
        lblMovie->hide();
        lblComplete->show();
        frmError->hide();
    }
}

void ConnectWnd::setError(const char *text)
{
    lblConnect->hide();
    lblMovie->hide();
    lblComplete->hide();
    lblError->setText(i18n(text));
    frmError->show();
}

#ifndef WIN32
#include "connectwnd.moc"
#endif

