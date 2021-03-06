/***************************************************************************
                          msgview.cpp  -  description
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

#include "msgview.h"
#include "core.h"
#include "history.h"
#include "html.h"

#include <qstringlist.h>

MsgViewBase::MsgViewBase(QWidget *parent, unsigned id)
        : TextShow(parent)
{
    m_id = id;
    m_nSelection = 0;
    m_popupPos = QPoint(0, 0);
}

MsgViewBase::~MsgViewBase()
{
}

void MsgViewBase::setSelect(const QString &str)
{
    m_nSelection = 0;
    m_selectStr = str;
}

#define FONT_FORMAT "%06lX"
QString MsgViewBase::messageText(Message *msg)
{
    QString color;
    unsigned long c_sender   = 0x800000;
    unsigned long c_receiver = 0x000080;
    unsigned long c_send     = 0x000000;
    unsigned long c_receive  = 0x000000;
    if (CorePlugin::m_plugin->getOwnColors()) {
        c_send     = (CorePlugin::m_plugin->getColorSend())    & 0xFFFFFF;
        c_receive  = (CorePlugin::m_plugin->getColorReceive()) & 0xFFFFFF;
        c_sender   = (CorePlugin::m_plugin->getColorSender())  & 0xFFFFFF;
        c_receiver = (CorePlugin::m_plugin->getColorReceiver())& 0xFFFFFF;
    }
    color.sprintf(FONT_FORMAT,
                  ((msg->getFlags() & MESSAGE_RECEIVED) ? c_receiver : c_sender));
    const char *icon = "message";
    const CommandDef *def = CorePlugin::m_plugin->messageTypes.find(msg->type());
    if (def)
        icon = def->icon;
    QString contactName;
    Client *client = NULL;
    Contact *contact = getContacts()->contact(msg->contact());
    if (contact){
        ClientDataIterator it(contact->clientData);
        void *data;
        while ((data = ++it) != NULL){
            if (it.client()->dataName(data) == msg->client()){
                client = it.client();
                break;
            }
        }
    }
    if (msg->type() == MessageStatus){
        icon = "empty";
        StatusMessage *sm = static_cast<StatusMessage*>(msg);
        if (client == NULL)
            client = getContacts()->getClient(0);
        if (client){
            for (def = client->protocol()->statusList(); def->text; def++){
                if (def->id == sm->getStatus()){
                    icon = def->icon;
                    break;
                }
            }
        }
    }
    bool bUnread = false;
    if (msg->getFlags() & MESSAGE_RECEIVED){
        if (contact)
            contactName = contact->getName();
        for (list<msg_id>::iterator it = CorePlugin::m_plugin->unread.begin(); it != CorePlugin::m_plugin->unread.end(); ++it){
            msg_id &m = (*it);
            if ((m.id == msg->id()) &&
                    (m.contact == msg->contact()) &&
                    (m.client == msg->client())){
                bUnread = true;
                break;
            }
        }
    }else{
        if (client)
            contactName = client->ownerName();
        if (contactName.isEmpty())
            contactName = getContacts()->owner()->getName();
    }
    if (contactName.isEmpty())
        contactName = "???";
    QString id = QString::number(msg->id());
    id += ",";
    if (msg->getBackground() != msg->getForeground())
        id += QString::number(msg->getBackground() & 0xFFFFFF);
    string client_str;
    if (msg->client())
        client_str = msg->client();
    if (!client_str.empty()){
        id += ",";
        id += quoteText(client_str.c_str());
    }
    QString icons;
    if (msg->getFlags() & MESSAGE_SECURE)
        icons += "<img src=\"icon:encrypted\">";
    if (msg->getFlags() & MESSAGE_URGENT)
        icons += "<img src=\"icon:urgentmsg\">";
    if (msg->getFlags() & MESSAGE_LIST)
        icons += "<img src=\"icon:listmsg\">";

    QString s = QString("<p><nobr>"
                        "<a href=\"msg://%1\"><img src=\"icon:%2\"></a>%3"
                        "&nbsp;%4<font color=\"#%5\">%6</font> &nbsp;"
                        "<font size=-1>%7</font>%8"
                        "</nobr></p>")
                .arg(id)
                .arg(icon)
                .arg(icons)
                .arg(bUnread ? "<b>" : "")
                .arg(color)
                .arg(quoteString(contactName))
                .arg(formatTime(msg->getTime()))
                .arg(bUnread ? "</b>" : "");
    if (msg->type() != MessageStatus){
        QString msgText = msg->presentation();
        // replace font color if we use own colors
        // some of the incoming messages are saved as html with an extra <font> - tag
        // so we need to replace until no more <font> - tag is found
        // this behaviour should be checked!
        if (CorePlugin::m_plugin->getOwnColors()) {
            int pos = msgText.find("font color=\"#",0);
            while (pos != -1) {
                pos += 13;
                QString color;
                color.sprintf(FONT_FORMAT, (msg->getFlags() & MESSAGE_RECEIVED) ? c_receive : c_send);
                msgText.replace(pos,6,color);
                pos = msgText.find("font color=\"#",pos);
            }
        }
        if (msgText.isEmpty()){
            unsigned type = msg->type();
            for (;;){
                CommandDef *cmd = CorePlugin::m_plugin->messageTypes.find(type);
                if (cmd == NULL)
                    break;
                MessageDef *def = (MessageDef*)(cmd->param);
                if (def->base_type){
                    type = def->base_type;
                    continue;
                }
                msgText += "<p>";
                msgText += i18n(def->singular, def->plural, 1);
                msgText += "</p>";
                break;
            }
            QString text = msg->getRichText();
            if (!text.isEmpty()){
                msgText += "<p>";
                msgText += text;
                msgText += "</p>";
            }
        }
        Event e(EventEncodeText, &msgText);
        e.process();
        s += parseText(msgText, false, CorePlugin::m_plugin->getUseSmiles());
    }
    return s;
}

void MsgViewBase::setSource(const QString &url)
{
    QString proto;
    int n = url.find(':');
    if (n >= 0)
        proto = url.left(n);
    if (proto != "msg"){
        TextShow::setSource(url);
        return;
    }
    QString id = url.mid(proto.length() + 3);
    unsigned msg_id = atol(getToken(id, ',').latin1());
    getToken(id, ',');
    id = getToken(id, '/');
    QString client = SIM::unquoteString(id);
    if (client.isEmpty())
        client = QString::number(m_id);
    Message *msg = History::load(msg_id, client.utf8(), m_id);
    if (msg){
        Event e(EventOpenMessage, msg);
        e.process();
        delete msg;
    }
}

static char MSG_HREF[] = "<a href=\"msg://";

void MsgViewBase::setBackground(unsigned n)
{
    QColor c;
    bool bSet = false;
    for (unsigned i = n; i < (unsigned)paragraphs(); i++){
        QString s = text(i);
        int n = s.find(MSG_HREF);
        if (n < 0){
            if (bSet){
                setParagraphBackgroundColor(i, c);
            }else{
                clearParagraphBackground(i);
            }
            continue;
        }
        clearParagraphBackground(i);
        s = s.mid(n + strlen(MSG_HREF));
        int p = s.find('\"');
        if (p >= 0)
            s = s.left(p);
        getToken(s, ',');
        s = getToken(s, ',');
        if (s.isEmpty()){
            bSet = false;
            continue;
        }
        c = QColor(atol(s.latin1()));
        bSet = true;
    }
}

void MsgViewBase::addMessage(Message *msg)
{
    unsigned n = paragraphs() - 1;
    append(messageText(msg));
    if (!CorePlugin::m_plugin->getOwnColors()) {
        // set all Backgrounds to the right colors
        setBackground(0);
    }
    if (!m_selectStr.isEmpty()){
        bool bStart = false;
        for (; n < (unsigned)paragraphs(); n++){
            QString s = text(n);
            if (s.find(MSG_HREF) >= 0){
                bStart = true;
                continue;
            }
            if (bStart)
                break;
        }
        if (n < (unsigned)paragraphs()){
            int savePara;
            int saveIndex;
            getCursorPosition(&savePara, &saveIndex);
            int para = n;
            int index = 0;
            while (find(m_selectStr, false, false, true, &para, &index)){
                setSelection(para, index, para, index + m_selectStr.length(), ++m_nSelection);
                setSelectionAttributes(m_nSelection, colorGroup().highlight(), true);
                index += m_selectStr.length();
            }
            setCursorPosition(savePara, saveIndex);
        }
    }
    sync();
}

bool MsgViewBase::findMessage(Message *msg)
{
    bool bFound = false;
    for (unsigned i = 0; i < (unsigned)paragraphs(); i++){
        QString s = text(i);
        int n = s.find(MSG_HREF);
        if (n < 0)
            continue;
        s = s.mid(n + strlen(MSG_HREF));
        n = s.find("\"");
        if (n < 0)
            continue;
        if (bFound){
            setCursorPosition(i, 0);
            moveCursor(MoveBackward, false);
            ensureCursorVisible();
            return true;
        }
        s = s.left(n);
        unsigned id = atol(getToken(s, ',').latin1());
        if (id != msg->id())
            continue;
        getToken(s, ',');
        if (s != msg->client())
            continue;
        setCursorPosition(i, 0);
        ensureCursorVisible();
        bFound = true;
    }
    if (!bFound)
        return false;
    moveCursor(MoveEnd, false);
    ensureCursorVisible();
    return true;
}

void *MsgViewBase::processEvent(Event *e)
{
    if (e->type() == EventMessageRead){
        Message *msg = (Message*)(e->param());
        if (msg->contact() != m_id)
            return NULL;
        for (unsigned i = 0; i < (unsigned)paragraphs(); i++){
            QString s = text(i);
            int n = s.find(MSG_HREF);
            if (n < 0)
                continue;
            s = s.mid(n + strlen(MSG_HREF));
            n = s.find("\"");
            if (n < 0)
                continue;
            s = s.left(n);
            unsigned id = atol(getToken(s, ',').latin1());
            if (id != msg->id())
                continue;
            getToken(s, ',');
            if (s != msg->client())
                continue;
            int paraFrom, indexFrom;
            int paraTo, indexTo;
            getSelection(&paraFrom, &indexFrom, &paraTo, &indexTo);
            setSelection(i, 0, i, 0xFFFF);
            setBold(false);
            if ((paraFrom == -1) && (paraTo == -1)){
                removeSelection();
                scrollToBottom();
            }else{
                setSelection(paraFrom, indexFrom, paraTo, indexTo);
            }
            break;
        }
        return NULL;
    }
    if (e->type() == EventHistoryConfig){
        unsigned id = (unsigned)(e->param());
        if (id && (id != m_id))
            return NULL;
        QString t;
        for (unsigned i = 0; i < (unsigned)paragraphs(); i++){
            QString s = text(i);
            int n = s.find(MSG_HREF);
            if (n < 0)
                continue;
            s = s.mid(n + strlen(MSG_HREF));
            n = s.find("\"");
            if (n < 0)
                continue;
            s = s.left(n);
            unsigned id = atol(getToken(s, ',').latin1());
            getToken(s, ',');
            Message *msg = History::load(id, s.utf8(), m_id);
            if (msg == NULL)
                continue;
            t += messageText(msg);
            delete msg;
        }
        QPoint p = QPoint(0, height());
        p = mapToGlobal(p);
        p = viewport()->mapFromGlobal(p);
        int x, y;
        viewportToContents(p.x(), p.y(), x, y);
        int para;
        int pos = charAt(QPoint(x, y), &para);
        setText(t);
        if (!CorePlugin::m_plugin->getOwnColors())
            setBackground(0);
        if (pos == -1){
            scrollToBottom();
        }else{
            setCursorPosition(para, pos);
            ensureCursorVisible();
        }
    }
    if (e->type() == EventCheckState){
        CommandDef *cmd = (CommandDef*)(e->param());
        if ((cmd->param != this) || (cmd->menu_id != MenuMsgView))
            return NULL;
        Message *msg;
        switch (cmd->id){
        case CmdCopy:
            cmd->flags &= ~(COMMAND_DISABLED | COMMAND_CHECKED);
            if (!hasSelectedText())
                cmd->flags |= COMMAND_DISABLED;
            return e->param();
        case CmdMsgOpen:
            msg = currentMessage();
            if (msg){
                unsigned type = msg->type();
                delete msg;
                for (;;){
                    CommandDef *def = CorePlugin::m_plugin->messageTypes.find(type);
                    if (def == NULL){
                        return NULL;
                    }
                    MessageDef *mdef = (MessageDef*)(def->param);
                    if (mdef->base_type){
                        type = mdef->base_type;
                        continue;
                    }
                    cmd->icon = def->icon;
                    cmd->flags &= ~COMMAND_CHECKED;
                    return e->param();
                }
            }
            return NULL;
        case CmdMsgSpecial:
            msg = currentMessage();
            if (msg){
                Event eMenu(EventGetMenuDef, (void*)MenuMsgCommand);
                CommandsDef *cmdsMsg = (CommandsDef*)(eMenu.process());

                unsigned n = 0;
                MessageDef *mdef = NULL;
                if (msg->getFlags() & MESSAGE_RECEIVED){
                    unsigned type = msg->type();
                    for (;;){
                        CommandDef *msgCmd = CorePlugin::m_plugin->messageTypes.find(type);
                        if (msgCmd == NULL)
                            break;
                        mdef = (MessageDef*)(msgCmd->param);
                        if (mdef->base_type == 0)
                            break;
                        type = mdef->base_type;
                    }
                }
                if (mdef && mdef->cmd){
                    for (const CommandDef *d = mdef->cmd; d->text; d++)
                        n++;
                }

                {
                    CommandsList it(*cmdsMsg, true);
                    while (++it)
                        n++;
                }
                if (n == 0)
                    return NULL;

                n++;
                CommandDef *cmds = new CommandDef[n];
                memset(cmds, 0, sizeof(CommandDef) * n);
                n = 0;
                if (mdef && mdef->cmd){
                    for (const CommandDef *d = mdef->cmd; d->text; d++){
                        cmds[n] = *d;
                        cmds[n].id = CmdMsgSpecial + n;
                        n++;
                    }
                }
                CommandDef *c;
                CommandsList it(*cmdsMsg, true);
                while ((c = ++it) != NULL){
                    CommandDef cmd = *c;
                    cmd.menu_id = MenuMsgCommand;
                    cmd.param   = msg;
                    Event e(EventCheckState, &cmd);
                    if (!e.process())
                        continue;
                    cmd.flags &= ~COMMAND_CHECK_STATE;
                    cmds[n++] = cmd;
                }
                cmd->param = cmds;
                cmd->flags |= COMMAND_RECURSIVE;
                delete msg;
                return e->param();
            }
            return NULL;
        }
    }
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if ((cmd->param != this) || (cmd->menu_id != MenuMsgView))
            return NULL;
        Message *msg;
        switch (cmd->id){
        case CmdCopy:
            copy();
            return e->param();
        case CmdMsgOpen:
            msg = currentMessage();
            if (msg){
                Event eOpen(EventOpenMessage, msg);
                eOpen.process();
                delete msg;
                return e->param();
            }
            return NULL;
        default:
            msg = currentMessage();
            if (msg){
                if (cmd->id >= CmdMsgSpecial){
                    MessageDef *mdef = NULL;
                    unsigned type = msg->type();
                    for (;;){
                        CommandDef *msgCmd = CorePlugin::m_plugin->messageTypes.find(type);
                        if (msgCmd == NULL)
                            break;
                        mdef = (MessageDef*)(msgCmd->param);
                        if (mdef->base_type == 0)
                            break;
                        type = mdef->base_type;
                    }
                    if (mdef && mdef->cmd){
                        unsigned n = cmd->id - CmdMsgSpecial;
                        for (const CommandDef *d = mdef->cmd; d->text; d++){
                            if (n-- == 0){
                                CommandDef cmd = *d;
                                cmd.param = msg;
                                Event eCmd(EventCommandExec, &cmd);
                                eCmd.process();
                                return e->param();
                            }
                        }
                    }
                }
                Command c;
                c->id = cmd->id;
                c->menu_id = MenuMsgCommand;
                c->param = msg;
                Event e(EventCommandExec, c);
                void *res = e.process();
                delete msg;
                return res;
            }
            return NULL;
        }
    }
    return NULL;
}

Message *MsgViewBase::currentMessage()
{
    int para = paragraphAt(m_popupPos);
    if (para < 0)
        return NULL;
    for (; para >= 0; para--){
        QString s = text(para);
        int n = s.find(MSG_HREF);
        if (n < 0)
            continue;
        s = s.mid(n + strlen(MSG_HREF));
        unsigned long id = atol(getToken(s, ',').latin1());
        getToken(s, ',');
        s = getToken(s, '\"');
        Message *msg = History::load(id, s.utf8(), m_id);
        if (msg)
            return msg;
    }
    return NULL;
}

QPopupMenu *MsgViewBase::createPopupMenu(const QPoint& pos)
{
    m_popupPos = pos;
    Command cmd;
    cmd->popup_id	= MenuMsgView;
    cmd->param		= this;
    cmd->flags		= COMMAND_NEW_POPUP;
    Event e(EventGetMenu, cmd);
    return (QPopupMenu*)(e.process());
}

MsgView::MsgView(QWidget *parent, unsigned id)
        : MsgViewBase(parent, id)
{
    int nCopy = CorePlugin::m_plugin->getCopyMessages();
    unsigned nUnread = 0;
    for (list<msg_id>::iterator it = CorePlugin::m_plugin->unread.begin(); it != CorePlugin::m_plugin->unread.end(); ++it){
        msg_id &m = (*it);
        if (m.contact == m_id)
            nUnread++;
    }
    if (nCopy || nUnread){
        QString t = text();
        HistoryIterator it(m_id);
        it.end();
        while ((nCopy > 0) || nUnread){
            Message *msg = --it;
            if (msg == NULL)
                break;
            t = messageText(msg) + t;
            nCopy--;
            if (nUnread == 0)
                continue;
            for (list<msg_id>::iterator it = CorePlugin::m_plugin->unread.begin(); it != CorePlugin::m_plugin->unread.end(); ++it){
                msg_id &m = (*it);
                if ((m.contact == msg->contact()) &&
                        (m.id == msg->id()) &&
                        (m.client == msg->client())){
                    nUnread--;
                    break;
                }
            }
        }
        setText(t);
        if (!CorePlugin::m_plugin->getOwnColors())
            setBackground(0);
    }
    scrollToBottom();
}

MsgView::~MsgView()
{
}

void *MsgView::processEvent(Event *e)
{
    if ((e->type() == EventSent) || (e->type() == EventMessageReceived)){
        Message *msg = (Message*)(e->param());
        if (msg->contact() != m_id)
            return NULL;
        bool bAdd = true;
        if (msg->type() == MessageStatus){
            bAdd = false;
            Contact *contact = getContacts()->contact(msg->contact());
            if (contact){
                CoreUserData *data = (CoreUserData*)(contact->getUserData(CorePlugin::m_plugin->user_data_id));
                if (data && data->LogStatus)
                    bAdd = true;
            }
        }
        if (bAdd && (e->type() == EventMessageReceived)){
            Contact *contact = getContacts()->contact(msg->contact());
            if (contact){
                CoreUserData *data = (CoreUserData*)(contact->getUserData(CorePlugin::m_plugin->user_data_id));
                if (data->OpenOnReceive)
                    bAdd = false;
            }
        }
        if (bAdd){
            addMessage(msg);
            if (!hasSelectedText())
                scrollToBottom();
        }
    }
    return MsgViewBase::processEvent(e);
}

class ViewParser : public HTMLParser
{
public:
	ViewParser(bool bIgnoreColors, bool bUseSmiles);
	QString parse(const QString &str);
protected:
	QString res;
	bool m_bIgnoreColors;
	bool m_bUseSmiles;
	virtual void text(const QString &text);
	virtual void tag_start(const QString &tag, const list<QString> &options);
	virtual void tag_end(const QString &tag);
};

ViewParser::ViewParser(bool bIgnoreColors, bool bUseSmiles)
{
	m_bIgnoreColors = bIgnoreColors;
	m_bUseSmiles    = bUseSmiles;
}

QString ViewParser::parse(const QString &str)
{
	res = "";
	HTMLParser::parse(str);
	return res;
}

typedef struct Smile
{
	unsigned nSmile;
	int		 pos;
	string	 smile;
} Smile;

void ViewParser::text(const QString &text)
{
	if (!m_bUseSmiles)
		res += text;
	QString str = text;
	list<Smile> s;
	for (unsigned i = 0;; i++){
		const char *p = smiles(i);
		if (p == 0)
			break;
		for (; *p; p += strlen(p) + 1){
			Smile sm;
			sm.nSmile = i;
			sm.smile  = p;
			sm.pos    = str.find(p);
			s.push_back(sm);
		}
	}
	for (;;){
		unsigned pos = (unsigned)(-1);
		Smile *curSmile = NULL;
		list<Smile>::iterator it;
		for (it = s.begin(); it != s.end(); ++it){
			if ((*it).pos < 0)
				continue;
			if ((*it).pos < pos){
				pos = (*it).pos;
				curSmile = &(*it);
			}
		}
		if (curSmile == NULL)
			break;
		if (pos)
			res += quoteString(str.left(pos));
		res += "<img src=\"icon:smile";
		res += QString::number(curSmile->nSmile, 16).upper();
		res += "\">";
		int len = pos + curSmile->smile.size();
		str = str.mid(len);
		for (it = s.begin(); it != s.end(); ++it){
			if ((*it).pos < 0)
				continue;
			(*it).pos -= len;
			if ((*it).pos < 0)
				(*it).pos = str.find((*it).smile.c_str());
		}
	}
	res += quoteString(str);
}

static const char *formatTags[] = 
{
	"b",
	"i",
	"u",
	"br",
	"p",
	NULL
};

void ViewParser::tag_start(const QString &tag, const list<QString> &options)
{
	if (m_bIgnoreColors){
		const char **p;
		for (p = formatTags; *p; p++)
			if (tag == *p)
				break;
		if (*p == NULL)
			return;
	}
	res += "<";
	res += tag;
	if (!m_bIgnoreColors){
		for (list<QString>::const_iterator it = options.begin(); it != options.end(); ++it){
			QString opt = *it;
			++it;
			QString val = *it;
			res += " ";
			res += opt;
			if (!val.isEmpty()){
				res += "=\"";
				res += quoteString(val);
				res += "\"";
			}
		}
	}
	res += ">";
}

void ViewParser::tag_end(const QString &tag)
{
	if (m_bIgnoreColors){
		const char **p;
		for (p = formatTags; *p; p++)
			if (tag == *p)
				break;
		if (*p == NULL)
			return;
	}
	res += "</";
	res += tag;
	res += ">";
}

QString MsgViewBase::parseText(const QString &text, bool bIgnoreColors, bool bUseSmiles)
{
	ViewParser parser(bIgnoreColors, bUseSmiles);
	return parser.parse(text);
}

#ifndef WIN32
#include "msgview.moc"
#endif

