/***************************************************************************
                          msgedit.cpp  -  description
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

#include "msgedit.h"
#include "textshow.h"
#include "toolbtn.h"
#include "core.h"
#include "msgsms.h"
#include "msgfile.h"
#include "msggen.h"
#include "msgauth.h"
#include "msgrecv.h"
#include "ballonmsg.h"
#include "userwnd.h"
#include "userlist.h"
#include "history.h"
#include "container.h"

#include <qfontmetrics.h>
#include <qtoolbar.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qtimer.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qclipboard.h>

#include <time.h>

#include <vector>
#include <algorithm>
using namespace std;

const unsigned NO_TYPE = (unsigned)(-1);

class MsgTextEdit : public TextEdit
{
public:
    MsgTextEdit(QWidget *parent);
protected:
    virtual QPopupMenu *createPopupMenu(const QPoint& pos);
};

MsgTextEdit::MsgTextEdit(QWidget *parent)
        : TextEdit(parent)
{
}

QPopupMenu *MsgTextEdit::createPopupMenu(const QPoint&)
{
    Command cmd;
    cmd->popup_id	= MenuTextEdit;
    cmd->param		= parentWidget()->parentWidget();
    cmd->flags		= COMMAND_NEW_POPUP;
    Event e(EventGetMenu, cmd);
    return (QPopupMenu*)(e.process());
}

MsgEdit::MsgEdit(QWidget *parent, UserWnd *userWnd, bool bReceived)
        : QMainWindow(parent, NULL, 0)
{
    m_userWnd	= userWnd;
    m_msg		= NULL;
    m_bTyping	= false;
    m_type		= NO_TYPE;

    connect(CorePlugin::m_plugin, SIGNAL(modeChanged()), this, SLOT(modeChanged()));

    m_frame = new QFrame(this);
    setCentralWidget(m_frame);
    m_layout = new QVBoxLayout(m_frame);

    m_edit = new MsgTextEdit(m_frame);
    m_edit->setBackground(QColor(CorePlugin::m_plugin->getEditBackground()));
    m_edit->setForeground(QColor(CorePlugin::m_plugin->getEditForeground()));
    m_edit->setFont(CorePlugin::m_plugin->editFont);
    m_edit->setCtrlMode(!CorePlugin::m_plugin->getSendOnEnter());

    connect(m_edit, SIGNAL(lostFocus()), this, SLOT(editLostFocus()));
    connect(m_edit, SIGNAL(textChanged()), this, SLOT(editTextChanged()));
    connect(m_edit, SIGNAL(ctrlEnterPressed()), this, SLOT(editEnterPressed()));

    QFontMetrics fm(m_edit->font());
    m_edit->setMinimumSize(QSize(fm.maxWidth(), fm.height() + 10));
    m_layout->addWidget(m_edit);

    BarShow b;
    b.bar_id = bReceived ? BarReceived : ToolBarEdit;
    b.parent = this;
    Event e(EventShowBar, &b);
    m_bar = (CToolBar*)(e.process());
    m_bar->setParam(this);

    if (CorePlugin::m_plugin->getContainerMode() == 0)
        showCloseSend(false);

    setDockEnabled(m_bar, Left, false);
    setDockEnabled(m_bar, Right, false);
}

MsgEdit::~MsgEdit()
{
    editLostFocus();
    Command cmd;
    cmd->id = CmdCustomInput;
    cmd->param = this;
    Event e(EventCommandWidget, cmd);
    CToolCustom *w = (CToolCustom*)(e.process());
    if (w)
        w->removeWidgets();
}

void MsgEdit::showCloseSend(bool bState)
{
    Command cmd;
    cmd->id			= CmdSendClose;
    cmd->text		= I18N_NOOP("C&lose after send");
    cmd->icon		= "exit";
    cmd->icon_on	= "exit";
    cmd->bar_grp	= 0x7010;
    cmd->flags		= bState ? COMMAND_DEFAULT : BTN_HIDE;
    cmd->param		= this;
    if (CorePlugin::m_plugin->getCloseSend())
        cmd->flags |= COMMAND_CHECKED;
    Event eCmd(EventCommandChange, cmd);
    eCmd.process();
}

void MsgEdit::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    emit heightChanged(height());
}

void MsgEdit::setMessage(Message *msg, bool bSetFocus)
{
    unsigned type = msg->type();
    QObject *(*create)(QWidget *custom, Message *msg) = NULL;
    for (;;){
        CommandDef *cmd = CorePlugin::m_plugin->messageTypes.find(type);
        if (cmd == NULL)
            return;
        MessageDef *def = (MessageDef*)(cmd->param);
        if (def == NULL)
            return;
        if (msg->getFlags() & MESSAGE_RECEIVED){
            create = def->show;
        }else{
            create = def->generate;
        }
        if (create)
            break;
        if (def->base_type == 0){
            if (msg->getFlags() & MESSAGE_RECEIVED)
                break;
            return;
        }
        type = def->base_type;
    }

    Command cmd;
    cmd->id = CmdCustomInput;
    cmd->param = this;
    Event e(EventCommandWidget, cmd);
    CToolCustom *wInput = (CToolCustom*)(e.process());
    cmd->id = CmdCustomReceive;
    CToolCustom *wReceive = (CToolCustom*)(e.process());
    m_userWnd->setStatus("");

    QObject *initObj = NULL;
    if (wInput){
        if (msg->getFlags() & MESSAGE_RECEIVED){
            wInput->hide();
            if (m_type == NO_TYPE){
                wInput->removeWidgets();
                m_type = MessageGeneric;
                Message msg(MessageGeneric);
                initObj = new MsgGen(wInput, &msg);
            }
        }else{
            wInput->removeWidgets();
            initObj = create(wInput, msg);
            wInput->show();
        }
    }
    if (wReceive){
        if (msg->getFlags() & MESSAGE_RECEIVED){
            wReceive->removeWidgets();
            initObj = new MsgReceived(wReceive, msg);
            wReceive->show();
        }else{
            wReceive->hide();
        }
    }
    if (msg->client()){
        m_client = msg->client();
    }else{
        m_client = "";
    }
    Contact *contact = getContacts()->contact(m_userWnd->id());
    if (contact){
        Event e(EventContactClient, contact);
        e.process();
    }

    cmd->id			= CmdMultiply;
    cmd->text		= I18N_NOOP("&Multiply send");
    cmd->icon		= "1rightarrow";
    cmd->icon_on	= "1leftarrow";
    cmd->flags		= COMMAND_DEFAULT;
    if (msg->getFlags() & MESSAGE_FORWARD){
        cmd->flags = COMMAND_CHECKED;
        m_userWnd->showListView(true, false);
    }
    Event eChange(EventCommandChange, cmd);
    eChange.process();

    if (initObj && bSetFocus){
        connect(this, SIGNAL(init()), initObj, SLOT(init()));
        emit init();
        disconnect(this, SIGNAL(init()), initObj, SLOT(init()));
    }
}

Client *MsgEdit::client(void *&data)
{
    data = NULL;
    if (m_client.empty())
        return NULL;
    Contact *contact = getContacts()->contact(m_userWnd->id());
    if (contact == NULL)
        return NULL;
    void *d;
    ClientDataIterator it(contact->clientData);
    while ((d = ++it) != NULL){
        if (m_client == it.client()->dataName(d)){
            data = d;
            return it.client();
        }
    }
    return NULL;
}

void MsgEdit::setInput()
{
    Command cmd;
    cmd->id = CmdCustomInput;
    cmd->param = this;
    Event e(EventCommandWidget, cmd);
    CToolCustom *wInput = (CToolCustom*)(e.process());
    cmd->id = CmdCustomReceive;
    CToolCustom *wReceive = (CToolCustom*)(e.process());
    m_userWnd->setStatus("");

    if (wInput){
        wInput->show();
    }
    if (wReceive){
        wReceive->removeWidgets();
        wReceive->hide();
    }
}

static Message *createGeneric(const char *cfg)
{
    return new Message(MessageGeneric, cfg);
}

static QObject *generateGeneric(QWidget *w, Message *msg)
{
    return new MsgGen(static_cast<CToolCustom*>(w), msg);
}

#if 0
i18n("Message", "%n messages", 1);
#endif

static MessageDef defGeneric =
    {
        NULL,
        MESSAGE_DEFAULT,
        0,
        "Message",
        "%n messages",
        createGeneric,
        NULL,
        generateGeneric
    };

static Message *createSMS(const char *cfg)
{
    return new SMSMessage(cfg);
}

static QObject* generateSMS(QWidget *w, Message *msg)
{
    return new MsgSMS(static_cast<CToolCustom*>(w), msg);
}

#if 0
i18n("SMS", "%n SMSs", 1);
#endif

static MessageDef defSMS =
    {
        NULL,
        MESSAGE_DEFAULT,
        0,
        "SMS",
        "SMSs",
        createSMS,
        NULL,
        generateSMS
    };

static Message *createFile(const char *cfg)
{
    return new FileMessage(cfg);
}

static QObject* generateFile(QWidget *w, Message *msg)
{
    return new MsgFile(static_cast<CToolCustom*>(w), msg);
}

#if 0
i18n("File", "%n files", 1);
#endif

static MessageDef defFile =
    {
        NULL,
        MESSAGE_DEFAULT,
        0,
        "File",
        "%n files",
        createFile,
        NULL,
        generateFile
    };

static Message *createAuthRequest(const char *cfg)
{
    return new AuthMessage(MessageAuthRequest, cfg);
}

static QObject* generateAuthRequest(QWidget *w, Message *msg)
{
    return new MsgAuth(static_cast<CToolCustom*>(w), msg);
}

#if 0
i18n("Authorize request", "%n authorize requests", 1);
#endif

static CommandDef authRequestCommands[] =
    {
        {
            CmdGrantAuth,
            I18N_NOOP("&Grant"),
            NULL,
            NULL,
            NULL,
            0,
            0,
            MenuMessage,
            0,
            0,
            COMMAND_DEFAULT,
            NULL,
            NULL
        },
        {
            CmdRefuseAuth,
            I18N_NOOP("&Refuse"),
            NULL,
            NULL,
            NULL,
            0,
            0,
            MenuMessage,
            0,
            0,
            COMMAND_DEFAULT,
            NULL,
            NULL
        },
        {
            0,
            NULL,
            NULL,
            NULL,
            NULL,
            0,
            0,
            0,
            0,
            0,
            COMMAND_DEFAULT,
            NULL,
            NULL
        }
    };

static MessageDef defAuthRequest =
    {
        authRequestCommands,
        MESSAGE_DEFAULT,
        0,
        "Authorize request",
        "%n authorize requests",
        createAuthRequest,
        NULL,
        generateAuthRequest
    };

static Message *createAuthGranted(const char *cfg)
{
    return new AuthMessage(MessageAuthGranted, cfg);
}

static QObject* generateAuthGranted(QWidget *w, Message *msg)
{
    return new MsgAuth(static_cast<CToolCustom*>(w), msg);
}

#if 0
i18n("Authorization granted", "%n authorization granted", 1);
#endif

static MessageDef defAuthGranted =
    {
        NULL,
        MESSAGE_SILENT,
        0,
        "Authorization granted",
        "%n authorization granted",
        createAuthGranted,
        NULL,
        generateAuthGranted
    };

static Message *createAuthRefused(const char *cfg)
{
    return new AuthMessage(MessageAuthRefused, cfg);
}

static QObject* generateAuthRefused(QWidget *w, Message *msg)
{
    return new MsgAuth(static_cast<CToolCustom*>(w), msg);
}

#if 0
i18n("Authorization refused", "%n authorization refused", 1);
#endif

static MessageDef defAuthRefused =
    {
        NULL,
        MESSAGE_SILENT,
        0,
        "Authorization refused",
        "%n authorization refused",
        createAuthRefused,
        NULL,
        generateAuthRefused
    };

static Message *createAdded(const char *cfg)
{
    return new AuthMessage(MessageAdded, cfg);
}

#if 0
i18n("Add to contacts", "%n add to contacts", 1);
#endif

static MessageDef defAdded =
    {
        NULL,
        MESSAGE_DEFAULT,
        0,
        "Add to contacts",
        "%n add to contacts",
        createAdded,
        NULL,
        NULL
    };

static Message *createRemoved(const char *cfg)
{
    return new AuthMessage(MessageRemoved, cfg);
}

#if 0
i18n("Removed from contacts", "%n removed from contacts", 1);
#endif

static MessageDef defRemoved =
    {
        NULL,
        MESSAGE_DEFAULT,
        0,
        "Removed from contacts",
        "%n removed from contacts",
        createRemoved,
        NULL,
        NULL
    };

static Message *createStatus(const char *cfg)
{
    return new StatusMessage(cfg);
}

#if 0
i18n("Status changed", "%n times status changed", 1);
#endif

static MessageDef defStatus =
    {
        NULL,
        MESSAGE_DEFAULT,
        0,
        "Status changed",
        "%n times status changed",
        createStatus,
        NULL,
        NULL
    };

typedef struct ClientStatus
{
    unsigned long	status;
    unsigned		client;
    void			*data;
} ClientStatus;

static bool cmp_status(ClientStatus s1, ClientStatus s2)
{
    if (s1.status > s2.status)
        return true;
    if (s1.status < s2.status)
        return false;
    return s1.client < s2.client;
}

bool MsgEdit::sendMessage(Message *msg)
{
    if (m_msg){
        delete msg;
        Event e(EventMessageCancel, m_msg);
        if (e.process())
            m_msg = NULL;
        stopSend(false);
        return false;
    }

    if (m_userWnd->m_list){
        multiply = m_userWnd->m_list->selected;
        if (multiply.empty())
            return false;
        multiply_it = multiply.begin();
        msg->setContact(*multiply_it);
        ++multiply_it;
        if (multiply_it != multiply.end())
            msg->setFlags(msg->getFlags() | MESSAGE_MULTIPLY);
    }

    editLostFocus();
    Command cmd;
    cmd->id		= CmdSend;
    cmd->text	= I18N_NOOP("Cancel");
    cmd->icon	= "cancel";
    cmd->flags	= BTN_PICT;
    cmd->param	= this;
    Event eCmd(EventCommandChange, cmd);
    eCmd.process();
    m_msg = msg;
    return send();
}

bool MsgEdit::send()
{
    unsigned i;
    vector<ClientStatus> cs;
    Contact *contact = getContacts()->contact(m_msg->contact());
    string client_str = m_msg->client();
    bool bSent = false;
    if (contact){
        if (client_str.empty()){
            for (i = 0; i < getContacts()->nClients(); i++){
                Client *client = getContacts()->getClient(i);
                void *data;
                ClientDataIterator it(contact->clientData, client);
                while ((data = ++it) != NULL){
                    unsigned long status = STATUS_UNKNOWN;
                    unsigned style = 0;
                    const char *statusIcon = NULL;
                    client->contactInfo(data, status, style, statusIcon);
                    ClientStatus s;
                    s.client = i;
                    s.status = status;
                    s.data   = data;
                    cs.push_back(s);
                }
            }
            sort(cs.begin(), cs.end(), cmp_status);
            for (i = 0; i < cs.size(); i++){
                Client *client = getContacts()->getClient(cs[i].client);
                if (client->send(m_msg, cs[i].data)){
                    bSent = true;
                    break;
                }
            }
            if (!bSent){
                for (i = 0; i < getContacts()->nClients(); i++){
                    Client *client = getContacts()->getClient(i);
                    if (client->send(m_msg, NULL)){
                        bSent = true;
                        break;
                    }
                }
            }
        }else{
            void *data;
            ClientDataIterator it(contact->clientData);
            while ((data = ++it) != NULL){
                if (it.client()->dataName(data) == client_str){
                    if (it.client()->send(m_msg, data))
                        bSent = true;
                    break;
                }
            }
        }
    }
    if (!bSent){
        if (m_msg){
            delete m_msg;
            m_msg = NULL;
        }
        stopSend();
        CToolButton *btnSend = NULL;
        Command cmd;
        cmd->id		= CmdSend;
        cmd->param	= this;
        Event e(EventCommandWidget, cmd);
        btnSend = (CToolButton*)(e.process());
        QWidget *msgWidget = btnSend;
        if (msgWidget == NULL)
            msgWidget = this;
        BalloonMsg::message(i18n("No such client for send message"), msgWidget);
        return false;
    }
    return true;
}

void MsgEdit::stopSend(bool bCheck)
{
    if (m_userWnd->m_list){
        Command cmd;
        m_userWnd->showListView(false, false);
        cmd->id			= CmdMultiply;
        cmd->text		= I18N_NOOP("&Multiply send");
        cmd->icon		= "1rightarrow";
        cmd->icon_on	= "1leftarrow";
        cmd->flags		= COMMAND_DEFAULT;
        cmd->param		= this;
        Event eChange(EventCommandChange, cmd);
        eChange.process();
    }
    multiply.clear();
    Command cmd;
    cmd->id		= CmdSend;
    cmd->text	= I18N_NOOP("&Send");
    cmd->icon	= "mail_generic";
    cmd->bar_id		= ToolBarEdit;
    cmd->bar_grp	= 0x8000;
    cmd->flags		= BTN_PICT;
    cmd->param		= this;
    Event eCmd(EventCommandChange, cmd);
    eCmd.process();
    if (bCheck && (m_msg == NULL))
        return;
    if (m_msg)
        m_msg = NULL;
}

void MsgEdit::modeChanged()
{
    showCloseSend(CorePlugin::m_plugin->getContainerMode() != 0);
    m_edit->setCtrlMode(CorePlugin::m_plugin->getSendOnEnter());
}

void *MsgEdit::processEvent(Event *e)
{
    if (e->type() == EventMessageReceived){
        Message *msg = (Message*)(e->param());
        if ((msg->contact() == m_userWnd->id()) && (msg->type() != MessageStatus)){
            if (CorePlugin::m_plugin->getContainerMode()){
                bool bSetFocus = false;
                if (topLevelWidget() && topLevelWidget()->inherits("Container")){
                    Container *container = static_cast<Container*>(topLevelWidget());
                    if (container->wnd() == m_userWnd)
                        bSetFocus = true;
                }
                setMessage(msg, bSetFocus);
            }else{
                if (m_edit->isReadOnly())
                    QTimer::singleShot(0, this, SLOT(setupNext()));
            }
        }
    }
    if (e->type() == EventRealSendMessage){
        MsgSend *s = (MsgSend*)(e->param());
        if (s->edit == this){
            sendMessage(s->msg);
            return e->param();
        }
    }
    if (e->type() == EventCheckState){
        CommandDef *cmd = (CommandDef*)(e->param());
        if ((cmd->menu_id != MenuTextEdit) || (cmd->param != this))
            return NULL;
        cmd->flags &= ~(COMMAND_CHECKED | COMMAND_DISABLED);
        switch (cmd->id){
        case CmdUndo:
            if (m_edit->isReadOnly())
                return NULL;
            if (!m_edit->isUndoAvailable())
                cmd->flags |= COMMAND_DISABLED;
            return e->param();
        case CmdRedo:
            if (m_edit->isReadOnly())
                return NULL;
            if (!m_edit->isRedoAvailable())
                cmd->flags |= COMMAND_DISABLED;
            return e->param();
        case CmdCut:
            if (m_edit->isReadOnly())
                return NULL;
        case CmdCopy:
            if (!m_edit->hasSelectedText())
                cmd->flags |= COMMAND_DISABLED;
            return e->param();
        case CmdPaste:
            if (m_edit->isReadOnly())
                return NULL;
            if (QApplication::clipboard()->text().isEmpty())
                cmd->flags |= COMMAND_DISABLED;
            return e->param();
        case CmdClear:
            if (m_edit->isReadOnly())
                return NULL;
        case CmdSelectAll:
            if (m_edit->text().isEmpty())
                cmd->flags |= COMMAND_DISABLED;
            return e->param();
        }
        return NULL;
    }
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if ((cmd->id == CmdSmile) && (cmd->param == this)){
            Event eBtn(EventCommandWidget, cmd);
            QToolButton *btnSmile = (QToolButton*)(eBtn.process());
            if (btnSmile){
                SmilePopup *popup = new SmilePopup(this);
                connect(popup, SIGNAL(insert(int)), this, SLOT(insertSmile(int)));
                QPoint p = CToolButton::popupPos(btnSmile, popup);
                popup->move(p);
                popup->show();
            }
            return e->param();
        }
        if ((cmd->id == CmdMultiply) && (cmd->param == this)){
            m_userWnd->showListView(cmd->flags & COMMAND_CHECKED, true);
            return e->param();
        }
        if ((cmd->bar_id == BarReceived) && m_edit->isReadOnly() && (cmd->param == this)){
            switch (cmd->id){
            case CmdMsgAnswer:{
                    Message msg(MessageGeneric);
                    msg.setContact(m_userWnd->id());
                    msg.setClient(m_client.c_str());
                    Event e(EventOpenMessage, &msg);
                    e.process();
                }
            case CmdNextMessage:
                QTimer::singleShot(0, this, SLOT(goNext()));
                break;
            }
        }
        if ((cmd->menu_id != MenuTextEdit) || (cmd->param != this))
            return NULL;
        switch (cmd->id){
        case CmdUndo:
            m_edit->undo();
            return e->param();
        case CmdRedo:
            m_edit->redo();
            return e->param();
        case CmdCut:
            m_edit->cut();
            return e->param();
        case CmdCopy:
            m_edit->copy();
            return e->param();
        case CmdPaste:
            m_edit->paste();
            return e->param();
        case CmdClear:
            m_edit->clear();
            return e->param();
        case CmdSelectAll:
            m_edit->selectAll();
            return e->param();
        }
        return NULL;
    }
    if (e->type() == EventMessageSent){
        Message *msg = (Message*)(e->param());
        if (msg == m_msg){
            QString err;
            const char *err_str = msg->getError();
            if (err_str && *err_str)
                err = i18n(err_str);
            Contact *contact = getContacts()->contact(msg->contact());
            if (err){
                stopSend();
                Command cmd;
                cmd->id		= CmdSend;
                cmd->param	= this;
                Event e(EventCommandWidget, cmd);
                QWidget *msgWidget = (QWidget*)(e.process());
                if (msgWidget == NULL)
                    msgWidget = this;
                BalloonMsg::message(err, msgWidget);
            }else{
                if (contact){
                    time_t now;
                    time(&now);
                    contact->setLastActive(now);
                    Event e(EventContactStatus, contact);
                    e.process();
                }
                if (!multiply.empty() && (multiply_it != multiply.end())){
                    CommandDef *def = CorePlugin::m_plugin->messageTypes.find(m_msg->type());
                    if (def){
                        MessageDef *mdef = (MessageDef*)(def->param);
                        string cfg = m_msg->save();
                        m_msg = (mdef->create)(cfg.c_str());
                        m_msg->setContact(*multiply_it);
                        ++multiply_it;
                        if (multiply_it == multiply.end())
                            m_msg->setFlags(m_msg->getFlags() | MESSAGE_LAST);
                        send();
                        return NULL;
                    }
                }
                stopSend();
                bool bClose = true;
                if (CorePlugin::m_plugin->getContainerMode()){
                    bClose = false;
                    Command cmd;
                    cmd->id		= CmdSendClose;
                    cmd->param	= this;
                    Event e(EventCommandWidget, cmd);
                    QToolButton *btnClose = (QToolButton*)(e.process());
                    if (btnClose)
                        bClose = btnClose->isOn();
                }
                if (bClose){
                    QTimer::singleShot(0, m_userWnd, SLOT(close()));
                }else{
                    m_edit->setText("");
                    m_edit->setFont(CorePlugin::m_plugin->editFont);
                    m_edit->setForeground(CorePlugin::m_plugin->getEditForeground());
                }
            }
            return NULL;
        }
    }
    return NULL;
}

void MsgEdit::editTextChanged()
{
    bool bTyping = !m_edit->text().isEmpty();
    if (qApp->focusWidget() != m_edit)
        bTyping = false;
    if (m_bTyping == bTyping)
        return;
    m_bTyping = bTyping;
    Event e(bTyping ? EventStartTyping : EventStopTyping, (void*)m_userWnd->m_id);
    e.process();
}

void MsgEdit::editLostFocus()
{
    if (!m_bTyping)
        return;
    m_bTyping = false;
    Event e(EventStopTyping, (void*)m_userWnd->m_id);
    e.process();
}

void MsgEdit::insertSmile(int id)
{
    if (m_edit->textFormat() == QTextEdit::PlainText){
        m_edit->insert(smiles()[id], false, true, true);
        return;
    }
    QString tail;
    int paraPos, idxPos;
    int paraEnd, idxEnd;
    if (m_edit->hasSelectedText()){
        m_edit->getSelection(&paraPos, &idxPos, &paraEnd, &idxEnd);
    }else{
        m_edit->getCursorPosition(&paraPos, &idxPos);
        paraEnd = paraPos;
        idxEnd  = idxPos;
    }
    tail = m_edit->text(paraEnd);
    tail = tail.mid(m_edit->textPosition(tail, idxEnd));
    for (int para = paraPos + 1; para < m_edit->paragraphs(); para++)
        tail += m_edit->text(para);
    m_edit->setCursorPosition(paraPos, idxPos);
    if (idxPos == 0)
        m_edit->moveCursor(QTextEdit::MoveBackward, false);
    m_edit->moveCursor(QTextEdit::MoveEnd, true);
    m_edit->removeSelectedText();
    QFont f = m_edit->font();
    QColor fgColor = m_edit->foreground();
    char b[10];
    sprintf(b, "%X", id);
    m_edit->append(QString("<img src=icon:smile") + b + ">");
    m_edit->append(tail);
    m_edit->setCursorPosition(paraPos, idxPos);
    m_edit->setFont(f);
    m_edit->setForeground(fgColor);
    m_edit->moveCursor(QTextEdit::MoveForward, false);
    m_edit->changeText();
}

void MsgEdit::goNext()
{
    for (list<msg_id>::iterator it = CorePlugin::m_plugin->unread.begin(); it != CorePlugin::m_plugin->unread.end(); ++it){
        if ((*it).contact != m_userWnd->id())
            continue;
        Message *msg = History::load((*it).id, (*it).client.c_str(), (*it).contact);
        if (msg == NULL)
            continue;
        Event e(EventOpenMessage, msg);
        e.process();
        delete msg;
        return;
    }
    if (CorePlugin::m_plugin->getContainerMode())
        return;
    QTimer::singleShot(0, m_userWnd, SLOT(close()));
}

void MsgEdit::setupNext()
{
    Command cmd;
    cmd->id    = CmdNextMessage;
    cmd->param = this;
    Event e(EventCommandWidget, cmd);
    CToolButton *btnNext = (CToolButton*)(e.process());
    if (btnNext == NULL)
        return;

    unsigned type  = 0;
    unsigned count = 0;
    for (list<msg_id>::iterator it = CorePlugin::m_plugin->unread.begin(); it != CorePlugin::m_plugin->unread.end(); ++it){
        if ((*it).contact != m_userWnd->id())
            continue;
        if (count == 0)
            type = (*it).type;
        count++;
    }
    QString str = i18n("&Next");
    if (count > 1)
        str += QString(" [%1]") .arg(count);

    CommandDef *def = NULL;
    for (;;){
        def = CorePlugin::m_plugin->messageTypes.find(type);
        if ((def == NULL) || (def->param == NULL))
            break;
        MessageDef *mdef = (MessageDef*)(def->param);
        if (mdef->base_type == 0)
            break;
        type = mdef->base_type;
    }

    CommandDef c = *btnNext->def();
    c.text_wrk = strdup(str.utf8());
    if (def)
        c.icon     = def->icon;
    if (count){
        c.flags &= ~COMMAND_DISABLED;
    }else{
        c.flags |= COMMAND_DISABLED;
    }
    btnNext->setCommand(&c);
}

void MsgEdit::editEnterPressed()
{
    Command cmd;
    cmd->id = CmdSend;
    cmd->param = this;
    Event e(EventCommandExec, cmd);
    e.process();
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
            unsigned n = i * 4 + j;
            QWidget *w = new SmileLabel(n, this);
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

void MsgEdit::setupMessages()
{
    Command cmd;

    cmd->id			= MessageGeneric;
    cmd->text		= I18N_NOOP("&Message");
    cmd->icon		= "message";
    cmd->menu_grp	= 0x3010;
    cmd->accel		= "Ctrl+M";
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= &defGeneric;
    Event eMsg(EventCreateMessageType, cmd);
    eMsg.process();

    cmd->id			= MessageFile;
    cmd->text		= I18N_NOOP("&File");
    cmd->icon		= "file";
    cmd->accel		= "Ctrl+F";
    cmd->menu_grp	= 0x3020;
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= &defFile;
    eMsg.process();

    cmd->id			= MessageSMS;
    cmd->text		= I18N_NOOP("&SMS");
    cmd->icon		= "sms";
    cmd->accel		= "Ctrl+S";
    cmd->menu_grp	= 0x3030;
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= &defSMS;
    eMsg.process();

    cmd->id			= MessageAuthRequest;
    cmd->text		= I18N_NOOP("&Authorization request");
    cmd->icon		= "auth";
    cmd->accel		= "Ctrl+A";
    cmd->menu_grp	= 0x3040;
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= &defAuthRequest;
    eMsg.process();

    cmd->id			= MessageAuthGranted;
    cmd->text		= I18N_NOOP("&Grant autorization");
    cmd->icon		= "auth";
    cmd->accel		= "Ctrl+G";
    cmd->menu_grp	= 0x3050;
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= &defAuthGranted;
    eMsg.process();

    cmd->id			= MessageAuthRefused;
    cmd->text		= I18N_NOOP("&Refuse autorization");
    cmd->icon		= "auth";
    cmd->accel		= "Ctrl+R";
    cmd->menu_grp	= 0x3050;
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= &defAuthRefused;
    eMsg.process();

    cmd->id			= MessageAdded;
    cmd->text		= "Added";
    cmd->icon		= "auth";
    cmd->menu_grp	= 0;
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= &defAdded;
    eMsg.process();

    cmd->id			= MessageRemoved;
    cmd->text		= "Removed";
    cmd->icon		= "auth";
    cmd->menu_grp	= 0;
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= &defRemoved;
    eMsg.process();

    cmd->id			= MessageStatus;
    cmd->text		= "Status";
    cmd->icon		= NULL;
    cmd->menu_grp	= 0;
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= &defStatus;
    eMsg.process();
}

#ifndef WIN32
#include "msgedit.moc"
#endif

