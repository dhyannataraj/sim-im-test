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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.
*/

#include "qcolorbutton.h"
#ifdef USE_KDE

QColorButton::QColorButton( QWidget *parent, const char *name)
        : KColorButton(parent, name)
{
}

#else

#include <qpainter.h>
#include <qdrawutil.h>
#include <qapplication.h>
#include <qstyle.h>
#include <qcolordialog.h>
#include <QStyle>
#include <QStyleOption>
#include <QStyleOptionButton>

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

void QColorButton::drawButtonLabel(QPainter *painter)
{
	int x, y, w, h;
	QStyleOptionButton option;
	option.initFrom(this);
	QRect r = style()->subElementRect(QStyle::SE_PushButtonContents, &option, this);
	r.rect(&x, &y, &w, &h);

	int margin = style()->pixelMetric(QStyle::PM_ButtonMargin, &option, this);
	x += margin;
	y += margin;
	w -= 2*margin;
	h -= 2*margin;

	if(isOn() || isDown())
	{
		x += style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal, &option, this);
		y += style()->pixelMetric(QStyle::PM_ButtonShiftVertical, &option, this);
	}

	QColor fillCol = isEnabled() ? col : backgroundColor();
	qDrawShadePanel(painter, x, y, w, h, colorGroup(), true, 1, NULL);
	if(fillCol.isValid())
		painter->fillRect(x + 1, y + 1, w - 2, h - 2, fillCol);

	if(hasFocus())
	{
		QRect focusRect = style()->subElementRect(QStyle::SE_PushButtonFocusRect, &option, this);
		option.rect = focusRect;
		//style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, painter, colorGroup());
		style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, painter, this);
	}
}

QSize QColorButton::sizeHint() const
{
	QStyleOptionButton option;
	option.initFrom(this);
    return style()->sizeFromContents(QStyle::CT_PushButton, &option, QSize(40, 15), this).expandedTo(QApplication::globalStrut());
}

void QColorButton::chooseColor()
{
    QColor c = QColorDialog::getColor( color(), this );
    if( c.isValid() )
        setColor( c );
}

/*
#ifndef NO_MOC_INCLUDES
#include "qcolorbutton.moc"
#endif
*/

#endif

