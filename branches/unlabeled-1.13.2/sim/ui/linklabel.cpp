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
#include "stl.h"

#include <qsimplerichtext.h>
#include <qcursor.h>
#include <qapplication.h>
#include <qtooltip.h>
#include <qstylesheet.h>
#include <qpainter.h>

#ifdef WIN32
#include <windows.h>
#endif

LinkLabel::LinkLabel(QWidget *parent, const char *name)
        : QLabel(parent, name)
{
    setCursor(QCursor(PointingHandCursor));
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
    if ((e->button() == LeftButton) && !m_url.isEmpty()){
        string url;
        url = m_url.latin1();
        Event e(EventGoURL, (void*)(url.c_str()));
        e.process();
    }
}

TipLabel::TipLabel(const QString &text)
        : QLabel(NULL, "toolTipTip", WStyle_StaysOnTop | WStyle_Customize | WStyle_NoBorder | WStyle_Tool | WX11BypassWM)
{
    setMargin(3);
    setAutoMask( FALSE );
    setFrameStyle(QFrame::Plain | QFrame::Box);
    setLineWidth(1);
    polish();
    m_text = text;
    setPalette(QToolTip::palette());
}

TipLabel::~TipLabel()
{
    emit finished();
}

void TipLabel::setText(const QString &text)
{
    m_text = text;
}

static char DIV[] = "<br>__________<br>";

void TipLabel::show(const QRect &tipRect, bool _bState)
{
    int prevH = 0;
    int x = 0;
    int y = 0;
    unsigned totalH = 0;
    QStringList l;
    vector<unsigned> heights;
    QRect rc = screenGeometry();
    for (unsigned nDiv = 0;; nDiv++){
        bool bState = _bState;
        QString text = m_text;
        if (nDiv){
            text = "<table><tr><td>";
            unsigned hPart = totalH / (nDiv + 1);
            unsigned h = 0;
            unsigned i = 0;
            QString part;
            for (QStringList::Iterator it = l.begin(); it != l.end(); ++it, i++){
                string s;
                s = (*it).local8Bit();
                if (!part.isEmpty()){
                    if (heights[i] >= hPart){
                        text += part;
                        text += "</td><td>";
                        part = "";
                        h = 0;
                    }else{
                        part += DIV;
                    }
                }
                part += *it;
                h += heights[i];
                if (h >= hPart){
                    text += part;
                    text += "</td><td>";
                    part = "";
                    h = 0;
                }
            }
            text += part;
            text += "</td></tr></table>";
        }
        QSimpleRichText richText(text, font(), "", QStyleSheet::defaultSheet(), QMimeSourceFactory::defaultFactory(), -1, Qt::blue, false);
        richText.adjustSize();
        QSize s(richText.widthUsed() + 8, richText.height() + 8);
        resize(s.width(), s.height());
        x = tipRect.left() + tipRect.width() / 2 - width();
        if (x < 0)
            x = tipRect.left() + tipRect.width() / 2;
        if (x + width() > rc.width() - 2)
            x = rc.width() - 2 - width();
        y = 0;
        if (bState){
            y = tipRect.top() - 4 - height();
            if (y < 0)
                bState = false;
        }
        if (!bState)
            y = tipRect.top() + tipRect.height() + 4;
        if (y + height() > rc.height())
            y = tipRect.top() - 4 - height();
        if (y < 0)
            y = tipRect.top() + tipRect.height() + 4;
        if ((y + s.height() < rc.height()) || (prevH == s.height())){
            m_text = text;
            break;
        }
        prevH = s.height();
        if (totalH == 0){
            totalH = prevH;
            l = QStringList::split(DIV, m_text);
            unsigned i = 0;
            for (QStringList::Iterator it = l.begin(); it != l.end(); ++it, i++){
                QSimpleRichText richText(*it, font(), "", QStyleSheet::defaultSheet(), QMimeSourceFactory::defaultFactory(), -1, Qt::blue, false);
                richText.adjustSize();
                heights.push_back(richText.height() + 8);
            }
        }
    }
    move(x, y);
    QLabel::show();
}

void TipLabel::drawContents(QPainter *p)
{
    QSimpleRichText richText(m_text, font(), "", QStyleSheet::defaultSheet(), QMimeSourceFactory::defaultFactory(), -1, Qt::blue, false);
    richText.adjustSize();
    richText.draw(p, 4, 4, QRect(0, 0, width(), height()), QToolTip::palette().active());
}

#ifndef WIN32
#include "linklabel.moc"
#endif

