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
#include <qapplication.h>
#include <qcombobox.h>

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
    bCtrl = false;
    bProcessCtrl = false;
}

CToolButton::CToolButton( const QIconSet & pOff, const QPixmap & pOn, const QString & textLabel,
                          const QString & grouptext, QObject * receiver, const char * slot, QToolBar * parent,
                          const char * name)
        : QToolButton(pOff, textLabel, grouptext, receiver, slot, parent, name)
{
    popup = NULL;
    bCtrl = false;
    bProcessCtrl = false;
#if QT_VERSION < 300
    setOnIconSet(pOn);
#else
    QIconSet icon = pOff;
    icon.setPixmap(pOn, QIconSet::Small, QIconSet::Normal, QIconSet::On);
    setIconSet(icon);
#endif
    setTextLabel(textLabel);
}

CToolButton::CToolButton ( QWidget * parent, const char * name  )
        : QToolButton( parent, name)
{
    popup = NULL;
    bCtrl = false;
    bProcessCtrl = false;
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
    QPoint pos = popupPos(popup);
    popup->popup(pos);
}

QPoint CToolButton::popupPos(QWidget *p)
{
    QPoint pos;
    QToolBar *bar = static_cast<QToolBar*>(parent());
    if (bar->orientation() == Vertical){
        pos = QPoint(width(), 0);
    }else{
        pos = QPoint(0, height());
    }
    pos = mapToGlobal(pos);
    QWidget *desktop = qApp->desktop();
    QSize s = p->sizeHint();
    if (pos.x() + s.width() > desktop->width())
        pos.setX(desktop->width() - s.width());
    if (pos.y() + s.height() > desktop->height())
        pos.setY(pos.y() - height() - s.height());
    return pos;
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

void CToolButton::mouseReleaseEvent(QMouseEvent *e)
{
    bCtrl = (e->state() & ControlButton) != 0;
    QToolButton::mouseReleaseEvent(e);
    bCtrl = false;
}

void CToolButton::setTextLabel(const QString &text)
{
    int key = QAccel::shortcutKey(text);
    setAccel(key);
    if (bProcessCtrl){
        QAccel *a = new QAccel(this);
        key |= CTRL;
        string s;
        s = QAccel::keyToString(key).local8Bit();
        a->connectItem(a->insertItem(key), this, SLOT(ctrlClick()));
    }
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

void CToolButton::ctrlClick()
{
    if (isOn()){
        animateClick();
        return;
    }
    bCtrl = true;
    toggled(true);
    bCtrl = false;
}

PictButton::PictButton( QToolBar *parent)
        : CToolButton(parent)
{
    accelKey = 0;
    connect(pMain, SIGNAL(iconChanged()), this, SLOT(iconChanged()));
}

PictButton::~PictButton()
{
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

void CToolButton::setAccel(int key)
{
    accelKey = key;
    if (isVisible())
        QToolButton::setAccel(key);
}

void CToolButton::showEvent(QShowEvent *e)
{
    if (accelKey)
        QToolButton::setAccel(accelKey);
    QToolButton::showEvent(e);
}

void CToolButton::hideEvent(QHideEvent *e)
{
    if (accelKey)
        QToolButton::setAccel(0);
    QToolButton::hideEvent(e);
}

void PictButton::setState(const QString& _icon, const QString& _text)
{
    icon = _icon;
    text = _text;
    int accelKey = QAccel::shortcutKey(text);
    QString t = _text;
    int pos;
    while ((pos = t.find('&')) >= 0){
        t = t.left(pos) + "<u>" + t.mid(pos+1, 1) + "</u>" + t.mid(pos+2);
    }
    setTextLabel(t);
    setAccel(accelKey);
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

CToolCombo::CToolCombo(QToolBar *t, const QString &toolTip)
        : QComboBox(t)
{
    QToolTip::add(this, toolTip);
    setEditable(true);
}

QSizePolicy CToolCombo::sizePolicy() const
{
    QSizePolicy p = QComboBox::sizePolicy();
    QToolBar *bar = static_cast<QToolBar*>(parent());
    if (bar->orientation() == Vertical){
        p.setVerData(QSizePolicy::Expanding);
    }else{
        p.setHorData(QSizePolicy::Expanding);
    }
    return p;
}

#ifndef _WINDOWS
#include "toolbtn.moc"
#endif
