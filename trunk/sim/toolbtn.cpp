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
#include "ui/toolsetup.h"

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

#include <vector>
using namespace std;

typedef struct ToolBarState
{
    QWidget		*button;
    QPopupMenu	*popup;
	bool		isOn;
	bool		disabled;
	bool		hidden;
} ToolBarState;

class ToolBarStates : public vector<ToolBarState>
{
public:
    ToolBarStates() {}
};

CPushButton::CPushButton(QWidget *parent, const char *name)
        : QPushButton(parent, name)
{
}

void CPushButton::setTip(const QString &_tip)
{
    QToolTip::add(this, _tip);
}

CToolButton::CToolButton (QWidget * parent, const char *name)
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

PictButton::PictButton(QToolBar *parent, const char *name)
        : CToolButton(parent, name)
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

CToolCombo::CToolCombo(QToolBar *t, const QString &toolTip, const char *name)
        : QComboBox(t, name)
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

CToolBar::CToolBar(const ToolBarDef *def, list<unsigned long> *active, QMainWindow *parent, QWidget *receiver)
        : QToolBar(parent)
{
    m_def = def;
    m_active = active;
    m_receiver = receiver;

    setHorizontalStretchable(true);
    setVerticalStretchable(true);

    states = new ToolBarStates;
    bool bFirst = true;
    int nButtons = 0;
	const ToolBarDef *d;
    for (d = m_def; ; d++){
        if (d->id == BTN_END_DEF){
            if (bFirst){
                bFirst = false;
            }else{
                break;
            }
        }
        if (d->id == BTN_SEPARATOR) continue;
        if (d->id > nButtons) nButtons = d->id;
    }

    for (; nButtons >= 0; nButtons--)
    {
        ToolBarState state;
        state.button = NULL;
        state.popup  = NULL;
		state.isOn = false;
		state.disabled = false;
		state.hidden = false;
        states->push_back(state);
    }
    for (d = m_def; ; d++){
        if (d->id == BTN_END_DEF){
            if (bFirst){
                bFirst = false;
            }else{
                break;
            }
        }
        if (d->id == BTN_SEPARATOR) continue;
        if (def->flags & BTN_HIDE)
			(*states)[d->id].hidden = true;
    }

    connect(pMain, SIGNAL(toolBarChanged(const ToolBarDef*)), this, SLOT(toolBarChanged(const ToolBarDef*)));
    toolBarChanged(def);
}

CToolBar::~CToolBar()
{
    delete states;
}

void CToolBar::toolBarChanged(const ToolBarDef *def)
{
    if (def != m_def) return;
    clear();
    for (int i = 0; i < states->size(); i++){
		if ((*states)[i].button == NULL) continue;
		ToolBarState &s = (*states)[i];
		QWidget *w = s.button;
		if (isButton(i)){
			CToolButton *btn = static_cast<CToolButton*>(w);
			s.isOn = btn->isOn();
		}
        s.button = NULL;
	}
    if (m_active->size() == 0){
        for (; def->id != BTN_END_DEF; def++)
            m_active->push_back(def->id);
    }

    for (list<unsigned long>::iterator it = m_active->begin(); it != m_active->end(); ++it){
        int id = *it;
        if (id == BTN_SEPARATOR){
            addSeparator();
            continue;
        }
        const ToolBarDef *def;
        for (def = m_def; def->id != BTN_END_DEF; def++)
            if (def->id == id) break;
        if (def->id == BTN_END_DEF){
            for (def++; def->id != BTN_END_DEF; def++)
                if (def->id == id) break;
            if (def->id == BTN_END_DEF) continue;
        }

        QWidget *w;
        if (def->flags & BTN_PICT){
            PictButton *btn = new PictButton(this, QString::number(def->id));
            w = btn;
            btn->setState(def->icon, i18n(def->text));
        }else if (def->flags & BTN_COMBO){
            CToolCombo *cmb = new CToolCombo(this, i18n(def->text), QString::number(def->id));
            w = cmb;
        }else{
            CToolButton *btn = new CToolButton(this, QString::number(def->id));
			w = btn;
            if (def->flags & BTN_CTRL) btn->bProcessCtrl = true;
            btn->setTextLabel(i18n(def->text));
            if (def->flags & BTN_TOGGLE)
                btn->setToggleButton(true);
            if (def->flags & BTN_TOGGLE_PICT){
                string s;
                s = def->icon;
                s += (def->flags & BTN_PICT_INVERT) ? "_on" : "_off";
                QIconSet offIcon = Icon(s.c_str());
                s = def->icon;
                s += (def->flags & BTN_PICT_INVERT) ? "_off" : "_on";
#if QT_VERSION < 300
                btn->setIconSet(offIcon);
                btn->setOnIconSet(Icon(s.c_str()));
#else
                offIcon.setPixmap(Pict(s.c_str()), QIconSet::Small, QIconSet::Normal, QIconSet::On);
                btn->setIconSet(offIcon);
#endif
            }else{
                btn->setIconSet(Icon(def->icon));
            }
			if ((*states)[i].isOn) btn->setOn(true);
        }
		ToolBarState &s = (*states)[id];
        s.button = w;
        if (s.popup) setPopup(id, s.popup);
        if (s.hidden) w->hide();
        if (s.disabled) w->setEnabled(false);
        if (def->slot){
            if (def->flags & BTN_COMBO){
                connect(w, SIGNAL(textChanged(const QString&) ), m_receiver, def->slot);
            }else if (def->flags & BTN_TOGGLE){
                connect(w, SIGNAL(toggled(bool)), m_receiver, def->slot);
            }else{
                connect(w, SIGNAL(clicked()), m_receiver, def->slot);
            }
        }
        if (def->popup_slot){
            connect(w, SIGNAL(showPopup(QPoint)), m_receiver, def->popup_slot);
        }else{
            connect(w, SIGNAL(showPopup(QPoint)), this, SLOT(showPopup(QPoint)));
        }
    }
}

bool CToolBar::isButton(int id)
{
    for (const ToolBarDef *d = m_def; d->id != BTN_END_DEF; d++){
        if (d->id != id) continue;
        return !(d->flags & BTN_COMBO);
    }
    return false;
}

bool CToolBar::isPictButton(int id)
{
    for (const ToolBarDef *d = m_def; d->id != BTN_END_DEF; d++){
        if (d->id != id) continue;
        return (d->flags & BTN_PICT);
    }
    return false;
}

bool CToolBar::isCombo(int id)
{
    for (const ToolBarDef *d = m_def; d->id != BTN_END_DEF; d++){
        if (d->id != id) continue;
        return (d->flags & BTN_COMBO);
    }
    return false;
}

QWidget *CToolBar::getWidget(int id)
{
    QObject *res = child(QString::number(id));
    if (res == NULL) return NULL;
    return static_cast<QWidget*>(res);
}

void CToolBar::setPopup(int id, QPopupMenu *popup)
{
    if (!isButton(id)) return;
    (*states)[id].popup = popup;
    QWidget *b = getWidget(id);
    if (b == NULL) return;
    CToolButton *btn = static_cast<CToolButton*>(b);
    btn->setPopup(popup);
    btn->setPopupDelay(0);
}

void CToolBar::setState(int id, const char *icon, const QString &text)
{
    if (!isPictButton(id)) return;
    QWidget *b = getWidget(id);
    if (b == NULL) return;
    PictButton *btn = static_cast<PictButton*>(b);
    btn->setState(icon, text);
}

void CToolBar::setEnabled(int id, bool bEnable)
{
    QWidget *b = getWidget(id);
	(*states)[id].disabled = !bEnable;
    if (b == NULL) return;
    b->setEnabled(bEnable);
}

void CToolBar::setIcon(int id, const char *icon)
{
    if (!isButton(id)) return;
    QWidget *b = getWidget(id);
    if (b == NULL) return;
    CToolButton *btn = static_cast<CToolButton*>(b);
    btn->setIcon(Pict(icon));
}

void CToolBar::show(int id)
{
    QWidget *b = getWidget(id);
	(*states)[id].hidden = false;
    if (b == NULL) return;
    b->show();
}

void CToolBar::hide(int id)
{
    QWidget *b = getWidget(id);
	(*states)[id].hidden = true;
    if (b == NULL) return;
    b->hide();
}

bool CToolBar::isVisible(int id)
{
    QWidget *b = getWidget(id);
    if (b == NULL) return !(*states)[id].hidden;
    return b->isVisible();
}

bool CToolBar::isEnabled(int id)
{
    QWidget *b = getWidget(id);
    if (b == NULL) return !(*states)[id].disabled;
    return b->isEnabled();
}

bool CToolBar::isOn(int id)
{
    if (!isButton(id)) return false;
    QWidget *b = getWidget(id);
    if (b == NULL) return (*states)[id].isOn;
    CToolButton *btn = static_cast<CToolButton*>(b);
    return btn->isOn();
}

bool CToolBar::isCtrl(int id)
{
    if (!isButton(id)) return false;
    QWidget *b = getWidget(id);
    if (b == NULL) return false;
    CToolButton *btn = static_cast<CToolButton*>(b);
    return btn->bCtrl;
}

void CToolBar::setOn(int id, bool bOn)
{
    if (!isButton(id)) return;
	(*states)[id].isOn = bOn;
    QWidget *b = getWidget(id);
    if (b == NULL) return;
    CToolButton *btn = static_cast<CToolButton*>(b);
    btn->setOn(bOn);
}

QPoint CToolBar::popupPos(int id, QWidget *popup)
{
    if (!isButton(id)) return QPoint();
    QWidget *b = getWidget(id);
    if (b == NULL) return QPoint();
    CToolButton *btn = static_cast<CToolButton*>(b);
    return btn->popupPos(popup);
}

void CToolBar::showPopup(QPoint p)
{
    QPopupMenu *popup = new QPopupMenu(this);
    popup->insertItem(i18n("Customize toolbar..."), 1);
    connect(popup, SIGNAL(activated(int)), this, SLOT(popupActivated(int)));
    popup->popup(p);
}

void CToolBar::popupActivated(int id)
{
    if (id == 1){
        ToolBarSetup::show(m_def, m_active);
        return;
    }
}

void CToolBar::save(const ToolBarDef *def, list<unsigned long> *active)
{
    list<unsigned long>::iterator it;
    for (it = active->begin(); it != active->end(); ++it, def++){
        if ((*it) != def->id) break;
    }
    if ((it == active->end()) && def->id == BTN_END_DEF)
        active->clear();
}

#ifndef _WINDOWS
#include "toolbtn.moc"
#endif
