/***************************************************************************
                          msggen.cpp  -  description
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

#include "msggen.h"
#include "toolbtn.h"
#include "msgedit.h"
#include "textshow.h"
#include "userwnd.h"
#include "userlist.h"
#include "core.h"

#include <qtimer.h>
#include <qtoolbutton.h>
#include <qaccel.h>
#include <qtooltip.h>
#include <qlayout.h>

#ifdef USE_KDE
#include <kcolordialog.h>
#include <kfontdialog.h>
#else
#include <qcolordialog.h>
#include <qfontdialog.h>
#endif

static void set_button(QToolButton *btn, const char *icon, const char *label)
{
    btn->setIconSet(*Icon(icon));
    QString text = i18n(label);
    int key = QAccel::shortcutKey(text);
    btn->setAccel(key);
    QString t = text;
    int pos = t.find("<br>");
    if (pos >= 0) t = t.left(pos);
    btn->setTextLabel(t);
    t = text;
    while ((pos = t.find('&')) >= 0){
        t = t.left(pos) + "<u>" + t.mid(pos+1, 1) + "</u>" + t.mid(pos+2);
    }
    QToolTip::add(btn, t);
}

MsgGen::MsgGen(CToolCustom *parent, Message *msg)
        : QObject(parent)
{
    m_client = msg->client();
    m_edit = NULL;
    for (QWidget *p = parent->parentWidget(); p; p = p->parentWidget()){
        if (p->inherits("MsgEdit")){
            m_edit = static_cast<MsgEdit*>(p);
            break;
        }
    }
    connect(m_edit->m_userWnd, SIGNAL(multiplyChanged()), this, SLOT(textChanged()));
    parent->setText(i18n(" "));
    m_edit->m_edit->setTextFormat(RichText);
    QString text = msg->getRichText();
    if (!text.isEmpty()){
        m_edit->m_edit->setText(text);
        m_edit->m_edit->moveCursor(QTextEdit::MoveEnd, false);
        if ((msg->getBackground() != msg->getForeground()) && !CorePlugin::m_plugin->getOwnColors()){
            m_edit->m_edit->setBackground(msg->getBackground());
            m_edit->m_edit->setForeground(msg->getForeground());
        }
    }
    Command cmd;
    cmd->id    = CmdSend;
    cmd->param = m_edit;
    Event e(EventCommandWidget, cmd);
    btnSend = (QToolButton*)(e.process());

    QToolButton *btn = new ColorToolButton(parent, QColor(CorePlugin::m_plugin->getEditBackground()));
    set_button(btn, "bgcolor", I18N_NOOP("Bac&kground color"));
    connect(btn, SIGNAL(colorChanged(QColor)), this, SLOT(bgColorChanged(QColor)));
    parent->addWidget(btn);
    btn->show();

    btn = new ColorToolButton(parent, QColor(CorePlugin::m_plugin->getEditForeground()));
    set_button(btn, "fgcolor", I18N_NOOP("&Text color"));
    connect(btn, SIGNAL(colorChanged(QColor)), this, SLOT(fgColorChanged(QColor)));
    parent->addWidget(btn);
    btn->show();

    btnBold = new QToolButton(parent);
    set_button(btnBold, "text_bold", I18N_NOOP("&Bold"));
    btnBold->setToggleButton(true);
    connect(btnBold, SIGNAL(toggled(bool)), this, SLOT(toggleBold(bool)));
    parent->addWidget(btnBold);
    btnBold->show();

    btnItalic = new QToolButton(parent);
    set_button(btnItalic, "text_italic", I18N_NOOP("&Italic"));
    btnItalic->setToggleButton(true);
    connect(btnItalic, SIGNAL(toggled(bool)), this, SLOT(toggleItalic(bool)));
    parent->addWidget(btnItalic);
    btnItalic->show();

    btnUnderline = new QToolButton(parent);
    set_button(btnUnderline, "text_under", I18N_NOOP("&Underline"));
    btnUnderline->setToggleButton(true);
    connect(btnUnderline, SIGNAL(toggled(bool)), this, SLOT(toggleUnderline(bool)));
    parent->addWidget(btnUnderline);
    btnUnderline->show();

    btn = new QToolButton(parent);
    set_button(btn, "text", I18N_NOOP("Text &font"));
    connect(btn, SIGNAL(clicked()), this, SLOT(selectFont()));
    parent->addWidget(btn);
    btn->show();

    connect(m_edit->m_edit, SIGNAL(currentFontChanged(const QFont&)), this, SLOT(fontChanged(const QFont&)));
    connect(m_edit->m_edit, SIGNAL(textChanged()), this, SLOT(textChanged()));
    fontChanged(m_edit->m_edit->font());
    textChanged();
}

void MsgGen::init()
{
    m_edit->m_edit->setFocus();
}

void MsgGen::textChanged()
{
    if (btnSend == NULL)
        return;
    bool bEnable = !m_edit->m_edit->text().isEmpty();
    if (bEnable && m_edit->m_userWnd->m_list && m_edit->m_userWnd->m_list->selected.empty())
        bEnable = false;
    btnSend->setEnabled(bEnable);
}

void MsgGen::fontChanged(const QFont &f)
{
    btnBold->setOn(f.bold());
    btnItalic->setOn(f.italic());
    btnUnderline->setOn(f.underline());
}

void MsgGen::bgColorChanged(QColor c)
{
    m_edit->m_edit->setBackground(c);
    CorePlugin::m_plugin->setEditBackground(c.rgb());
}

void MsgGen::fgColorChanged(QColor c)
{
    m_edit->m_edit->setForeground(c);
    CorePlugin::m_plugin->setEditForeground(c.rgb());
}

void MsgGen::toggleBold(bool bState)
{
    m_edit->m_edit->setBold(bState);
    CorePlugin::m_plugin->editFont.setBold(bState);
}

void MsgGen::toggleItalic(bool bState)
{
    m_edit->m_edit->setItalic(bState);
    CorePlugin::m_plugin->editFont.setItalic(bState);
}

void MsgGen::toggleUnderline(bool bState)
{
    m_edit->m_edit->setUnderline(bState);
    CorePlugin::m_plugin->editFont.setUnderline(bState);
}

void MsgGen::selectFont()
{
#ifdef USE_KDE
    QFont f = m_edit->m_edit->font();
    if (KFontDialog::getFont(f, false, m_edit->topLevelWidget()) != KFontDialog::Accepted)
        return;
#else
    bool ok = false;
    QFont f = QFontDialog::getFont(&ok, m_edit->m_edit->font(), m_edit->topLevelWidget());
    if (!ok)
        return;
#endif
    m_edit->m_edit->setCurrentFont(f);
    CorePlugin::m_plugin->editFont = f;
}

void *MsgGen::processEvent(Event *e)
{
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if ((cmd->id == CmdSend) && (cmd->param == m_edit)){
            QString msgText = m_edit->m_edit->text();
            if (!msgText.isEmpty()){
                Message *msg = new Message;
                msg->setText(msgText);
                msg->setContact(m_edit->m_userWnd->id());
                msg->setClient(m_client.c_str());
                msg->setFlags(MESSAGE_RICHTEXT);
                msg->setForeground(m_edit->m_edit->foreground().rgb() & 0xFFFFFF);
                msg->setBackground(m_edit->m_edit->background().rgb() & 0xFFFFFF);
                msg->setFont(CorePlugin::m_plugin->getEditFont());
                m_edit->sendMessage(msg);
            }
            return e->param();
        }
    }
    return NULL;
}

ColorToolButton::ColorToolButton(QWidget *parent, QColor color)
        : QToolButton(parent)
{
    m_color = color;
    m_popup = NULL;
    connect(this, SIGNAL(clicked()), this, SLOT(btnClicked()));
}

void ColorToolButton::btnClicked()
{
    m_popup = new ColorPopup(this, m_color);
    connect(m_popup, SIGNAL(colorChanged(QColor)), this, SLOT(selectColor(QColor)));
    connect(m_popup, SIGNAL(colorCustom()), this, SLOT(selectCustom()));
    QPoint p = CToolButton::popupPos(this, m_popup);
    m_popup->move(p);
    m_popup->show();
}

void ColorToolButton::selectColor(QColor c)
{
    m_color = c;
    emit colorChanged(c);
    QTimer::singleShot(0, this, SLOT(closePopup()));
}

void ColorToolButton::closePopup()
{
    if (m_popup){
        delete m_popup;
        m_popup = NULL;
    }
}

void ColorToolButton::selectCustom()
{
#ifdef USE_KDE
    QColor c = m_color;
    if (KColorDialog::getColor(c, this) != KColorDialog::Accepted) return;
#else
    QColor c = QColorDialog::getColor(m_color, this);
    if (!c.isValid()) return;
#endif
    m_color = c;
    emit colorChanged(c);
}

static unsigned colors[16] =
    {
        0x000000,
        0xFF0000,
        0x00FF00,
        0x0000FF,
        0xFFFF00,
        0xFF00FF,
        0x00FFFF,
        0xFFFFFF,
        0x404040,
        0x800000,
        0x008000,
        0x000080,
        0x808000,
        0x800080,
        0x008080,
        0x808080
    };

const int CUSTOM_COLOR	= 100;

ColorPopup::ColorPopup(QWidget *popup, QColor color)
        : QFrame(popup, "colors", WType_Popup | WStyle_Customize | WStyle_Tool | WDestructiveClose)
{
    setFrameShape(PopupPanel);
    setFrameShadow(Sunken);
    QGridLayout *lay = new QGridLayout(this, 5, 4);
    lay->setMargin(4);
    lay->setSpacing(2);
    for (unsigned i = 0; i < 4; i++){
        for (unsigned j = 0; j < 4; j++){
            unsigned n = i*4+j;
            QWidget *w = new ColorLabel(this, QColor(colors[n]), n, "");
            connect(w, SIGNAL(selected(int)), this, SLOT(colorSelected(int)));
            lay->addWidget(w, i, j);
        }
    }
    QWidget *w = new ColorLabel(this, color, CUSTOM_COLOR, i18n("Custom"));
    lay->addMultiCellWidget(w, 5, 5, 0, 3);
    connect(w, SIGNAL(selected(int)), this, SLOT(colorSelected(int)));
    resize(minimumSizeHint());
}

void ColorPopup::colorSelected(int id)
{
    if (id == CUSTOM_COLOR){
        emit colorCustom();
    }else{
        emit colorChanged(QColor(colors[id]));
    }
}

ColorLabel::ColorLabel(QWidget *parent, QColor c, int id, const QString &text)
        : QLabel(parent)
{
    m_id = id;
    setText(text);
    setBackgroundColor(c);
    setAlignment(AlignHCenter | AlignVCenter);
    setFrameShape(StyledPanel);
    setFrameShadow(Sunken);
    setLineWidth(2);
}

void ColorLabel::mouseReleaseEvent(QMouseEvent*)
{
    emit selected(m_id);
}

QSize ColorLabel::sizeHint() const
{
    QSize s = QLabel::sizeHint();
    if (s.width() < s.height())
        s.setWidth(s.height());
    return s;
}

QSize ColorLabel::minimumSizeHint() const
{
    QSize s = QLabel::minimumSizeHint();
    if (s.width() < s.height())
        s.setWidth(s.height());
    return s;
}

#ifndef WIN32
#include "msggen.moc"
#endif

