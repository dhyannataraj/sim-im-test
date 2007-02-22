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

#include <qaccel.h>
#include <qapplication.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qiconset.h>
#include <qlayout.h>
#include <qmainwindow.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qstyle.h>
#include <qtimer.h>
#include <qtoolbar.h>
#include <qtooltip.h>

#include "cmddef.h"
#include "log.h"
#include "icons.h"
#include "misc.h"

using namespace SIM;

/*****************************
 *  internal ButtonsMap      *
******************************/
class ButtonsMap : public std::map<unsigned, CToolItem*>
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

/*****************************
 *  CToolButton              *
******************************/
CToolItem::CToolItem(CommandDef *def)
{
    m_def = *def;
    m_text = def->text_wrk;
    def->text_wrk = QString::null;
}

void CToolItem::setCommand(CommandDef *def)
{
    m_text = def->text_wrk;
    def->text_wrk = QString::null;
    def->bar_id   = m_def.bar_id;
    def->bar_grp  = m_def.bar_grp;
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
        EventCheckState(&m_def).process();
        m_def.flags |= COMMAND_CHECK_STATE;
        setState();
    }
}

/*****************************
 *  CToolButton              *
******************************/
CToolButton::CToolButton (CToolBar *parent, CommandDef *def)
        : QToolButton(parent), CToolItem(def)
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
        QIconSet offIcon = Icon(m_def.icon);
        if (!offIcon.pixmap(QIconSet::Small, QIconSet::Normal).isNull()){
            QIconSet icons = offIcon;
            QPixmap off = Pict(m_def.icon_on);
            if (!off.isNull())
                icons.setPixmap(off, QIconSet::Small, QIconSet::Normal, QIconSet::On);
            setIconSet(icons);
        }
    }else{
        QIconSet icon = Icon(m_def.icon);
        if (!icon.pixmap(QIconSet::Small, QIconSet::Normal).isNull())
            setIconSet(icon);
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
        EventMenuGet e(&m_def);
        e.process();
        QPopupMenu *popup = e.menu();
        if (popup){
            QPoint pos = popupPos(popup);
            popup->popup(pos);
        }
        return;
    }
    if (isToggleButton())
        return;
    EventCommandExec(&m_def).process();
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
    EventCommandExec(&m_def).process();
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

/*****************************
 *  CToolPictButton          *
******************************/
CToolPictButton::CToolPictButton(CToolBar *parent, CommandDef *def)
        : CToolButton(parent, def)
{
    setState();
}

CToolPictButton::~CToolPictButton()
{
}

QSizePolicy CToolPictButton::sizePolicy() const
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

QSize CToolPictButton::minimumSizeHint() const
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

QSize CToolPictButton::sizeHint() const
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

void CToolPictButton::setState()
{
    setIconSet(QIconSet());
    setTextLabel();
    CToolItem::setState();
    repaint();
}

void CToolPictButton::paintEvent(QPaintEvent*)
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
    drawButton(&p);
    int w = 4;
    QRect rc(4, 4, width() - 4, height() - 4);
    if (m_def.icon && strcmp(m_def.icon, "empty")){
        QIconSet icons = Icon(m_def.icon);
        if (!icons.pixmap(QIconSet::Small, QIconSet::Normal).isNull()){
            const QPixmap &pict = icons.pixmap(QIconSet::Small, isEnabled() ? QIconSet::Active : QIconSet::Disabled);
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
            text = QString::null;
            for (unsigned i = 0; i < n; i++){
                if (!text.isEmpty())
                    text += ' ';
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

/*****************************
 *  CToolCombo               *
******************************/
CToolCombo::CToolCombo(CToolBar *parent, CommandDef *def, bool bCheck)
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
    if (!m_def.text.isEmpty()){
        QString t = i18n(m_def.text);
        int pos;
        while ((pos = t.find('&')) >= 0)
            t = t.left(pos) + "<u>" + t.mid(pos+1, 1) + "</u>" + t.mid(pos+2);
        QToolTip::remove(this);
        QToolTip::add(this, t);
    }
    if (m_btn){
        m_btn->setDef(m_def);
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


/*****************************
 *  CToolEdit                *
******************************/
CToolEdit::CToolEdit(CToolBar *parent, CommandDef *def)
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
        m_btn->setDef(m_def);
        m_btn->setState();
    }
}

QSizePolicy CToolEdit::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

QSize CToolEdit::minimumSizeHint() const
{
    return QLineEdit::minimumSizeHint();
}

QSize CToolEdit::sizeHint() const
{
    return QLineEdit::sizeHint();
}


/*****************************
 *  CToolLabel               *
******************************/
CToolLabel::CToolLabel(CToolBar *parent, CommandDef *def)
        : QLabel(parent), CToolItem(def)
{}

CToolLabel::~CToolLabel()
{}

QSizePolicy CToolLabel::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

QSize CToolLabel::minimumSizeHint() const
{
    return pixmap() ? pixmap()->size() : QSize();
}

QSize CToolLabel::sizeHint() const
{
    return pixmap() ? pixmap()->size() : QSize();
}

/*****************************
 *  CToolBar                 *
******************************/
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
    QToolBar::mousePressEvent(e);
}

void CToolBar::contextMenuEvent(QContextMenuEvent *e)
{
    e->accept();
    showPopup(e->globalPos());
}

bool CToolBar::processEvent(Event *e)
{
    switch (e->type()){
    case eEventToolbarChanged: {
        EventToolbarChanged *tc = static_cast<EventToolbarChanged*>(e);
        if (tc->defs() == m_def)
            toolBarChanged();
        break;
    }
    case eEventCommandRemove: {
        EventCommandRemove *ecr = static_cast<EventCommandRemove*>(e);
        delete  buttons->remove(ecr->id());
        break;
    }
    case eEventCommandWidget: {
        EventCommandWidget *ecw = static_cast<EventCommandWidget*>(e);
        CommandDef *cmd = ecw->cmd();
        if ((cmd->param == NULL) || (cmd->param == m_param)){
            ButtonsMap::iterator it = buttons->find(cmd->id);
            if (it != buttons->end())
                ecw->setWidget((*it).second->widget());
                return true;
        }
        return false;
    }
    case eEventLanguageChanged:
    case eEventIconChanged: {
        ButtonsMap::iterator it;
        for (it = buttons->begin(); it != buttons->end(); ++it){
            (*it).second->setState();
        }
        return false;
    }
    case eEventCommandCreate: {
        EventCommandCreate *ecc = static_cast<EventCommandCreate*>(e);
        CommandDef *cmd = ecc->cmd();
        if (cmd->bar_id == m_def->id())
            toolBarChanged();
        break;
    }
    case eEventCommandChange: {
        EventCommandChange *ecc = static_cast<EventCommandChange*>(e);
        CommandDef *cmd = ecc->cmd();
        if ((cmd->param == NULL) || (cmd->param == m_param)){
            ButtonsMap::iterator it = buttons->find(cmd->id);
            if (it != buttons->end())
                (*it).second->setCommand(cmd);
        }
        return false;
    }
    case eEventCommandChecked: {
        EventCommandChecked *ecc = static_cast<EventCommandChecked*>(e);
        CommandDef *cmd = ecc->cmd();
        if ((cmd->param == NULL) || (cmd->param == m_param)){
            ButtonsMap::iterator it = buttons->find(cmd->id);
            if (it != buttons->end())
                (*it).second->setChecked(cmd);
        }
        return false;
    }
    case eEventCommandDisabled: {
        EventCommandDisabled *ecd = static_cast<EventCommandDisabled*>(e);
        CommandDef *cmd = ecd->cmd();
        if ((cmd->param == NULL) || (cmd->param == m_param)){
            ButtonsMap::iterator it = buttons->find(cmd->id);
            if (it != buttons->end())
                (*it).second->setDisabled(cmd);
        }
        return false;
    }
    case eEventCommandShow: {
        EventCommandShow *ecs = static_cast<EventCommandShow*>(e);
        CommandDef *cmd = ecs->cmd();
        if ((cmd->param == NULL) || (cmd->param == m_param)){
            ButtonsMap::iterator it = buttons->find(cmd->id);
            if (it != buttons->end())
                (*it).second->setShow(cmd);
        }
        return false;
    }
    default:
        break;
    }
    return false;
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
        s->text_wrk = QString::null;
        CToolItem *btn = NULL;
        switch (s->flags & BTN_TYPE){
        case BTN_PICT:
            btn = new CToolPictButton(this, s);
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
        case BTN_LABEL:
            btn = new CToolLabel(this, s);
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

QSizePolicy CToolBar::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

#ifndef NO_MOC_INCLUDES
#include "toolbtn.moc"
#endif
