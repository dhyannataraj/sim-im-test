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
#include <qregexp.h>

#ifdef USE_KDE
#include <kcolordialog.h>
#include <kfontdialog.h>
#else
#include <qcolordialog.h>
#include <qfontdialog.h>
#endif

static void set_button(QToolButton *btn, const char *icon, const char *label)
{
    btn->setAutoRaise(true);
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
    if (msg->getFlags() & MESSAGE_INSERT){
        QString text = msg->getPlainText();
        m_edit->m_edit->insert(text, false, true, true);
    }else{
        QString text = msg->getRichText();
        if (!text.isEmpty()){
            m_edit->m_edit->setText(text);
            m_edit->m_edit->moveCursor(QTextEdit::MoveEnd, false);
            if ((msg->getBackground() != msg->getForeground()) && !CorePlugin::m_plugin->getOwnColors()){
                m_edit->m_edit->setBackground(msg->getBackground());
                m_edit->m_edit->setForeground(msg->getForeground());
            }
        }
    }
    Command cmd;
    cmd->id    = CmdSend;
    cmd->param = m_edit;
    Event e(EventCommandWidget, cmd);
    btnSend = (QToolButton*)(e.process());

    btnBG = new ColorToolButton(parent, QColor(CorePlugin::m_plugin->getEditBackground()));
    set_button(btnBG, "bgcolor", I18N_NOOP("Bac&kground color"));
    connect(btnBG, SIGNAL(colorChanged(QColor)), this, SLOT(bgColorChanged(QColor)));
    parent->addWidget(btnBG);
    btnBG->show();

    btnFG = new ColorToolButton(parent, QColor(CorePlugin::m_plugin->getEditForeground()));
    set_button(btnFG, "fgcolor", I18N_NOOP("&Text color"));
    connect(btnFG, SIGNAL(colorChanged(QColor)), this, SLOT(fgColorChanged(QColor)));
    connect(btnFG, SIGNAL(aboutToShow()), this, SLOT(showFG()));
    parent->addWidget(btnFG);
    btnFG->show();

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

    QToolButton *btn = new QToolButton(parent);
    set_button(btn, "text", I18N_NOOP("Text &font"));
    connect(btn, SIGNAL(clicked()), this, SLOT(selectFont()));
    parent->addWidget(btn);
    btn->show();

    connect(m_edit->m_edit, SIGNAL(currentFontChanged(const QFont&)), this, SLOT(fontChanged(const QFont&)));
    connect(m_edit->m_edit, SIGNAL(textChanged()), this, SLOT(textChanged()));
    fontChanged(m_edit->m_edit->font());
    textChanged();
}

void MsgGen::showFG()
{
    btnFG->setColor(m_edit->m_edit->foreground());
}

void MsgGen::init()
{
    m_edit->m_edit->setFocus();
}

void MsgGen::textChanged()
{
    if (btnSend == NULL)
        return;
    // we need to unqoute this text...
    QString text = m_edit->m_edit->text();
    text = unquoteText(text);
    bool bEnable = !text.isEmpty();
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
    if (c.rgb() == m_edit->m_edit->background().rgb())
        return;
    m_edit->m_edit->setBackground(c);
    CorePlugin::m_plugin->setEditBackground(c.rgb());
    Event e(EventHistoryColors);
    e.process();
}

void MsgGen::fgColorChanged(QColor c)
{
    if (c.rgb() == m_edit->m_edit->foreground().rgb())
        return;
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

#ifndef WIN32
#include "msggen.moc"
#endif

