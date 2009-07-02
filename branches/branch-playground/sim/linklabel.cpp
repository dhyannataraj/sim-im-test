/***************************************************************************
                          linklabel.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#include "linklabel.h"
#include "event.h"
#include "misc.h"
#include "icons.h"
#include "log.h"

#include <qcursor.h>
#include <q3valuevector.h>
#include <q3stylesheet.h>
#include <qtooltip.h>
#include <qpainter.h>
#include <q3simplerichtext.h>
#include <QUrl>
#include <QMouseEvent>
#include <QLabel>
#include <Q3Frame>
#include <Q3MimeSourceFactory>
#include <QApplication>

using namespace SIM;

LinkLabel::LinkLabel(QWidget *parent, const char *name) : QLabel(parent, name)
{
    setCursor(QCursor(Qt::PointingHandCursor));
    QFont f = font();
    f.setUnderline(true);
    setFont(f);
}

void LinkLabel::setUrl(const QString &url)
{
    m_url = url;
}

void LinkLabel::mouseReleaseEvent(QMouseEvent * e)
{
    if ((e->button() == Qt::LeftButton) && !m_url.isEmpty()){
        EventGoURL e(m_url);
        e.process();
    }
}
