/***************************************************************************
                          xosd.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "xosd.h"
#include "mainwin.h"
#include "ui/effect.h"

#include <qpainter.h>
#include <qfont.h>
#include <qapplication.h>
#include <qbitmap.h>

XOSD::XOSD(QWidget *p)
        : QWidget(p, "xosd",
                  WType_TopLevel | WStyle_Customize | WStyle_NoBorderEx | WStyle_StaysOnTop | WRepaintNoErase | WStyle_Tool | WX11BypassWM)
{
    init();
}

void XOSD::init()
{
    if (pMain->XOSD_FontSize()){
        setFont(QFont(pMain->XOSD_FontFamily.c_str(), pMain->XOSD_FontSize(),
                      pMain->XOSD_FontWeight(), pMain->XOSD_FontItalic()));
    }else{
        QFont f = pMain->font();
        int size = f.pixelSize();
        if (size == -1){
            size = f.pointSize();
            f.setPointSize(size * 2);
        }else{
            f.setPixelSize(size * 2);
        }
        f.setBold(true);
        setFont(f);
    }
}

#define SHADOW_OFFS	2

void XOSD::set(const QString &str)
{
    hide();
    QPainter p(this);
    p.setFont(font());
    QWidget *d = qApp->desktop();
    QRect rc(0, 0,
             d->width() - SHADOW_OFFS - pMain->XOSD_offset(),
             d->height() - SHADOW_OFFS - pMain->XOSD_offset());
    rc = p.boundingRect(rc, AlignLeft | AlignTop | WordBreak, str);
    p.end();
    QSize s(rc.width() + SHADOW_OFFS, rc.height() + SHADOW_OFFS);
    resize(s);
    switch (pMain->XOSD_pos()){
    case 1:
        move(pMain->XOSD_offset(), pMain->XOSD_offset());
        break;
    case 2:
        move(d->width() - pMain->XOSD_offset() - s.width(), d->height() - pMain->XOSD_offset() - s.height());
        break;
    case 3:
        move(d->width() - pMain->XOSD_offset() - s.width(), pMain->XOSD_offset());
        break;
    case 4:
        move((d->width() - s.width()) / 2, d->height() - pMain->XOSD_offset() - s.height());
        break;
    case 5:
        move((d->width() - s.width()) / 2, pMain->XOSD_offset());
        break;
    default:
        move(pMain->XOSD_offset(), d->height() - pMain->XOSD_offset() - s.height());
    }
    QBitmap mask(s.width(), s.height());
    p.begin(&mask);
#ifdef WIN32
    QColor bg(255, 255, 255);
    QColor fg(0, 0, 0);
#else
    QColor bg(0, 0, 0);
    QColor fg(255, 255, 255);
#endif
    p.fillRect(0, 0, width(), height(), bg);
    p.setPen(fg);
    p.setFont(font());
    rc = QRect(SHADOW_OFFS, SHADOW_OFFS, s.width() - SHADOW_OFFS, s.height() - SHADOW_OFFS);
    p.drawText(rc, AlignLeft | AlignTop | WordBreak, str);
    rc = QRect(0, 0, s.width() - SHADOW_OFFS, s.height() - SHADOW_OFFS);
    p.drawText(rc, AlignLeft | AlignTop | WordBreak, str);
    p.end();
    setMask(mask);
    qApp->syncX();
    QPixmap pict = QPixmap::grabWindow(QApplication::desktop()->winId(), x(), y(), width(), height());
    intensity(pict, -0.50f);
    p.begin(&pict);
    p.setFont(font());
    p.setPen(QColor(pMain->XOSD_color()));
    p.drawText(rc, AlignLeft | AlignTop | WordBreak, str);
    p.end();
    bgPict = pict;
    show();
    QTimer::singleShot(pMain->XOSD_timeout() * 1000, this, SLOT(timeout()));
}

void XOSD::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(0, 0, bgPict);
    p.end();
}

void XOSD::setMessage(const QString &str)
{
    if (!pMain->XOSD_on()) return;
    if ((msg.count() == 0) && !isVisible()){
        set(str);
        return;
    }
    msg.append(str);
}

void XOSD::timeout()
{
    hide();
    if (msg.count() == 0) return;
    QStringList::Iterator it = msg.begin();
    QString str = *it;
    msg.remove(it);
    set(str);
}

#ifndef _WINDOWS
#include "xosd.moc"
#endif

