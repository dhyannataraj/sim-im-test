/***************************************************************************
                          msgsms.cpp  -  description
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

#include "msgsms.h"
#include "toolbtn.h"
#include "msgedit.h"
#include "userwnd.h"
#include "textshow.h"
#include "core.h"

#include <qtimer.h>
#include <qtoolbutton.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qcheckbox.h>

const unsigned MAX_SMS_LEN_LATIN1	= 160;
const unsigned MAX_SMS_LEN_UNICODE	= 70;

MsgSMS::MsgSMS(CToolCustom *parent, Message *msg)
        : QComboBox(parent)
{
    for (QWidget *p = parent->parentWidget(); p; p = p->parentWidget()){
        if (p->inherits("MsgEdit")){
            m_edit = static_cast<MsgEdit*>(p);
            break;
        }
    }

    m_bExpand = false;
    QString t = msg->getPlainText();
    if (!t.isEmpty())
        m_edit->m_edit->setText(t);
    m_panel	= NULL;
    setEditable(true);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    parent->addWidget(this);
    btnTranslit = new QToolButton(parent);
    btnTranslit->setIconSet(*Icon("translit"));
    btnTranslit->setTextLabel(i18n("Send in translit"));
    btnTranslit->setToggleButton(true);
    parent->addWidget(btnTranslit);
    btnTranslit->show();
    parent->setText(i18n("Phone:"));
    m_edit->m_edit->setTextFormat(PlainText);
    Command cmd;
    cmd->id    = CmdSend;
    cmd->param = m_edit;
    Event e(EventCommandWidget, cmd);
    btnSend = (QToolButton*)(e.process());
    connect(lineEdit(), SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(m_edit->m_edit, SIGNAL(textChanged()), this, SLOT(textChanged()));
    connect(btnTranslit, SIGNAL(toggled(bool)), this, SLOT(translitToggled(bool)));
    Contact *contact = getContacts()->contact(msg->contact());
    if (contact == NULL)
        return;
    SMSUserData *data = (SMSUserData*)(contact->getUserData(CorePlugin::m_plugin->sms_data_id));
    btnTranslit->setOn(data->SMSTranslit);
    QString phones = contact->getPhones();
    while (phones.length()){
        QString phoneItem = getToken(phones, ';', false);
        phoneItem = getToken(phoneItem, '/', false);
        QString phone = getToken(phoneItem, ',');
        getToken(phoneItem, ',');
        if (phoneItem.toUInt() == CELLULAR)
            insertItem(phone);
    }
    t = static_cast<SMSMessage*>(msg)->getPhone();
    if (!t.isEmpty())
        lineEdit()->setText(t);
    textChanged();
    if (contact->getTemporary()){
        m_panel = new SMSPanel(m_edit->m_frame);
        m_edit->m_layout->insertWidget(0, m_panel);
        connect(m_panel, SIGNAL(destroyed()), this, SLOT(panelDestroyed()));
        m_panel->show();
    }
    if (m_edit->m_edit->text().isEmpty()){
        TemplateExpand t;
        if (data->SMSSignatureBefore){
            t.tmpl = QString::fromUtf8(data->SMSSignatureBefore);
            t.contact  = contact;
            t.receiver = this;
            t.param    = NULL;
            Event eTmpl(EventTemplateExpand, &t);
            eTmpl.process();
        }else{
            m_bExpand = true;
            if (data->SMSSignatureAfter){
                t.tmpl = QString::fromUtf8(data->SMSSignatureAfter);
                t.contact = contact;
                t.receiver = this;
                t.param = NULL;
                Event eTmpl(EventTemplateExpand, &t);
                eTmpl.process();
            }
        }
    }
    show();
}

MsgSMS::~MsgSMS()
{
    if (m_panel)
        delete m_panel;
}

void MsgSMS::panelDestroyed()
{
    m_panel = NULL;
}

void MsgSMS::init()
{
    if (lineEdit()->text().isEmpty()){
        setFocus();
        return;
    }
    m_edit->m_edit->setFocus();
}

void MsgSMS::translitToggled(bool)
{
    textChanged();
}

void MsgSMS::textChanged(const QString&)
{
    textChanged();
}

void MsgSMS::textChanged()
{
    if (btnSend == NULL)
        return;
    QString msgText = m_edit->m_edit->text();
    if (btnTranslit->isOn())
        msgText = toTranslit(msgText);
    btnSend->setEnabled(!lineEdit()->text().isEmpty() && !msgText.isEmpty());
    unsigned size = msgText.length();
    unsigned max_size = MAX_SMS_LEN_UNICODE;
    if (isLatin(msgText))
        max_size = MAX_SMS_LEN_LATIN1;
    QString status = i18n("Size: %1 / Max. size: %2")
                     .arg(size) .arg(max_size);
    if (size > max_size){
        status += " ! ";
        status += i18n("Message will be split");
    }
    m_edit->m_userWnd->setStatus(status);
}

void *MsgSMS::processEvent(Event *e)
{
    if (e->type() == EventTemplateExpanded){
        TemplateExpand *t = (TemplateExpand*)(e->param());
        if (m_bExpand){
            m_edit->m_edit->append(t->tmpl);
        }else{
            m_edit->m_edit->setText(t->tmpl);
            m_edit->m_edit->moveCursor(QTextEdit::MoveEnd, false);
            m_bExpand = true;
            Contact *contact = getContacts()->contact(m_id);
            if (contact){
                SMSUserData *data = (SMSUserData*)(contact->getUserData(CorePlugin::m_plugin->sms_data_id));
                if (data->SMSSignatureAfter){
                    t->tmpl = QString::fromUtf8(data->SMSSignatureAfter);
                    Event eTmpl(EventTemplateExpand, t);
                    eTmpl.process();
                }
            }
        }
        return e->param();
    }
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if ((cmd->id == CmdSend) && (cmd->param == m_edit)){
            unsigned flags = 0;
            if (btnTranslit->isOn())
                flags |= MESSAGE_TRANSLIT;
            QString msgText = m_edit->m_edit->text();
            QString phone = lineEdit()->text();
            if (!msgText.isEmpty() && !phone.isEmpty()){
                SMSMessage *msg = new SMSMessage;
                msg->setText(msgText);
                msg->setFlags(flags);
                msg->setPhone(phone);
                msg->setContact(m_edit->m_userWnd->id());
                if (m_edit->sendMessage(msg)){
                    Contact *contact = getContacts()->contact(m_edit->m_userWnd->id());
                    if (contact){
                        if (contact->getTemporary()){
                            contact->setName(phone);
                            if (m_panel && m_panel->chkSave->isChecked()){
                                contact->setTemporary(0);
                                delete m_panel;
                            }
                            Event e(EventContactChanged, contact);
                            e.process();
                        }
                        SMSUserData *data = (SMSUserData*)(contact->userData.getUserData(CorePlugin::m_plugin->sms_data_id, true));
                        data->SMSTranslit = btnTranslit->isOn();
                        QString newPhones;
                        QString phones = contact->getPhones();
                        QString type = "Private Cellular";
                        QString src  = "-";
                        while (phones.length()){
                            QString phoneItem = getToken(phones, ';', false);
                            QString item = phoneItem;
                            QString phoneStr  = getToken(phoneItem, '/', false);
                            QString phone     = getToken(phoneStr, ',');
                            QString phoneType = getToken(phoneStr, ',');
                            if ((phone != msg->getPhone()) || (phoneStr.toUInt() != CELLULAR)){
                                if (!newPhones.isEmpty())
                                    newPhones += ";";
                                newPhones += item;
                                continue;
                            }
                            type = phoneType;
                            src  = phoneItem;
                        }
                        phone += ",";
                        phone += type;
                        phone += ",";
                        phone += QString::number(CELLULAR);
                        phone += "/";
                        phone += src;
                        if (!newPhones.isEmpty())
                            phone += ";";
                        newPhones = phone + newPhones;
                        if (contact->setPhones(newPhones)){
                            Event e(EventContactChanged, contact);
                            e.process();
                        }
                    }
                }
            }
            return e->param();
        }
    }
    return NULL;
}

SMSPanel::SMSPanel(QWidget *parent)
        : QFrame(parent)
{
    setMargin(3);
    QHBoxLayout *lay = new QHBoxLayout(this);
    chkSave = new QCheckBox(i18n("Save phone in contact list"), this);
    lay->addSpacing(7);
    lay->addWidget(chkSave);
}

SMSPanel::~SMSPanel()
{}

#ifndef WIN32
#include "msgsms.moc"
#endif

