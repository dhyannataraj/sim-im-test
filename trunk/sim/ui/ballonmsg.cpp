/***************************************************************************
                          ballonmsg.cpp  -  description
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

#include "ballonmsg.h"
#include "effect.h"

#include <qtimer.h>
#include <qpainter.h>
#include <qtooltip.h>
#include <qbitmap.h>
#include <qpointarray.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qimage.h>
#include <qframe.h>

#define BALLOON_R		10
#define BALLOON_WIDTH	150
#define BALLOON_TAIL		20
#define BALLOON_TAIL_WIDTH 12
#define BALLOON_SHADOW	2
#define BALLOON_MARGIN	8

BalloonMsg::BalloonMsg(const QString &_text, const QRect &rc, QStringList &btn, QWidget *parent, bool bModal, bool bAutoHide)
        : QDialog(parent, "ballon", bModal,
                  (bAutoHide ? WType_Popup : WType_TopLevel | WStyle_StaysOnTop)
                  | WStyle_Customize | WStyle_NoBorderEx | WStyle_Tool | WDestructiveClose | WX11BypassWM)
{
    m_bAutoHide = bAutoHide;
    bool bTailDown = true;
    setPalette(QToolTip::palette());
    text = _text;
    int hButton = 0;
    QFrame *frm = new QFrame(this);
    frm->setPalette(palette());
    QHBoxLayout *lay = new QHBoxLayout(frm);
    lay->setSpacing(5);
    lay->addStretch();
    unsigned id = 0;
    for (QStringList::Iterator it = btn.begin(); it != btn.end(); ++it, id++){
        BalloonButton *b = new BalloonButton(*it, frm, id);
        connect(b, SIGNAL(action(int)), this, SIGNAL(action(int)));
        lay->addWidget(b);
        hButton = b->height();
    }
    lay->addStretch();
    int wndWidth = frm->minimumSizeHint().width();

    int txtWidth = BALLOON_WIDTH;
    if (rc.width() > txtWidth) txtWidth = rc.width();

    QPainter p(this);
    QRect rcText = p.boundingRect(0, 0, QMAX(wndWidth, txtWidth), 1000, AlignLeft | AlignTop | WordBreak, _text);
    if (rcText.width() < wndWidth) rcText.setWidth(wndWidth);
    resize(rcText.width() + BALLOON_R * 2 + BALLOON_SHADOW,
           rcText.height() + BALLOON_R * 2 + BALLOON_TAIL + BALLOON_SHADOW + hButton + BALLOON_MARGIN);
    p.end();
    mask = QBitmap(width(), height());
    int w = width() - BALLOON_SHADOW;
    int tailX = w / 2;
    int posX = rc.left() + rc.width() / 2 + BALLOON_TAIL_WIDTH - tailX;
    if (posX <= 0) posX = 1;
    if (posX + width() >= QApplication::desktop()->width())
        posX = QApplication::desktop()->width() - 1 - width();
    int tx = posX + tailX - BALLOON_TAIL_WIDTH;
    if (tx < rc.left()) tx = rc.left();
    if (tx > rc.left() + rc.width()) tx = rc.left() + rc.width();
    tailX = tx + BALLOON_TAIL_WIDTH - posX;
    if (tailX < BALLOON_R) tailX = BALLOON_R;
    if (tailX > width() - BALLOON_R - BALLOON_TAIL_WIDTH) tailX = width() - BALLOON_R - BALLOON_TAIL_WIDTH;
    if (rc.top() <= height() + 2){
        bTailDown = false;
        move(posX, rc.top() + rc.height() + 1);
    }else{
        move(posX, rc.top() - height() - 1);
    }
    int pos = 0;
    int h = height() - BALLOON_SHADOW - BALLOON_TAIL;
    if (!bTailDown) pos += BALLOON_TAIL;
    textRect.setRect(BALLOON_R, pos + BALLOON_R, w - BALLOON_R * 2, h);
    frm->resize(rcText.width(), hButton);
    frm->move(BALLOON_R, pos + h - BALLOON_R - hButton);
    p.begin(&mask);
#ifdef WIN32
    QColor bg(255, 255, 255);
    QColor fg(0, 0, 0);
#else
    QColor bg(0, 0, 0);
    QColor fg(255, 255, 255);
#endif
    p.fillRect(0, 0, width(), height(), bg);
    p.fillRect(0, pos + BALLOON_R, w, h - BALLOON_R * 2, fg);
    p.fillRect(BALLOON_R, pos, w - BALLOON_R * 2, h, fg);
    p.fillRect(BALLOON_SHADOW, pos + BALLOON_R + BALLOON_SHADOW, w, h - BALLOON_R * 2, fg);
    p.fillRect(BALLOON_R + BALLOON_SHADOW, pos + BALLOON_SHADOW, w - BALLOON_R * 2, h, fg);
    p.setBrush(fg);
    p.drawEllipse(0, pos, BALLOON_R * 2, BALLOON_R * 2);
    p.drawEllipse(w - BALLOON_R * 2, pos, BALLOON_R * 2, BALLOON_R * 2);
    p.drawEllipse(w - BALLOON_R * 2, pos + h - BALLOON_R * 2, BALLOON_R * 2, BALLOON_R * 2);
    p.drawEllipse(0, pos + h - BALLOON_R * 2, BALLOON_R * 2, BALLOON_R * 2);
    p.drawEllipse(BALLOON_SHADOW, pos + BALLOON_SHADOW, BALLOON_R * 2, BALLOON_R * 2);
    p.drawEllipse(w - BALLOON_R * 2 + BALLOON_SHADOW, pos + BALLOON_SHADOW, BALLOON_R * 2, BALLOON_R * 2);
    p.drawEllipse(w - BALLOON_R * 2 + BALLOON_SHADOW, pos + h - BALLOON_R * 2 + BALLOON_SHADOW, BALLOON_R * 2, BALLOON_R * 2);
    p.drawEllipse(BALLOON_SHADOW, pos + h - BALLOON_R * 2 + BALLOON_SHADOW, BALLOON_R * 2, BALLOON_R * 2);
    QPointArray arr(3);
    arr.setPoint(0, tailX, bTailDown ? h : pos);
    arr.setPoint(1, tailX + BALLOON_TAIL_WIDTH, bTailDown ? h : pos);
    arr.setPoint(2, tailX - BALLOON_TAIL_WIDTH, bTailDown ? height() - BALLOON_SHADOW : 0);
    p.drawPolygon(arr);
    arr.setPoint(0, tailX + BALLOON_SHADOW, (bTailDown ? h : pos) + BALLOON_SHADOW);
    arr.setPoint(1, tailX + BALLOON_TAIL_WIDTH + BALLOON_SHADOW, (bTailDown ? h : pos) + BALLOON_SHADOW);
    arr.setPoint(2, tailX - BALLOON_TAIL_WIDTH + BALLOON_SHADOW, bTailDown ? height() : BALLOON_SHADOW);
    p.drawPolygon(arr);
    p.end();
    setMask(mask);
    qApp->syncX();
    QPixmap pict = QPixmap::grabWindow(QApplication::desktop()->winId(), x(), y(), width(), height());
    intensity(pict, -0.50f);
    p.begin(&pict);
    p.setBrush(colorGroup().background());
    p.drawEllipse(0, pos, BALLOON_R * 2, BALLOON_R * 2);
    p.drawEllipse(w - BALLOON_R * 2, pos, BALLOON_R * 2, BALLOON_R * 2);
    p.drawEllipse(w - BALLOON_R * 2, pos + h - BALLOON_R * 2, BALLOON_R * 2, BALLOON_R * 2);
    p.drawEllipse(0, pos + h - BALLOON_R * 2, BALLOON_R * 2, BALLOON_R * 2);
    arr.setPoint(0, tailX, bTailDown ? h - 1 : pos + 1);
    arr.setPoint(1, tailX + BALLOON_TAIL_WIDTH, bTailDown ? h - 1 : pos + 1);
    arr.setPoint(2, tailX - BALLOON_TAIL_WIDTH, bTailDown ? height() - BALLOON_SHADOW : 0);
    p.drawPolygon(arr);
    p.fillRect(0, pos + BALLOON_R, w, h - BALLOON_R * 2, colorGroup().background());
    p.fillRect(BALLOON_R, pos, w - BALLOON_R * 2, h, colorGroup().background());
    p.drawLine(0, pos + BALLOON_R, 0, pos + h - BALLOON_R);
    p.drawLine(w - 1, pos + BALLOON_R, w - 1, pos + h - BALLOON_R);
    if (bTailDown){
        p.drawLine(BALLOON_R, 0, w - BALLOON_R, 0);
        p.drawLine(BALLOON_R, h - 1, tailX, h - 1);
        p.drawLine(tailX + BALLOON_TAIL_WIDTH, h - 1, w - BALLOON_R, h - 1);
    }else{
        p.drawLine(BALLOON_R, pos + h - 1, w - BALLOON_R, pos + h - 1);
        p.drawLine(BALLOON_R, pos, tailX, pos);
        p.drawLine(tailX + BALLOON_TAIL_WIDTH, pos, w - BALLOON_R, pos);
    }
    p.end();
    setBackgroundPixmap(pict);
    setAutoMask(true);
    if (!bAutoHide)
        setFocusPolicy(NoFocus);
}

void BalloonMsg::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawText(textRect, (m_bAutoHide ? AlignHCenter : AlignLeft) | AlignTop | WordBreak, text);
    p.end();
}

void BalloonMsg::mousePressEvent(QMouseEvent *e)
{
    if (m_bAutoHide && rect().contains(e->pos())){
        QImage img = mask.convertToImage();
        QRgb rgb = img.pixel(e->pos().x(), e->pos().y());
        if (rgb & 0xFFFFFF)
            QTimer::singleShot(10, this, SLOT(close()));
    }
    QWidget::mousePressEvent(e);
}

void BalloonMsg::message(const QString &text, QWidget *parent, bool bModal)
{
    QPoint p = parent->mapToGlobal(parent->rect().topLeft());
    QRect rc(p.x(), p.y(), parent->width(), parent->height());
    QStringList btns;
    btns.append(i18n("&Ok"));
    BalloonMsg *msg = new BalloonMsg(text, rc, btns, parent, bModal);
    if (bModal){
        msg->exec();
    }else{
        msg->show();
    }
}

BalloonButton::BalloonButton(QString string, QWidget *parent, int _id)
        : QPushButton(string, parent)
{
    id = _id;
    setPalette(parent->palette());
    resize(sizeHint());
    connect(this, SIGNAL(clicked()), this, SLOT(click()));
}

void BalloonButton::click()
{
    topLevelWidget()->hide();
    emit action(id);
    topLevelWidget()->close();
}


#ifndef _WINDOWS
#include "ballonmsg.moc"
#endif

