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
#include "tmpl.h"
#include "xml.h"
#include "userview.h"
#include "ui/enable.h"
#include "ui/editfile.h"
#include "ui/ballonmsg.h"
#include "ui/wndcancel.h"

#include <stdio.h>
#include <qlayout.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
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
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qmainwindow.h>

#include <qtoolbar.h>

#ifdef USE_KDE
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

#define MAX_SMS_LEN_LATIN1	160
#define MAX_SMS_LEN_UNICODE	70

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

const int btnBgColor	= 1;
const int btnFgColor	= 2;
const int btnBold		= 3;
const int btnItalic		= 4;
const int btnUnder		= 5;
const int btnFont		= 6;
const int btnSmile		= 7;
const int btnGrant		= 8;
const int btnRefuse		= 9;
const int btnAccept		= 10;
const int btnDecline	= 11;
const int btnReply		= 12;
const int btnQuote		= 13;
const int btnForward	= 14;
#ifdef USE_SPELL
const int btnSpell		= 15;
#endif
const int btnCloseSend	= 16;
const int btnSend		= 17;
const int btnNext		= 18;
const int btnMultiply	= 19;

ToolBarDef msgEditToolBar[] =
    {
        { btnBgColor, "bgcolor", NULL, I18N_NOOP("Bac&kground color"), BTN_HIDE, SLOT(setMsgBackgroundColor()), NULL },
        { btnFgColor, "fgcolor", NULL, I18N_NOOP("&Text color"), BTN_HIDE, SLOT(setMsgForegroundColor()), NULL },
        { btnBold, "text_bold", NULL, I18N_NOOP("&Bold"), BTN_TOGGLE | BTN_HIDE, SLOT(setBold(bool)), NULL },
        { btnItalic, "text_italic", NULL, I18N_NOOP("&Italic"), BTN_TOGGLE | BTN_HIDE, SLOT(setItalic(bool)), NULL },
        { btnUnder, "text_under", NULL, I18N_NOOP("&Underline"), BTN_TOGGLE | BTN_HIDE, SLOT(setUnder(bool)), NULL },
        { btnFont, "text", NULL, I18N_NOOP("Text &font"), BTN_HIDE, SLOT(setFont()), NULL },
        { btnSmile, "smile0", NULL, I18N_NOOP("Inser&t smile"), BTN_HIDE, SLOT(insertSmile()), NULL },
        { btnGrant, "apply", NULL, I18N_NOOP("&Grant"), BTN_PICT | BTN_HIDE, SLOT(grantClick()), NULL },
        { btnRefuse, "cancel", NULL, I18N_NOOP("&Refuse"), BTN_PICT | BTN_HIDE, SLOT(refuseClick()), NULL },
        { btnAccept, "apply", NULL, I18N_NOOP("&Accept"), BTN_PICT | BTN_HIDE, SLOT(acceptMessage()), NULL },
        { btnDecline, "cancel", NULL, I18N_NOOP("&Decline"), BTN_PICT | BTN_HIDE, NULL, NULL },
        { btnReply, "mail_reply", NULL, I18N_NOOP("&Reply"), BTN_PICT | BTN_HIDE, SLOT(replyClick()), NULL },
        { btnQuote, "mail_replylist", NULL, I18N_NOOP("&Quote"), BTN_PICT | BTN_HIDE, SLOT(quoteClick()), NULL },
        { btnForward, "mail_forward", NULL, I18N_NOOP("&Forward"), BTN_PICT | BTN_HIDE, SLOT(forwardClick()), NULL },
#ifdef USE_SPELL
        { btnSpell, "spellcheck", NULL, I18N_NOOP("Spell &check"), BTN_HIDE, SLOT(spell()), NULL },
#endif
        { btnCloseSend, "fileclose", NULL, I18N_NOOP("C&lose after send"), BTN_TOGGLE | BTN_HIDE, SLOT(closeToggle(bool)), NULL },
        SEPARATOR,
        { btnSend, "mail_send", NULL, I18N_NOOP("&Send"), BTN_PICT, SLOT(sendClick()), NULL },
        { btnNext, "message", NULL, I18N_NOOP("&Next"), BTN_PICT | BTN_HIDE, SLOT(nextClick()), NULL },
        SEPARATOR,
        { btnMultiply, "1rightarrow", "1leftarrow", I18N_NOOP("&Multiply send"), BTN_TOGGLE, SLOT(toggleMultiply(bool)), NULL },
        END_DEF,
        END_DEF
    };

const ToolBarDef *pMsgEditToolBar = msgEditToolBar;

MsgEdit::MsgEdit(QWidget *p, unsigned long uin)
        : QSplitter(Vertical, p)
{
    bFirstShow = true;
    Uin = uin;
    EditHeight = 0;
    msg = NULL;
    tabId = -1;
    tab = NULL;
    sendEvent = NULL;
    mHistory = NULL;
    msgView = NULL;
    setWFlags(WDestructiveClose);
    wndEdit = new WMainWindow(this, "msgedit");
    connect(wndEdit, SIGNAL(heightChanged(int)), this, SLOT(heightChanged(int)));

    frmEdit = new QFrame(wndEdit);
    wndEdit->setCentralWidget(frmEdit);
    toolbar = new CToolBar(msgEditToolBar, &pMain->ToolBarMsg, wndEdit, this);
    toolbar->installEventFilter(this);
    QVBoxLayout *lay = new QVBoxLayout(frmEdit);
    tmpl = new Tmpl(this);

    declineMenu = new QPopupMenu(this);
    connect(declineMenu, SIGNAL(activated(int)), this, SLOT(declineMessage(int)));
    declineMenu->insertItem(reason_string(DECLINE_WITHOUT_REASON), DECLINE_WITHOUT_REASON);
    declineMenu->insertItem(reason_string(DECLINE_REASON_BUSY), DECLINE_REASON_BUSY);
    declineMenu->insertItem(reason_string(DECLINE_REASON_LATER), DECLINE_REASON_LATER);
    declineMenu->insertItem(reason_string(DECLINE_REASON_INPUT), DECLINE_REASON_INPUT);
    toolbar->setPopup(btnDecline, declineMenu);
    toolbar->setOn(btnCloseSend, pMain->CloseAfterSend);

    phone = new QHGroupBox(frmEdit);
    phone->hide();
    lay->addWidget(phone);
    QLabel *title = new QLabel(i18n("Phone:"), phone);
    phoneEdit = new QComboBox(phone);
    phoneEdit->setEditable(true);
    phoneEdit->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    connect(phoneEdit, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));

    url   = new QHGroupBox(frmEdit);
    url->hide();
    lay->addWidget(url);
    title = new QLabel(i18n("URL:"), url);
    urlEdit = new QLineEdit(url);
    connect(urlEdit, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));

    file = new QHGroupBox(frmEdit);
    file->hide();
    lay->addWidget(file);
    lblFile = new QLabel(i18n("File")+":", file);
    fileEdit = new EditFile(file);
    connect(fileEdit, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));

    lblUsers = new QHGroupBox(frmEdit);
    lblUsers->hide();
    lay->addWidget(lblUsers);
    new QLabel(i18n("Drag users here"), lblUsers);

    edit  = new EditSpell(frmEdit);
    //    edit->hide();
    lay->addWidget(edit);
    users = new UserTbl(frmEdit);
    users->hide();
    lay->addWidget(users);
    view  = new TextShow(frmEdit);
    view->hide();
    lay->addWidget(view);

    connect(edit, SIGNAL(textChanged()), this, SLOT(editTextChanged()));
    connect(edit, SIGNAL(currentFontChanged(const QFont&)), this, SLOT(editFontChanged(const QFont&)));
    connect(edit, SIGNAL(ctrlEnterPressed()), this, SLOT(sendClick()));
    connect(users, SIGNAL(changed()), this, SLOT(textChanged()));
    connect(topLevelWidget(), SIGNAL(messageReceived(ICQMessage*)), this, SLOT(messageReceived(ICQMessage*)));
    connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    connect(pMain, SIGNAL(chatChanged()), this, SLOT(chatChanged()));
    connect(pMain, SIGNAL(ftChanged()), this, SLOT(ftChanged()));
    setState();
    setUin(uin);
    connect(pMain, SIGNAL(modeChanged(bool)), this, SLOT(modeChanged(bool)));
    modeChanged(pMain->SimpleMode);
    wndCancel = new WndCancel(this);
}

MsgEdit::~MsgEdit()
{
    if (sendEvent) pClient->cancelMessage(sendEvent->message());
    if (msg && (msg->Id < MSG_PROCESS_ID))
        delete msg;
    if (mHistory) delete mHistory;
    Uin = 0;
    ICQUser *u = pClient->getUser(Uin);
    if (u && u->bIsTemp)
        pClient->deleteUser(u);
    emit destroyChild(tabId);
}

cfgParam MsgEdit_Params[] =
    {
        { "Uin", OFFSET_OF(MsgEdit, Uin), PARAM_ULONG, 0 },
        { "EditHeight", OFFSET_OF(MsgEdit, EditHeight), PARAM_USHORT, 0 },
        { "", 0, 0, 0 }
    };

bool MsgEdit::load(QFile &s, string &part)
{
    ::load(this, MsgEdit_Params, s, part);
    setUin(Uin);
    ICQUser *u = pClient->getUser(Uin);
    if (u == NULL) return false;
    adjustSplitter();
    action(mnuAction);
    return true;
}

void MsgEdit::save(QFile &s)
{
    EditHeight = wndEdit->height();
    ::save(this, MsgEdit_Params, s);
}

void MsgEdit::closeToggle(bool state)
{
    pMain->CloseAfterSend = state;
}

bool MsgEdit::isMultiply()
{
    return toolbar->isOn(btnMultiply);
}

SmileLabel::SmileLabel(int _id, QWidget *parent)
        : QLabel(parent)
{
    id = _id;
    char b[20];
    snprintf(b, sizeof(b), "smile%X", id);
    setPixmap(Pict(b));
}

void SmileLabel::mouseReleaseEvent(QMouseEvent*)
{
    emit clicked(id);
}

SmilePopup::SmilePopup(QWidget *popup)
        : QFrame(popup, "smile", WType_Popup | WStyle_Customize | WStyle_Tool | WDestructiveClose)
{
    setFrameShape(PopupPanel);
    setFrameShadow(Sunken);
    QGridLayout *lay = new QGridLayout(this, 4, 4);
    lay->setMargin(4);
    lay->setSpacing(2);
    for (unsigned i = 0; i < 4; i++){
        for (unsigned j = 0; j < 4; j++){
            QWidget *w = new SmileLabel(i*4+j, this);
            connect(w, SIGNAL(clicked(int)), this, SLOT(labelClicked(int)));
            lay->addWidget(w, i, j);
        }
    }
    resize(minimumSizeHint());
}

void SmilePopup::labelClicked(int id)
{
    insert(id);
    close();
}

void MsgEdit::insertSmile()
{
    SmilePopup *smile = new SmilePopup(this);
    connect(smile, SIGNAL(insert(int)), this, SLOT(insertSmile(int)));
    QPoint p = toolbar->popupPos(btnSmile, smile);
    smile->move(p);
    smile->show();
}

void MsgEdit::insertSmile(int id)
{
    QFont f = edit->font();
    QColor fgColor = edit->foreground();
    int p = edit->paragraphs();
    edit->append(QString("<img src=icon:smile") + (char)(id < 10 ? '0' + id : 'A' + id - 10) + ">");
    edit->moveCursor(QTextEdit::MoveEnd, false);
    if (p < edit->paragraphs()){
        edit->moveCursor(QTextEdit::MoveBackward, false);
        edit->moveCursor(QTextEdit::MoveBackward, true);
        edit->del();
        edit->moveCursor(QTextEdit::MoveEnd, false);
    }
    edit->setFont(f);
    edit->setForeground(fgColor);
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
        emit setMessageType(SIMClient::getMessageIcon(msgType), SIMClient::getMessageText(msgType, 1));
    }
    if (sendEvent){
        toolbar->setState(btnSend, "cancel", i18n("&Cancel"));
    }else{
        toolbar->setState(btnSend, "mail_send", i18n("&Send"));
    }
    toolbar->setEnabled(btnCloseSend, sendEvent == NULL);
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
    if (type == mnuActionAuto) type = mnuAction;
    if ((type == mnuAction) || (type == mnuActionInt)){
        if ((type == mnuAction) && canSend())
            return;
        ICQUser *u = pClient->getUser(Uin);
        if (u){
            ICQMessage *msg = NULL;
            if (u->unreadMsgs.size())
                msg = history()->getMessage(u->unreadMsgs.front());
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
                if (info->Type == SMS){
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
    if (Uin == 0) return "[New]";
    CUser user(Uin);
    return user.name();
}

void MsgEdit::fillPhones()
{
    QString phoneNumber = phoneEdit->lineEdit()->text();
    phoneEdit->clear();
    ICQUser *u = pClient->getUser(Uin);
    if (u == NULL) return;
    if (u->bIsTemp) return;
    for (PhoneBook::iterator it = u->Phones.begin(); it != u->Phones.end(); it++){
        PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
        if (phone->Type != SMS) continue;
        phoneEdit->insertItem(phone->getNumber().c_str());
        QString t = phoneEdit->lineEdit()->text();
        if (phoneNumber.isEmpty())
            phoneNumber = pClient->from8Bit(Uin, phone->getNumber());
    }
    string s;
    if (!phoneNumber.isEmpty()) s = phoneNumber.local8Bit();
    phoneEdit->lineEdit()->setText(phoneNumber);
}

static const char* translatedMsg[] =
    {
        I18N_NOOP("No files for transfer"),
        NULL
    };

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
        if (e->Uin() != Uin) break;
        fillPhones();
        u = pClient->getUser(Uin);
        if (!bInIgnore && u && u->IgnoreId)
            QTimer::singleShot(10, this, SLOT(close()));
        break;
    case EVENT_USER_DELETED:
        if (e->Uin() == Uin)
            QTimer::singleShot(10, this, SLOT(close()));
        break;
    }
    if (e->message() && (e->message() == message())){
        if (e->type() == EVENT_MESSAGE_SEND){
            if (e->state == ICQEvent::Success){
                if ((message()->Type() == ICQ_MSGxMSG) && (message()->Charset == "utf-8")){
                    ICQMsg *m = static_cast<ICQMsg*>(message());
                    QTextCodec *codec = pClient->codecForUser(Uin);
                    string msg_text = m->Message;
                    SIMClient::fromUTF(msg_text, codec->name());
                    SIMClient::toUTF(msg_text, codec->name());
                    if (msg_text == m->Message){
                        SIMClient::fromUTF(m->Message, codec->name());
                        m->Charset = codec->name();
                    }
                }
                for (list<unsigned long>::iterator it = message()->Uin.begin(); it != message()->Uin.end(); ++it){
                    if (*it == Uin){
                        history()->addMessage(message());
                        continue;
                    }
                    History h(*it);
                    h.addMessage(message());
                }
                if (!msgTail.isEmpty()){
                    if (msgView){
                        msgView->addMessage(message(), false, true);
                        msgView->setMessage(Uin, message()->Id);
                    }
                    if (e->message()->Type() == ICQ_MSGxSMS){
                        ICQSMS *m = new ICQSMS;
                        m->Uin.push_back(Uin);
                        m->Message = smsChunk();
                        m->Phone = pClient->to8Bit(Uin, phoneEdit->lineEdit()->text());
                        m->Charset = pClient->codecForUser(Uin)->name();
                        msg = m;
                        sendEvent = pClient->sendMessage(msg);
                        return;
                    }
                    log(L_WARN, "Bad type for chunked message");
                }
                if (bCloseSend){
                    sendEvent = NULL;
                    close();
                    return;
                }
                if (msgView){
                    msgView->addMessage(message(), false, true);
                    msgView->setMessage(Uin, message()->Id);
                }
                setMessage();
                sendEvent = NULL;
                action(mnuAction);
                emit setStatus(i18n("Message sent"), 2000);
            }else{
                e->message()->bDelete = false;
                emit setStatus(i18n("Send failed"), 2000);
                if (e->message() && *(e->message()->DeclineReason.c_str())){
                    QString declineReason;
                    const char **trMsg;
                    for (trMsg = translatedMsg; *trMsg; trMsg++){
                        if (msg->DeclineReason == *trMsg){
                            declineReason = i18n(*trMsg);
                        }
                    }
                    if (*trMsg == NULL)
                        declineReason = pClient->from8Bit(Uin, msg->DeclineReason.c_str(), msg->Charset.c_str());
                    BalloonMsg::message(declineReason, toolbar->getWidget(btnSend));
                }
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
    if (sendEvent) setState();
}

void MsgEdit::setUin(unsigned long uin)
{
    bInIgnore = false;
    Uin = uin;
    ICQUser *u = pClient->getUser(uin);
    if (u == NULL) return;
    bInIgnore = (u->IgnoreId != 0);
    fillPhones();
    view->setUin(uin);
    if (msgView) msgView->setUin(uin);
}

void MsgEdit::sendClick()
{
    if (sendEvent){
        pClient->cancelMessage(sendEvent->message());
        return;
    }
    bCloseSend = pMain->SimpleMode || toolbar->isOn(btnCloseSend);
    send();
}

void MsgEdit::markAsRead()
{
    if (pMain->SimpleMode){
        if (msg == NULL) return;
        if (!msg->Received) return;
        if (pClient->markAsRead(msg))
            setupNext();
    }else{
        ICQUser *u = pClient->getUser(Uin);
        if (u){
            History h(Uin);
            bool bChanged = false;
            for (;;){
                bool bExit = true;
                for (list<unsigned long>::iterator it = u->unreadMsgs.begin(); it != u->unreadMsgs.end(); it++){
                    ICQMessage *msg = h.getMessage(*it);
                    if (msg == NULL) continue;
                    switch (msg->Type()){
                    case ICQ_MSGxCHAT:
                    case ICQ_MSGxFILE:
                    case ICQ_MSGxAUTHxREQUEST:
                    case ICQ_MSGxCONTACTxLIST:
                        break;
                    default:
                        bChanged = pClient->markAsRead(msg) | bChanged;
                        bExit = false;
                    }
                    if (!bExit) break;
                }
                if (bExit) break;
            }
            if (msg == NULL) return;
            if (!msg->Received) return;
            if (pClient->markAsRead(msg))
                bChanged = true;
            if (bChanged) setupNext();
        }
    }
}

void MsgEdit::messageReceived(ICQMessage *m)
{
    if (m->getUin() != Uin) return;
    if (msgView){
        bool bUnread = false;
        if (msgView->findMsg(m->Id, 0) < 0){
            ICQUser *u = pClient->getUser(m->getUin());
            if (u){
                for (list<unsigned long>::iterator it = u->unreadMsgs.begin(); it != u->unreadMsgs.end(); it++){
                    if ((*it) == m->Id){
                        bUnread = true;
                        break;
                    }
                }
                msgView->addMessage(m, bUnread, true);
            }
        }
    }
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
    return (edit->isVisible() && edit->length());
}

bool MsgEdit::canSend()
{
    if (msg){
        if (toolbar->isOn(btnMultiply)){
            UserBox *box = static_cast<UserBox*>(topLevelWidget());
            if (box->users == NULL) return false;
            UserView *users = box->users;
            if (!users->hasChecked()) return false;
        }
        switch (msg->Type()){
        case ICQ_MSGxMSG:
            return (edit->length());
        case ICQ_MSGxURL:
            return (urlEdit->text().length());
        case ICQ_MSGxFILE:
            return (fileEdit->text().length());
        case ICQ_MSGxSMS:
            return (edit->length() && phoneEdit->lineEdit()->text().length());
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
#ifdef USE_KDE
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
    toolbar->setOn(btnBold, f.bold());
    toolbar->setOn(btnItalic, f.italic());
    toolbar->setOn(btnUnder, f.underline());
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
            QString name = *((QString*)param);
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
        toolbar->setState(btnNext, SIMClient::getMessageIcon(msgType), i18n("Next") + s);
        if (toolbar->isVisible(btnSend)){
            toolbar->hide(btnNext);
        }else{
            toolbar->show(btnNext);
        }
        return;
    }
    if (toolbar->isVisible(btnReply) || toolbar->isVisible(btnSend)){
        toolbar->hide(btnNext);
    }else{
        toolbar->setState(btnNext, "message", i18n("&New message"));
        toolbar->show(btnNext);
    }
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
    if (msg && msg->Received){
        switch (msg->Type()){
        case ICQ_MSGxMSG:
            msgText = pClient->from8Bit(Uin, static_cast<ICQMsg*>(msg)->Message, msg->Charset.c_str());
            break;
        case ICQ_MSGxURL:
            msgText = pClient->from8Bit(Uin, static_cast<ICQUrl*>(msg)->Message, msg->Charset.c_str());
            break;
        }
    }else{
        if (view->hasSelectedText()){
            msgText = view->selectedText();
        }else{
            msgText = view->text();
        }
    }
    string text(msgText.utf8());
    text = pClient->clearHTML(text);
    msgText = QString::fromUtf8(text.c_str());
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
            nMsg->Message = pClient->to8Bit(Uin, msgText);
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
        nMsg->Message = pClient->to8Bit(Uin, msgText);
        newMsg = nMsg;
    }
    newMsg->Charset = pClient->codecForUser(Uin)->name();
    setMessage(newMsg);
    edit->moveCursor(QTextEdit::MoveEnd, false);
    toolbar->setOn(btnMultiply, true);
    emit showUsers(true, 0);
    textChanged();
}

void MsgEdit::toggleMultiply(bool bMultiply)
{
    emit showUsers(bMultiply, Uin);
}

void MsgEdit::quoteClick()
{
    QString msgText;
    if (msg && msg->Received){
        switch (msg->Type()){
        case ICQ_MSGxMSG:
            msgText = pClient->from8Bit(Uin, static_cast<ICQMsg*>(msg)->Message, msg->Charset.c_str());
            break;
        case ICQ_MSGxURL:
            msgText = pClient->from8Bit(Uin, static_cast<ICQUrl*>(msg)->Message, msg->Charset.c_str());
            break;
        }
    }else{
        if (view->hasSelectedText()){
            msgText = view->selectedText();
        }else{
            msgText = view->text();
        }
    }
    string s;
    s = msgText.local8Bit();
    string text;
    if (!msgText.isEmpty()) text = msgText.utf8();
    text = pClient->clearHTML(text);
    msgText = QString::fromUtf8(text.c_str());
    QStringList l = QStringList::split('\n', msgText, true);
    QStringList::Iterator it;
    for (it = l.begin(); it != l.end(); it++){
        *it = QString("<p>&gt;") + *it + "</p>\n";
    }
    msgText = l.join("");
    s = msgText.local8Bit();
    ICQMsg *msg = new ICQMsg;
    msg->Uin.push_back(Uin);
    msg->Message = pClient->to8Bit(Uin, msgText);
    msg->Charset = pClient->codecForUser(Uin)->name();
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

bool MsgEdit::eventFilter(QObject *o, QEvent *e)
{
    if ((e->type() != QEvent::Show) || !bFirstShow) return false;
    if (msgView && !msgView->isVisible()) return false;
    bFirstShow = false;
    o->event(e);
    if (msgView) msgView->scrollToBottom();
    setMessage(msg);
    return true;
}

static QString escape(const QString &r)
{
    QString s = r;
    s = s.replace(QRegExp("\\&"), "&amp;");
    s = s.replace(QRegExp("<"), "&lt;");
    s = s.replace(QRegExp(">"), "&gt;");
    s = s.replace(QRegExp("\n"), "<br>");
    return s;
}

void MsgEdit::topReady(Tmpl*, const QString &res)
{
    edit->setText(escape(res));
    edit->moveCursor(QTextEdit::MoveEnd, false);
    disconnect(tmpl, SIGNAL(ready(Tmpl*, const QString&)), this, SLOT(topReady(Tmpl*, const QString&)));
    connect(tmpl, SIGNAL(ready(Tmpl*, const QString&)), this, SLOT(bottomReady(Tmpl*, const QString&)));
    tmpl->expand(QString::fromLocal8Bit(pMain->SMSSignBottom.c_str()), Uin);
}

void MsgEdit::bottomReady(Tmpl*, const QString &res)
{
    int parag, index;
    edit->getCursorPosition(&parag, &index);
    edit->append(escape(res));
    edit->setCursorPosition(parag, index);
    disconnect(tmpl, SIGNAL(ready(Tmpl*, const QString&)), this, SLOT(bottomReady(Tmpl*, const QString&)));
}

void MsgEdit::setMessage(ICQMessage *_msg, bool bMark, bool bInTop, bool bSaveEdit)
{
    msgTail = "";
    setUpdatesEnabled(false);
    bool bChanged = false;
    if (msg != _msg){
        if (msg && (msg->Id < MSG_PROCESS_ID)) delete msg;
        msg = _msg;
        bChanged = true;
    }
    toolbar->setOn(btnMultiply, false);
    if (msg == NULL){
        edit->setText("");
        edit->resetColors(false);
        urlEdit->setText("");
        users->clear();
        url->hide();
        edit->hide();
        file->hide();
        lblUsers->hide();
        toolbar->hide(btnBgColor);
        toolbar->hide(btnFgColor);
        toolbar->hide(btnBold);
        toolbar->hide(btnItalic);
        toolbar->hide(btnUnder);
        toolbar->hide(btnFont);
        toolbar->hide(btnSmile);
#ifdef USE_SPELL
        toolbar->hide(btnSpell);
#endif
        toolbar->hide(btnSend);
        toolbar->hide(btnCloseSend);
        toolbar->hide(btnAccept);
        toolbar->hide(btnDecline);
        toolbar->hide(btnReply);
        toolbar->hide(btnQuote);
        toolbar->hide(btnGrant);
        toolbar->hide(btnRefuse);
        toolbar->hide(btnMultiply);
        toolbar->hide(btnForward);
        users->hide();
        view->hide();
        setupNext();
        setUpdatesEnabled(true);
        repaint();
        return;
    }
    if (msgView && msg->Id)
        msgView->setMessage(Uin, msg->Id);
    if (msg->Received){
        if (bMark) pClient->markAsRead(msg);
        phone->hide();
        url->hide();
        edit->hide();
        file->hide();
        lblUsers->hide();
        toolbar->hide(btnBgColor);
        toolbar->hide(btnFgColor);
        toolbar->hide(btnBold);
        toolbar->hide(btnItalic);
        toolbar->hide(btnUnder);
        toolbar->hide(btnFont);
        toolbar->hide(btnSmile);
#ifdef USE_SPELL
        toolbar->hide(btnSpell);
#endif
        toolbar->hide(btnSend);
        toolbar->hide(btnCloseSend);
        toolbar->hide(btnAccept);
        toolbar->hide(btnDecline);
        toolbar->hide(btnMultiply);
        setupNext();
        if (msg->Type() == ICQ_MSGxCONTACTxLIST){
            toolbar->hide(btnReply);
            toolbar->hide(btnQuote);
            toolbar->hide(btnGrant);
            toolbar->hide(btnRefuse);
            users->show();
            users->sender = false;
            view->hide();
            toolbar->show(btnForward);
            ICQContacts *m = static_cast<ICQContacts*>(msg);
            for (ContactList::iterator it = m->Contacts.begin(); it != m->Contacts.end(); it++){
                Contact *contact = static_cast<Contact*>(*it);
                users->addUser(contact->Uin, contact->Alias.c_str());
            }
        }else{
            switch (msg->Type()){
            case ICQ_MSGxFILE:{
                    ICQFile *f = static_cast<ICQFile*>(msg);
                    toolbar->hide(btnReply);
                    toolbar->hide(btnQuote);
                    toolbar->hide(btnGrant);
                    toolbar->hide(btnRefuse);
                    if (f->Id >= MSG_PROCESS_ID){
                        toolbar->show(btnAccept);
                        toolbar->show(btnDecline);
                        file->show();
                        ICQUser *u = pClient->getUser(f->getUin());
                        if (u == NULL) u = pClient->owner;
                        SIMUser *_u = static_cast<SIMUser*>(u);
                        if (!_u->AcceptFileOverride)
                            _u = static_cast<SIMUser*>(pClient->owner);
                        string path = _u->AcceptFilePath.c_str();
                        if (*path.c_str() == 0)
                            pMain->buildFileName(path, "IncomingFiles/");
                        QString name = QString::fromLocal8Bit(path.c_str());
#ifdef WIN32
                        name.replace(QRegExp("/"), "\\");
                        if ((name.length() == 0) || (name[(int)(name.length() - 1)] != '\\'))
                            name += "\\";
#else
                        if ((name.length() == 0) || (name[(int)(name.length() - 1)] != '/'))
                            name += "/";
#endif
                        lblFile->setText(i18n("Save to") + ":");
                        fileEdit->setText(name);
                        fileEdit->setDirMode(true);
                        fileEdit->setMultiplyMode(false);
                        ftChanged();
                    }else{
                        toolbar->hide(btnAccept);
                        toolbar->hide(btnDecline);
                    }
                    break;
                }
            case ICQ_MSGxAUTHxREQUEST:
                toolbar->hide(btnReply);
                toolbar->hide(btnQuote);
                toolbar->show(btnGrant);
                toolbar->show(btnRefuse);
                toolbar->hide(btnAccept);
                toolbar->hide(btnDecline);
                toolbar->hide(btnForward);
                break;
            case ICQ_MSGxMSG:
            case ICQ_MSGxURL:
                if ((bInTop || !bChanged) && !pMain->SimpleMode){
                    toolbar->hide(btnReply);
                    toolbar->show(btnQuote);
                    toolbar->show(btnForward);
                    toolbar->hide(btnGrant);
                    toolbar->hide(btnRefuse);
                    toolbar->hide(btnAccept);
                    toolbar->hide(btnDecline);
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
                toolbar->show(btnReply);
                toolbar->show(btnQuote);
                toolbar->hide(btnGrant);
                toolbar->hide(btnRefuse);
                toolbar->hide(btnAccept);
                toolbar->hide(btnDecline);
                toolbar->show(btnForward);
                break;
            case ICQ_MSGxCHAT:
                toolbar->hide(btnReply);
                toolbar->hide(btnQuote);
                toolbar->hide(btnGrant);
                toolbar->hide(btnRefuse);
                toolbar->hide(btnAccept);
                toolbar->hide(btnDecline);
                toolbar->hide(btnForward);
                if (msg->Id >= MSG_PROCESS_ID){
                    toolbar->show(btnAccept);
                    toolbar->show(btnDecline);
                    chatChanged();
                }
                break;
            default:
                toolbar->hide(btnReply);
                toolbar->hide(btnQuote);
                toolbar->hide(btnGrant);
                toolbar->hide(btnRefuse);
                toolbar->hide(btnAccept);
                toolbar->hide(btnDecline);
                toolbar->hide(btnForward);
            }
            users->hide();
            view->show();
            view->setText(view->makeMessageText(msg, pMain->UseOwnColors));
            if ((msg->Type() == ICQ_MSGxMSG) && !pMain->UseOwnColors){
                ICQMsg *m = static_cast<ICQMsg*>(msg);
                if (m->BackColor != m->ForeColor){
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
        toolbar->hide(btnReply);
        toolbar->hide(btnForward);
        toolbar->hide(btnQuote);
        toolbar->hide(btnGrant);
        toolbar->hide(btnRefuse);
        toolbar->hide(btnAccept);
        toolbar->hide(btnDecline);
        if (pMain->SimpleMode){
            toolbar->hide(btnCloseSend);
        }else{
            toolbar->show(btnCloseSend);
        }
        toolbar->show(btnSend);
        switch (msg->Type()){
        case ICQ_MSGxMSG:{
                phone->hide();
                url->hide();
                edit->show();
                users->hide();
                view->hide();
                file->hide();
                lblUsers->hide();
                toolbar->show(btnBgColor);
                toolbar->show(btnFgColor);
                toolbar->show(btnBold);
                toolbar->show(btnItalic);
                toolbar->show(btnUnder);
                toolbar->show(btnFont);
                toolbar->show(btnSmile);
                toolbar->show(btnMultiply);
#ifdef USE_SPELL
                toolbar->show(btnSpell);
#endif
                if (!bSaveEdit){
                    edit->setTextFormat(RichText);
                    ICQMsg *m = static_cast<ICQMsg*>(msg);
                    if (bChanged)
                        edit->setText(pClient->from8Bit(Uin, m->Message, m->Charset.c_str()));
                    if (m->BackColor != m->ForeColor){
                        edit->setBackground(QColor(m->BackColor));
                        edit->setForeground(QColor(m->ForeColor));
                    }else{
                        edit->resetColors(true);
                    }
                    edit->setFocus();
                    edit->moveCursor(QTextEdit::MoveEnd, false);
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
                toolbar->hide(btnBgColor);
                toolbar->hide(btnFgColor);
                toolbar->hide(btnBold);
                toolbar->hide(btnItalic);
                toolbar->hide(btnUnder);
                toolbar->hide(btnFont);
                toolbar->hide(btnSmile);
                toolbar->show(btnMultiply);
#ifdef USE_SPELL
                toolbar->show(btnSpell);
#endif
                ICQUrl *m = static_cast<ICQUrl*>(msg);
                edit->resetColors(false);
                if (bChanged){
                    edit->setText(pClient->from8Bit(Uin, m->Message, m->Charset.c_str()));
                    urlEdit->setText(pClient->from8Bit(Uin, m->URL, m->Charset.c_str()));
                }
                urlEdit->setFocus();
                urlEdit->end(true);
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
                toolbar->hide(btnBgColor);
                toolbar->hide(btnFgColor);
                toolbar->hide(btnBold);
                toolbar->hide(btnItalic);
                toolbar->hide(btnUnder);
                toolbar->hide(btnFont);
                toolbar->hide(btnSmile);
                toolbar->hide(btnMultiply);
#ifdef USE_SPELL
                toolbar->show(btnSpell);
#endif
                ICQFile *m = static_cast<ICQFile*>(msg);
                edit->resetColors(false);
                lblFile->setText(i18n("File") + ":");
                fileEdit->setDirMode(false);
                fileEdit->setMultiplyMode(true);
                if (bChanged){
                    fileEdit->setText(pClient->from8Bit(Uin, m->Name, m->Charset.c_str()));
                    edit->setText(pClient->from8Bit(Uin, m->Description, m->Charset.c_str()));
                }
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
                toolbar->hide(btnBgColor);
                toolbar->hide(btnFgColor);
                toolbar->hide(btnBold);
                toolbar->hide(btnItalic);
                toolbar->hide(btnUnder);
                toolbar->hide(btnFont);
                toolbar->hide(btnSmile);
                toolbar->hide(btnMultiply);
#ifdef USE_SPELL
                toolbar->show(btnSpell);
#endif
                ICQChat *m = static_cast<ICQChat*>(msg);
                edit->resetColors(false);
                if (bChanged)
                    edit->setText(pClient->from8Bit(Uin, m->Reason, m->Charset.c_str()));
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
                toolbar->hide(btnBgColor);
                toolbar->hide(btnFgColor);
                toolbar->hide(btnBold);
                toolbar->hide(btnItalic);
                toolbar->hide(btnUnder);
                toolbar->hide(btnFont);
                toolbar->hide(btnSmile);
                toolbar->hide(btnMultiply);
#ifdef USE_SPELL
                toolbar->show(btnSpell);
#endif
                ICQSMS *m = static_cast<ICQSMS*>(msg);
                edit->resetColors(false);
                if (bChanged){
                    if (*m->Message.c_str()){
                        edit->setText(pClient->from8Bit(Uin, pClient->clearHTML(m->Message.c_str())), m->Charset.c_str());
                    }else{
                        connect(tmpl, SIGNAL(ready(Tmpl*, const QString&)), this, SLOT(topReady(Tmpl*, const QString&)));
                        tmpl->expand(QString::fromLocal8Bit(pMain->SMSSignTop.c_str()), Uin);
                    }
                    if (*m->Phone.c_str())
                        phoneEdit->lineEdit()->setText(pClient->from8Bit(Uin, m->Phone, NULL));
                }
                edit->setFocus();
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
                toolbar->hide(btnBgColor);
                toolbar->hide(btnFgColor);
                toolbar->hide(btnBold);
                toolbar->hide(btnItalic);
                toolbar->hide(btnUnder);
                toolbar->hide(btnFont);
                toolbar->hide(btnSmile);
                toolbar->show(btnMultiply);
#ifdef USE_SPELL
                toolbar->hide(btnSpell);
#endif
                users->sender = true;
                if (bChanged){
                    ICQContacts *m = static_cast<ICQContacts*>(msg);
                    for (ContactList::iterator it = m->Contacts.begin(); it != m->Contacts.end(); it++){
                        Contact *contact = static_cast<Contact*>(*it);
                        users->addUser(contact->Uin, contact->Alias.c_str());
                    }
                }
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
                toolbar->hide(btnBgColor);
                toolbar->hide(btnFgColor);
                toolbar->hide(btnBold);
                toolbar->hide(btnItalic);
                toolbar->hide(btnUnder);
                toolbar->hide(btnFont);
                toolbar->hide(btnSmile);
                toolbar->hide(btnMultiply);
#ifdef USE_SPELL
                toolbar->hide(btnSpell);
#endif
                ICQAuthRequest *m = static_cast<ICQAuthRequest*>(msg);
                edit->resetColors(false);
                edit->setText(pClient->from8Bit(Uin, m->Message, m->Charset.c_str()));
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
                toolbar->hide(btnBgColor);
                toolbar->hide(btnFgColor);
                toolbar->hide(btnBold);
                toolbar->hide(btnItalic);
                toolbar->hide(btnUnder);
                toolbar->hide(btnFont);
                toolbar->hide(btnSmile);
                toolbar->hide(btnMultiply);
#ifdef USE_SPELL
                toolbar->hide(btnSpell);
#endif
                ICQAuthRefused *m = static_cast<ICQAuthRefused*>(msg);
                edit->resetColors(false);
                edit->setText(pClient->from8Bit(Uin, m->Message, m->Charset.c_str()));
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
                toolbar->hide(btnBgColor);
                toolbar->hide(btnFgColor);
                toolbar->hide(btnBold);
                toolbar->hide(btnItalic);
                toolbar->hide(btnUnder);
                toolbar->hide(btnFont);
                toolbar->hide(btnSmile);
                toolbar->hide(btnMultiply);
#ifdef USE_SPELL
                toolbar->hide(btnSpell);
#endif
                break;
            }
        default:
            log(L_WARN, "Unknown message type %u", msg->Type());
        }
    }
    setupNext();
    textChanged();
    setUpdatesEnabled(true);
    repaint();
}

void MsgEdit::editTextChanged()
{
    if (msg){
        if (msg->Received){
            if (edit->length())
                action(mnuMessage, true);
        }else if (msg->Type() == ICQ_MSGxSMS){
            if (edit->length() == 0){
                emit setStatus("");
            }else{
                int size = edit->length();
                int max_size = MAX_SMS_LEN_LATIN1;
                if (!isLatin1(edit->text())) max_size = MAX_SMS_LEN_UNICODE;
                QString status = i18n("Size: %1 / Max. size: %2")
                                 .arg(size) .arg(max_size);
                if (size > max_size){
                    status += " ! ";
                    status += i18n("Message will be split");
                }
                emit setStatus(status);
            }
        }
    }
    textChanged();
}

void MsgEdit::textChanged(const QString&)
{
    textChanged();
}

void MsgEdit::textChanged()
{
#ifdef USE_SPELL
    toolbar->setEnabled(btnSpell, canSpell());
#endif
    toolbar->setEnabled(btnSend, canSend());
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
            m->Message = edit->text().utf8();
            m->Charset = "utf-8";
            m->BackColor = (edit->background().rgb() & 0xFFFFFF);
            m->ForeColor = (edit->foreground().rgb() & 0xFFFFFF);

            pMain->MessageBgColor = m->BackColor;
            pMain->MessageFgColor = m->ForeColor;
            if (edit->fontChanged())
                pMain->UserBoxFont = pMain->font2str(edit->currentFont(), false).local8Bit();
            break;
        }
    case ICQ_MSGxURL:{
            ICQUrl *m = static_cast<ICQUrl*>(msg);
            m->Message = pClient->to8Bit(Uin, edit->text());
            m->URL = pClient->to8Bit(Uin, urlEdit->text());
            break;
        }
    case ICQ_MSGxFILE:{
            ICQFile *m = static_cast<ICQFile*>(msg);
            m->Description = pClient->to8Bit(Uin, edit->text());
            m->Name = pClient->to8Bit(Uin, fileEdit->text());
            break;
        }
    case ICQ_MSGxCHAT:{
            ICQChat *m = static_cast<ICQChat*>(msg);
            m->Reason = pClient->to8Bit(Uin, edit->text());
            break;
        }
    case ICQ_MSGxSMS:{
            ICQSMS *m = static_cast<ICQSMS*>(msg);
            QString s = edit->text();
            string text(s.utf8());
            text = pClient->clearHTML(text);
            s = QString::fromUtf8(text.c_str());
            msgTail = trim(s);
            m->Message = smsChunk();
            m->Phone = phoneEdit->lineEdit()->text().local8Bit();
            m->Charset = pClient->codecForUser(Uin)->name();
            break;
        }
    case ICQ_MSGxCONTACTxLIST:{
            ICQContacts *m = static_cast<ICQContacts*>(msg);
            users->fillList(m->Contacts);
            break;
        }
    case ICQ_MSGxAUTHxREQUEST:{
            ICQAuthRequest *m = static_cast<ICQAuthRequest*>(msg);
            m->Message = pClient->to8Bit(Uin, edit->text());
            break;
        }
    case ICQ_MSGxAUTHxREFUSED:{
            ICQAuthRefused *m = static_cast<ICQAuthRefused*>(msg);
            m->Message = pClient->to8Bit(Uin, edit->text());
            break;
        }
    case ICQ_MSGxAUTHxGRANTED:
        break;
    default:
        log(L_WARN, "Bad message type %u", msg->Type());
        return;
    }
    if (msg->Charset.length() == 0)
        msg->Charset = pClient->codecForUser(Uin)->name();
    if (toolbar->isOn(btnMultiply)){
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
    if (!canSend()) return;
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
        if (fileEdit->text().length())
            f->localName = (const char*)(fileEdit->text().local8Bit());
    }
    pClient->acceptMessage(msg);
    if (pMain->SimpleMode)
        QTimer::singleShot(50, this, SLOT(close()));
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
    string declineStr = pClient->to8Bit(Uin, reason);
    pClient->declineMessage(msg, declineStr.c_str());
    if (pMain->SimpleMode)
        QTimer::singleShot(50, this, SLOT(close()));
}

void MsgEdit::chatChanged()
{
    if (msg && (msg->Type() == ICQ_MSGxCHAT) && msg->Received){
        QWidget *chat = pMain->chatWindow(Uin);
        toolbar->setEnabled(btnAccept, chat == NULL);
    }
}

void MsgEdit::ftChanged()
{
    if (msg && (msg->Type() == ICQ_MSGxFILE) && msg->Received){
        ICQFile *f = static_cast<ICQFile*>(msg);
        QWidget *file = pMain->ftWindow(Uin, f->shortName());
        toolbar->setEnabled(btnAccept, file == NULL);
    }
}

QString MsgEdit::trim(const QString &s)
{
    QString res = s;
    int n;
    for (n = 0; n < (int)res.length(); n++)
        if (!res[n].isSpace()) break;
    if (n) res = res.mid(n);
    if (res.isEmpty()) return res;
    for (n = (int)res.length() - 1; n >= 0; n--)
        if (!res[n].isSpace()) break;
    if (n < (int)res.length() - 1) res = res.left(n + 1);
    return res;
}

QString MsgEdit::chunk(const QString &s, int len)
{
    if ((int)s.length() < len) return s;
    QString res = s.left(len+1);
    int n = res.length() - 1;
    for (n = res.length() - 1; n >= 0; n--)
        if (res[n].isSpace()) break;
    for (; n >= 0; n--)
        if (!res[n].isSpace()) break;
    if (n < 0){
        res = s.left(len);
        return res;
    }
    res = s.left(n + 1);
    return res;
}

bool MsgEdit::isLatin1(const QString &s)
{
    for (int n = 0; n < (int)s.length(); n++)
        if (!s[n].latin1()) return false;
    return true;
}

string MsgEdit::smsChunk()
{
    string res;
    if (msgTail.isEmpty()) return res;
    QString part = chunk(msgTail, MAX_SMS_LEN_LATIN1);
    if (!isLatin1(part))
        part = chunk(msgTail, MAX_SMS_LEN_UNICODE);
    msgTail = trim(msgTail.mid(part.length()));
    part = part.replace(QRegExp("&"), "&amp;");
    part = part.replace(QRegExp("\""), "&quot;");
    part = part.replace(QRegExp("<"), "&lt;");
    part = part.replace(QRegExp(">"), "&gt;");
    part = part.replace(QRegExp("\n"), "<br>");
    res = pClient->to8Bit(Uin, part);
    return res;
}

void MsgEdit::modeChanged(bool bSimple)
{
    if (bSimple){
        if (msgView){
            delete msgView;
            msgView = NULL;
        }
        return;
    }
    if (msgView) return;
    msgView = new MsgViewConv(this);
    msgView->installEventFilter(this);
    msgView->setUin(Uin);
    moveToFirst(msgView);
    connect(msgView, SIGNAL(goMessage(unsigned long, unsigned long)), topLevelWidget(), SLOT(showMessage(unsigned long, unsigned long)));
    if (isVisible())
        msgView->show();
    adjustSplitter();
}

void MsgEdit::adjustSplitter()
{
    if (EditHeight == 0)
        EditHeight = pMain->UserBoxEditHeight;
    if (EditHeight == 0){
        QSize s = wndEdit->sizeHint();
        EditHeight = s.height();
    }
    setResizeMode(wndEdit, QSplitter::KeepSize);
    QValueList<int> s;
    s.append(1);
    s.append(EditHeight);
    setSizes(s);
}

void MsgEdit::heightChanged(int h)
{
    if (pMain->SimpleMode) return;
    EditHeight = h;
    pMain->UserBoxEditHeight = h;
}

void MsgEdit::showEvent(QShowEvent *e)
{
    QSplitter::showEvent(e);
    toolbar->enableAccel(true);
}

void MsgEdit::hideEvent(QHideEvent *e)
{
    QSplitter::hideEvent(e);
    toolbar->enableAccel(false);
}

WMainWindow::WMainWindow(QWidget *parent, const char *name)
        : QMainWindow(parent, name, 0)
{
}

void WMainWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    emit heightChanged(height());
}

#ifndef _WINDOWS
#include "msgedit.moc"
#endif
