/***************************************************************************
                          msgedit.cpp  -  description
                             -------------------
    begin                : Sat Mar 16 2002
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

#include "msgedit.h"
#include "msgview.h"
#include "editspell.h"
#include "mainwin.h"
#include "client.h"
#include "usertbl.h"
#include "icons.h"
#include "log.h"
#include "history.h"
#include "toolbtn.h"
#include "userbox.h"
#include "cuser.h"
#include "xml.h"
#include "userview.h"
#include "ui/editfile.h"
#include "ui/ballonmsg.h"

#include <qlayout.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qcolor.h>
#include <qapplication.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qregexp.h>
#include <qinputdialog.h>
#include <qpopupmenu.h>
#if USE_KDE
#include <kcolordialog.h>
#include <kfontdialog.h>
#else
#include <qcolordialog.h>
#include <qfontdialog.h>
#endif

#define DECLINE_WITHOUT_REASON	1
#define DECLINE_REASON_INPUT	2
#define DECLINE_REASON_BUSY	3
#define DECLINE_REASON_LATER	4

static QString reason_string(int i)
{
    switch (i){
    case DECLINE_WITHOUT_REASON:
        return i18n("Decline file without reason");
    case DECLINE_REASON_BUSY:
        return i18n("Sorry, I'm busy right now, and can not respond to your request");
    case DECLINE_REASON_LATER:
        return i18n("Sorry, I'm busy right now, but I'll be able to respond to you later");
    case DECLINE_REASON_INPUT:
        return i18n("Enter a decline reason");
    }
    return "";
}

MsgEdit::MsgEdit(QWidget *p, unsigned long uin)
        : QFrame(p),
        Uin(this, "Uin")
{
    Uin = uin;
    msg = NULL;
    tabId = -1;
    tab = NULL;
    sendEvent = NULL;
    mHistory = NULL;
    bMultiply = false;
    setWFlags(WDestructiveClose);
    QVBoxLayout *lay = new QVBoxLayout(this);
    boxSend = new QVGroupBox(this);
    lay->addWidget(boxSend);
    QFrame *frmHead = new QFrame(boxSend);
    QHBoxLayout *hLay = new QHBoxLayout(frmHead);
    btnBgColor = new CPushButton(frmHead);
    btnBgColor->setTip(i18n("Background color"));
    btnBgColor->setPixmap(Pict("bgcolor"));
    connect(btnBgColor, SIGNAL(clicked()), this, SLOT(setMsgBackgroundColor()));
    btnBgColor->hide();
    hLay->addWidget(btnBgColor);
    btnReply = new QPushButton(i18n("&Reply"), frmHead);
    btnReply->hide();
    hLay->addWidget(btnReply);
    connect(btnReply, SIGNAL(clicked()), this, SLOT(replyClick()));
    btnGrant = new QPushButton(i18n("&Grant"), frmHead);
    btnGrant->hide();
    hLay->addWidget(btnGrant);
    connect(btnGrant, SIGNAL(clicked()), this, SLOT(grantClick()));
    btnAccept = new QPushButton(i18n("&Accept"), frmHead);
    btnAccept->hide();
    hLay->addWidget(btnAccept);
    connect(btnAccept, SIGNAL(clicked()), this, SLOT(acceptMessage()));
    btnFgColor = new CPushButton(frmHead);
    btnFgColor->setTip(i18n("Text color"));
    btnFgColor->setPixmap(Pict("fgcolor"));
    connect(btnFgColor, SIGNAL(clicked()), this, SLOT(setMsgForegroundColor()));
    hLay->addSpacing(2);
    btnFgColor->hide();
    hLay->addWidget(btnFgColor);
    btnQuote = new QPushButton(i18n("&Quote"), frmHead);
    btnQuote->hide();
    hLay->addWidget(btnQuote);
    connect(btnQuote, SIGNAL(clicked()), this, SLOT(quoteClick()));
    btnRefuse = new QPushButton(i18n("&Refuse"), frmHead);
    btnRefuse->hide();
    hLay->addWidget(btnRefuse);
    declineMenu = new QPopupMenu(this);
    connect(btnRefuse, SIGNAL(clicked()), this, SLOT(refuseClick()));
    connect(declineMenu, SIGNAL(activated(int)), this, SLOT(declineMessage(int)));
    declineMenu->insertItem(reason_string(DECLINE_WITHOUT_REASON), DECLINE_WITHOUT_REASON);
    declineMenu->insertItem(reason_string(DECLINE_REASON_BUSY), DECLINE_REASON_BUSY);
    declineMenu->insertItem(reason_string(DECLINE_REASON_LATER), DECLINE_REASON_LATER);
    declineMenu->insertItem(reason_string(DECLINE_REASON_INPUT), DECLINE_REASON_INPUT);
    btnDecline = new QPushButton(i18n("&Decline"), frmHead);
    btnDecline->hide();
    btnDecline->setPopup(declineMenu);
    hLay->addWidget(btnDecline);
    btnBold = new CPushButton(frmHead);
    btnBold->setTip(i18n("Bold"));
    btnBold->setPixmap(Pict("text_bold"));
    btnBold->setToggleButton(true);
    connect(btnBold, SIGNAL(toggled(bool)), this, SLOT(setBold(bool)));
    hLay->addSpacing(2);
    btnBold->hide();
    hLay->addWidget(btnBold);
    btnForward = new QPushButton(i18n("&Forward"), frmHead);
    connect(btnForward, SIGNAL(clicked()), this, SLOT(forwardClick()));
    btnForward->hide();
    hLay->addWidget(btnForward);
    btnItalic = new CPushButton(frmHead);
    btnItalic->setTip(i18n("Italic"));
    btnItalic->setPixmap(Pict("text_italic"));
    btnItalic->setToggleButton(true);
    connect(btnItalic, SIGNAL(toggled(bool)), this, SLOT(setItalic(bool)));
    hLay->addSpacing(2);
    btnItalic->hide();
    hLay->addWidget(btnItalic);
    btnUnder = new CPushButton(frmHead);
    btnUnder->setTip(i18n("Underline"));
    btnUnder->setPixmap(Pict("text_under"));
    btnUnder->setToggleButton(true);
    connect(btnUnder, SIGNAL(toggled(bool)), this, SLOT(setUnder(bool)));
    hLay->addSpacing(2);
    btnUnder->hide();
    hLay->addWidget(btnUnder);
    btnFont = new CPushButton(frmHead);
    btnFont->setTip(i18n("Text font"));
    btnFont->setPixmap(Pict("text"));
    connect(btnFont, SIGNAL(clicked()), this, SLOT(setFont()));
    hLay->addSpacing(2);
    btnFont->hide();
    hLay->addWidget(btnFont);
#ifdef USE_SPELL
    btnSpell = new CPushButton(frmHead);
    btnSpell->setTip(i18n("Spell check"));
    btnSpell->setPixmap(Pict("spellcheck"));
    connect(btnSpell, SIGNAL(clicked()), this, SLOT(spell()));
    hLay->addSpacing(2);
    btnSpell->hide();
    hLay->addWidget(btnSpell);
#endif
    hLay->addSpacing(2);
    hLay->addStretch();
    chkClose = new QCheckBox(i18n("C&lose after send"), frmHead);
    chkClose->setChecked(pMain->CloseAfterSend());
    connect(chkClose, SIGNAL(toggled(bool)), this, SLOT(closeToggle(bool)));
    hLay->addWidget(chkClose);
    btnSend = new QPushButton(frmHead);
    connect(btnSend, SIGNAL(clicked()), this, SLOT(sendClick()));
    hLay->addWidget(btnSend);
    btnNext = new PictPushButton(frmHead);
    connect(btnNext, SIGNAL(clicked()), this, SLOT(nextClick()));
    btnNext->hide();
    hLay->addWidget(btnNext);
    phone = new QFrame(boxSend);
    phone->hide();
    QHBoxLayout *hlay = new QHBoxLayout(phone);
    QLabel *title = new QLabel(i18n("Phone:"), phone);
    hlay->addWidget(title);
    phoneEdit = new QComboBox(phone);
    phoneEdit->setEditable(true);
    phoneEdit->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    hlay->addWidget(phoneEdit);
    connect(phoneEdit, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    url   = new QFrame(boxSend);
    url->hide();
    hlay = new QHBoxLayout(url);
    title = new QLabel(i18n("URL:"), url);
    hlay->addWidget(title);
    urlEdit = new QLineEdit(url);
    hlay->addWidget(urlEdit);
    connect(urlEdit, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    file = new QFrame(boxSend);
    file->hide();
    hlay = new QHBoxLayout(file);
    title = new QLabel(i18n("File:"), file);
    hlay->addWidget(title);
    fileEdit = new EditFile(file);
    hlay->addWidget(fileEdit);
    connect(fileEdit, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    btnMultiply = new CPushButton(frmHead);
    btnMultiply->setTip(i18n("Multiply send"));
    btnMultiply->setPixmap(Pict("1rightarrow"));
    connect(btnMultiply, SIGNAL(clicked()), this, SLOT(toggleMultiply()));
    hLay->addSpacing(2);
    hLay->addWidget(btnMultiply);
    lblUsers = new QLabel(i18n("Drag users here"), boxSend);
    edit  = new EditSpell(this);
    edit->hide();
    lay->addWidget(edit);
    users = new UserTbl(this);
    users->hide();
    lay->addWidget(users);
    view  = new TextShow(this);
    view->hide();
    lay->addWidget(view);
    connect(edit, SIGNAL(textChanged()), this, SLOT(editTextChanged()));
    connect(edit, SIGNAL(currentFontChanged(const QFont&)), this, SLOT(editFontChanged(const QFont&)));
    connect(edit, SIGNAL(ctrlEnterPressed()), this, SLOT(sendClick()));
    connect(users, SIGNAL(changed()), this, SLOT(textChanged()));
    connect(pClient, SIGNAL(messageReceived(ICQMessage*)), this, SLOT(messageReceived(ICQMessage*)));
    connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    connect(pMain, SIGNAL(chatChanged()), this, SLOT(chatChanged()));
    connect(pMain, SIGNAL(ftChanged()), this, SLOT(ftChanged()));
    setState();
    setUin(uin);
}

MsgEdit::~MsgEdit()
{
    if (sendEvent) pClient->cancelMessage(sendEvent->message());
    if (msg && (msg->Id < MSG_PROCESS_ID))
        delete msg;
    if (mHistory) delete mHistory;
    emit destroyChild(tabId);
}

bool MsgEdit::load(std::istream &s, string &part)
{
    ConfigArray::load(s, part);
    setUin(Uin);
    ICQUser *u = pClient->getUser(Uin);
    if (u == NULL) return false;
    action(mnuAction);
    return true;
}

void MsgEdit::closeToggle(bool state)
{
    pMain->CloseAfterSend = state;
}

History *MsgEdit::history()
{
    if (mHistory) return mHistory;
    mHistory = new History(Uin);
    return mHistory;
}

void MsgEdit::setState()
{
    if (sendEvent)
        emit setStatus(i18n("Sending"));
    emit setSendState(sendEvent == NULL);
    unsigned short msgType = 0;
    if (message()){
        msgType = message()->Type();
        emit setMessageType(Client::getMessageIcon(msgType), Client::getMessageText(msgType));
    }
    btnSend->setText(sendEvent ? i18n("&Cancel") : i18n("&Send"));
    chkClose->setEnabled(sendEvent == NULL);
    phone->setEnabled(sendEvent == NULL);
    url->setEnabled(sendEvent == NULL);
    edit->setEnabled(sendEvent == NULL);
    users->setEnabled(sendEvent == NULL);
    file->setEnabled(sendEvent == NULL);
}

void MsgEdit::showMessage(unsigned long msgId)
{
    if (sendEvent) return;
    ICQMessage *showMsg = history()->getMessage(msgId);
    if (showMsg == NULL){
        log(L_WARN, "Message not found");
        return;
    }
    setMessage(showMsg, true);
    setState();
}

void MsgEdit::action(int type)
{
    action(type, false);
}

void MsgEdit::action(int type, bool bSaveEdit)
{
    if (type == mnuMail){
        pMain->sendMail(Uin);
        return;
    }
    if (sendEvent) return;
    if (Uin == 0)
        log(L_WARN, "Bad UIN for message create");
    if ((type == mnuAction) || (type == mnuActionInt)){
        if ((type == mnuAction) && canSend())
            return;
        ICQUser *u = pClient->getUser(Uin);
        if (u && u->unreadMsgs.size()){
            ICQMessage *msg = history()->getMessage(u->unreadMsgs.front());
            if (msg){
                setMessage(msg, true, true);
                setState();
                return;
            }
        }
        if (u && (u->Type == USER_TYPE_EXT)){
            PhoneBook::iterator it;
            for (it = u->Phones.begin(); it != u->Phones.end(); ++it){
                PhoneInfo *info = static_cast<PhoneInfo*>(*it);
                if (info->Type() == SMS){
                    type = mnuSMS;
                    break;
                }
            }
            if (type == mnuActionInt){
                pMain->sendMail(u->Uin);
                return;
            }
        }else{
            type = mnuMessage;
        }
    }
    ICQMessage *editMsg = NULL;
    switch (type){
    case mnuMessage:
        editMsg = new ICQMsg;
        editMsg->Uin.push_back(Uin);
        break;
    case mnuURL:
        editMsg = new ICQUrl;
        editMsg->Uin.push_back(Uin);
        break;
    case mnuSMS:
        editMsg = new ICQSMS;
        editMsg->Uin.push_back(Uin);
        break;
    case mnuAuth:
        editMsg = new ICQAuthRequest;
        editMsg->Uin.push_back(Uin);
        break;
    case mnuContacts:
        editMsg = new ICQContacts;
        editMsg->Uin.push_back(Uin);
        break;
    case mnuFile:
        editMsg = new ICQFile;
        editMsg->Uin.push_back(Uin);
        break;
    case mnuChat:
        editMsg = new ICQChat;
        editMsg->Uin.push_back(Uin);
        break;
    default:
        log(L_WARN, "Unknown message type: %u", type);
    }
    setMessage(editMsg, false, false, bSaveEdit);
    setState();
}

QString MsgEdit::userName()
{
    if (Uin() == 0) return "[New]";
    CUser user(Uin);
    return user.name();
}

void MsgEdit::fillPhones()
{
    phoneEdit->clear();
    ICQUser *u = pClient->getUser(Uin);
    if (u == NULL) return;
    for (PhoneBook::iterator it = u->Phones.begin(); it != u->Phones.end(); it++){
        PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
        if (phone->Type != SMS) continue;
        phoneEdit->insertItem(phone->getNumber().c_str());
        QString t = phoneEdit->lineEdit()->text();
        if (phoneEdit->lineEdit()->text().length() == 0)
            phoneEdit->lineEdit()->setText(QString::fromLocal8Bit(phone->getNumber().c_str()));
    }
}

void MsgEdit::processEvent(ICQEvent *e)
{
    ICQUser *u;
    switch (e->type()){
    case EVENT_ACKED:
        if (e->message() && (e->message() == message())){
            sendEvent = NULL;
            setMessage();
            if (bCloseSend){
                close();
            }else{
                action(mnuAction);
                switch (e->message()->Type()){
                case ICQ_MSGxFILE:
                    emit setStatus(i18n("Transfer started"), 2000);
                    break;
                case ICQ_MSGxCHAT:
                    emit setStatus(i18n("Chat started"), 2000);
                    break;
                }
            }
        }
        break;
    case EVENT_INFO_CHANGED:
        fillPhones();
        u = pClient->getUser(Uin);
        if (u && u->inIgnore())
            QTimer::singleShot(10, this, SLOT(close()));
        break;
    case EVENT_USER_DELETED:
        if (e->Uin() == Uin)
            close();
        break;
    }
    if (e->message() && (e->message() == message())){
        if (e->type() == EVENT_MESSAGE_SEND){
            if (e->state == ICQEvent::Success){
                history()->addMessage(message());
                if (bCloseSend){
                    close();
                }else{
                    emit addMessage(message(), false, true);
                    emit showMessage(Uin(), message()->Id);
                    setMessage();
                    action(mnuAction);
                    emit setStatus(i18n("Message sent"), 2000);
                }
            }else{
                e->message()->bDelete = false;
                emit setStatus(i18n("Send failed"), 2000);
                if (e->message() && *(e->message()->DeclineReason.c_str()))
                    BalloonMsg::message(QString::fromLocal8Bit(msg->DeclineReason.c_str()),
                                        btnNext);
            }
            bCloseSend = false;
            sendEvent = NULL;
            setState();
        }else if ((e->type() == EVENT_MESSAGE_RECEIVED) && (e->state == ICQEvent::Fail)){
            msg = NULL;
            setMessage();
            action(mnuAction);
        }
    }
}

void MsgEdit::closeEvent(QCloseEvent *e)
{
    if (sendEvent){
        if (!pClient->cancelMessage(sendEvent->message()))
            log(L_WARN, "Error cancel send");
        e->ignore();
        return;
    }
    QWidget::closeEvent(e);
}

void MsgEdit::realSend()
{
    sendEvent = pClient->sendMessage(message());
    setState();
}

void MsgEdit::setUin(unsigned long uin)
{
    Uin = uin;
    ICQUser *u = pClient->getUser(uin);
    if (u == NULL) return;
    fillPhones();
}

void MsgEdit::sendClick()
{
    if (sendEvent){
        pClient->cancelMessage(sendEvent->message());
        return;
    }
    bCloseSend = chkClose->isChecked();
    send();
}

void MsgEdit::markAsRead()
{
    if (msg == NULL) return;
    if (pMain->SimpleMode()){
        if (!msg->Received) return;
        pClient->markAsRead(msg);
    }else{
        ICQUser *u = pClient->getUser(Uin());
        if (u){
            History h(Uin());
            for (;;){
                bool bExit = true;
                for (list<unsigned long>::iterator it = u->unreadMsgs.begin(); it != u->unreadMsgs.end(); it++){
                    ICQMessage *msg = h.getMessage(*it);
                    if (msg == NULL) continue;
                    unsigned type = msg->Type();
                    if ((type == ICQ_MSGxMSG) || (type == ICQ_MSGxURL) || (type == ICQ_MSGxSMS)){
                        pClient->markAsRead(msg);
                        bExit = false;
                        break;
                    }
                }
                if (bExit) break;
            }
        }
    }
    setupNext();
}

void MsgEdit::messageReceived(ICQMessage *m)
{
    if (m->getUin() != Uin()) return;
    setupNext();
    if (msg && msg->Received && (static_cast<UserBox*>(topLevelWidget())->currentUser() == Uin)) return;
    if (canSend()) return;
    ICQMessage *msg_copy = history()->getMessage(m->Id);
    setMessage(msg_copy, false, true);
    if (qApp->activeWindow() != topLevelWidget()) return;
    if (static_cast<UserBox*>(topLevelWidget())->currentUser() != Uin) return;
    pClient->markAsRead(m);
    setupNext();
}

bool MsgEdit::canSpell()
{
    return (edit->isVisible() && edit->text().length());
}

bool MsgEdit::canSend()
{
    if (msg){
        if (bMultiply){
            UserBox *box = static_cast<UserBox*>(topLevelWidget());
            if (box->users == NULL) return false;
            UserView *users = box->users;
            if (!users->hasChecked()) return false;
        }
        switch (msg->Type()){
        case ICQ_MSGxMSG:
            return (edit->text().length());
        case ICQ_MSGxURL:
            return (urlEdit->text().length());
        case ICQ_MSGxFILE:
            return (fileEdit->text().length());
        case ICQ_MSGxSMS:
            return (edit->text().length() && phoneEdit->lineEdit()->text().length());
        case ICQ_MSGxCONTACTxLIST:
            return (!users->isEmpty());
        case ICQ_MSGxCHAT:
        case ICQ_MSGxAUTHxREFUSED:
        case ICQ_MSGxAUTHxGRANTED:
        case ICQ_MSGxAUTHxREQUEST:
            return true;
        }
    }
    return false;
}

void MsgEdit::setMsgForegroundColor()
{
    if ((msg == NULL) || (msg->Type() != ICQ_MSGxMSG)) return;
#ifdef USE_KDE
    QColor c = edit->foreground();
    if (KColorDialog::getColor(c, this) != KColorDialog::Accepted) return;
#else
    QColor c = QColorDialog::getColor(edit->foreground(), this);
    if (!c.isValid()) return;
#endif
    edit->setForeground(c);
}

void MsgEdit::setMsgBackgroundColor()
{
    if ((msg == NULL) || (msg->Type() != ICQ_MSGxMSG)) return;
#ifdef USE_KDE
    QColor c = edit->background();
    if (KColorDialog::getColor(c, this) != KColorDialog::Accepted) return;
#else
    QColor c = QColorDialog::getColor(edit->background(), this);
    if (!c.isValid()) return;
#endif
    (static_cast<ICQMsg*>(msg))->ForeColor = c.rgb();
    edit->setBackground(c);
}

void MsgEdit::setBold(bool bState)
{
    if ((msg == NULL) || (msg->Type() != ICQ_MSGxMSG)) return;
    edit->setBold(bState);
}

void MsgEdit::setItalic(bool bState)
{
    if ((msg == NULL) || (msg->Type() != ICQ_MSGxMSG)) return;
    edit->setItalic(bState);
}

void MsgEdit::setUnder(bool bState)
{
    if ((msg == NULL) || (msg->Type() != ICQ_MSGxMSG)) return;
    edit->setUnderline(bState);
}

void MsgEdit::setFont()
{
    if ((msg == NULL) || (msg->Type() != ICQ_MSGxMSG)) return;
#if USE_KDE
    QFont f = edit->font();
    if (KFontDialog::getFont(f, false, topLevelWidget()) == KFontDialog::Accepted)
        edit->setCurrentFont(f);
#else
    bool ok = false;
    QFont f = QFontDialog::getFont(&ok, edit->font(), topLevelWidget());
    if (ok) edit->setCurrentFont(f);
#endif
}

void MsgEdit::editFontChanged(const QFont &f)
{
    btnBold->setOn(f.bold());
    btnItalic->setOn(f.italic());
    btnUnder->setOn(f.underline());
}

static bool fromHex(char c, char &res)
{
    if ((c >= '0') && (c <= '9')){
        res |= (c - '0');
        return true;
    }
    if ((c >= 'a') && (c <= 'f')){
        res |= (c - 'a' + 10);
        return true;
    }
    if ((c >= 'A') && (c <= 'F')){
        res |= (c - 'A' + 10);
        return true;
    }
    return false;
}

void MsgEdit::setParam(unsigned long param)
{
    switch (msg->Type()){
    case ICQ_MSGxCONTACTxLIST:
        users->addUin(param);
        break;
    case ICQ_MSGxMSG:
        edit->setText(*((QString*)param));
        break;
    case ICQ_MSGxURL:
        urlEdit->setText(*((QString*)param));
        break;
    case ICQ_MSGxFILE:{
            string fileName;
            fileName = ((QString*)param)->local8Bit();
            string decodeName;
            for (unsigned i = 0; i < fileName.length(); i++){
                char p = fileName[i];
                switch (p){
                case '+':
                    decodeName += ' ';
                    break;
                case '%':
                    p = 0;
                    if (fromHex(fileName[i+1], p) && ((p = p << 4), fromHex(fileName[i+2], p))){
                        if (p == 0x7C) p = ':';
                        decodeName += p;
                        i += 2;
                    }else{
                        decodeName += '%';
                    }
                    break;
                default:
                    decodeName += p;
                }
            }
            QString name = QString::fromLocal8Bit(decodeName.c_str());
            if (name.left(5) == "file:"){
                name = name.mid(5);
#ifdef WIN32
                name.replace(QRegExp("^/+"), "");
#else
                name.replace(QRegExp("^/+"), "/");
#endif
            }
#ifdef WIN32
            name.replace(QRegExp("/"), "\\");
#endif
            fileEdit->setText(name);
            break;
        }
    }
}

void MsgEdit::setupNext()
{
    unsigned nUnread = 0;
    int msgType = 0;
    ICQUser *u = pClient->getUser(Uin);
    if (u && u->unreadMsgs.size()){
        nUnread = u->unreadMsgs.size();
        ICQMessage *msg = history()->getMessage(u->unreadMsgs.front());
        if (msg){
            msgType = msg->Type();
            if (msg->Id < MSG_PROCESS_ID)
                delete msg;
        }
    }
    if (nUnread){
        QString s;
        if (nUnread > 1) s.sprintf(" [%u]", nUnread);
        btnNext->setState(Client::getMessageIcon(msgType), i18n("Next") + s);
        return;
    }
    btnNext->setState("", i18n("New"));
}

void MsgEdit::nextClick()
{
    action(mnuActionInt);
}

void MsgEdit::replyClick()
{
    ICQMsg *msg = new ICQMsg;
    msg->Uin.push_back(Uin);
    setMessage(msg);
}

void MsgEdit::forwardClick()
{
    if (msg == NULL) return;
    QString msgText;
    if (msg && msg->Received()){
        switch (msg->Type()){
        case ICQ_MSGxMSG:
            msgText = QString::fromLocal8Bit(static_cast<ICQMsg*>(msg)->Message.c_str());
            break;
        case ICQ_MSGxURL:
            msgText = QString::fromLocal8Bit(static_cast<ICQUrl*>(msg)->Message.c_str());
            break;
        }
    }else{
        if (view->hasSelectedText()){
            msgText = view->selectedText();
        }else{
            msgText = view->text();
        }
    }
    msgText = QString::fromLocal8Bit(pClient->clearHTML(msgText.local8Bit()).c_str());
    QStringList l = QStringList::split('\n', msgText);
    CUser u(msg->getUin());
    msgText = i18n("%1 (ICQ# %2) wrote:<br>\n")
              .arg(u.name(true)) .arg(msg->getUin());
    msgText += l.join("<br>\n");
    msgText += "\n";
    ICQMessage *newMsg = NULL;
    switch (msg->Type()){
    case ICQ_MSGxURL:{
            ICQUrl *nMsg = new ICQUrl;
            nMsg->Uin.push_back(Uin);
            nMsg->Message = msgText.local8Bit();
            nMsg->URL = static_cast<ICQUrl*>(msg)->URL;
            newMsg = nMsg;
            break;
        }
    case ICQ_MSGxCONTACTxLIST:{
            ICQContacts *nMsg = new ICQContacts;
            nMsg->Uin.push_back(Uin);
            nMsg->Contacts = static_cast<ICQContacts*>(msg)->Contacts;
            newMsg = nMsg;
            break;
        }
    default:
        ICQMsg *nMsg = new ICQMsg;
        nMsg->Uin.push_back(Uin);
        nMsg->Message = msgText.local8Bit();
        newMsg = nMsg;
    }
    setMessage(newMsg);
    edit->moveCursor(QTextEdit::MoveEnd, false);
    bMultiply = true;
    btnMultiply->setPixmap(Pict("1leftarrow"));
    emit showUsers(true, 0);
    textChanged();
}

void MsgEdit::toggleMultiply()
{
    bMultiply = !bMultiply;
    btnMultiply->setPixmap(Pict(bMultiply ? "1leftarrow" : "1rightarrow"));
    emit showUsers(bMultiply, Uin);
}

void MsgEdit::quoteClick()
{
    QString msgText;
    if (msg && msg->Received()){
        switch (msg->Type()){
        case ICQ_MSGxMSG:
            msgText = QString::fromLocal8Bit(static_cast<ICQMsg*>(msg)->Message.c_str());
            break;
        case ICQ_MSGxURL:
            msgText = QString::fromLocal8Bit(static_cast<ICQUrl*>(msg)->Message.c_str());
            break;
        }
    }else{
        if (view->hasSelectedText()){
            msgText = view->selectedText();
        }else{
            msgText = view->text();
        }
    }
    msgText = QString::fromLocal8Bit(pClient->clearHTML(msgText.local8Bit()).c_str());
    QStringList l = QStringList::split('\n', msgText);
    QStringList::Iterator it;
    for (it = l.begin(); it != l.end(); it++){
        *it = QString(">") + *it;
    }
    msgText = l.join("<br>\n");
    msgText += "<br>\n";
    ICQMsg *msg = new ICQMsg;
    msg->Uin.push_back(Uin);
    msg->Message = msgText.local8Bit();
    setMessage(msg);
    edit->moveCursor(QTextEdit::MoveEnd, false);
}

void MsgEdit::grantClick()
{
    ICQMessage *msg = new ICQAuthGranted;
    msg->Uin.push_back(Uin);
    setMessage(msg);
    bCloseSend = false;
    send();
}

void MsgEdit::refuseClick()
{
    ICQMessage *msg = new ICQAuthRefused;
    msg->Uin.push_back(Uin);
    setMessage(msg);
}

void MsgEdit::setMessage(ICQMessage *_msg, bool bMark, bool bInTop, bool bSaveEdit)
{
    setUpdatesEnabled(false);
    if (msg != _msg){
        if (msg && (msg->Id < MSG_PROCESS_ID)) delete msg;
        msg = _msg;
    }
    if (bMultiply) toggleMultiply();
    if (msg == NULL){
        edit->setText("");
        edit->resetColors(false);
        urlEdit->setText("");
        users->clear();
        url->hide();
        edit->hide();
        file->hide();
        lblUsers->hide();
        btnBgColor->hide();
        btnFgColor->hide();
        btnBold->hide();
        btnItalic->hide();
        btnUnder->hide();
        btnFont->hide();
#ifdef USE_SPELL
        btnSpell->hide();
#endif
        btnSend->hide();
        chkClose->hide();
        btnAccept->hide();
        btnDecline->hide();
        btnNext->show();
        btnReply->hide();
        btnQuote->hide();
        btnGrant->hide();
        btnRefuse->hide();
        btnMultiply->hide();
        btnForward->hide();
        users->hide();
        view->hide();
        setupNext();
        setUpdatesEnabled(true);
        repaint();
        return;
    }
    emit showMessage(Uin, msg->Id);
    if (msg->Received()){
        if (bMark) pClient->markAsRead(msg);
        phone->hide();
        url->hide();
        edit->hide();
        file->hide();
        lblUsers->hide();
        btnBgColor->hide();
        btnFgColor->hide();
        btnBold->hide();
        btnItalic->hide();
        btnUnder->hide();
        btnFont->hide();
#ifdef USE_SPELL
        btnSpell->hide();
#endif
        btnSend->hide();
        chkClose->hide();
        btnAccept->hide();
        btnDecline->hide();
        btnNext->show();
        btnMultiply->hide();
        setupNext();
        if (msg->Type() == ICQ_MSGxCONTACTxLIST){
            btnReply->hide();
            btnQuote->hide();
            btnGrant->hide();
            btnRefuse->hide();
            users->show();
            view->hide();
            btnForward->show();
            ICQContacts *m = static_cast<ICQContacts*>(msg);
            for (ContactList::iterator it = m->Contacts.begin(); it != m->Contacts.end(); it++){
                Contact *contact = static_cast<Contact*>(*it);
                users->addUser(contact->Uin, contact->Alias);
            }
            users->sender = false;
        }else{
            switch (msg->Type()){
            case ICQ_MSGxFILE:{
                    ICQFile *f = static_cast<ICQFile*>(msg);
                    btnReply->hide();
                    btnQuote->hide();
                    btnGrant->hide();
                    btnRefuse->hide();
                    if (f->Id >= MSG_PROCESS_ID){
                        btnAccept->show();
                        btnDecline->show();
                        file->show();
                        ICQUser *u = pClient->getUser(f->getUin());
                        if ((u == NULL) || !u->AcceptFileOverride()) u = pClient;
                        string path = u->AcceptFilePath.c_str();
                        if (*path.c_str() == 0)
                            pMain->buildFileName(path, "IncommingFiles/");
                        QString name = QString::fromLocal8Bit(path.c_str());
#ifdef WIN32
                        name.replace(QRegExp("/"), "\\");
                        if ((name.length() == 0) || (name[(int)(name.length() - 1)] != '\\'))
                            name += "\\";
#else
                        if ((name.length() == 0) || (name[(int)(name.length() - 1)] != '/'))
                            name += "/";
#endif
                        name += QString::fromLocal8Bit(f->Name.c_str());
                        fileEdit->setText(name);
                        fileEdit->setSaveMode(true);
                        ftChanged();
                    }else{
                        btnAccept->hide();
                        btnDecline->hide();
                    }
                    break;
                }
            case ICQ_MSGxAUTHxREQUEST:
                btnReply->hide();
                btnQuote->hide();
                btnGrant->show();
                btnRefuse->show();
                btnAccept->hide();
                btnDecline->hide();
                btnForward->hide();
                break;
            case ICQ_MSGxMSG:
            case ICQ_MSGxURL:
                if (bInTop && !pMain->SimpleMode()){
                    btnReply->hide();
                    btnQuote->show();
                    btnForward->show();
                    btnGrant->hide();
                    btnRefuse->hide();
                    btnAccept->hide();
                    btnDecline->hide();
                    users->hide();
                    view->hide();
                    edit->setTextFormat(RichText);
                    edit->setText("");
                    edit->show();
                    edit->resetColors(true);
                    textChanged();
                    setUpdatesEnabled(true);
                    edit->setFocus();
                    repaint();
                    return;
                }
                btnReply->show();
                btnQuote->show();
                btnGrant->hide();
                btnRefuse->hide();
                btnAccept->hide();
                btnDecline->hide();
                btnForward->show();
                break;
            case ICQ_MSGxCHAT:
                btnReply->hide();
                btnQuote->hide();
                btnGrant->hide();
                btnRefuse->hide();
                btnAccept->hide();
                btnDecline->hide();
                btnForward->hide();
                if (msg->Id >= MSG_PROCESS_ID){
                    btnAccept->show();
                    btnDecline->show();
                    chatChanged();
                }
                break;
            default:
                btnReply->hide();
                btnQuote->hide();
                btnGrant->hide();
                btnRefuse->hide();
                btnAccept->hide();
                btnDecline->hide();
                btnForward->hide();
            }
            users->hide();
            view->show();
            view->setText(view->makeMessageText(msg));
            if (msg->Type() == ICQ_MSGxMSG){
                ICQMsg *m = static_cast<ICQMsg*>(msg);
                if (m->BackColor() != m->ForeColor()){
                    view->setForeground(QColor(m->ForeColor));
                    view->setBackground(QColor(m->BackColor));
                }else{
                    view->resetColors();
                }
            }else{
                view->resetColors();
            }
        }
    }else{
        btnReply->hide();
        btnForward->hide();
        btnQuote->hide();
        btnGrant->hide();
        btnRefuse->hide();
        btnNext->hide();
        btnAccept->hide();
        btnDecline->hide();
        chkClose->show();
        btnSend->show();
        switch (msg->Type()){
        case ICQ_MSGxMSG:{
                phone->hide();
                url->hide();
                edit->show();
                users->hide();
                view->hide();
                file->hide();
                lblUsers->hide();
                btnBgColor->show();
                btnFgColor->show();
                btnBold->show();
                btnItalic->show();
                btnUnder->show();
                btnFont->show();
                btnMultiply->show();
#ifdef USE_SPELL
                btnSpell->show();
#endif
                if (!bSaveEdit){
                    edit->setTextFormat(RichText);
                    ICQMsg *m = static_cast<ICQMsg*>(msg);
                    edit->setText(QString::fromLocal8Bit(m->Message.c_str()));
                    if (m->BackColor() != m->ForeColor()){
                        edit->setBackground(QColor(m->BackColor));
                        edit->setForeground(QColor(m->ForeColor));
                    }else{
                        edit->resetColors(true);
                    }
                    edit->setFocus();
                }
                break;
            }
        case ICQ_MSGxURL:{
                phone->hide();
                url->show();
                edit->show();
                edit->setTextFormat(PlainText);
                users->hide();
                view->hide();
                file->hide();
                lblUsers->hide();
                btnBgColor->hide();
                btnFgColor->hide();
                btnBold->hide();
                btnItalic->hide();
                btnUnder->hide();
                btnFont->hide();
                btnMultiply->show();
#ifdef USE_SPELL
                btnSpell->show();
#endif
                ICQUrl *m = static_cast<ICQUrl*>(msg);
                edit->resetColors(false);
                edit->setText(QString::fromLocal8Bit(m->Message.c_str()));
                urlEdit->setText(QString::fromLocal8Bit(m->URL.c_str()));
                urlEdit->setFocus();
                break;
            }
        case ICQ_MSGxFILE:{
                phone->hide();
                url->hide();
                edit->show();
                edit->setTextFormat(PlainText);
                users->hide();
                view->hide();
                file->show();
                lblUsers->hide();
                btnBgColor->hide();
                btnFgColor->hide();
                btnBold->hide();
                btnItalic->hide();
                btnUnder->hide();
                btnFont->hide();
                btnMultiply->hide();
#ifdef USE_SPELL
                btnSpell->show();
#endif
                ICQFile *m = static_cast<ICQFile*>(msg);
                edit->resetColors(false);
                edit->setText(QString::fromLocal8Bit(m->Description.c_str()));
                fileEdit->setSaveMode(false);
                fileEdit->setText(QString::fromLocal8Bit(m->Name.c_str()));
                fileEdit->setFocus();
                break;
            }
        case ICQ_MSGxCHAT:{
                phone->hide();
                url->hide();
                edit->show();
                edit->setTextFormat(PlainText);
                users->hide();
                view->hide();
                file->hide();
                lblUsers->hide();
                btnBgColor->hide();
                btnFgColor->hide();
                btnBold->hide();
                btnItalic->hide();
                btnUnder->hide();
                btnFont->hide();
                btnMultiply->hide();
#ifdef USE_SPELL
                btnSpell->show();
#endif
                ICQChat *m = static_cast<ICQChat*>(msg);
                edit->resetColors(false);
                edit->setText(QString::fromLocal8Bit(m->Reason.c_str()));
                break;
            }
        case ICQ_MSGxSMS:{
                phone->show();
                url->hide();
                edit->show();
                edit->setTextFormat(RichText);
                users->hide();
                view->hide();
                file->hide();
                lblUsers->hide();
                btnBgColor->hide();
                btnFgColor->hide();
                btnBold->hide();
                btnItalic->hide();
                btnUnder->hide();
                btnFont->hide();
                btnMultiply->hide();
#ifdef USE_SPELL
                btnSpell->show();
#endif
                ICQSMS *m = static_cast<ICQSMS*>(msg);
                edit->resetColors(false);
                edit->setText(QString::fromLocal8Bit(pClient->clearHTML(m->Message.c_str()).c_str()));
                if (*m->Phone.c_str())
                    phoneEdit->lineEdit()->setText(QString::fromLocal8Bit(m->Phone.c_str()));
                phoneEdit->setFocus();
                break;
            }
        case ICQ_MSGxCONTACTxLIST:{
                phone->hide();
                url->hide();
                edit->hide();
                users->show();
                view->hide();
                file->hide();
                lblUsers->show();
                btnBgColor->hide();
                btnFgColor->hide();
                btnBold->hide();
                btnItalic->hide();
                btnUnder->hide();
                btnFont->hide();
                btnMultiply->show();
#ifdef USE_SPELL
                btnSpell->hide();
#endif
                ICQContacts *m = static_cast<ICQContacts*>(msg);
                for (ContactList::iterator it = m->Contacts.begin(); it != m->Contacts.end(); it++){
                    Contact *contact = static_cast<Contact*>(*it);
                    users->addUser(contact->Uin, contact->Alias);
                }
                users->sender = true;
                break;
            }
        case ICQ_MSGxAUTHxREQUEST:{
                phone->hide();
                url->hide();
                edit->show();
                edit->setTextFormat(RichText);
                users->hide();
                view->hide();
                file->hide();
                lblUsers->hide();
                btnBgColor->hide();
                btnFgColor->hide();
                btnBold->hide();
                btnItalic->hide();
                btnUnder->hide();
                btnFont->hide();
                btnMultiply->hide();
#ifdef USE_SPELL
                btnSpell->hide();
#endif
                ICQAuthRequest *m = static_cast<ICQAuthRequest*>(msg);
                edit->resetColors(false);
                edit->setText(QString::fromLocal8Bit(m->Message.c_str()));
                edit->setFocus();
                break;
            }
        case ICQ_MSGxAUTHxREFUSED:{
                phone->hide();
                url->hide();
                edit->show();
                edit->setTextFormat(RichText);
                users->hide();
                view->hide();
                file->hide();
                lblUsers->hide();
                btnBgColor->hide();
                btnFgColor->hide();
                btnBold->hide();
                btnItalic->hide();
                btnUnder->hide();
                btnFont->hide();
                btnMultiply->hide();
#ifdef USE_SPELL
                btnSpell->hide();
#endif
                ICQAuthRefused *m = static_cast<ICQAuthRefused*>(msg);
                edit->resetColors(false);
                edit->setText(QString::fromLocal8Bit(m->Message.c_str()));
                edit->setFocus();
                break;
            }
        case ICQ_MSGxAUTHxGRANTED:{
                phone->hide();
                url->hide();
                edit->hide();
                users->hide();
                view->show();
                file->hide();
                lblUsers->hide();
                btnBgColor->hide();
                btnFgColor->hide();
                btnBold->hide();
                btnItalic->hide();
                btnUnder->hide();
                btnFont->hide();
                btnMultiply->hide();
#ifdef USE_SPELL
                btnSpell->hide();
#endif
                break;
            }
        default:
            log(L_WARN, "Unknown message type %u", msg->Type());
        }
    }
    textChanged();
    setUpdatesEnabled(true);
    repaint();
}

void MsgEdit::editTextChanged()
{
    if (msg && msg->Received())
        action(mnuMessage, true);
    textChanged();
}

void MsgEdit::textChanged(const QString&)
{
    textChanged();
}

void MsgEdit::textChanged()
{
#if USE_SPELL
    btnSpell->setEnabled(canSpell());
#endif
    btnSend->setEnabled(canSend());
}

void MsgEdit::spell()
{
    edit->spell();
}

void MsgEdit::makeMessage()
{
    if (msg == NULL) return;
    switch (msg->Type()){
    case ICQ_MSGxMSG:{
            ICQMsg *m = static_cast<ICQMsg*>(msg);
            m->Message = edit->text().local8Bit();
            if (edit->colorChanged()){
                m->BackColor = (edit->background().rgb() & 0xFFFFFF);
                m->ForeColor = (edit->foreground().rgb() & 0xFFFFFF);
                pMain->MessageBgColor = m->BackColor();
                pMain->MessageFgColor = m->ForeColor();
            }
            break;
        }
    case ICQ_MSGxURL:{
            ICQUrl *m = static_cast<ICQUrl*>(msg);
            m->Message = edit->text().local8Bit();
            m->URL = urlEdit->text().local8Bit();
            break;
        }
    case ICQ_MSGxFILE:{
            ICQFile *m = static_cast<ICQFile*>(msg);
            m->Description = edit->text().local8Bit();
            m->Name = fileEdit->text().local8Bit();
            break;
        }
    case ICQ_MSGxCHAT:{
            ICQChat *m = static_cast<ICQChat*>(msg);
            m->Reason = edit->text().local8Bit();
            break;
        }
    case ICQ_MSGxSMS:{
            ICQSMS *m = static_cast<ICQSMS*>(msg);
            m->Message = edit->text().local8Bit();
            m->Phone = phoneEdit->lineEdit()->text().local8Bit();
            break;
        }
    case ICQ_MSGxCONTACTxLIST:{
            ICQContacts *m = static_cast<ICQContacts*>(msg);
            users->fillList(m->Contacts);
            break;
        }
    case ICQ_MSGxAUTHxREQUEST:{
            ICQAuthRequest *m = static_cast<ICQAuthRequest*>(msg);
            m->Message = edit->text().local8Bit();
            break;
        }
    case ICQ_MSGxAUTHxREFUSED:{
            ICQAuthRefused *m = static_cast<ICQAuthRefused*>(msg);
            m->Message = edit->text().local8Bit();
            break;
        }
    case ICQ_MSGxAUTHxGRANTED:
        break;
    default:
        log(L_WARN, "Bad message type %u", msg->Type());
        return;
    }
    if (bMultiply){
        msg->Uin.clear();
        UserBox *box = static_cast<UserBox*>(topLevelWidget());
        if (box->users){
            UserView *users = box->users;
            if (users) users->fillChecked(msg);
        }
    }
    realSend();
}

void MsgEdit::spellDone(bool bResult)
{
    disconnect(edit, SIGNAL(spellDone(bool)), this, SLOT(spellDone(bool)));
    if (bResult){
        makeMessage();
    }
}

void MsgEdit::send()
{
    if (msg->Type() == ICQ_MSGxSMS){
        ICQUser *u = pClient->getUser(Uin);
        if (u){
            string msgPhone;
            msgPhone = phoneEdit->lineEdit()->text().local8Bit();
            PhoneBook::iterator it;
            for (it = u->Phones.begin(); it != u->Phones.end(); it++){
                PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
                if (phone->Type != SMS) continue;
                if (phone->getNumber() == msgPhone) break;
            }
            if (it == u->Phones.end()){
                PhoneInfo *phone = new PhoneInfo;
                phone->setNumber(msgPhone.c_str(), SMS);
                phone->MyInfo = true;
                u->Phones.push_back(phone);
                ICQEvent e(EVENT_INFO_CHANGED, Uin);
                pClient->process_event(&e);
            }
        }
    }
    if (pMain->SpellOnSend){
        switch (msg->Type()){
        case ICQ_MSGxSMS:
        case ICQ_MSGxMSG:
        case ICQ_MSGxURL:
        case ICQ_MSGxFILE:
        case ICQ_MSGxCHAT:
            connect(edit, SIGNAL(spellDone(bool)), this, SLOT(spellDone(bool)));
            edit->spell();
            return;
        default:
            break;
        }
    }
    makeMessage();
}

void MsgEdit::acceptMessage()
{
    bCloseSend = false;
    if (msg->Type() == ICQ_MSGxFILE){
        ICQFile *f = static_cast<ICQFile*>(msg);
        f->localName = fileEdit->text();
    }
    pClient->acceptMessage(msg);
}

void MsgEdit::declineMessage(int action)
{
    QString reason;
    switch (action){
    case DECLINE_REASON_INPUT:{
            bool ok = false;
            reason = QInputDialog::getText(i18n("Decline reason"), i18n("Please, input decline reason:"),
                                           QLineEdit::Normal, QString::null, &ok, this);
            if (!ok) return;
            break;
        }
    case DECLINE_WITHOUT_REASON:
        break;
    default:
        reason = reason_string(action);
    }
    pClient->declineMessage(msg, reason.local8Bit());
}

void MsgEdit::chatChanged()
{
    if (msg && (msg->Type() == ICQ_MSGxCHAT) && msg->Received()){
        QWidget *chat = pMain->chatWindow(Uin());
        btnAccept->setEnabled(chat == NULL);
    }
}

void MsgEdit::ftChanged()
{
    if (msg && (msg->Type() == ICQ_MSGxFILE) && msg->Received()){
        ICQFile *f = static_cast<ICQFile*>(msg);
        QWidget *file = pMain->ftWindow(Uin(), f->shortName());
        btnAccept->setEnabled(file == NULL);
    }
}

#ifndef _WINDOWS
#include "msgedit.moc"
#endif
