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

MsgSMS::MsgSMS(MsgEdit *parent, Message *msg)
        : QObject(parent)
{
	m_edit     = parent;
    m_bExpand  = false;
	m_bCanSend = false;
    QString t  = msg->getPlainText();
    if (!t.isEmpty())
        m_edit->m_edit->setText(t);
    m_panel	= NULL;
    m_edit->m_edit->setTextFormat(PlainText);
	m_edit->m_edit->setReadOnly(false);
    Command cmd;
    cmd->id    = CmdPhoneNumber;
    cmd->param = m_edit;
    Event e(EventCommandWidget, cmd);
    CToolCombo *cmbPhone = (CToolCombo*)(e.process());
	if (cmbPhone)
		connect(cmbPhone->lineEdit(), SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(m_edit->m_edit, SIGNAL(textChanged()), this, SLOT(textChanged()));
    Contact *contact = getContacts()->contact(msg->contact());
    if (contact == NULL)
        return;
	if (cmbPhone){
    QString phones = contact->getPhones();
    while (phones.length()){
        QString phoneItem = getToken(phones, ';', false);
        phoneItem = getToken(phoneItem, '/', false);
        QString phone = getToken(phoneItem, ',');
        getToken(phoneItem, ',');
        if (phoneItem.toUInt() == CELLULAR)
            cmbPhone->insertItem(phone);
    }
    t = static_cast<SMSMessage*>(msg)->getPhone();
    if (!t.isEmpty())
        cmbPhone->setText(t);
	}
    textChanged();
	SMSUserData *data = (SMSUserData*)(contact->getUserData(CorePlugin::m_plugin->sms_data_id));
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
    Command cmd;
    cmd->id    = CmdPhoneNumber;
    cmd->param = m_edit;
    Event e(EventCommandWidget, cmd);
    CToolCombo *cmbPhone = (CToolCombo*)(e.process());
	if (cmbPhone && cmbPhone->lineEdit()->text().isEmpty()){
        cmbPhone->setFocus();
        return;
    }
    m_edit->m_edit->setFocus();
}

void MsgSMS::textChanged(const QString&)
{
    textChanged();
}

void MsgSMS::textChanged()
{
	QString phone;
    QString msgText = m_edit->m_edit->text();
    Command cmd;
    cmd->id    = CmdTranslit;
    cmd->param = m_edit;
    Event eBtn(EventCommandWidget, cmd);
    CToolButton *btnTranslit = (CToolButton*)(eBtn.process());
    if (btnTranslit && btnTranslit->isOn())
        msgText = toTranslit(msgText);
    cmd->id    = CmdPhoneNumber;
    cmd->param = m_edit;
    Event e(EventCommandWidget, cmd);
    CToolCombo *cmbPhone = (CToolCombo*)(e.process());
    if (cmbPhone)
		phone = cmbPhone->lineEdit()->text();
	bool bCanSend = !phone.isEmpty() || !msgText.isEmpty();
	if (bCanSend != m_bCanSend){
		m_bCanSend = bCanSend;
		cmd->id    = CmdSend;
		cmd->flags = m_bCanSend ? 0 : COMMAND_DISABLED;
		Event e(EventCommandDisabled, cmd);
		e.process();
	}
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
	if (e->type() == EventCheckState){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->param == m_edit){
			unsigned id = cmd->bar_grp;
			if ((id >= MIN_INPUT_BAR_ID) && (id < MAX_INPUT_BAR_ID)){
				cmd->flags |= BTN_HIDE;
				if (cmd->id == CmdPhoneNumber)
					cmd->flags &= ~BTN_HIDE;
				return e->param();
			}
			switch (cmd->id){
			case CmdTranslit:
			case CmdSmile:
			case CmdSend:
			case CmdSendClose:
				cmd->flags &= ~BTN_HIDE;
				return NULL;
			case CmdNextMessage:
			case CmdMsgAnswer:
				cmd->flags |= BTN_HIDE;
				return NULL;
			}
		}
	}
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
            QString msgText = m_edit->m_edit->text();
            QString phone;
			Command c;
    c->id    = CmdPhoneNumber;
    c->param = m_edit;
    Event eWidget(EventCommandWidget, c);
    CToolCombo *cmbPhone = (CToolCombo*)(eWidget.process());
    if (cmbPhone)
		phone = cmbPhone->lineEdit()->text();

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

