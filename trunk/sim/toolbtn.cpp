/***************************************************************************
                          toolbtn.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#include "toolbtn.h"
#include "icons.h"
#include "mainwin.h"
#include "log.h"

#include <qpainter.h>
#include <qtoolbar.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <qpopupmenu.h>
#include <qstyle.h>
#include <qmainwindow.h>
#include <qiconset.h>
#include <qpalette.h>
#include <qaccel.h>
#include <qregexp.h>

CPushButton::CPushButton(QWidget *parent, const char *name)
        : QPushButton(parent, name)
{
}

void CPushButton::setTip(const QString &_tip)
{
    QToolTip::add(this, _tip);
}

CToolButton::CToolButton( const QIconSet & p, const QString & textLabel,
                          const QString & grouptext, QObject * receiver, const char * slot, QToolBar * parent,
                          const char * name)
        : QToolButton(p, textLabel, grouptext, receiver, slot, parent, name)
{
    popup = NULL;
}

CToolButton::CToolButton( const QIconSet & pOff, const QPixmap & pOn, const QString & textLabel,
                          const QString & grouptext, QObject * receiver, const char * slot, QToolBar * parent,
                          const char * name)
        : QToolButton(pOff, textLabel, grouptext, receiver, slot, parent, name)
{
    popup = NULL;
#if QT_VERSION < 300
    setOnIconSet(pOn);
#else
    QIconSet icon = pOff;
    icon.setPixmap(pOn, QIconSet::Small, QIconSet::Normal, QIconSet::On);
    setIconSet(icon);
#endif
}

CToolButton::CToolButton ( QWidget * parent, const char * name  )
        : QToolButton( parent, name)
{
    popup = NULL;
}

void CToolButton::setPopup(QPopupMenu *_popup)
{
#if QT_VERSION < 300
    QToolButton::setPopup(_popup);
#else
    if (popup) disconnect(this, SIGNAL(clicked()), this, SLOT(btnClicked()));
    popup = _popup;
    if (popup) connect(this, SIGNAL(clicked()), this, SLOT(btnClicked()));
#endif
}

void CToolButton::btnClicked()
{
    if (popup == NULL) return;
    QPoint pos;
    QToolBar *bar = static_cast<QToolBar*>(parent());
    if (bar->orientation() == Vertical){
        pos = QPoint(width(), 0);
    }else{
        pos = QPoint(0, height());
    }
    pos = mapToGlobal(pos);
    popup->popup(pos);
}

void CToolButton::mousePressEvent(QMouseEvent *e)
{
#if QT_VERSION < 300
    if (e->button() == RightButton){
        emit showPopup(e->globalPos());
        return;
    }
#endif
    QToolButton::mousePressEvent(e);
}

void CToolButton::setTextLabel(const QString &text)
{
    setAccel(QAccel::shortcutKey(text));
    QString t = text;
    int pos;
    while ((pos = t.find('&')) >= 0){
        t = t.left(pos) + "<u>" + t.mid(pos+1, 1) + "</u>" + t.mid(pos+2);
    }
    QToolButton::setTextLabel(t);
}

void CToolButton::contextMenuEvent(QContextMenuEvent *e)
{
    e->accept();
    emit showPopup(e->globalPos());
}

PictButton::PictButton( QToolBar *parent)
        : CToolButton(parent)
{
	accelKey = 0;
    connect(pMain, SIGNAL(iconChanged()), this, SLOT(iconChanged()));
}

void PictButton::iconChanged()
{
    repaint();
}

QSizePolicy PictButton::sizePolicy() const
{
    QSizePolicy p = QToolButton::sizePolicy();
    QToolBar *bar = static_cast<QToolBar*>(parent());
    if (bar->orientation() == Vertical){
        p.setVerData(QSizePolicy::Expanding);
    }else{
        p.setHorData(QSizePolicy::Expanding);
    }
    return p;
}

QSize PictButton::minimumSizeHint() const
{
    int wChar = QFontMetrics(font()).width('0');
    QSize p = QToolButton:: minimumSizeHint();
    QToolBar *bar = static_cast<QToolBar*>(parent());
    QMainWindow::ToolBarDock tDock;
    int index;
    bool nl;
    int extraOffset;
    bar->mainWindow()->getLocation(bar, tDock, index, nl, extraOffset);
    if (tDock == QMainWindow::TornOff){
        if (bar->orientation() == Vertical){
            p.setHeight(p.height() + 2 * wChar + 16);
        }else{
            p.setWidth(p.width() + 2 * wChar + 16);
        }
    }else{
        p = QSize(22, 22);
    }
    return p;
}

QSize PictButton::sizeHint() const
{
    int wChar = QFontMetrics(font()).width('0');
    QSize p = QToolButton:: sizeHint();
    QToolBar *bar = static_cast<QToolBar*>(parent());
    QMainWindow::ToolBarDock tDock;
    int index;
    bool nl;
    int extraOffset;
    bar->mainWindow()->getLocation(bar, tDock, index, nl, extraOffset);
    if (tDock == QMainWindow::TornOff){
        if (bar->orientation() == Vertical){
            p.setHeight(p.height() + 2 * wChar + 16);
        }else{
            p.setWidth(p.width() + 2 * wChar + 16);
        }
    }else{
        p = QSize(22, 22);
    }
    return p;
}

void PictButton::setText(const QString &text)
{
    setState(icon, text);
}

void PictButton::showEvent(QShowEvent *e)
{
	if (accelKey) setAccel(accelKey);
	CToolButton::showEvent(e);
}

void PictButton::hideEvent(QHideEvent *e)
{
	if (accelKey) setAccel(0);
	CToolButton::hideEvent(e);
}

void PictButton::setState(const QString& _icon, const QString& _text)
{
    icon = _icon;
    text = _text;
	int accelKey = QAccel::shortcutKey(text);
    if (isVisible())
		setAccel(accelKey);
    QString t = _text;
    int pos;
    while ((pos = t.find('&')) >= 0){
        t = t.left(pos) + "<u>" + t.mid(pos+1, 1) + "</u>" + t.mid(pos+2);
    }
    setTextLabel(t);
    repaint();
}

void PictButton::paintEvent(QPaintEvent*)
{
    QPixmap pict(width(), height());
    QPainter p(&pict);
    QWidget *pw = static_cast<QWidget*>(parent());
    if (pw){
        if (pw->backgroundPixmap()){
            p.drawTiledPixmap(0, 0, width(), height(), *pw->backgroundPixmap(), x(), y());
        }else{
            p.fillRect(0, 0, width(), height(), colorGroup().button());
        }
    }
#if QT_VERSION < 300
    style().drawToolButton(this, &p);
#else
    drawButton(&p);
#endif
    QRect rc(4, 4, width() - 4, height() - 4);
    if (icon){
        const QIconSet &icons = Icon(icon);
        const QPixmap &pict = icons.pixmap(QIconSet::Small, isEnabled() ? QIconSet::Active : QIconSet::Disabled);
        QToolBar *bar = static_cast<QToolBar*>(parent());
        if (bar->orientation() == Vertical){
            p.drawPixmap((width() - pict.width()) / 2, 4, pict);
            QWMatrix m;
            m.rotate(90);
            p.setWorldMatrix(m);
            rc = QRect(8 + pict.height(), -4, height() - 4, 4 - width());
        }else{
            p.drawPixmap(4, (height()  - pict.height()) / 2, pict);
            rc = QRect(8 + pict.width(), 4, width() - 4, height() - 4);
        }
    }
    const QColorGroup &cg = isEnabled() ? palette().active() : palette().disabled();
    p.setPen(cg.text());
    p.drawText(rc, AlignLeft | AlignVCenter | ShowPrefix, text);
    p.end();
    p.begin(this);
    p.drawPixmap(0, 0, pict);
    p.end();
}

PictPushButton::PictPushButton( QWidget *parent, const char *name)
        : QPushButton(parent, name)
{
    connect(pMain, SIGNAL(iconChanged()), this, SLOT(iconChanged()));
}

void PictPushButton::iconChanged()
{
    repaint();
}

QSize PictPushButton::minimumSizeHint() const
{
    QSize p = QPushButton:: minimumSizeHint();
    if (icon.length()) p.setWidth(p.width() + Pict(icon).width() + 4);
    p.setWidth(p.width() + QFontMetrics(font()).width(text));
    return p;
}

QSize PictPushButton::sizeHint() const
{
    QSize p = QPushButton:: sizeHint();
    if (icon.length()) p.setWidth(p.width() + Pict(icon).width() + 4);
    p.setWidth(p.width() + QFontMetrics(font()).width(text));
    return p;
}

void PictPushButton::setState(const QString& _icon, const QString& _text)
{
    icon = _icon;
    text = _text;
    if (layout()) layout()->invalidate();
    repaint();
}

void PictPushButton::paintEvent(QPaintEvent *e)
{
    QPushButton::paintEvent(e);
    QPainter p(this);
    QRect rc(4, 0, width() - 8, height());
    if (icon.length()){
        const QPixmap &pict = Pict(icon);
        p.drawPixmap(4, (height()  - pict.height()) / 2, pict);
        rc.setLeft(4 + pict.width());
    }
    p.drawText(rc, AlignHCenter | AlignVCenter, text);
}

#ifndef _WINDOWS
#include "toolbtn.moc"
#endif
