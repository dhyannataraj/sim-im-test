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
#include "xsl.h"

#include <qstringlist.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qdatetime.h>

static char MSG_ANCHOR[] = "<a name=\"m:";
static char MSG_BEGIN[]  = "<a name=\"b\">";

typedef struct Smile
{
    unsigned	nSmile;
    int			pos;
    int			size;
    QRegExp		re;
} Smile;

class ViewParser : public HTMLParser
{
public:
    ViewParser(bool bIgnoreColors, bool bUseSmiles);
    QString parse(const QString &str);
protected:
    QString res;
    bool m_bIgnoreColors;
    bool m_bUseSmiles;
    bool m_bInLink;
    bool m_bInHead;
    bool m_bInParagraph;
    bool m_bFirst;
    bool m_bSpan;
    // Marks the position in 'res' where " DIR=\"whatever\"" should be inserted,
    // if the paragraph is DIR-less and we determine the DIR later on.
    unsigned m_paraDirInsertionPos;
    enum {
        DirAuto, // Initial BiDi dir when not explicitly specified.
        // Real dir will be determined from the first
        // strong BiDi character.
        DirLTR,
        DirRTL,
        DirUnknown
    } m_paragraphDir;
    list<Smile> m_smiles;
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &options);
    virtual void tag_end(const QString &tag);
};

/*
   This parser is run on the output of the 'history XSL'.
   The text which the XSL process output should generally be HTML,
   but may contain the following special tags:
 
    <prepend>...</prepend>
    Strips the PREPEND tags and prepends their contents to the beginning
    of the next paragraph. Useful to make sure chat prefixes are prepended
    to the first paragraph of a multi-paragraph message (instead of residing
    on a new paragraph).
*/

class XslOutputParser : public HTMLParser
{
public:
    XslOutputParser();
    QString parse(const QString &str);

protected:
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &options);
    virtual void tag_end(const QString &tag);

protected:
    QString res;
    bool m_bInPrepend;
    QString m_sPrepend;
};

XslOutputParser::XslOutputParser()
        : m_bInPrepend(false)
{
}

QString XslOutputParser::parse(const QString &str)
{
    res = "";
    HTMLParser::parse(str);
    if (!m_sPrepend.isEmpty())
        res = m_sPrepend + res;
    return res;
}

void XslOutputParser::text(const QString& text)
{
    if (m_bInPrepend)
        m_sPrepend += quoteString(text);
    else
        res += quoteString(text);
}

void XslOutputParser::tag_start(const QString &tag, const list<QString> &attrs)
{
    QString ltag = tag.lower();

    if (ltag == "prepend")
    {
        m_bInPrepend = true;
        return;
    }

    QString tagText;
    tagText += "<";
    tagText += tag;
    for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
        QString name = *it;
        ++it;
        QString value = *it;
        tagText += " ";
        tagText += name;
        if (!value.isEmpty()){
            tagText += "=\"";
            tagText += value;
            tagText += "\"";
        }
    }
    tagText += ">";

    if (m_bInPrepend)
    {
        m_sPrepend += tagText;
    }
    else
    {
        res += tagText;

        // It's time to prepend whatever we've got in m_sPrepend
        // to the start of a paragraph.
        if ((ltag == "p") && !m_sPrepend.isEmpty())
        {
            res += m_sPrepend;
            m_sPrepend = "";
        }
    }
}

void XslOutputParser::tag_end(const QString &tag)
{
    QString ltag = tag.lower();

    if (ltag == "prepend")
    {
        m_bInPrepend = false;
        return;
    }

    QString tagText;
    tagText += "</";
    tagText += tag;
    tagText += ">";

    if (m_bInPrepend)
        m_sPrepend += tagText;
    else
        res += tagText;
}

MsgViewBase::MsgViewBase(QWidget *parent, const char *name, unsigned id)
        : TextShow(parent, name)
{
    m_id = id;
    m_nSelection = 0;
    m_popupPos = QPoint(0, 0);
    xsl = NULL;

    QStyleSheet *style = new QStyleSheet(this);
    QStyleSheetItem *style_p = style->item("p");
    // Disable top and bottom margins for P tags. This will make sure
    // paragraphs have no more spacing than regular lines, thus matching
    // RTF's defaut look for paragraphs.
    style_p->setMargin(QStyleSheetItem::MarginTop, 0);
    style_p->setMargin(QStyleSheetItem::MarginBottom, 0);
    setStyleSheet(style);

    setColors();
    setFont(CorePlugin::m_plugin->editFont);
}

MsgViewBase::~MsgViewBase()
{
    if (xsl)
        delete xsl;
}

void MsgViewBase::setXSL(XSL *n_xsl)
{
    if (xsl)
        delete xsl;
    xsl = n_xsl;
}

void MsgViewBase::setSelect(const QString &str)
{
    m_nSelection = 0;
    m_selectStr = str;
}

void MsgViewBase::update()
{
    if (m_updated.empty())
        return;
    unsigned i;
    for (i = 0; i < (unsigned)paragraphs(); i++){
        QString s = text(i);
        int n = s.find(MSG_ANCHOR);
        if (n < 0)
            continue;
        s = s.mid(n + strlen(MSG_ANCHOR));
        n = s.find("\"");
        if (n < 0)
            continue;
        string client;
        unsigned id = messageId(s.left(n), client);
        list<Msg_Id>::iterator it;
        for (it = m_updated.begin(); it != m_updated.end(); ++it){
            if (((*it).id == id) && ((*it).client == client))
                break;
        }
        if (it != m_updated.end())
            break;
    }
    m_updated.clear();
    if (i >= (unsigned)paragraphs())
        return;
    int x = contentsX();
    int y = contentsY();
    viewport()->setUpdatesEnabled(false);

    unsigned start = i;
    list<Msg_Id> msgs;
    for (; i < (unsigned)paragraphs(); i++){
        QString s = text(i);
        int n = s.find(MSG_ANCHOR);
        if (n < 0)
            continue;
        s = s.mid(n + strlen(MSG_ANCHOR));
        n = s.find("\"");
        if (n < 0)
            continue;
        string client;
        unsigned id = messageId(s.left(n), client);
        list<Msg_Id>::iterator it;
        for (it = msgs.begin(); it != msgs.end(); ++it){
            if (((*it).id == id) && ((*it).client == client))
                break;
        }
        if (it != msgs.end())
            continue;
        Msg_Id m_id;
        m_id.id     = id;
        m_id.client = client;
        msgs.push_back(m_id);
    }
    int paraFrom, indexFrom;
    int paraTo, indexTo;
    getSelection(&paraFrom, &indexFrom, &paraTo, &indexTo);
    setReadOnly(false);
    setSelection(start, 0, paragraphs() - 1, 0xFFFF, 0);
    removeSelectedText();
    setReadOnly(true);
    QString text;
    for (list<Msg_Id>::iterator it = msgs.begin(); it != msgs.end(); ++it){
        Message *msg = History::load((*it).id, (*it).client.c_str(), m_id);
        if (msg == NULL)
            continue;
        bool bUnread = false;
        for (list<msg_id>::iterator itu = CorePlugin::m_plugin->unread.begin(); itu != CorePlugin::m_plugin->unread.end(); ++itu){
            msg_id &m = (*itu);
            if ((m.contact == msg->contact()) &&
                    (m.id == msg->id()) &&
                    (m.client == msg->client())){
                bUnread = true;
                break;
            }
        }
        text += messageText(msg, bUnread);
    }
    viewport()->setUpdatesEnabled(true);
    append(text);
    if (!CorePlugin::m_plugin->getOwnColors())
        setBackground(i);
    if ((paraFrom != paraTo) || (indexFrom != indexTo))
        setSelection(paraFrom, indexFrom, paraTo, indexTo, 0);
    TextShow::sync();
    setContentsPos(x, y);
    viewport()->repaint();
}

QString MsgViewBase::messageText(Message *msg, bool bUnread)
{
    QString options;
    QString info;
    QString status;

    const char *icon = "message";
    const CommandDef *def = CorePlugin::m_plugin->messageTypes.find(msg->type());
    if (def)
        icon = def->icon;
    bool bDirection = false;
    if (msg->type() == MessageStatus){
        icon = "empty";
        StatusMessage *sm = static_cast<StatusMessage*>(msg);
        Client *client = NULL;
        string clientStr;
        if (msg->client())
            clientStr = msg->client();
        int n = clientStr.find_last_of('.');
        if (n >= 0){
            clientStr = clientStr.substr(0, n);
        }else{
            clientStr = "";
        }
        if (!clientStr.empty()){
            for (unsigned i = 0; i < getContacts()->nClients(); i++){
                string n = getContacts()->getClient(i)->name();
                if (n.length() < clientStr.length())
                    continue;
                n = n.substr(0, clientStr.length());
                if (clientStr == n){
                    client = getContacts()->getClient(i);
                    break;
                }
            }
        }
        if ((client == NULL) && getContacts()->nClients())
            client = getContacts()->getClient(0);
        if (client){
            for (def = client->protocol()->statusList(); def->text; def++){
                if (def->id == sm->getStatus()){
                    icon = def->icon;
                    status = i18n(def->text);
                    break;
                }
            }
        }
        options += " direction=\"2\"";
        bDirection = true;
    }else{
        MessageDef *m_def = (MessageDef*)(def->param);
        if (m_def->flags & MESSAGE_INFO){
            options += " direction=\"2\"";
            bDirection = true;
        }
    }
    info = QString("<icon>%1</icon>") .arg(icon);

    QString contactName;
    if (msg->getFlags() & MESSAGE_RECEIVED){
        if (!bDirection)
            options += " direction=\"1\"";
        Contact *contact = getContacts()->contact(msg->contact());
        if (contact){
            contactName = contact->getName();
            if (contactName.isEmpty()){
                Client *client = NULL;
                ClientDataIterator it(contact->clientData);
                void *data;
                while ((data = ++it) != NULL){
                    if (it.client()->dataName(data) == msg->client()){
                        client = it.client();
                        break;
                    }
                }
            }
        }
        if (!bUnread){
            for (list<msg_id>::iterator it = CorePlugin::m_plugin->unread.begin(); it != CorePlugin::m_plugin->unread.end(); ++it){
                msg_id &m = (*it);
                if ((m.id == msg->id()) &&
                        (m.contact == msg->contact()) &&
                        (m.client == msg->client())){
                    bUnread = true;
                    break;
                }
            }
        }
        if (bUnread)
            options += " unread=\"1\"";
    }else{
        if (!bDirection)
            options += " direction=\"0\"";
        contactName = getContacts()->owner()->getName();
    }
    if (contactName.isEmpty())
        contactName = "???";
    info += QString("<from>%1</from>") .arg(quoteString(contactName));
    QString id = QString::number(msg->id());
    id += ",";
    // <hack>
    // Terrible hack to set message bgcolor. We prefer to insert the entire history
    // in one chunk (since it's more efficient and causes less redraws), and there's
    // no way to set block's background color directly in Qt's HTML), so we make a note
    // of it in the HTML and set it retroactively in setBackground.
    if ((msg->getBackground() != 0xFFFFFFFF) && (msg->getForeground() != msg->getBackground()))
        id += QString::number(msg->getBackground());
    // </hack>
    string client_str;
    if (msg->client())
        client_str = msg->client();
    if (!client_str.empty()){
        id += ",";
        id += quoteString(client_str.c_str());
    }
    if (m_cut.size()){
        id += ",";
        id += QString::number(m_cut.size());
    }
    info += "<id>";
    info += id;
    info += "</id>";


    QString icons;
    if (msg->getFlags() & MESSAGE_SECURE)
        options += " encrypted=\"1\"";
    if (msg->getFlags() & MESSAGE_URGENT)
        options += " urgent=\"1\"";
    if (msg->getFlags() & MESSAGE_LIST)
        options += " list=\"1\"";

    QString s;
    QDateTime t;
    t.setTime_t(msg->getTime());
    info += s.sprintf("<time><date>%%1</date><hour>%02u</hour><minute>%02u</minute><second>%02u</second></time>",
                      t.time().hour(), t.time().minute(), t.time().second()) .arg(formatDate(msg->getTime()));

    s = "<?xml version=\"1.0\"?><message";
    s += options;
    s += ">";
    s += info;

    QString msgText;
    if (msg->type() != MessageStatus){
        msgText = msg->presentation();
        if (msgText.isEmpty()){
            unsigned type = msg->baseType();
            CommandDef *cmd = CorePlugin::m_plugin->messageTypes.find(type);
            if (cmd){
                MessageDef *def = (MessageDef*)(cmd->param);
                msgText = i18n(def->singular, def->plural, 1);
                int n = msgText.find("1 ");
                if (n == 0){
                    msgText = msgText.mid(2);
                }else if (n > 0){
                    msgText = msgText.left(n);
                }
                msgText = QString("<p>") + msgText + "</p>";
            }
            QString text = msg->getRichText();
            msgText += text;
        }
    }else{
        msgText = status;
    }

    Event e(EventEncodeText, &msgText);
    e.process();
    ViewParser parser(CorePlugin::m_plugin->getOwnColors(), CorePlugin::m_plugin->getUseSmiles());
    msgText = parser.parse(msgText);
    s += "<body";

    if ((msg->getForeground() != 0xFFFFFFFF) && (msg->getForeground() != msg->getBackground()))
    {
        s += " fgcolor=\"#";
        s += QString::number(msg->getForeground(), 16).rightJustify(6, '0');
        s += "\"";
    }

    // Some bright day might come when one could specify background color from inside Qt's richtext.
    // Meanwhile, this is useless:
    if ((msg->getBackground() != 0xFFFFFFFF) && (msg->getForeground() != msg->getBackground()))
    {
        s += " bgcolor=\"#";
        s += QString::number(msg->getBackground(), 16).rightJustify(6, '0');
        s += "\"";
    }
    s += ">";

    // We pass the rich text quoted, since we're not sure of its' XML validity.
    // The XSL engine should copy it as-is (using xsl:value-of with disable-output-escaping="yes").
    s += quoteString(QString(MSG_BEGIN) + msgText);

    s += "</body>";
    s += "</message>";
    XSL *p = xsl;
    if (p == NULL)
        p = CorePlugin::m_plugin->historyXSL;
    QString res = p->process(s);

    XslOutputParser outParser;
    res = outParser.parse(res);

    QString anchor = MSG_ANCHOR;
    anchor += id;
    anchor += "\">";
    res = anchor + res;
    return res;
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
        Event e(EventOpenMessage, &msg);
        e.process();
        delete msg;
    }
}

// <hack>
// We have to use this function since Qt has no tag to set background color per-paragraph
// from within HTML. See matching hack in MsgViewBase::messageText.
void MsgViewBase::setBackground(unsigned n)
{
    QColor bgcolor;
    bool bInMsg = false;
    bool bSet   = false;

    QString sAnchor = QString::fromLatin1(MSG_ANCHOR),
                      sBegin = QString::fromLatin1(MSG_BEGIN);

    int i;
    for (i = n; i >= 0; i--){
        QString s = text(i);
        if (s.find(sAnchor) >= 0)
            break;
    }
    for (; i < paragraphs(); i++){
        QString s = text(i);
        int anchorPos = s.find(sAnchor);
        if (anchorPos >= 0)
        {
            bInMsg = false;
            bSet   = false;

            // This code could be a bit faster by making assumptions.
            // However, I prefer to be correct HTML-parser-wise.

            int idStart = anchorPos + sAnchor.length();
            int idEnd = s.find('\"', idStart);
            if ((idStart >= 0) && (idEnd >= 0))
            {
                QString id = s.mid(idStart, idEnd - idStart);

                // Parse the message id (msgId,backgroundColor,...)
                int bgcolorStart = id.find(',');
                if (bgcolorStart >= 0)
                {
                    QString sBgcolor = id.mid(bgcolorStart + 1);
                    int bgcolorEnd = sBgcolor.find(',');
                    if (bgcolorEnd > 0)
                        sBgcolor = sBgcolor.left(bgcolorEnd);
                    if (!sBgcolor.isEmpty())
                        bgcolor = QColor(sBgcolor.toULong(&bSet));
                }
            }
        }
        if (s.find(sBegin) >= 0)
            bInMsg = true;

        if (bInMsg && bSet){
            setParagraphBackgroundColor(i, bgcolor);
        }else{
            clearParagraphBackground(i);
        }
    }

}
// </hack>

void MsgViewBase::addMessage(Message *msg, bool bUnread, bool bSync)
{
    unsigned n = paragraphs();
    if (n > 0)
        n--;
    append(messageText(msg, bUnread));
    if (!CorePlugin::m_plugin->getOwnColors())
        setBackground(n);
    if (bSync)
        sync(n);
}

#ifdef WIN32

void MsgViewBase::sync(unsigned n)
{
    if (!m_selectStr.isEmpty()){
        bool bStart = false;
        for (; n < (unsigned)paragraphs(); n++){
            QString s = text(n);
            if (s.find(MSG_ANCHOR) >= 0){
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
            repaintChanged();
        }
    }
    TextShow::sync();
}

#else

void MsgViewBase::sync(unsigned)
{
    TextShow::sync();
}

#endif

bool MsgViewBase::findMessage(Message *msg)
{
    bool bFound = false;
    for (unsigned i = 0; i < (unsigned)paragraphs(); i++){
        QString s = text(i);
        int n = s.find(MSG_ANCHOR);
        if (n < 0)
            continue;
        s = s.mid(n + strlen(MSG_ANCHOR));
        n = s.find("\"");
        if (n < 0)
            continue;
        if (bFound){
            setCursorPosition(i, 0);
            moveCursor(MoveBackward, false);
            ensureCursorVisible();
            return true;
        }
        string client;
        if ((messageId(s.left(n), client) != msg->id()) || (client != msg->client()))
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

void MsgViewBase::setColors()
{
    TextShow::setBackground(CorePlugin::m_plugin->getEditBackground());
    TextShow::setForeground(CorePlugin::m_plugin->getEditForeground());
}

unsigned MsgViewBase::messageId(const QString &_s, string &client)
{
    QString s(_s);
    unsigned id = atol(getToken(s, ',').latin1());
    getToken(s, ',');
    client = getToken(s, ',').utf8();
    if (id >= 0x80000000)
        return id;
    for (unsigned cut_id = atol(s.latin1()); cut_id < m_cut.size(); cut_id++){
        if (m_cut[cut_id].client != client)
            continue;
        if (id < m_cut[cut_id].from)
            continue;
        id -= m_cut[cut_id].size;
    }
    return id;
}

void MsgViewBase::reload()
{
    QString t;
    vector<Msg_Id> msgs;
    unsigned i;
    for (i = 0; i < (unsigned)paragraphs(); i++){
        QString s = text(i);
        int n = s.find(MSG_ANCHOR);
        if (n < 0)
            continue;
        s = s.mid(n + strlen(MSG_ANCHOR));
        n = s.find("\"");
        if (n < 0)
            continue;
        string client;
        Msg_Id id;
        id.id = messageId(s.left(n), client);
        id.client = client;
        unsigned nn;
        for (nn = 0; nn < msgs.size(); nn++){
            if ((msgs[nn].id == id.id) && (msgs[nn].client == id.client))
                break;
        }
        if (nn >= msgs.size())
            msgs.push_back(id);
    }
    for (i = 0; i < msgs.size(); i++){
        Message *msg = History::load(msgs[i].id, msgs[i].client.c_str(), m_id);
        if (msg == NULL)
            continue;
        t += messageText(msg, false);
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

void *MsgViewBase::processEvent(Event *e)
{
    if ((e->type() == EventRewriteMessage) || (e->type() == EventMessageRead)){
        Message *msg = (Message*)(e->param());
        if (msg->contact() != m_id)
            return NULL;
        unsigned i;
        for (i = 0; i < (unsigned)paragraphs(); i++){
            QString s = text(i);
            int n = s.find(MSG_ANCHOR);
            if (n < 0)
                continue;
            s = s.mid(n + strlen(MSG_ANCHOR));
            n = s.find("\"");
            if (n < 0)
                continue;
            string client;
            if ((messageId(s.left(n), client) == msg->id()) && (client == msg->client()))
                break;
        }
        if (i >= (unsigned)paragraphs())
            return NULL;
        Msg_Id id;
        id.id     = msg->id();
        id.client = msg->client();
        m_updated.push_back(id);
        QTimer::singleShot(0, this, SLOT(update()));
        return NULL;
    }
    if (e->type() == EventCutHistory){
        CutHistory *ch = (CutHistory*)(e->param());
        if (ch->contact != m_id)
            return NULL;

        bool bDelete = false;
        vector<unsigned> start_pos;
        vector<unsigned> end_pos;
        for (unsigned i = 0; i < (unsigned)paragraphs(); i++){
            QString s = text(i);
            int n = s.find(MSG_ANCHOR);
            if (n < 0)
                continue;
            s = s.mid(n + strlen(MSG_ANCHOR));
            n = s.find("\"");
            if (n < 0)
                continue;
            string client;
            unsigned id = messageId(s.left(n), client);
            if ((client == ch->client) && (id >= ch->from) && (id < ch->from + ch->size)){
                if (!bDelete){
                    bDelete = true;
                    start_pos.push_back(i);
                }
            }else{
                if (bDelete){
                    bDelete = false;
                    end_pos.push_back(i);
                }
            }
        }
        if (bDelete)
            end_pos.push_back(paragraphs());
        if (start_pos.size()){
            int paraFrom, indexFrom;
            int paraTo, indexTo;
            getSelection(&paraFrom, &indexFrom, &paraTo, &indexTo);
            QPoint p = QPoint(0, 0);
            p = mapToGlobal(p);
            p = viewport()->mapFromGlobal(p);
            int x, y;
            viewportToContents(p.x(), p.y(), x, y);
            int para;
            int pos = charAt(QPoint(x, y), &para);
            setReadOnly(false);
            for (unsigned i = 0; i < start_pos.size(); i++){
                setSelection(start_pos[i], 0, end_pos[i], 0, 0);
                removeSelectedText();
                if ((unsigned)pos >= start_pos[i])
                    pos = end_pos[i] - start_pos[i];
            }
            if ((paraFrom == -1) && (paraTo == -1)){
                if (pos == -1){
                    scrollToBottom();
                }else{
                    setCursorPosition(para, pos);
                    ensureCursorVisible();
                }
            }else{
                setSelection(paraFrom, indexFrom, paraTo, indexTo, 0);
            }
            setReadOnly(true);
            repaint();
        }
        m_cut.push_back(*ch);
        return NULL;
    }
    if (e->type() == EventMessageDeleted){
        Message *msg = (Message*)(e->param());
        if (msg->contact() != m_id)
            return NULL;
        for (unsigned i = 0; i < (unsigned)paragraphs(); i++){
            QString s = text(i);
            int n = s.find(MSG_ANCHOR);
            if (n < 0)
                continue;
            s = s.mid(n + strlen(MSG_ANCHOR));
            n = s.find("\"");
            if (n < 0)
                continue;
            string client;
            if ((messageId(s.left(n), client) != msg->id()) || (client != msg->client()))
                continue;
            string ss;
            ss = text(i).local8Bit();
            log(L_DEBUG, "?: %s", ss.c_str());

            unsigned j;
            for (j = i + 1; j < (unsigned)paragraphs(); j++){
                QString s = text(j);
                ss = text(j).local8Bit();
                log(L_DEBUG, ">: %s", ss.c_str());
                int n = s.find(MSG_ANCHOR);
                if (n < 0)
                    continue;
                s = s.mid(n + strlen(MSG_ANCHOR));
                n = s.find("\"");
                if (n < 0)
                    continue;
                string client;
                if ((messageId(s.left(n), client) != msg->id()) || (client != msg->client()))
                    break;
            }
            int paraFrom, indexFrom;
            int paraTo, indexTo;
            getSelection(&paraFrom, &indexFrom, &paraTo, &indexTo);
            unsigned pos = 0xFFFF;
            if (j == (unsigned)paragraphs()){
                j++;
                pos = 0;
            }
            setSelection(i, 0, j - 1, pos, 0);
            setReadOnly(false);
            removeSelectedText();
            setReadOnly(true);
            if ((paraFrom == -1) && (paraTo == -1)){
                scrollToBottom();
            }else{
                setSelection(paraFrom, indexFrom, paraTo, indexTo, 0);
            }
            break;
        }
        return NULL;
    }
    if (e->type() == EventHistoryConfig){
        unsigned id = (unsigned)(e->param());
        if (id && (id != m_id))
            return NULL;
        reload();
    }
    if (e->type() == EventHistoryColors)
        setColors();
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
                unsigned type = msg->baseType();
                delete msg;
                CommandDef *def = CorePlugin::m_plugin->messageTypes.find(type);
                if (def == NULL)
                    return NULL;
                cmd->icon = def->icon;
                cmd->flags &= ~COMMAND_CHECKED;
                return e->param();
            }
            return NULL;
        case CmdMsgSpecial:
            msg = currentMessage();
            if (msg){
                Event eMenu(EventGetMenuDef, (void*)MenuMsgCommand);
                CommandsDef *cmdsMsg = (CommandsDef*)(eMenu.process());

                unsigned n = 0;
                MessageDef *mdef = NULL;
                unsigned type = msg->baseType();
                const CommandDef *cmdsSpecial = NULL;
                CommandDef *msgCmd = CorePlugin::m_plugin->messageTypes.find(type);
                if (msgCmd)
                    mdef = (MessageDef*)(msgCmd->param);

                if (mdef){
                    if (msg->getFlags() & MESSAGE_RECEIVED){
                        cmdsSpecial = mdef->cmdReceived;
                    }else{
                        cmdsSpecial = mdef->cmdSent;
                    }
                    if (cmdsSpecial)
                        for (const CommandDef *d = cmdsSpecial; d->text; d++)
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
                if (cmdsSpecial){
                    for (const CommandDef *d = cmdsSpecial; d->text; d++){
                        cmds[n] = *d;
                        cmds[n].id = CmdMsgSpecial + n;
                        cmds[n].flags = COMMAND_DEFAULT;
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
        case CmdCutHistory:
            msg = currentMessage();
            if (msg){
                History::cut(msg, 0, 0);
                delete msg;
                return e->param();
            }
            return NULL;
        case CmdDeleteMessage:
            msg = currentMessage();
            if (msg){
                History::del(msg);
                delete msg;
                return e->param();
            }
            return NULL;
        case CmdCopy:
            copy();
            return e->param();
        case CmdMsgOpen:
            msg = currentMessage();
            if (msg){
                msg->setFlags(msg->getFlags() | MESSAGE_OPEN);
                Event eOpen(EventOpenMessage, &msg);
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
                    unsigned type = msg->baseType();
                    CommandDef *msgCmd = CorePlugin::m_plugin->messageTypes.find(type);
                    if (msgCmd)
                        mdef = (MessageDef*)(msgCmd->param);
                    const CommandDef *cmds = NULL;
                    if (mdef){
                        if (msg->getFlags() & MESSAGE_RECEIVED){
                            cmds = mdef->cmdReceived;
                        }else{
                            cmds = mdef->cmdSent;
                        }
                    }

                    if (cmds){
                        unsigned n = cmd->id - CmdMsgSpecial;
                        for (const CommandDef *d = cmds; d->text; d++){
                            if (n-- == 0){
                                CommandDef cmd = *d;
                                cmd.param = msg;
                                cmd.menu_id = 0;
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
        int n = s.find(MSG_ANCHOR);
        if (n < 0)
            continue;
        s = s.mid(n + strlen(MSG_ANCHOR));
        n = s.find("\"");
        if (n < 0)
            continue;
        string client;
        unsigned id = messageId(s.left(n), client);
        Message *msg = History::load(id, client.c_str(), m_id);
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
        : MsgViewBase(parent, NULL, id)
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
            t = messageText(msg, false) + t;
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
    QTimer::singleShot(0, this, SLOT(init()));
}

MsgView::~MsgView()
{
}

void MsgView::init()
{
    TextShow::sync();
    scrollToBottom();
}

void *MsgView::processEvent(Event *e)
{
    if ((e->type() == EventSent) || (e->type() == EventMessageReceived)){
        Message *msg = (Message*)(e->param());
        if (msg->contact() != m_id)
            return NULL;
        if (msg->getFlags() & MESSAGE_NOVIEW)
            return NULL;
        bool bAdd = true;
        if (msg->type() == MessageStatus){
            bAdd = false;
            Contact *contact = getContacts()->contact(msg->contact());
            if (contact){
                CoreUserData *data = (CoreUserData*)(contact->getUserData(CorePlugin::m_plugin->user_data_id));
                if (data && data->LogStatus.bValue)
                    bAdd = true;
            }
        }
        if (bAdd && (e->type() == EventMessageReceived)){
            Contact *contact = getContacts()->contact(msg->contact());
            if (contact){
                CoreUserData *data = (CoreUserData*)(contact->getUserData(CorePlugin::m_plugin->user_data_id));
                if (data->OpenNewMessage.bValue)
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

ViewParser::ViewParser(bool bIgnoreColors, bool bUseSmiles)
{
    m_bIgnoreColors = bIgnoreColors;
    m_bUseSmiles    = bUseSmiles;
    m_bInLink       = false;
    m_bInHead       = false;
    m_bInParagraph  = false;
    m_bFirst     	= true;
    m_bSpan			= false;
    if (m_bUseSmiles){
        for (unsigned i = 0; ;i++){
            const smile *s = smiles(i);
            if (s == NULL)
                break;
#if COMPAT_QT_VERSION < 0x030000
            string str;
            for (const char *p = s->exp;; p++){
                if ((*p == 0) || (*p == '|')){
                    if (!str.empty()){
                        Smile ss;
                        ss.nSmile = i;
                        ss.re = QRegExp(str.c_str());
                        if (ss.re.isValid())
                            m_smiles.push_back(ss);
                    }
                    if (*p == 0)
                        break;
                    str = "";
                    continue;
                }
                if (*p == '\\'){
                    if (*(++p) == 0)
                        break;
                    str += '\\';
                    str += *p;
                    continue;
                }
                str += *p;
            }
#else
            if (*(s->exp)){
                Smile ss;
                ss.nSmile = i;
                ss.re = QRegExp(s->exp);
                if (ss.re.isValid())
                    m_smiles.push_back(ss);
            }
#endif
        }
    }
}

QString ViewParser::parse(const QString &str)
{
    res = "";
    HTMLParser::parse(str);
    return res;
}

void ViewParser::text(const QString &text)
{
    if (text.isEmpty())
        return;

    if (m_bInParagraph && (m_paragraphDir == DirAuto))
    {
        for(const QChar* c = text.unicode(); !c->isNull() && (m_paragraphDir == DirAuto); ++c)
        {
            // Note: Qt expects ltr/rtl to be lower-case.
            switch(c->direction())
            {
            case QChar::DirL:
                res.insert(m_paraDirInsertionPos, " dir=\"ltr\"");
                m_paragraphDir = DirLTR;
                break;
            case QChar::DirR:
                res.insert(m_paraDirInsertionPos, " dir=\"rtl\"");
                m_paragraphDir = DirRTL;
                break;
            default: // avoid gcc warning
                break;
            }
        }
    }

    if (!m_bUseSmiles || m_bInLink){
        res += quoteString(text);
        return;
    }
    m_bFirst = false;
    QString str = text;
    for (list<Smile>::iterator it = m_smiles.begin(); it != m_smiles.end(); ++it){
        Smile &s = *it;
        s.size = 0;
        s.pos = s.re.match(str, 0, &s.size);
        if (s.size == 0)
            s.pos = -1;
    }
    for (;;){
        unsigned pos = (unsigned)(-1);
        unsigned size = 0;
        Smile *curSmile = NULL;
        list<Smile>::iterator it;
        for (it = m_smiles.begin(); it != m_smiles.end(); ++it){
            Smile &s = *it;
            if (s.pos < 0)
                continue;
            if (((unsigned)(s.pos) < pos) || (((unsigned)(s.pos) == pos) && ((unsigned)(s.size) > size) && (s.pos != -1))){
                pos = s.pos;
                size = s.size;
                curSmile = &s;
            }
        }
        if ((curSmile == NULL) || (size == 0))
            break;
        if (pos)
            res += quoteString(str.left(pos));
        res += "<img src=\"icon:smile";
        res += QString::number(curSmile->nSmile, 16).upper();
        res += "\"/>";
        int len = pos + curSmile->size;
        str = str.mid(len);
        for (it = m_smiles.begin(); it != m_smiles.end(); ++it){
            Smile &s = *it;
            if (s.pos < 0)
                continue;
            s.pos -= len;
            if (s.pos < 0){
                s.size = 0;
                s.pos = s.re.match(str, 0, &s.size);
                if (s.size == 0)
                    s.pos = -1;
            }
        }
    }
    res += quoteString(str);
}

void ViewParser::tag_start(const QString &tag, const list<QString> &attrs)
{
    // the tag that will be actually written out
    QString oTag = tag;

    if (m_bInHead)
        return;

    QString style;

    if (tag == "img"){
        QString src;
        for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
            QString name = (*it).lower();
            ++it;
            QString value = *it;
            if (name == "src"){
                src = value;
                break;
            }
        }
        if (src.left(10) == "icon:smile"){
            bool bOK;
            unsigned nSmile = src.mid(10).toUInt(&bOK, 16);
            if (bOK){
                const smile *s = smiles(nSmile);
                if (s == NULL)
                    return;
                if (*s->exp == 0){
                    res += quoteString(s->paste);
                    return;
                }
            }
        }
    }else if (tag == "a"){
        m_bInLink = true;
    }else if (tag == "p"){
        m_bInParagraph = true;
        m_paragraphDir = DirAuto;
    }else if (tag == "html"){ // we display as a part of a larger document
        return;
    }else if (tag == "head"){
        m_bInHead = 1;
        return;
    }else if (tag == "body"){ // we display as a part of a larger document
        oTag = "span";
    }

    QString tagText;
    tagText += "<";
    tagText += oTag;

    if (tag == "p")
    {
        m_paraDirInsertionPos = res.length() + tagText.length();
    }

    for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
        QString name = (*it).lower();
        ++it;
        QString value = *it;

        // Handling for attributes of specific tags.
        if (tag == "body"){
            if (name == "bgcolor"){
                style += "background-color:" + value + ";";
                continue;
            }
        }else if (tag == "p"){
            if (name == "dir"){
                QString dir = value.lower();
                if (dir == "ltr")
                    m_paragraphDir = DirLTR;
                else if (dir == "rtl")
                    m_paragraphDir = DirRTL;
                else
                    m_paragraphDir = DirUnknown;
            }
        }else if (tag == "font"){
            if (name == "color" && m_bIgnoreColors)
                continue;
        }

        // Handle for generic attributes.
        if (name == "style"){
            style += value;
            continue;
        }

        tagText += " ";
        tagText += name;
        if (!value.isEmpty()){
            tagText += "=\"";
            tagText += value;
            tagText += "\"";
        }
    }

    // Quite crude but working CSS to remove color styling.
    // It won't filter out colors as part of 'background', but life's tough.
    // (If it's any comfort, Qt probably won't display it either.)
    if (!style.isEmpty()){
        if (m_bIgnoreColors){
            list<QString> opt = parseStyle(style);
            list<QString> new_opt;
            for (list<QString>::iterator it = opt.begin(); it != opt.end(); ++it){
                QString name = *it;
                it++;
                if (it == opt.end())
                    break;
                QString value = *it;
                if ((name == "color") ||
                        (name == "background-color") ||
                        (name == "font-size") ||
                        (name == "font-style") ||
                        (name == "font-weight") ||
                        (name == "font-family"))
                    continue;
                new_opt.push_back(name);
                new_opt.push_back(value);
            }
            style = makeStyle(new_opt);
        }
        if (!style.isEmpty())
            tagText += " style=\"" + style + "\"";
    }
    tagText += ">";
    res += tagText;
}

void ViewParser::tag_end(const QString &tag)
{
    QString oTag = tag;
    if (tag == "a"){
        m_bInLink = false;
    }else if (tag == "p"){
        m_bInParagraph = false;
        return;
    }else if (tag == "head"){
        m_bInHead = false;
        return;
    }else if (tag == "html"){
        return;
    }else if (tag == "body"){
        oTag = "span";
    }
    if (m_bInHead)
        return;
    res += "</";
    res += oTag;
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

