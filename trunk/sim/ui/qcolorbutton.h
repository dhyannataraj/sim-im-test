/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)

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

#ifndef __QCOLBTN_H__
#define __QCOLBTN_H__

#include <qpushbutton.h>

#undef QColorButton

class QColorButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY( QColor color READ color WRITE setColor )

public:
    QColorButton( QWidget *parent, const char *name = 0L );
    QColorButton( const QColor &c, QWidget *parent, const char *name = 0L );
    virtual ~QColorButton() {}
    QColor color() const
        {	return col; }
    void setColor( const QColor &c );

signals:
    void changed( const QColor &newColor );

protected slots:
    void chooseColor();

protected:
    virtual void drawButtonLabel( QPainter *p );

private:
    QColor col;
    QPoint mPos;
};

#ifdef USE_KDE
#define QColorButton KColorButton
#endif

#endif

