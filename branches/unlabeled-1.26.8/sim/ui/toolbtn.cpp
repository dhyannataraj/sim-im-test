/***************************************************************************
                          toolbtn.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#include "toolbtn.h"

#include <qpainter.h>
#include <qtimer.h>
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
#include <qobjectlist.h>
#include <qlayout.h>
#include <qobjectlist.h>

class ButtonsMap : public map<unsigned, CToolItem*>
{
public:
    ButtonsMap() {}
    CToolItem *remove(unsigned id);
    void add(unsigned id, CToolItem *w);
};

CToolItem *ButtonsMap::remove(unsigned id)
{
    iterator it = find(id);
    if (it == end())
        return NULL;
    CToolItem *res = (*it).second;
    erase(it);
    return res;
}

void ButtonsMap::add(unsigned id, CToolItem *w)
{
    insert(value_type(id, w));
}

CToolItem::CToolItem(CommandDef *def)
{
    m_def = *def;
    if (def->text_wrk){
        m_text = QString::fromUtf8(def->text_wrk);
        free(def->text_wrk);
        def->text_wrk = NULL;
    }
}

void CToolItem::setCommand(CommandDef *def)
{
    if (def->text_wrk){
        m_text = QString::fromUtf8(def->text_wrk);
        free(def->text_wrk);
        def->text_wrk = NULL;
    }else{
        m_text = "";
    }
    def->bar_id  = m_def.bar_id;
    def->bar_grp = m_def.bar_grp;
    m_def = *def;
    setState();
}

void CToolItem::setChecked(CommandDef *def)
{
    m_def.flags &= ~COMMAND_CHECKED;
    m_def.flags |= (def->flags & COMMAND_CHECKED);
    setState();
}

void CToolItem::setDisabled(CommandDef *def)
{
    m_def.flags &= ~COMMAND_DISABLED;
    m_def.flags |= (def->flags & COMMAND_DISABLED);
    setState();
}

void CToolItem::setShow(CommandDef *def)
{
    m_def.flags &= ~BTN_HIDE;
    m_def.flags |= (def->flags & BTN_HIDE);
    setState();
}

void CToolItem::setState()
{
    if (m_def.flags & BTN_HIDE){
        widget()->hide();
    }else if (!widget()->isVisible()){
        widget()->show();
        widget()->setEnabled((m_def.flags & COMMAND_DISABLED) == 0);
    }
    widget()->setEnabled((m_def.flags & COMMAND_DISABLED) == 0);
}

void CToolItem::checkState()
{
    if (m_def.flags & COMMAND_CHECK_STATE){
        m_def.param = static_cast<CToolBar*>(widget()->parent())->param();
        Event e(EventCheckState, &m_def);
        e.process();
        m_def.flags |= COMMAND_CHECK_STATE;
        setState();
    }
}

CommandDef *CToolItem::def()
{
    return &m_def;
}

CToolButton::CToolButton (QWidget * parent, CommandDef *def)
        : QToolButton( parent), CToolItem(def)
{
    accelKey = 0;
    connect(this, SIGNAL(clicked()), this, SLOT(btnClicked()));
    connect(this, SIGNAL(toggled(bool)), this, SLOT(btnToggled(bool)));
    accel = NULL;
    if (def->accel){
        accel = new QAccel(this);
        accel->insertItem(QAccel::stringToKey(def->accel));
        connect(accel, SIGNAL(activated(int)), this, SLOT(accelActivated(int)));
    }
    setState();
}

CToolButton::~CToolButton()
{
    emit buttonDestroyed();
}

void CToolButton::setTextLabel(const QString &text)
{
    m_text = text;
}

void CToolButton::setTextLabel()
{
    QString text = m_text;
    if (text.isEmpty()) {
        if (m_def.text && *m_def.text)
            text = i18n(m_def.text);
    }
    int key = QAccel::shortcutKey(text);
    setAccel(key);
    QString t = text;
    int pos = t.find("<br>");
    if (pos >= 0) t = t.left(pos);
    QToolButton::setTextLabel(t);
    t = text;
    while ((pos = t.find('&')) >= 0){
        t = t.left(pos) + "<u>" + t.mid(pos+1, 1) + "</u>" + t.mid(pos+2);
    }
    QToolTip::add(this, t);
}

void CToolButton::setState()
{
    setTextLabel();
    if (m_def.icon_on){
        setToggleButton(true);
        setOn((m_def.flags & COMMAND_CHECKED) != 0);
    }
    if (m_def.icon_on && strcmp(m_def.icon, m_def.icon_on)){
        const QIconSet *offIcon = Icon(m_def.icon);
        if (offIcon){
#if COMPAT_QT_VERSION < 0x030000
            setIconSet(*offIcon);
            const QIconSet *onIcon = Icon(m_def.icon_on);
            if (onIcon)
                setOnIconSet(*onIcon);
#else
            QIconSet icons = *offIcon;
            icons.setPixmap(Pict(m_def.icon_on), QIconSet::Small, QIconSet::Normal, QIconSet::On);
            setIconSet(icons);
#endif
        }
    }else{
        const QIconSet *icon = Icon(m_def.icon);
        if (icon){
            QPixmap p = icon->pixmap(QIconSet::Small, QIconSet::Normal);
            setIconSet(*icon);
        }
    }
    CToolItem::setState();
}

void CToolButton::accelActivated(int)
{
    btnClicked();
}

void CToolButton::btnClicked()
{
    m_def.param = static_cast<CToolBar*>(parent())->param();
    if (m_def.popup_id){
        Event e(EventGetMenu, &m_def);
        QPopupMenu *popup = (QPopupMenu*)(e.process());
        if (popup){
            QPoint pos = popupPos(popup);
            popup->popup(pos);
        }
        return;
    }
    if (isToggleButton())
        return;
    Event e(EventCommandExec, &m_def);
    e.process();
}

void CToolButton::btnToggled(bool state)
{
    m_def.param = static_cast<CToolBar*>(parent())->param();
    if (!isToggleButton())
        return;
    if (state){
        m_def.flags |= COMMAND_CHECKED;
    }else{
        m_def.flags &= ~COMMAND_CHECKED;
    }
    Event e(EventCommandExec, &m_def);
    e.process();
}

QPoint CToolButton::popupPos(QWidget *p)
{
    return popupPos(this, p);
}

QPoint CToolButton::popupPos(QWidget *btn, QWidget *p)
{
    QPoint pos;
    QToolBar *bar = NULL;
    for (QWidget *pw = btn->parentWidget(); pw; pw = pw->parentWidget()){
        if (pw->inherits("QToolBar")){
            bar = static_cast<QToolBar*>(pw);
            break;
        }
    }
    QWidget *desktop = qApp->desktop();
    QSize s = p->sizeHint();
    s = p->sizeHint();
    if (bar){
        if  (bar->orientation() == Vertical){
            pos = QPoint(btn->width(), 0);
        }else{
            pos = QPoint(0, btn->height());
        }
    }else{
        pos = QPoint(btn->width() - s.width(), btn->height());
    }
    pos = btn->mapToGlobal(pos);
    if (pos.x() + s.width() > desktop->width()){
        pos.setX(desktop->width() - s.width());
        if (pos.x() < 0)
            pos.setX(0);
    }
    if (pos.y() + s.height() > desktop->height()){
        pos.setY(pos.y() - btn->height() - s.height());
        if (pos.y() < 0)
            pos.setY(0);
    }
    return pos;
}

void CToolButton::mousePressEvent(QMouseEvent *e)
{
#if COMPAT_QT_VERSION < 0x030000
    if (e->button() == RightButton){
        emit showPopup(e->globalPos());
        return;
    }
#endif
    QToolButton::mousePressEvent(e);
}

void CToolButton::contextMenuEvent(QContextMenuEvent *e)
{
    e->accept();
    emit showPopup(e->globalPos());
}

void CToolButton::setAccel(int key)
{
    accelKey = key;
    if (isVisible())
        QToolButton::setAccel(key);
}

void CToolButton::showEvent(QShowEvent *e)
{
    enableAccel(true);
    QToolButton::showEvent(e);
}

void CToolButton::hideEvent(QHideEvent *e)
{
    enableAccel(false);
    QToolButton::hideEvent(e);
}

void CToolButton::enableAccel(bool bState)
{
    if (accelKey == 0) return;
    QToolButton::setAccel(bState ? accelKey : 0);
    if (accel)
        accel->setEnabled(bState);
}

// ______________________________________________________________________________________


PictButton::PictButton(QToolBar *parent, CommandDef *def)
        : CToolButton(parent, def)
{
    setState();
}

PictButton::~PictButton()
{
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

void PictButton::setState()
{
    setIconSet(QIconSet());
    setTextLabel();
    CToolItem::setState();
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
#if COMPAT_QT_VERSION < 0x030000
    style().drawToolButton(this, &p);
#else
    drawButton(&p);
#endif
    int w = 4;
    QRect rc(4, 4, width() - 4, height() - 4);
    if (m_def.icon && strcmp(m_def.icon, "empty")){
        const QIconSet *icons = Icon(m_def.icon);
        if (icons){
            const QPixmap &pict = icons->pixmap(QIconSet::Small, isEnabled() ? QIconSet::Active : QIconSet::Disabled);
            QToolBar *bar = static_cast<QToolBar*>(parent());
            if (bar->orientation() == Vertical){
                p.drawPixmap((width() - pict.width()) / 2, 4, pict);
                QWMatrix m;
                m.rotate(90);
                p.setWorldMatrix(m);
                rc = QRect(8 + pict.height(), -4, height() - 4, 4 - width());
                w = pict.height() + 4;
            }else{
                p.drawPixmap(4, (height()  - pict.height()) / 2, pict);
                rc = QRect(8 + pict.width(), 4, width() - 4, height() - 4);
                w = pict.width() + 4;
            }
        }
    }else{
        QToolBar *bar = static_cast<QToolBar*>(parent());
        if (bar->orientation() == Vertical){
            QWMatrix m;
            m.rotate(90);
            p.setWorldMatrix(m);
            rc = QRect(4, -4, height() - 4, 4 - width());
        }else{
            rc = QRect(4, 4, width() - 4, height() - 4);
        }
    }
    const QColorGroup &cg = isEnabled() ? palette().active() : palette().disabled();
    p.setPen(cg.text());
    QString text = m_text;
    if (text.isEmpty())
        text = i18n(m_def.text);
    if ((m_def.flags & BTN_DIV) && (text.find(" | ") >= 0)){
        QStringList parts = QStringList::split(" | ", text);
        unsigned n;
        for (n = parts.count(); n > 0; n--){
            text = "";
            for (unsigned i = 0; i < n; i++){
                if (!text.isEmpty())
                    text += " ";
                text += parts[i];
            }
            QRect rcb(0, 0, qApp->desktop()->width(), qApp->desktop()->height());
            rcb = p.boundingRect(rcb, AlignLeft | ShowPrefix | SingleLine, text);
            if (rcb.width() + w < rc.width())
                break;
        }
    }
    p.drawText(rc, AlignLeft | AlignVCenter | ShowPrefix | SingleLine, text);
    p.end();
    p.begin(this);
    p.drawPixmap(0, 0, pict);
    p.end();
}

CToolCombo::CToolCombo(QToolBar* parent, CommandDef *def, bool bCheck)
        : QComboBox(parent), CToolItem(def)
{
    m_bCheck = bCheck;
    m_btn    = NULL;
    setEditable(true);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    if ((def->flags & BTN_NO_BUTTON) == 0){
        m_btn = new CToolButton(parent, def);
        connect(m_btn, SIGNAL(buttonDestroyed()), this, SLOT(btnDestroyed()));
        if (bCheck)
            connect(lineEdit(), SIGNAL(textChanged(const QString&)), this, SLOT(slotTextChanged(const QString&)));
    }
    setState();
    setMinimumSize(minimumSizeHint());
}

CToolCombo::~CToolCombo()
{
    if (m_btn)
        delete m_btn;
}

void CToolCombo::setText(const QString &str)
{
    lineEdit()->setText(str);
}

void CToolCombo::btnDestroyed()
{
    m_btn = NULL;
}

void CToolCombo::slotTextChanged(const QString &str)
{
    if (m_btn && m_bCheck)
        m_btn->setEnabled(!str.isEmpty());
}

void CToolCombo::setState()
{
    CToolItem::setState();
    if (m_def.text && *m_def.text){
        QString t = i18n(m_def.text);
        int pos;
        while ((pos = t.find('&')) >= 0)
            t = t.left(pos) + "<u>" + t.mid(pos+1, 1) + "</u>" + t.mid(pos+2);
        QToolTip::remove(this);
        QToolTip::add(this, t);
    }
    if (m_btn){
        *m_btn->def() = m_def;
        m_btn->setState();
        if (m_bCheck)
            m_btn->setEnabled(!lineEdit()->text().isEmpty());
    }
}

QSizePolicy CToolCombo::sizePolicy() const
{
    QSizePolicy p = QComboBox::sizePolicy();
    p.setHorData(QSizePolicy::Expanding);
    return p;
}

QSize CToolCombo::minimumSizeHint() const
{
    int wChar = QFontMetrics(font()).width('0');
    int w = wChar * 10 + 6;
    int h = 22;
    return QSize(w, h);
}

QSize CToolCombo::sizeHint() const
{
    int wChar = QFontMetrics(font()).width('0');
    int w = wChar * 15 + 6;
    int h = 22;
    return QSize(w, h);
}


CToolEdit::CToolEdit(QToolBar* parent, CommandDef *def)
        : QLineEdit(parent), CToolItem(def)
{
    m_btn = NULL;
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    if ((def->flags & BTN_NO_BUTTON) == 0){
        m_btn = new CToolButton(parent, def);
        connect(m_btn, SIGNAL(buttonDestroyed()), this, SLOT(btnDestroyed()));
    }
    setState();
}

CToolEdit::~CToolEdit()
{
    if (m_btn)
        delete m_btn;
}

void CToolEdit::btnDestroyed()
{
    m_btn = NULL;
}

void CToolEdit::setState()
{
    CToolItem::setState();
    if (m_btn){
        *m_btn->def() = m_def;
        m_btn->setState();
    }
}

CToolBar::CToolBar(CommandsDef *def, QMainWindow *parent)
        : QToolBar(parent), EventReceiver(LowPriority)
{
    m_def = def;
    setHorizontalStretchable(true);
    setVerticalStretchable(true);
    buttons = new ButtonsMap;
    bChanged = false;
    m_param = this;
    toolBarChanged();
}

CToolBar::~CToolBar()
{
    delete buttons;
}

void CToolBar::checkState()
{
    for (ButtonsMap::iterator it = buttons->begin(); it != buttons->end(); ++it){
        (*it).second->checkState();
    }
}

void CToolBar::mousePressEvent(QMouseEvent *e)
{
#if COMPAT_QT_VERSION < 0x030000
    if (e->button() == RightButton){
        showPopup(e->globalPos());
        return;
    }
#endif
    QToolBar::mousePressEvent(e);
}

void CToolBar::contextMenuEvent(QContextMenuEvent *e)
{
    e->accept();
    showPopup(e->globalPos());
}

void* CToolBar::processEvent(Event *e)
{
    CToolItem *button;
    CommandDef *cmd;
    ButtonsMap::iterator it;
    switch (e->type()){
    case EventToolbarChanged:
        if ((CommandsDef*)(e->param()) == m_def)
            toolBarChanged();
        break;
    case EventCommandRemove:
        button = buttons->remove((unsigned)(e->param()));
        if (button)
            delete button;
        break;
    case EventCommandWidget:
        cmd = (CommandDef*)(e->param());
        if ((cmd->param == NULL) || (cmd->param == m_param)){
            it = buttons->find(cmd->id);
            if (it != buttons->end())
                return (*it).second->widget();
        }
        return NULL;
    case EventLanguageChanged:
    case EventIconChanged:
        for (it = buttons->begin(); it != buttons->end(); ++it){
            (*it).second->setState();
        }
        return NULL;
    case EventCommandCreate:
        cmd = (CommandDef*)(e->param());
        if (cmd->bar_id == m_def->id())
            toolBarChanged();
        break;
    case EventCommandChange:
        cmd = (CommandDef*)(e->param());
        if ((cmd->param == NULL) || (cmd->param == m_param)){
            it = buttons->find(cmd->id);
            if (it != buttons->end())
                (*it).second->setCommand(cmd);
        }
        return NULL;
    case EventCommandChecked:
        cmd = (CommandDef*)(e->param());
        if ((cmd->param == NULL) || (cmd->param == m_param)){
            it = buttons->find(cmd->id);
            if (it != buttons->end())
                (*it).second->setChecked(cmd);
        }
        return NULL;
    case EventCommandDisabled:
        cmd = (CommandDef*)(e->param());
        if ((cmd->param == NULL) || (cmd->param == m_param)){
            it = buttons->find(cmd->id);
            if (it != buttons->end())
                (*it).second->setDisabled(cmd);
        }
        return NULL;
    case EventCommandShow:
        cmd = (CommandDef*)(e->param());
        if ((cmd->param == NULL) || (cmd->param == m_param)){
            it = buttons->find(cmd->id);
            if (it != buttons->end())
                (*it).second->setShow(cmd);
        }
        return NULL;
    }
    return NULL;
}

void CToolBar::toolBarChanged()
{
    if (bChanged)
        return;
    bChanged = true;
    for (ButtonsMap::iterator it = buttons->begin(); it != buttons->end(); ++it)
        m_def->set((*it).second->def());

    clear();
    buttons->clear();
    CommandsList list(*m_def);
    CommandDef *s;
    while ((s = ++list) != NULL){
        if (s->id == 0){
            addSeparator();
            continue;
        }
        s->text_wrk = NULL;
        CToolItem *btn = NULL;
        switch (s->flags & BTN_TYPE){
        case BTN_PICT:
            btn = new PictButton(this, s);
            connect(btn->widget(), SIGNAL(showPopup(QPoint)), this, SLOT(showPopup(QPoint)));
            break;
        case BTN_COMBO:
            btn = new CToolCombo(this, s, false);
            break;
        case BTN_COMBO_CHECK:
            btn = new CToolCombo(this, s, true);
            break;
        case BTN_EDIT:
            btn = new CToolEdit(this, s);
            break;
        case BTN_DEFAULT:
            btn = new CToolButton(this, s);
            connect(btn->widget(), SIGNAL(showPopup(QPoint)), this, SLOT(showPopup(QPoint)));
            break;
        default:
            log(L_WARN, "Unknown button type");
        }
        if (btn == NULL)
            continue;
        btn->checkState();
        buttons->add(s->id, btn);
    }
    bChanged = false;
    QTimer::singleShot(0, this, SLOT(checkState()));
}

void CToolBar::showPopup(QPoint p)
{
    QPopupMenu *popup = new QPopupMenu(this);
    popup->popup(p);
}

#ifndef _WINDOWS
#include "toolbtn.moc"
#endif
