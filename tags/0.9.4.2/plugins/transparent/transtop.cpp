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
#include <kpixmapeffect.h>
#include <kpixmap.h>
#include <qpixmap.h>

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
    if (bg == NULL || bg->isNull())
        return QPixmap();
    KPixmap pix(*bg);
#else
    if (bg.isNull())
        return QPixmap();
    KPixmap pix(bg);
#endif
    return KPixmapEffect::fade(pix, m_transparent, c);
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

#ifndef _MSC_VER
#include "transtop.moc"
#endif

