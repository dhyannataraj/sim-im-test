/***************************************************************************
                          floatywnd.cpp  -  description
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

#include "floatywnd.h"
#include "floaty.h"
#include "simapi.h"
#include "userview.h"
#include "linklabel.h"

#include "core.h"

#include <qpixmap.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qapplication.h>

#ifdef USE_KDE
#include <kwin.h>
#endif

FloatyWnd::FloatyWnd(FloatyPlugin *plugin, unsigned id)
        : QWidget(NULL, "floaty",
                  WType_TopLevel | WStyle_Customize | WStyle_NoBorder | WStyle_Tool |
                  WStyle_StaysOnTop | WRepaintNoErase)
{
    m_plugin = plugin;
    m_id = id;
    init();
    setAcceptDrops(true);
    setBackgroundMode(NoBackground);
#ifdef USE_KDE
    KWin::setState(winId(), NET::SkipTaskbar | NET::SkipPager);
    KWin::setOnAllDesktops(winId(), true);
#endif
    m_tip = NULL;
    tipTimer = new QTimer(this);
    connect(tipTimer, SIGNAL(timeout()), this, SLOT(showTip()));
    moveTimer = new QTimer(this);
    connect(moveTimer, SIGNAL(timeout()), this, SLOT(startMove()));
    setMouseTracking(true);
}

FloatyWnd::~FloatyWnd()
{
}

void FloatyWnd::init()
{
    m_style = 0;
    m_icons = "";
    m_unread = 0;
    Contact *contact = getContacts()->contact(m_id);
    if (contact == NULL)
        return;
    m_text = contact->getName();
    m_status = contact->contactInfo(m_style, m_statusIcon, &m_icons);
    QRect br = fontMetrics().boundingRect(m_text);
    unsigned h = br.height();
    unsigned w = br.width() + 5;
    const QPixmap &pict = Pict(m_statusIcon);
    w += pict.width() + 2;
    if ((unsigned)(pict.height()) > h)
        h = pict.height();
    string icons = m_icons;
    while (icons.length()){
        string icon = getToken(icons, ',');
        const QPixmap &pict = Pict(icon.c_str());
        w += pict.width() + 2;
        if ((unsigned)(pict.height()) > h)
            h = pict.height();
    }
    w += 8;
    h += 6;
    resize(w, h);
    for (list<msg_id>::iterator it = m_plugin->core->unread.begin(); it != m_plugin->core->unread.end(); ++it){
        if ((*it).contact != m_id)
            continue;
        m_unread = (*it).type;
        m_plugin->startBlink();
        break;
    }
}

void FloatyWnd::paintEvent(QPaintEvent*)
{
    int w = width()  - 4;
    int h = height() - 4;

    QPixmap pict(w, h);
    QPainter p(&pict);
    p.fillRect(QRect(0, 0, width(), height()), colorGroup().base());
    PaintView pv;
    pv.p        = &p;
    pv.pos      = QPoint(2, 2);
    pv.size		= QSize(w, h);
    pv.win      = this;
    pv.isStatic = false;
    pv.height   = h;
    if (m_plugin->core->getUseSysColors()){
        p.setPen(colorGroup().text());
    }else{
        p.setPen(QColor(m_plugin->core->getColorOnline()));
    }
    Event e(EventPaintView, &pv);
    e.process();

    if (m_plugin->core->getUseSysColors()){
        if (m_status != STATUS_ONLINE)
            p.setPen(palette().disabled().text());
    }else{
        switch (m_status){
        case STATUS_ONLINE:
            break;
        case STATUS_AWAY:
            p.setPen(m_plugin->core->getColorAway());
            break;
        case STATUS_NA:
            p.setPen(m_plugin->core->getColorNA());
            break;
        case STATUS_DND:
            p.setPen(m_plugin->core->getColorDND());
            break;
        default:
            p.setPen(m_plugin->core->getColorOffline());
            break;
        }
    }

    int x = 0;
    const char *statusIcon = m_statusIcon;
    if (m_unread && m_plugin->m_bBlink){
        CommandDef *def = m_plugin->core->messageTypes.find(m_unread);
        if (def)
            statusIcon = def->icon;
    }

    if (statusIcon){
        const QPixmap &pict = Pict(statusIcon);
        x += 2;
        p.drawPixmap(x, (h - pict.height()) / 2, pict);
        x += pict.width() + 2;
    }
    QRect br;
    p.drawText(x, 0, w, h, AlignLeft | AlignVCenter, m_text, -1, &br);
    x = br.right() + 5;
    string icons = m_icons;
    while (icons.length()){
        string icon = getToken(icons, ',');
        const QPixmap &pict = Pict(icon.c_str());
        x += 2;
        p.drawPixmap(x, (h - pict.height()) / 2, pict);
        x += pict.width();
    }
    p.end();

    p.begin(this);
    p.drawPixmap(QPoint(2, 2), pict);
    p.setPen(colorGroup().dark());
    p.moveTo(1, 1);
    p.lineTo(width() - 2, 1);
    p.lineTo(width() - 2, height() - 2);
    p.lineTo(1, height() - 2);
    p.lineTo(1, 1);
    p.setPen(colorGroup().shadow());
    p.moveTo(0, height() - 1);
    p.lineTo(width() - 1, height() - 1);
    p.lineTo(width() - 1, 1);
    p.moveTo(width() - 3, 2);
    p.lineTo(2, 2);
    p.lineTo(2, height() - 3);
    p.setPen(colorGroup().light());
    p.moveTo(2, height() - 3);
    p.lineTo(width() - 3, height() - 3);
    p.lineTo(width() - 3, 2);
    p.moveTo(width() - 1, 0);
    p.lineTo(0, 0);
    p.lineTo(0, height() - 1);
}

void FloatyWnd::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == LeftButton){
        initMousePos = e->pos();
        moveTimer->start(QApplication::startDragTime());
    }
    if (e->button() == RightButton){
        m_plugin->popupPos = e->globalPos();
        m_plugin->popupId  = m_id;
        QTimer::singleShot(0, m_plugin, SLOT(showPopup()));
    }
}

void FloatyWnd::mouseReleaseEvent(QMouseEvent *e)
{
    moveTimer->stop();
    if (!mousePos.isNull()){
        move(e->globalPos() - mousePos);
        releaseMouse();
        Contact *contact = getContacts()->contact(m_id);
        if (contact){
            FloatyUserData *data = (FloatyUserData*)(contact->userData.getUserData(m_plugin->user_data_id, false));
            if (data){
                data->X.value = x();
                data->Y.value = y();
            }
        }
        mousePos = QPoint();
    }else{
        if ((e->pos() == initMousePos) && !m_plugin->core->getUseDblClick()){
            Event e(EventDefaultAction, (void*)m_id);
            e.process();
        }
    }
    initMousePos = QPoint(0, 0);
}

void FloatyWnd::mouseMoveEvent(QMouseEvent *e)
{
    if ((e->state() & QObject::LeftButton) && !initMousePos.isNull() &&
            (QPoint(e->pos() - initMousePos).manhattanLength() > QApplication::startDragDistance()))
        startMove();
    if (!mousePos.isNull())
        move(e->globalPos() - mousePos);
}

void FloatyWnd::startMove()
{
    if (initMousePos.isNull())
        return;
    moveTimer->stop();
    mousePos = initMousePos;
    initMousePos = QPoint(0, 0);
    grabMouse();
}

void FloatyWnd::mouseDoubleClickEvent(QMouseEvent *)
{
    Event e(EventDefaultAction, (void*)m_id);
    e.process();
}

void FloatyWnd::enterEvent(QEvent *e)
{
    QWidget::enterEvent(e);
    tipTimer->start(1000);
}

void FloatyWnd::leaveEvent(QEvent *e)
{
    hideTip();
    QWidget::leaveEvent(e);
}

void FloatyWnd::hideTip()
{
    tipTimer->stop();
    if (m_tip)
        m_tip->hide();
}

void FloatyWnd::tipDestroyed()
{
    m_tip = NULL;
}

void FloatyWnd::showTip()
{
    Contact *contact = getContacts()->contact(m_id);
    if (contact == NULL)
        return;
    QString text = contact->tipText();
    if (m_tip){
        m_tip->setText(text);
    }else{
        m_tip = new TipLabel(text);
    }
    m_tip->show(QRect(pos().x(), pos().y(), width(), height()));
}

void FloatyWnd::dragEnterEvent(QDragEnterEvent *e)
{
    dragEvent(e, false);
}

void FloatyWnd::dropEvent(QDropEvent *e)
{
    dragEvent(e, true);
}

void FloatyWnd::dragEvent(QDropEvent *e, bool isDrop)
{
    Message *msg = NULL;
    CommandDef *cmd;
    CommandsMapIterator it(m_plugin->core->messageTypes);
    while ((cmd = ++it) != NULL){
        MessageDef *def = (MessageDef*)(cmd->param);
        if (def && def->drag){
            msg = def->drag(e);
            if (msg){
                unsigned type = cmd->id;
                Command cmd;
                cmd->id      = type;
                cmd->menu_id = MenuMessage;
                cmd->param	 = (void*)m_id;
                Event e(EventCheckState, cmd);
                if (e.process())
                    break;
            }
        }
    }
    if (msg){
        e->accept();
        if (isDrop){
            msg->setContact(m_id);
            Event e(EventOpenMessage, &msg);
            e.process();
        }
        delete msg;
        return;
    }
    if (QTextDrag::canDecode(e)){
        QString str;
        if (QTextDrag::decode(e, str)){
            e->accept();
            if (isDrop){
                Message *msg = new Message(MessageGeneric);
                msg->setText(str);
                msg->setContact(m_id);
                Event e(EventOpenMessage, &msg);
                e.process();
                delete msg;
            }
            return;
        }
    }
}

#ifndef WIN32
#include "floatywnd.moc"
#endif

