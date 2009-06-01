/***************************************************************************
                          ballonmsg.cpp  -  description
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

#include "ballonmsg.h"

#include <qapplication.h>
#include <qbitmap.h>
#include <qframe.h>
#include <qimage.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <QHBoxLayout>
#include <QPaintEvent>
#include <QPixmap>
#include <QMouseEvent>
#include <QEvent>
#include <QVBoxLayout>
#include <QTextDocument>

#include "misc.h"
#include "unquot.h"

#ifdef WIN32
# include <windows.h>
# ifndef CS_DROPSHADOW
#  define CS_DROPSHADOW   0x00020000
# endif
#endif

#define BALLOON_R			10
#define BALLOON_TAIL		20
#define BALLOON_TAIL_WIDTH	12
#define BALLOON_MARGIN		8
#define BALLOON_SHADOW_DEF	2

using namespace SIM;

SIM_EXPORT QPixmap& intensity(QPixmap &pict, float percent)
{
    QImage image = pict.toImage();
    int i, tmp, r, g, b;
    int segColors = image.depth() > 8 ? 256 : image.numColors();
    unsigned char *segTbl = new unsigned char[segColors];
    int pixels = image.depth() > 8 ? image.width()*image.height() :
                 image.numColors();
    unsigned int *data = image.depth() > 8 ? (unsigned int *)image.bits() :
                         (unsigned int *)image.colorTable().data();

    bool brighten = (percent >= 0);
    if(percent < 0)
        percent = -percent;

    if(brighten){ // keep overflow check out of loops
        for(i=0; i < segColors; ++i){
            tmp = (int)(i*percent);
            if(tmp > 255)
                tmp = 255;
            segTbl[i] = (unsigned char)tmp;
        }
    }
    else{
        for(i=0; i < segColors; ++i){
            tmp = (int)(i*percent);
            if(tmp < 0)
                tmp = 0;
            segTbl[i] = (unsigned char)tmp;
        }
    }

    if(brighten){ // same here
        for(i=0; i < pixels; ++i){
            r = qRed(data[i]);
            g = qGreen(data[i]);
            b = qBlue(data[i]);
            r = r + segTbl[r] > 255 ? 255 : r + segTbl[r];
            g = g + segTbl[g] > 255 ? 255 : g + segTbl[g];
            b = b + segTbl[b] > 255 ? 255 : b + segTbl[b];
            data[i] = qRgb(r, g, b);
        }
    }
    else{
        for(i=0; i < pixels; ++i){
            r = qRed(data[i]);
            g = qGreen(data[i]);
            b = qBlue(data[i]);
            r = r - segTbl[r] < 0 ? 0 : r - segTbl[r];
            g = g - segTbl[g] < 0 ? 0 : g - segTbl[g];
            b = b - segTbl[b] < 0 ? 0 : b - segTbl[b];
            data[i] = qRgb(r, g, b);
        }
    }
    delete [] segTbl;

    pict.fromImage(image);
    return pict;
}

BalloonMsg::BalloonMsg(void *param, const QString &text, QStringList &btn, QWidget *parent, const QRect *rcParent,
                       bool bModal, bool bAutoHide, unsigned bwidth, const QString &box_msg, bool *bChecked)
        : QDialog(parent,
                  (bAutoHide ? Qt::Popup : Qt::Window | Qt::WindowStaysOnTopHint)
                  | Qt::FramelessWindowHint | Qt::Tool)
{
    setModal(bModal);
    setAttribute(Qt::WA_DeleteOnClose);
    m_param = param;
    m_bAutoHide = bAutoHide;
    m_bYes = false;
    m_bChecked = bChecked;
    setPalette(QToolTip::palette());

    bool bTailDown = true;
    QFrame *frm = new QFrame(this);
    QVBoxLayout *vlay = new QVBoxLayout(frm);
    vlay->setMargin(0);
    m_check = NULL;
    if (!box_msg.isEmpty()){
        m_check = new QCheckBox(box_msg, frm);
        vlay->addWidget(m_check);
        if (m_bChecked)
            m_check->setChecked(*m_bChecked);
    }
    QHBoxLayout *lay = new QHBoxLayout();
    vlay->addLayout(lay);
    lay->setSpacing(5);
    lay->addStretch();
    unsigned id = 0;
    bool bFirst = true;
    for (QStringList::Iterator it = btn.begin(); it != btn.end(); ++it, id++){
        BalloonButton *b = new BalloonButton(*it, frm, id);
        connect(b, SIGNAL(action(int)), this, SLOT(action(int)));
        lay->addWidget(b);
        if (bFirst){
            b->setDefault(true);
            bFirst = false;
        }
    }
    setButtonsPict(this);
    lay->addStretch();
    int wndWidth = frm->minimumSizeHint().width();
    int hButton  = frm->minimumSizeHint().height();

    int txtWidth = bwidth;
    QRect rc;
    if (rcParent){
        rc = *rcParent;
    }else{
        QPoint p = parent->mapToGlobal(parent->rect().topLeft());
        rc = QRect(p.x(), p.y(), parent->width(), parent->height());
    }
    if (rc.width() > txtWidth)
        txtWidth = rc.width();

    QTextDocument doc;
    doc.setHtml(text);
    doc.adjustSize();
    doc.setDefaultFont(font());

    QSizeF s = doc.size();
    QSize sMin = frm->minimumSizeHint();
    if (s.width() < sMin.width())
        s.setWidth(sMin.width());
    int BALLOON_SHADOW = BALLOON_SHADOW_DEF;
    resize(s.width() + BALLOON_R * 2 + BALLOON_SHADOW,
           s.height() + BALLOON_R * 2 + BALLOON_TAIL + BALLOON_SHADOW + hButton + BALLOON_MARGIN);
    int w = width() - BALLOON_SHADOW;
    int tailX = w / 2;
    int posX = rc.left() + rc.width() / 2 + BALLOON_TAIL_WIDTH - tailX;
    if (posX <= screenGeometry().left())
        posX = 1;
    QRect rcScreen = screenGeometry();
    if (posX + width() >= rcScreen.width())
        posX = rcScreen.width() - 1 - width();
    int tx = posX + tailX - BALLOON_TAIL_WIDTH;
    if (tx < rc.left())
        tx = rc.left();
    if (tx > rc.left() + rc.width())
        tx = rc.left() + rc.width();
    tailX = tx + BALLOON_TAIL_WIDTH - posX;
    if (tailX < BALLOON_R) tailX = BALLOON_R;
    if (tailX > width() - BALLOON_R - BALLOON_TAIL_WIDTH)
        tailX = width() - BALLOON_R - BALLOON_TAIL_WIDTH;
    if (rc.top() <= height() + 2){
        bTailDown = false;
        move(posX, rc.top() + rc.height() + 1);
    }else{
        move(posX, rc.top() - height() - 1);
    }
    int pos = 0;
    int h = height() - BALLOON_SHADOW - BALLOON_TAIL;
    if (!bTailDown)
        pos += BALLOON_TAIL;
    frm->resize(s.width(), hButton);
    frm->move(BALLOON_R, pos + h - BALLOON_R - hButton);

    QPixmap pm(width(), height());
    // fill with transparent color
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    // we need it more than one time
    const QBrush backBrush = palette().brush(QPalette::ToolTipBase);
    p.setBrush(backBrush);

    // the four corners
    p.drawEllipse(0, pos, BALLOON_R * 2, BALLOON_R * 2);
    p.drawEllipse(w - BALLOON_R * 2, pos, BALLOON_R * 2, BALLOON_R * 2);
    p.drawEllipse(w - BALLOON_R * 2, pos + h - BALLOON_R * 2, BALLOON_R * 2, BALLOON_R * 2);
    p.drawEllipse(0, pos + h - BALLOON_R * 2, BALLOON_R * 2, BALLOON_R * 2);

    // the inner rest
    p.fillRect(0, pos + BALLOON_R, w, h - BALLOON_R * 2, backBrush);
    p.fillRect(BALLOON_R, pos, w - BALLOON_R * 2, h, backBrush);

    // ??
    QPolygon arr(3);
    arr.setPoint(0, tailX, bTailDown ? h - 1 : pos + 1);
    arr.setPoint(1, tailX + BALLOON_TAIL_WIDTH, bTailDown ? h - 1 : pos + 1);
    arr.setPoint(2, tailX - BALLOON_TAIL_WIDTH, bTailDown ? height() - BALLOON_SHADOW : 0);
    p.drawPolygon(arr);

    // black lines around
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

    // center text
    p.translate((pm.width() - s.width()) / 2, 0);
    doc.drawContents(&p);
    p.end();

    m_backgroundPixmap = pm;
    setMask(pm.mask());

    if (!bAutoHide)
        setFocusPolicy(Qt::NoFocus);

    QWidget *top = NULL;
    if (parent)
        top = parent->topLevelWidget();
    if (top){
        raiseWindow(top);
        top->installEventFilter(this);
    }
}

BalloonMsg::~BalloonMsg()
{
    if (!m_bYes)
        emit no_action(m_param);
    emit finished();
}

bool BalloonMsg::isChecked()
{
    if (m_check)
        return m_check->isChecked();
    return false;
}

bool BalloonMsg::eventFilter(QObject *o, QEvent *e)
{
    if ((e->type() == QEvent::Hide) && (o == static_cast<QWidget*>(parent())->topLevelWidget()))
        return true;
    return QDialog::eventFilter(o, e);
}

void BalloonMsg::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setBackgroundMode(Qt::OpaqueMode);
    p.fillRect(rect(), QColor(Qt::transparent));
    p.drawPixmap(0, 0, m_backgroundPixmap);
    p.end();
}

void BalloonMsg::mousePressEvent(QMouseEvent *e)
{
    if (m_bAutoHide && rect().contains(e->pos())){
        const QRgb rgb = m_backgroundPixmap.toImage().pixel(e->pos());
        m_backgroundPixmap.save("/tmp/test.png", "PNG");
        if (rgb & 0xFFFFFF)
            QTimer::singleShot(10, this, SLOT(close()));
    }
    QWidget::mousePressEvent(e);
}

void BalloonMsg::action(int id)
{
    if (m_bChecked && m_check)
        *m_bChecked = m_check->isChecked();
    emit action(id, m_param);
    if (id == 0){
        emit yes_action(m_param);
        m_bYes = true;
    }
}

void BalloonMsg::message(const QString &text, QWidget *parent, bool bModal, unsigned width, const QRect *rc)
{
    QStringList btns;
    btns.append(i18n("&Ok"));
    BalloonMsg *msg = new BalloonMsg(NULL, QString("<center>") + quoteString(text) + "</center>", btns, parent, rc, bModal, true, width);
    if (bModal){
        msg->exec();
    }else{
        msg->show();
    }
}

void BalloonMsg::ask(void *param, const QString &text, QWidget *parent,
                     const char *slotYes, const char *slotNo,
                     const QRect *rc, QObject *receiver,
                     const QString &checkText, bool *bCheck)
{
    QStringList btns;
    btns.append(i18n("&Yes"));
    btns.append(i18n("&No"));
    BalloonMsg *msg = new BalloonMsg(param, QString("<center>") + quoteString(text) + "</center>", btns, parent, rc, false, true, 300, checkText, bCheck);
    if (receiver == NULL)
        receiver = parent;
    if (slotYes)
        connect(msg, SIGNAL(yes_action(void*)), receiver, slotYes);
    if (slotNo)
        connect(msg, SIGNAL(no_action(void*)), receiver, slotNo);
    msg->show();
}

BalloonButton::BalloonButton(const QString &string, QWidget *parent, int _id)
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
