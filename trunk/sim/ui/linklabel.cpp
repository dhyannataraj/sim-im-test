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

#include <qcursor.h>
#include <qapplication.h>
#include <qtooltip.h>

LinkLabel::LinkLabel(QWidget *parent, const char *name)
        : QLabel(parent, name)
{
    setCursor(QCursor(PointingHandCursor));
    QFont f = font();
    f.setUnderline(true);
    setFont(f);
}

void LinkLabel::mouseReleaseEvent(QMouseEvent * e)
{
    if (e->button() == LeftButton)
        emit click();
}

TipLabel::TipLabel(const QString &text)
        : QLabel(NULL, "toolTipTip",
                 WStyle_StaysOnTop | WStyle_Customize | WStyle_NoBorder | WStyle_Tool | WX11BypassWM )
{
    setMargin(1);
    setIndent(0);
    setAutoMask( FALSE );
    setFrameStyle( QFrame::Plain | QFrame::Box );
    setLineWidth( 1 );
    polish();
    setText(text);
    adjustSize();
    setPalette(QToolTip::palette());
}

TipLabel::~TipLabel()
{
    emit finished();
}

void TipLabel::setText(const QString &text)
{
    QLabel::setText(QString("<div align=\"left\">") + text);
}

void TipLabel::show(const QRect &tipRect)
{
    setAlignment( WordBreak | AlignCenter );
    QRect rc = screenGeometry();
    QSize s = sizeHint();
    int h = heightForWidth(s.width());
    resize(s.width(), h );
    int x = tipRect.left() + tipRect.width() / 2 - width();
    if (x < 0)
        x = tipRect.left() + tipRect.width() / 2;
    if (x + width() > rc.width() - 2)
        x = rc.width() - 2 - width();
    int y = tipRect.top() + tipRect.height() + 4;
    if (y + height() > rc.height())
        y = tipRect.top() - 4 - height();
    if (y < 0)
        y = tipRect.top() + tipRect.height() + 4;
    move(x, y);
    QLabel::show();
}

#ifndef WIN32
#include "linklabel.moc"
#endif

