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
#include "msgurl.h"
#include "msgcontacts.h"
#include "msgrecv.h"
#include "ballonmsg.h"
#include "userwnd.h"
#include "userlist.h"
#include "history.h"
#include "container.h"
#include "buffer.h"

#include <qfontmetrics.h>
#include <qtoolbar.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qtimer.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qregexp.h>
#include <qtooltip.h>

#include <time.h>

#ifdef USE_KDE
#include "kdeisversion.h"
#endif

const unsigned NO_TYPE = (unsigned)(-1);

MsgTextEdit::MsgTextEdit(MsgEdit *edit, QWidget *parent)
        : TextEdit(parent)
{
    m_edit = edit;
    setBackground(CorePlugin::m_plugin->getEditBackground());
    setForeground(CorePlugin::m_plugin->getEditForeground(), true);
#if defined(USE_KDE)
#if KDE_IS_VERSION(3,2,0)
    setCheckSpellingEnabled(CorePlugin::m_plugin->getEnableSpell());
#endif
#endif
}

QPopupMenu *MsgTextEdit::createPopupMenu(const QPoint &pos)
{
    if (m_bInClick)
        return NULL;
    Command cmd;
    cmd->popup_id	= MenuTextEdit;
    cmd->param		= parentWidget()->parentWidget();
    cmd->flags		= COMMAND_NEW_POPUP;
    m_popupPos = pos;
    Event e(EventGetMenu, cmd);
    return (QPopupMenu*)(e.process());
}

Message *MsgTextEdit::createMessage(QMimeSource *src)
{
    Message *msg = NULL;
    CommandDef *cmd;
    CommandsMapIterator it(CorePlugin::m_plugin->messageTypes);
    while ((cmd = ++it) != NULL){
        MessageDef *def = (MessageDef*)(cmd->param);
        if (def && def->drag){
            msg = def->drag(src);
            if (msg){
                Command c;
                c->id      = cmd->id;
                c->menu_id = MenuMessage;
                c->param	 = (void*)(m_edit->m_userWnd->id());
                Event e(EventCheckState, c);
                if (e.process())
                    break;
                delete msg;
                msg = NULL;
            }
        }
    }
    return msg;
}

void MsgTextEdit::contentsDropEvent(QDropEvent *e)
{
    Message *msg = createMessage(e);
    if (msg){
        e->accept();
        msg->setContact(m_edit->m_userWnd->id());
        Event eOpen(EventOpenMessage, &msg);
        eOpen.process();
        delete msg;
        return;
    }
    TextEdit::contentsDropEvent(e);
}

void MsgTextEdit::contentsDragEnterEvent(QDragEnterEvent *e)
{
    Message *msg = createMessage(e);
    if (msg){
        delete msg;
        e->acceptAction();
        return;
    }
    TextEdit::contentsDragEnterEvent(e);
}

void MsgTextEdit::contentsDragMoveEvent(QDragMoveEvent *e)
{
    Message *msg = createMessage(e);
    if (msg){
        delete msg;
        e->acceptAction();
        return;
    }
    TextEdit::contentsDragMoveEvent(e);
}

MsgEdit::MsgEdit(QWidget *parent, UserWnd *userWnd)
        : QMainWindow(parent, NULL, 0)
{
    m_userWnd	= userWnd;
    m_msg		= NULL;
    m_bTyping	= false;
    m_type		= NO_TYPE;
    m_flags		= 0;
    m_retry.msg = NULL;
    m_bReceived = false;
    m_processor = NULL;
    m_recvProcessor = NULL;
    m_cmd.param = NULL;

    connect(CorePlugin::m_plugin, SIGNAL(modeChanged()), this, SLOT(modeChanged()));

    m_frame = new QFrame(this, "msgedit");
    setCentralWidget(m_frame);
    m_layout = new QVBoxLayout(m_frame);

    m_edit = new MsgTextEdit(this, m_frame);
    m_edit->setBackground(QColor(CorePlugin::m_plugin->getEditBackground() & 0xFFFFFF));
    m_edit->setForeground(QColor(CorePlugin::m_plugin->getEditForeground() & 0xFFFFFF), true);
    m_edit->setFont(CorePlugin::m_plugin->editFont);
    m_edit->setCtrlMode(!CorePlugin::m_plugin->getSendOnEnter());
    m_edit->setParam(this);
    setFocusProxy(m_edit);

    QStyleSheet *style = new QStyleSheet(m_edit);
    QStyleSheetItem *style_p = style->item("p");
    // Disable top and bottom margins for P tags. This will make sure
    // paragraphs have no more spacing than regular lines, thus matching
    // RTFs defaut look for paragraphs.
    style_p->setMargin(QStyleSheetItem::MarginTop, 0);
    style_p->setMargin(QStyleSheetItem::MarginBottom, 0);
    m_edit->setStyleSheet(style);

    connect(m_edit, SIGNAL(lostFocus()), this, SLOT(editLostFocus()));
    connect(m_edit, SIGNAL(textChanged()), this, SLOT(editTextChanged()));
    connect(m_edit, SIGNAL(ctrlEnterPressed()), this, SLOT(editEnterPressed()));
    connect(m_edit, SIGNAL(colorsChanged()), this, SLOT(colorsChanged()));
    connect(m_edit, SIGNAL(finished()), this, SLOT(editFinished()));
    connect(m_edit, SIGNAL(fontSelected(const QFont&)), this, SLOT(editFontChanged(const QFont&)));

    QFontMetrics fm(m_edit->font());
    m_edit->setMinimumSize(QSize(fm.maxWidth(), fm.height() + 10));
    m_layout->addWidget(m_edit);

    BarShow b;
    b.bar_id = ToolBarMsgEdit;
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
    typingStop();
    editLostFocus();
    if (m_retry.msg)
        delete m_retry.msg;
    emit finished();
}

void MsgEdit::editFinished()
{
    m_edit = NULL;
}

void MsgEdit::execCommand(CommandDef *cmd)
{
    if (m_cmd.param){
        Message *msg = (Message*)(m_cmd.param);
        delete msg;
    }
    m_cmd = *cmd;
    QTimer::singleShot(0, this, SLOT(execCommand()));
}

void MsgEdit::execCommand()
{
    if (m_cmd.param == NULL)
        return;
    Message *msg = (Message*)(m_cmd.param);
    Event e(EventCommandExec, &m_cmd);
    e.process();
    delete msg;
    m_cmd.param = NULL;
    switch (m_cmd.id){
    case CmdMsgQuote:
    case CmdMsgForward:
        break;
    default:
        goNext();
    }
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

void MsgEdit::editFontChanged(const QFont &f)
{
    if (!CorePlugin::m_plugin->getEditSaveFont())
        return;
    CorePlugin::m_plugin->editFont = f;
    Event e(EventHistoryFont);
    e.process();
}

bool MsgEdit::setMessage(Message *msg, bool bSetFocus)
{
    m_type = msg->type();
    m_userWnd->setMessageType(msg->type());
    m_resource  = msg->getResource();
    m_bReceived = msg->getFlags() & MESSAGE_RECEIVED;
    QObject *processor = NULL;
    MsgReceived *rcv = NULL;
    if (m_bReceived){
        if ((msg->getFlags() & MESSAGE_OPEN) || (CorePlugin::m_plugin->getContainerMode() == 0)){
            rcv = new MsgReceived(this, msg, true);
            processor = rcv;
        }else{
            if (m_recvProcessor == NULL){
                rcv = new MsgReceived(this, msg, false);
                m_recvProcessor = rcv;
            }
        }
    }else{
        QObject *(*create)(MsgEdit *custom, Message *msg) = NULL;
        CommandDef *cmd = CorePlugin::m_plugin->messageTypes.find(msg->baseType());
        if (cmd == NULL)
            return false;
        MessageDef *def = (MessageDef*)(cmd->param);
        if (def == NULL)
            return false;
        create = def->generate;
        if (create){
            m_userWnd->setStatus("");
            processor = create(this, msg);
        }
    }
    if (processor){
        if (m_recvProcessor){
            delete m_recvProcessor;
            m_recvProcessor = NULL;
        }
        if (m_processor){
            delete m_processor;
            m_processor = NULL;
        }
        m_processor = processor;
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

    m_bar->checkState();
    if (rcv)
        rcv->init();
    Command cmd;
    cmd->id			= CmdMultiply;
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= this;
    if (msg->getFlags() & MESSAGE_FORWARD){
        cmd->flags = COMMAND_CHECKED;
        m_userWnd->showListView(true);
    }
    Event eChange(EventCommandChecked, cmd);
    eChange.process();

    if (m_processor && bSetFocus)
        QTimer::singleShot(0, m_processor, SLOT(init()));
    return true;
}

Client *MsgEdit::client(void *&data, bool bCreate, bool bTyping, unsigned contact_id, bool bUseClient)
{
    data = NULL;
    Contact *contact = getContacts()->contact(contact_id);
    if (!bUseClient || m_client.empty()){
        if (contact == NULL)
            return NULL;
        vector<ClientStatus> cs;
        getWays(cs, contact);
        unsigned i;
        for (i = 0; i < cs.size(); i++){
            Client *client = getContacts()->getClient(cs[i].client);
            if (client->canSend(m_type, cs[i].data)){
                data = cs[i].data;
                if (bTyping)
                    changeTyping(client, data);
                return client;
            }
        }
        for (i = 0; i < cs.size(); i++){
            for (unsigned n = 0; n < getContacts()->nClients(); n++){
                Client *client = getContacts()->getClient(n);
                clientData *d = cs[i].data;
                Contact *c;
                if (!client->isMyData(d, c))
                    continue;
                if (c && (c != contact))
                    continue;
                if (client->canSend(m_type, d)){
                    if (bCreate)
                        client->createData(cs[i].data, contact);
                    data = cs[i].data;
                    if (bTyping)
                        changeTyping(client, data);
                    return client;
                }
            }
        }
        if (bTyping)
            changeTyping(NULL, NULL);
        return NULL;
    }
    if (contact == NULL)
        return NULL;
    void *d;
    ClientDataIterator it(contact->clientData);
    while ((d = ++it) != NULL){
        if (it.client()->dataName(d) == m_client){
            data = d;
            if (bTyping)
                changeTyping(it.client(), data);
            return it.client();
        }
    }
    if (bTyping)
        changeTyping(NULL, NULL);
    return NULL;
}

void MsgEdit::setInput()
{
    if (m_recvProcessor){
        delete m_recvProcessor;
        m_recvProcessor = NULL;
        m_bar->checkState();
    }
}

static Message *createGeneric(Buffer *cfg)
{
    return new Message(MessageGeneric, cfg);
}

static QObject *generateGeneric(MsgEdit *w, Message *msg)
{
    return new MsgGen(w, msg);
}

#if 0
i18n("Message", "%n messages", 1);
#endif

static MessageDef defGeneric =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT,
        "Message",
        "%n messages",
        createGeneric,
        generateGeneric,
        NULL
    };

static Message *createSMS(Buffer *cfg)
{
    return new SMSMessage(cfg);
}

static QObject* generateSMS(MsgEdit *w, Message *msg)
{
    return new MsgSMS(w, msg);
}

#if 0
i18n("SMS", "%n SMSs", 1);
#endif

static MessageDef defSMS =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT,
        "SMS",
        "%n SMSs",
        createSMS,
        generateSMS,
        NULL
    };

#if 0
i18n("URL", "%n URLs", 1);
#endif

static Message *createUrl(Buffer *cfg)
{
    return new UrlMessage(MessageUrl, cfg);
}

static QObject *generateUrl(MsgEdit *p, Message *msg)
{
    return new MsgUrl(p, msg);
}

static Message *dropUrl(QMimeSource *src)
{
    if (QUriDrag::canDecode(src)){
        QStringList l;
        if (QUriDrag::decodeLocalFiles(src, l))
            return NULL;
        if (!QUriDrag::decodeToUnicodeUris(src, l) || (l.count() < 1))
            return NULL;
        UrlMessage *msg = new UrlMessage;
        msg->setUrl(l[0]);
        return msg;
    }
    return NULL;
}

static MessageDef defUrl =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT,
        "URL",
        "%n URLs",
        createUrl,
        generateUrl,
        dropUrl
    };

static Message *createContacts(Buffer *cfg)
{
    return new ContactsMessage(MessageContacts, cfg);
}

static QObject *generateContacts(MsgEdit *p, Message *msg)
{
    return new MsgContacts(p, msg);
}

static Message *dropContacts(QMimeSource *src)
{
    if (ContactDragObject::canDecode(src)){
        Contact *contact = ContactDragObject::decode(src);
        ContactsMessage *msg = new ContactsMessage;
        QString name = contact->getName();
        msg->setContacts(QString("sim:") + QString::number(contact->id()) + "," + getToken(name, '/'));
        return msg;
    }
    return NULL;
}

#if 0
i18n("Contact list", "%n contact lists", 1);
#endif

static MessageDef defContacts =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT,
        "Contact list",
        "%n contact lists",
        createContacts,
        generateContacts,
        dropContacts
    };

static Message *createFile(Buffer *cfg)
{
    return new FileMessage(MessageFile, cfg);
}

static QObject* generateFile(MsgEdit *w, Message *msg)
{
    return new MsgFile(w, msg);
}

Message *dropFile(QMimeSource *src)
{
    if (QUriDrag::canDecode(src)){
        QStringList files;
        if (QUriDrag::decodeLocalFiles(src, files) && files.count()){
            QString fileName;
            for (QStringList::Iterator it = files.begin(); it != files.end(); ++it){
                if (!fileName.isEmpty())
                    fileName += ",";
                fileName += "\"";
                fileName += *it;
                fileName += "\"";
            }
            FileMessage *m = new FileMessage;
            m->setFile(fileName);
            return m;
        }
    }
    return NULL;
}

#if 0
i18n("File", "%n files", 1);
#endif

static CommandDef fileCommands[] =
    {
        {
            CmdFileAccept,
            I18N_NOOP("&Accept"),
            NULL,
            NULL,
            NULL,
            ToolBarMsgEdit,
            0x1090,
            MenuMessage,
            0,
            0,
            COMMAND_CHECK_STATE,
            NULL,
            NULL
        },
        {
            CmdFileDecline,
            I18N_NOOP("&Decline"),
            NULL,
            NULL,
            NULL,
            ToolBarMsgEdit,
            0x1091,
            MenuMessage,
            0,
            MenuFileDecline,
            COMMAND_CHECK_STATE,
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

static MessageDef defFile =
    {
        fileCommands,
        NULL,
        MESSAGE_DEFAULT,
        "File",
        "%n files",
        createFile,
        generateFile,
        dropFile
    };

static Message *createAuthRequest(Buffer *cfg)
{
    return new AuthMessage(MessageAuthRequest, cfg);
}

static QObject* generateAuth(MsgEdit *w, Message *msg)
{
    return new MsgAuth(w, msg);
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
            ToolBarMsgEdit,
            0x1080,
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
            ToolBarMsgEdit,
            0x1081,
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
        NULL,
        MESSAGE_DEFAULT | MESSAGE_SYSTEM,
        "Authorize request",
        "%n authorize requests",
        createAuthRequest,
        generateAuth,
        NULL
    };

static Message *createAuthGranted(Buffer *cfg)
{
    return new AuthMessage(MessageAuthGranted, cfg);
}

#if 0
i18n("Authorization granted", "%n authorization granted", 1);
#endif

static MessageDef defAuthGranted =
    {
        NULL,
        NULL,
        MESSAGE_SILENT,
        "Authorization granted",
        "%n authorization granted",
        createAuthGranted,
        generateAuth,
        NULL
    };

static Message *createAuthRefused(Buffer *cfg)
{
    return new AuthMessage(MessageAuthRefused, cfg);
}

#if 0
i18n("Authorization refused", "%n authorization refused", 1);
#endif

static MessageDef defAuthRefused =
    {
        NULL,
        NULL,
        MESSAGE_SILENT | MESSAGE_ERROR,
        "Authorization refused",
        "%n authorization refused",
        createAuthRefused,
        generateAuth,
        NULL
    };

static Message *createAdded(Buffer *cfg)
{
    return new AuthMessage(MessageAdded, cfg);
}

#if 0
i18n("Add to contacts", "%n add to contacts", 1);
#endif

static MessageDef defAdded =
    {
        NULL,
        NULL,
        MESSAGE_INFO | MESSAGE_SYSTEM,
        "Add to contacts",
        "%n add to contacts",
        createAdded,
        NULL,
        NULL
    };

static Message *createRemoved(Buffer *cfg)
{
    return new AuthMessage(MessageRemoved, cfg);
}

#if 0
i18n("Removed from contacts", "%n removed from contacts", 1);
#endif

static MessageDef defRemoved =
    {
        NULL,
        NULL,
        MESSAGE_INFO | MESSAGE_SYSTEM,
        "Removed from contacts",
        "%n removed from contacts",
        createRemoved,
        NULL,
        NULL
    };

static Message *createStatus(Buffer *cfg)
{
    return new StatusMessage(cfg);
}

#if 0
i18n("Status changed", "%n times status changed", 1);
#endif

static MessageDef defStatus =
    {
        NULL,
        NULL,
        MESSAGE_HIDDEN,
        "Status changed",
        "%n times status changed",
        createStatus,
        NULL,
        NULL
    };

static bool cmp_status(ClientStatus s1, ClientStatus s2)
{
    if (s1.status > s2.status)
        return true;
    if (s1.status < s2.status)
        return false;
    if (s1.data->LastSend.value > s2.data->LastSend.value)
        return true;
    if (s1.data->LastSend.value < s2.data->LastSend.value)
        return false;
    return s1.client < s2.client;
}

void MsgEdit::getWays(vector<ClientStatus> &cs, Contact *contact)
{
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        ClientDataIterator it(contact->clientData, client);
        clientData *data;
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
}

bool MsgEdit::sendMessage(Message *msg)
{
    if (m_retry.msg){
        delete m_retry.msg;
        m_retry.msg = NULL;
    }
    if (m_msg){
        delete msg;
        Event e(EventMessageCancel, m_msg);
        if (e.process())
            m_msg = NULL;
        stopSend(false);
        return false;
    }
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
    CorePlugin::m_plugin->setCloseSend(bClose);

    Contact *contact = getContacts()->contact(m_userWnd->id());
    if (contact){
        TranslitUserData *data = (TranslitUserData*)(contact->getUserData(CorePlugin::m_plugin->translit_data_id));
        if (data && data->Translit.bValue)
            msg->setFlags(msg->getFlags() | MESSAGE_TRANSLIT);
    }

    msg->setFlags(msg->getFlags() | m_flags);
    m_flags = 0;

    if (m_userWnd->m_list){
        multiply = m_userWnd->m_list->selected;
        if (multiply.empty())
            return false;
        multiply_it = multiply.begin();
        msg->setContact(*multiply_it);
        msg->setClient(NULL);
        ++multiply_it;
        if (multiply_it != multiply.end())
            msg->setFlags(msg->getFlags() | MESSAGE_MULTIPLY);
    }else if (!m_resource.isEmpty()){
        void *data = NULL;
        Client *c = client(data, true, false, msg->contact(), true);
        if (c){
            string resources = c->resources(data);
            while (!resources.empty()){
                string res = getToken(resources, ';');
                getToken(res, ',');
                if (m_resource == QString::fromUtf8(res.c_str())){
                    msg->setResource(m_resource);
                    break;
                }
            }
        }
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
    Contact *contact = getContacts()->contact(m_msg->contact());
    string client_str = m_msg->client();
    bool bSent = false;
    void *data = NULL;
    if (contact){
        Event e(EventMessageSend, m_msg);
        e.process();
        if (client_str.empty()){
            m_type = m_msg->type();
            Client *c = client(data, true, false, m_msg->contact(), (m_msg->getFlags() & MESSAGE_MULTIPLY) == 0);
            if (c){
                m_msg->setClient(c->dataName(data).c_str());
                bSent = c->send(m_msg, data);
            }else{
                data = NULL;
                for (i = 0; i < getContacts()->nClients(); i++){
                    Client *client = getContacts()->getClient(i);
                    if (client->send(m_msg, NULL)){
                        bSent = true;
                        break;
                    }
                }
            }
        }else{
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
    if (bSent){
        if (data){
            time_t now;
            time(&now);
            ((clientData*)data)->LastSend.value = now;
        }
    }else{
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
        m_userWnd->showListView(false);
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
    cmd->id		    = CmdSend;
    cmd->text	    = I18N_NOOP("&Send");
    cmd->icon	    = "mail_generic";
    cmd->bar_id		= ToolBarMsgEdit;
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

bool MsgEdit::setType(unsigned type)
{
    CommandDef *def;
    def = CorePlugin::m_plugin->messageTypes.find(type);
    if (def == NULL)
        return false;
    MessageDef *mdef = (MessageDef*)(def->param);
    if (mdef->flags & MESSAGE_SILENT)
        return false;
    if (mdef->create == NULL)
        return false;
    Message *msg = mdef->create(NULL);
    if (msg == NULL)
        return false;
    m_userWnd->setMessage(&msg);
    delete msg;
    return true;
}

bool MsgEdit::adjustType()
{
    if (m_bReceived)
        return true;
    Command cmd;
    cmd->menu_id = MenuMessage;
    cmd->param = (void*)(m_userWnd->m_id);
    cmd->id = m_userWnd->getMessageType();
    Event e1(EventCheckState, cmd);
    if ((m_userWnd->getMessageType() != m_type) && e1.process()){
        if (setType(m_userWnd->getMessageType()))
            return true;
    }
    cmd->id = m_type;
    Event e(EventCheckState, cmd);
    if (e.process())
        return true;
    Event eMenu(EventGetMenuDef, (void*)MenuMessage);
    CommandsDef *cmdsMsg = (CommandsDef*)(eMenu.process());
    CommandsList itc(*cmdsMsg, true);
    CommandDef *c;
    unsigned desired = m_userWnd->getMessageType();
    bool bSet = false;
    while ((c = ++itc) != NULL){
        if (c->id == CmdContactClients)
            continue;
        c->param = (void*)(m_userWnd->m_id);
        Event eCheck(EventCheckState, c);
        if (!eCheck.process())
            continue;
        if (setType(c->id)){
            bSet = true;
            break;
        }
    }
    m_userWnd->setMessageType(desired);
    return bSet;
}

void *MsgEdit::processEvent(Event *e)
{
    if ((e->type() == EventContactChanged) && (((Contact*)(e->param()))->id() == m_userWnd->m_id)){
        adjustType();
        return NULL;
    }
    if (e->type() == EventClientChanged){
        adjustType();
        return NULL;
    }
    if (e->type() == EventMessageReceived){
        Message *msg = (Message*)(e->param());
        if (msg->getFlags() & MESSAGE_NOVIEW)
            return NULL;
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
        if ((cmd->param == this) && (cmd->id == CmdTranslit)){
            Contact *contact = getContacts()->contact(m_userWnd->id());
            if (contact){
                TranslitUserData *data = (TranslitUserData*)(contact->getUserData(CorePlugin::m_plugin->translit_data_id));
                if (data){
                    cmd->flags &= ~COMMAND_CHECKED;
                    if (data->Translit.bValue)
                        cmd->flags |= COMMAND_CHECKED;
                }
            }
            return NULL;
        }
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
#if defined(USE_KDE)
#if KDE_IS_VERSION(3,2,0)
        if (cmd->id == CmdEnableSpell){
            m_edit->setCheckSpellingEnabled(cmd->flags & COMMAND_CHECKED);
            return NULL;
        }
        if ((cmd->id == CmdSpell) && (cmd->param == this)){
            m_edit->checkSpelling();
            return e->param();
        }
#endif
#endif
        if ((cmd->id == CmdSmile) && (cmd->param == this)){
            Event eBtn(EventCommandWidget, cmd);
            QToolButton *btnSmile = (QToolButton*)(eBtn.process());
            if (btnSmile){
                SmilePopup *popup = new SmilePopup(this);
                QSize s = popup->minimumSizeHint();
                popup->resize(s);
                connect(popup, SIGNAL(insert(int)), this, SLOT(insertSmile(int)));
                QPoint p = CToolButton::popupPos(btnSmile, popup);
                popup->move(p);
                popup->show();
            }
            return e->param();
        }
        if ((cmd->param == this) && (cmd->id == CmdTranslit)){
            Contact *contact = getContacts()->contact(m_userWnd->id());
            if (contact){
                TranslitUserData *data = (TranslitUserData*)(contact->getUserData(CorePlugin::m_plugin->translit_data_id, true));
                data->Translit.bValue = ((cmd->flags & COMMAND_CHECKED) != 0);
            }
            return e->param();;
        }
        if ((cmd->id == CmdMultiply) && (cmd->param == this)){
            m_userWnd->showListView((cmd->flags & COMMAND_CHECKED) != 0);
            return e->param();
        }
        if ((cmd->bar_id == ToolBarMsgEdit) && m_edit->isReadOnly() && (cmd->param == this)){
            switch (cmd->id){
            case CmdMsgAnswer:{
                    Message *msg = new Message(MessageGeneric);
                    msg->setContact(m_userWnd->id());
                    msg->setClient(m_client.c_str());
                    Event e(EventOpenMessage, &msg);
                    e.process();
                    delete msg;
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
    if ((e->type() == EventMessageSent) || (e->type() == EventMessageAcked)){
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
                if (msg->getRetryCode()){
                    m_retry.edit = this;
                    if (m_retry.msg)
                        delete m_retry.msg;
                    m_retry.msg  = new Message(msg->type());
                    m_retry.msg->setRetryCode(msg->getRetryCode());
                    m_retry.msg->setError(msg->getError());
                    Event e(EventMessageRetry, &m_retry);
                    if (e.process())
                        return NULL;
                }else{
                    BalloonMsg::message(err, msgWidget);
                }
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
                        Buffer config;
                        config << "[Title]\n" << cfg.c_str();
						config.setWritePos(0);
                        config.getSection();
                        m_msg = (mdef->create)(&config);
                        m_msg->setContact(*multiply_it);
                        m_msg->setClient(NULL);
                        m_msg->setFlags(m_msg->getFlags() | MESSAGE_MULTIPLY);
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
                CorePlugin::m_plugin->setCloseSend(bClose);
                if (bClose){
                    QTimer::singleShot(0, m_userWnd, SLOT(close()));
                }else{
                    setEmptyMessage();
                    m_edit->setFont(CorePlugin::m_plugin->editFont);
                    m_edit->setForeground(CorePlugin::m_plugin->getEditForeground(), true);
                    m_edit->setBackground(CorePlugin::m_plugin->getEditBackground());
                }
            }
            return NULL;
        }
    }
    return NULL;
}

void MsgEdit::setEmptyMessage()
{
    m_edit->setText("");
    Event eMenu(EventGetMenuDef, (void*)MenuMessage);
    CommandsDef *cmdsMsg = (CommandsDef*)(eMenu.process());
    CommandsList itc(*cmdsMsg, true);
    CommandDef *c;
    while ((c = ++itc) != NULL){
        c->param = (void*)(m_userWnd->m_id);
        Event eCheck(EventCheckState, c);
        if (eCheck.process()){
            Message *msg;
            CommandDef *def = CorePlugin::m_plugin->messageTypes.find(c->id);
            if (def == NULL)
                continue;
            MessageDef *mdef = (MessageDef*)(def->param);
            if (mdef->create == NULL)
                continue;
            msg = mdef->create(NULL);
            msg->setContact(m_userWnd->m_id);
            if (mdef->flags & MESSAGE_SILENT)
                continue;
            msg->setFlags(MESSAGE_NORAISE);
            Event eOpen(EventOpenMessage, &msg);
            eOpen.process();
            delete msg;
            return;
        }
    }
}

void MsgEdit::changeTyping(Client *client, void *data)
{
    if (!m_bTyping)
        return;
    if (client == NULL){
        typingStop();
        return;
    }
    if (client->dataName(data) == m_typingClient)
        return;
    typingStop();
    typingStart();
}

void MsgEdit::typingStart()
{
    typingStop();
    void *data = NULL;
    Client *cl = client(data, false, false, m_userWnd->id(), m_userWnd->m_list == NULL);
    if (cl == NULL)
        return;
    Message *msg = new Message(MessageTypingStart);
    if (cl->send(msg, data)){
        m_typingClient = cl->dataName(data);
    }else{
        delete msg;
    }
}

void MsgEdit::typingStop()
{
    if (m_typingClient.empty())
        return;
    Contact *contact = getContacts()->contact(m_userWnd->m_id);
    if (contact == NULL)
        return;
    ClientDataIterator it(contact->clientData);
    clientData *data;
    while ((data = ++it) != NULL){
        if (it.client()->dataName(data) == m_typingClient){
            Message *msg = new Message(MessageTypingStop);
            if (!it.client()->send(msg, data))
                delete msg;
            break;
        }
    }
    m_typingClient = "";
}

void MsgEdit::editTextChanged()
{
    bool bTyping = !m_edit->isEmpty();
    if (qApp->focusWidget() != m_edit)
        bTyping = false;
    if (m_bTyping == bTyping)
        return;
    m_bTyping = bTyping;
    if (m_bTyping){
        typingStart();
    }else{
        typingStop();
    }
}

void MsgEdit::editLostFocus()
{
    if (!m_bTyping)
        return;
    typingStop();
    m_bTyping = false;
}

void MsgEdit::colorsChanged()
{
    CorePlugin::m_plugin->setEditBackground(m_edit->background().rgb());
    CorePlugin::m_plugin->setEditForeground(m_edit->foreground().rgb());
    Event e(EventHistoryColors);
    e.process();
}

void MsgEdit::insertSmile(int id)
{
    if (m_edit->textFormat() == QTextEdit::PlainText){
        const smile *s = smiles(id);
        if (s)
            m_edit->insert(s->paste, false, true, true);
        return;
    }
    QString img_src = QString("<img src=icon:smile%1>").arg(QString::number(id, 16).upper());
    int para;
    int index;
    QFont saveFont = m_edit->font();
    QColor saveColor = m_edit->color();
    // determine the current position of the cursor
    m_edit->insert("\255", false, true, true);
    m_edit->getCursorPosition(&para,&index);
    // RTF doesnt like < and >
    QString txt = m_edit->text();
    txt.replace(QRegExp("\255"),img_src);
    m_edit->setText(txt);
    m_edit->setCursorPosition(para, index);
    m_edit->setCurrentFont(saveFont);
    m_edit->setColor(saveColor);
}

void MsgEdit::goNext()
{
    for (list<msg_id>::iterator it = CorePlugin::m_plugin->unread.begin(); it != CorePlugin::m_plugin->unread.end(); ++it){
        if ((*it).contact != m_userWnd->id())
            continue;
        Message *msg = History::load((*it).id, (*it).client.c_str(), (*it).contact);
        if (msg == NULL)
            continue;
        Event e(EventOpenMessage, &msg);
        e.process();
        delete msg;
        return;
    }
    if (CorePlugin::m_plugin->getContainerMode()){
        setEmptyMessage();
        return;
    }
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
    def = CorePlugin::m_plugin->messageTypes.find(type);

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

SmileLabel::SmileLabel(int _id, const char *tip, QWidget *parent)
        : QLabel(parent)
{
    id = _id;
    char b[20];
    sprintf(b, "smile%X", id);
    const QIconSet *icon = Icon(b);
    QPixmap pict;
    if (icon){
        if (!icon->isGenerated(QIconSet::Large, QIconSet::Normal)){
            pict = icon->pixmap(QIconSet::Large, QIconSet::Normal);
        }else{
            pict = icon->pixmap(QIconSet::Small, QIconSet::Normal);
        }
    }
    setPixmap(pict);
    if (tip && *tip)
        QToolTip::add(this, i18n(tip));
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
    QSize s;
    unsigned nSmiles = 0;
    unsigned i;
    for (i = 0; ; i++){
        const smile *p = smiles(i);
        if (p == NULL)
            break;
        if (*p->exp == 0)
            continue;
        char b[20];
        sprintf(b, "smile%X", i);
        const QIconSet *is = Icon(b);
        if (is == NULL)
            continue;
        QPixmap pict;
        if (!is->isGenerated(QIconSet::Large, QIconSet::Normal)){
            pict = is->pixmap(QIconSet::Large, QIconSet::Normal);
        }else{
            pict = is->pixmap(QIconSet::Small, QIconSet::Normal);
        }
        s = QSize(QMAX(s.width(), pict.width()), QMAX(s.height(), pict.height()));
        nSmiles++;
    }

    unsigned rows = 4;
    unsigned cols = (nSmiles + 3) / 4;
    if (cols > 8){
        cols = 8;
        rows = (nSmiles + 7) / cols;
    }

    QGridLayout *lay = new QGridLayout(this, rows, cols);
    lay->setMargin(4);
    lay->setSpacing(2);
    i = 0;
    unsigned j = 0;
    for (unsigned id = 0; ; id++){
        const smile *p = smiles(id);
        if (p == NULL)
            break;
        if (*p->exp == 0)
            continue;
        QWidget *w = new SmileLabel(id, p->title, this);
        w->setMinimumSize(s);
        connect(w, SIGNAL(clicked(int)), this, SLOT(labelClicked(int)));
        lay->addWidget(w, i, j);
        if (++j >= cols){
            i++;
            j = 0;
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

    cmd->id			 = MessageUrl;
    cmd->text		 = I18N_NOOP("&URL");
    cmd->icon		 = "url";
    cmd->accel		 = "Ctrl+U";
    cmd->menu_grp	 = 0x3030;
    cmd->flags		 = COMMAND_DEFAULT;
    cmd->param		 = &defUrl;
    eMsg.process();

    cmd->id			= MessageSMS;
    cmd->text		= I18N_NOOP("&SMS");
    cmd->icon		= "sms";
    cmd->accel		= "Ctrl+S";
    cmd->menu_grp	= 0x3040;
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= &defSMS;
    eMsg.process();

    cmd->id			= MessageContacts;
    cmd->text		= I18N_NOOP("&Contact list");
    cmd->icon		= "contacts";
    cmd->accel		= "Ctrl+L";
    cmd->menu_grp	= 0x3050;
    cmd->param		= &defContacts;
    eMsg.process();

    cmd->id			= MessageAuthRequest;
    cmd->text		= I18N_NOOP("&Authorization request");
    cmd->icon		= "auth";
    cmd->accel		= "Ctrl+A";
    cmd->menu_grp	= 0x3060;
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= &defAuthRequest;
    eMsg.process();

    cmd->id			= MessageAuthGranted;
    cmd->text		= I18N_NOOP("&Grant autorization");
    cmd->icon		= "auth";
    cmd->accel		= "Ctrl+G";
    cmd->menu_grp	= 0x3070;
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= &defAuthGranted;
    eMsg.process();

    cmd->id			= MessageAuthRefused;
    cmd->text		= I18N_NOOP("&Refuse autorization");
    cmd->icon		= "auth";
    cmd->accel		= "Ctrl+R";
    cmd->menu_grp	= 0x3071;
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

