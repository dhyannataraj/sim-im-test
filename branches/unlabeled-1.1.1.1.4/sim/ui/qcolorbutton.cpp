/*  This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)
    Copyright (C) 1999 Cristian Tibirna (ctibirna@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "defs.h"
#include "qcolorbutton.h"

#undef QColorButton

#include <qpainter.h>
#include <qdrawutil.h>
#include <qapplication.h>
#include <qstyle.h>
#include <qcolordialog.h>

QColorButton::QColorButton( QWidget *parent, const char *name )
        : QPushButton( parent, name )
{
    connect (this, SIGNAL(clicked()), this, SLOT(chooseColor()));
}

void QColorButton::setColor( const QColor &c )
{
    if ( col != c ) {
        col = c;
        repaint( false );
        emit changed( col );
    }
}

void QColorButton::drawButtonLabel( QPainter *painter )
{
#if QT_VERSION < 300
    QRect r = style().pushButtonContentsRect(this);
#else
    QRect r = style().subRect( QStyle::SR_PushButtonContents, this );
#endif
    int l = r.x();
    int t = r.y();
    int w = r.width();
    int h = r.height();
    int b = 5;

    QColor lnCol = colorGroup().text();
    QColor fillCol = isEnabled() ? col : backgroundColor();

    if ( isDown() ) {
        qDrawPlainRect( painter, l+b+1, t+b+1, w-b*2, h-b*2, lnCol, 1, 0 );
        b++;
        if ( fillCol.isValid() )
            painter->fillRect( l+b+1, t+b+1, w-b*2, h-b*2, fillCol );
    } else {
        qDrawPlainRect( painter, l+b, t+b, w-b*2, h-b*2, lnCol, 1, 0 );
        b++;
        if ( fillCol.isValid() )
            painter->fillRect( l+b, t+b, w-b*2, h-b*2, fillCol );
    }
}

void QColorButton::chooseColor()
{
    QColor c = QColorDialog::getColor(color(), this);
    if (!c.isValid()) return;
    setColor( c );
}

#ifndef WIN32
#include "qcolorbutton.moc"
#endif

