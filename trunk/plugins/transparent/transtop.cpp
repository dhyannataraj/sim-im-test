/***************************************************************************
                          transtop.cpp  -  description
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

#include "simapi.h"
#include "transtop.h"

#include <krootpixmap.h>
#include <kimageeffect.h>
#include <qimage.h>

TransparentTop::TransparentTop(QWidget *parent, unsigned transparent)
        : QObject(parent)
{
    m_transparent = transparent * 0.01;
    rootpixmap = new KRootPixmap(parent);
    rootpixmap->setCustomPainting(true);
    connect(rootpixmap, SIGNAL(backgroundUpdated(const QPixmap&)), this, SLOT(backgroundUpdated(const QPixmap&)));
    transparentChanged();
}

TransparentTop::~TransparentTop()
{}

void TransparentTop::transparentChanged()
{
    rootpixmap->start();
}

void TransparentTop::setTransparent(unsigned transparent)
{
    m_transparent = transparent * 0.01;
    transparentChanged();
}

QPixmap TransparentTop::background(const QColor &c)
{
    if (bg.isNull())
        return QPixmap();
    QImage img = bg.convertToImage();
    img = KImageEffect::fade(img, m_transparent, c);
    QPixmap res;
    res.convertFromImage(img);
    return res;
}

void TransparentTop::backgroundUpdated( const QPixmap &pm )
{
    bg = pm;
    SIM::Event e(SIM::EventRepaintView);
    e.process();
}

#ifndef _MSC_VER
#include "transtop.moc"
#endif
