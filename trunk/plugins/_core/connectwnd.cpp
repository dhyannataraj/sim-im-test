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
#include <qfile.h>

ConnectWnd::ConnectWnd(bool bStart)
{
    m_bStart = bStart;
    setConnecting(true);
    QMovie movie(QFile::decodeName(app_file("pict/connect.gif").c_str()));
    if (movie.isNull())
        movie = QMovie(QFile::decodeName(app_file("pict/connect.mng").c_str()));
    if (!movie.isNull()){
        lblMovie->setMovie(movie);
        movie.connectUpdate(this, SLOT(updateMovie()));
        movie.restart();
        updateMovie();
    }
    setConnecting(true);
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
        lblNext->hide();
        frmError->hide();
    }else{
        lblConnect->hide();
        lblMovie->hide();
        lblComplete->show();
        if (m_bStart){
            lblNext->show();
        }else{
            lblNext->hide();
        }
        frmError->hide();
    }
}

void ConnectWnd::setErr(const QString &text)
{
    lblConnect->hide();
    lblMovie->hide();
    lblComplete->hide();
    lblNext->hide();
    lblError->setText(text);
    frmError->show();
}

#ifndef WIN32
#include "connectwnd.moc"
#endif

