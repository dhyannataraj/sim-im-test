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
#if COMPAT_QT_VERSION < 0x030000
    parent->installEventFilter(this);
#else
rootpixmap->setCustomPainting(true);
    connect(rootpixmap, SIGNAL(backgroundUpdated(const QPixmap&)), this, SLOT(backgroundUpdated(const QPixmap&)));
#endif
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
#if COMPAT_QT_VERSION < 0x030000
    QWidget *w = (QWidget*)parent();
    const QPixmap *bg = w->backgroundPixmap();
    if (bg == NULL)
        return QPixmap();
    QImage img = bg->convertToImage();
#else
    if (bg.isNull())
        return QPixmap();
    QImage img = bg.convertToImage();
#endif
    img = KImageEffect::fade(img, m_transparent, c);
    QPixmap res;
    res.convertFromImage(img);
    return res;
}

#if COMPAT_QT_VERSION < 0x030000

bool TransparentTop::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::Show){
        Event e(EventRepaintView);
        e.process();
    }
    return QObject::eventFilter(o, e);
}

#else

void TransparentTop::backgroundUpdated( const QPixmap &pm )
{
    bg = pm;
    Event e(EventRepaintView);
    e.process();
}
#endif

#ifndef WIN32
#include "transtop.moc"
#endif

