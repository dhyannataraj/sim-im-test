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
#include <qstyle.h>

#ifdef WIN32
#include <windows.h>
#endif

XOSD::XOSD(QWidget *p)
        : QWidget(p, "xosd",
				WStyle_StaysOnTop |  WStyle_Customize | WStyle_NoBorder | WStyle_Tool |WRepaintNoErase | WX11BypassWM)
{
	setFocusPolicy(NoFocus);
    init();
}

void XOSD::init()
{
    QFont f = pMain->font();
    int size = f.pixelSize();
    if (size <= 0){
        size = f.pointSize();
        f.setPointSize(size * 2);
    }else{
        f.setPixelSize(size * 2);
    }
    f.setBold(true);
    setFont(pMain->str2font(pMain->XOSD_Font.c_str(), f));
}

#define SHADOW_OFFS	2
#define XOSD_MARGIN	5

void XOSD::set(const QString &str, unsigned long _uin)
{
    uin = _uin;
    hide();
    QPainter p(this);
    p.setFont(font());
    QWidget *d = qApp->desktop();
    QRect rc(0, 0,
             d->width() - SHADOW_OFFS - XOSD_MARGIN * 2 - pMain->XOSD_offset,
             d->height() - SHADOW_OFFS - XOSD_MARGIN * 2 - pMain->XOSD_offset);
    rc = p.boundingRect(rc, AlignLeft | AlignTop | WordBreak, str);
    p.end();
    int w = rc.width();
    int h = rc.height();
    if (pMain->XOSD_Shadow){
        w += SHADOW_OFFS;
        h += SHADOW_OFFS;
    }
    if (pMain->XOSD_Background){
        w += XOSD_MARGIN * 2;
        h += XOSD_MARGIN * 2;
    }
    resize(QSize(w, h));
    switch (pMain->XOSD_pos){
    case 1:
        move(pMain->XOSD_offset, pMain->XOSD_offset);
        break;
    case 2:
        move(d->width() - pMain->XOSD_offset - w, d->height() - pMain->XOSD_offset - h);
        break;
    case 3:
        move(d->width() - pMain->XOSD_offset - w, pMain->XOSD_offset);
        break;
    case 4:
        move((d->width() - w) / 2, d->height() - pMain->XOSD_offset - h);
        break;
    case 5:
        move((d->width() - w) / 2, pMain->XOSD_offset);
        break;
    default:
        move(pMain->XOSD_offset, d->height() - pMain->XOSD_offset - h);
    }
    if (!pMain->XOSD_Background || pMain->XOSD_Shadow){
        QBitmap mask(w, h);
        p.begin(&mask);
#ifdef WIN32
        QColor bg(255, 255, 255);
        QColor fg(0, 0, 0);
#else
        QColor bg(0, 0, 0);
        QColor fg(255, 255, 255);
#endif
        p.fillRect(0, 0, w, h, bg);
        if (pMain->XOSD_Background){
            p.fillRect(0, 0, w - SHADOW_OFFS, h - SHADOW_OFFS, fg);
            p.fillRect(SHADOW_OFFS, SHADOW_OFFS, w - SHADOW_OFFS, h - SHADOW_OFFS, fg);
        }else{
            p.setPen(fg);
            p.setFont(font());
            if (pMain->XOSD_Shadow){
                rc = QRect(SHADOW_OFFS, SHADOW_OFFS, w - SHADOW_OFFS, h - SHADOW_OFFS);
                p.drawText(rc, AlignLeft | AlignTop | WordBreak, str);
            }
            rc = QRect(0, 0, w - SHADOW_OFFS, h - SHADOW_OFFS);
            p.drawText(rc, AlignLeft | AlignTop | WordBreak, str);
        }
        p.end();
        setMask(mask);
    }
    qApp->syncX();
    QPixmap pict = QPixmap::grabWindow(QApplication::desktop()->winId(), x(), y(), width(), height());
    intensity(pict, -0.50f);
    p.begin(&pict);
    rc = QRect(0, 0, w, h);
    if (pMain->XOSD_Background){
        if (pMain->XOSD_Shadow){
            w -= SHADOW_OFFS;
            h -= SHADOW_OFFS;
            rc = QRect(0, 0, w, h);
        }
        QBrush bg(pMain->XOSD_BgColor);
        p.fillRect(rc, bg);
#if QT_VERSION < 300
        style().drawPopupPanel(&p, 0, 0, w, h, colorGroup(), 2, &bg);
#else
        style().drawPrimitive(QStyle::PE_PanelPopup, &p, rc, colorGroup());
#endif
        rc = QRect(XOSD_MARGIN, XOSD_MARGIN, w - XOSD_MARGIN * 2, h - XOSD_MARGIN * 2);
    }
    p.setFont(font());
    p.setPen(QColor(pMain->XOSD_color));
    p.drawText(rc, AlignLeft | AlignTop | WordBreak, str);
    p.end();
    bgPict = pict;
    show();
    QTimer::singleShot(pMain->XOSD_timeout * 1000, this, SLOT(timeout()));
}

void XOSD::paintEvent(QPaintEvent*)
{
#ifdef WIN32
    SetWindowPos(pMain->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
#endif
    QPainter p(this);
    p.drawPixmap(0, 0, bgPict);
    p.end();
}

void XOSD::mouseDoubleClickEvent(QMouseEvent*)
{
    pMain->userFunction(uin, mnuAction, 0);
}

void XOSD::setMessage(const QString &str, unsigned long uin)
{
    if (!pMain->XOSD_on) return;
    if ((msg.count() == 0) && !isVisible()){
        set(str, uin);
        return;
    }
    msg.append(str);
    uins.append(uin);
}

void XOSD::timeout()
{
    hide();
    if (msg.count() == 0) return;
    QStringList::Iterator it = msg.begin();
    QString str = *it;
    QValueList<unsigned long>::Iterator itUin = uins.begin();
    unsigned long uin = *itUin;
    msg.remove(it);
    uins.remove(itUin);
    set(str, uin);
}

#ifndef _WINDOWS
#include "xosd.moc"
#endif

