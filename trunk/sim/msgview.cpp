/***************************************************************************
                          msgview.cpp  -  description
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

#include "msgview.h"
#include "mainwin.h"
#include "client.h"
#include "cuser.h"
#include "history.h"
#include "log.h"
#include "icons.h"
#include "transparent.h"

#if USE_KDE
#include <kglobal.h>
#if HAVE_KROOTPIXMAP_H
#include <krootpixmap.h>
#endif
#endif

#include <qdatetime.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qpainter.h>
#include <qregexp.h>

#if _MSC_VER > 1020
#pragma warning(disable:4786)
#endif

QString ParseText(const char *text);

TextShow::TextShow(QWidget *p)
        : QTextBrowser(p)
{
    bg = new TransparentBg(this);
    baseBG = colorGroup().color(QColorGroup::Base);
    baseFG = colorGroup().color(QColorGroup::Text);
    setTextFormat(RichText);
    menu = NULL;
}

void TextShow::resizeEvent(QResizeEvent *e)
{
    QTextBrowser::resizeEvent(e);
    if (!curAnchor.isEmpty()) scrollToAnchor(curAnchor);
}

void TextShow::viewportMousePressEvent(QMouseEvent *e)
{
    if ((e->button() == RightButton) && hasSelectedText()){
        if (menu == NULL){
            menu = new QPopupMenu(this);
            menu->insertItem(Icon("editcopy"), i18n("&Copy"), this, SLOT(copy()), 0, 1);
        }
        menu->popup(e->globalPos());
        return;
    }
    QTextBrowser::viewportMousePressEvent(e);
}

void TextShow::copy()
{
    if (!hasSelectedText()) return;
    QString msgText = QString::fromLocal8Bit(pClient->clearHTML(selectedText().local8Bit()).c_str());
    QApplication::clipboard()->setText(msgText);
}

void TextShow::resetColors()
{
    setBackground(baseBG);
    setForeground(baseFG);
    bg->setTransparent(true);
}

void TextShow::setBackground(const QColor& c)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Active, QColorGroup::Base, c);
    pal.setColor(QPalette::Inactive, QColorGroup::Base, c);
    pal.setColor(QPalette::Disabled, QColorGroup::Base, c);
    setPalette(pal);
    bg->setTransparent(c == baseBG);
}

void TextShow::setForeground(const QColor& c)
{
    QPalette pal = palette();

    pal.setColor(QPalette::Active, QColorGroup::Text, c);
    pal.setColor(QPalette::Inactive, QColorGroup::Text, c);

    setPalette(pal);
}

void TextShow::setSource(const QString &href)
{
    QString l = href.left(6);
    if (l == "msg://"){
        QString id = href.mid(6);
        int p = id.find('/');
        if (p >= 0) id = id.left(p);
        QStringList s = QStringList::split('.', id);
        if (s.count() >= 2)
            emit goMessage(s[0].toULong(), s[1].toULong());
        return;
    }
    pMain->goURL(href);
    return;
}

QString TextShow::makeMessageText(ICQMessage *msg)
{
    QString s;
    switch (msg->Type()){
    case ICQ_MSGxMSG:
        s += ParseText((static_cast<ICQMsg*>(msg))->Message);
        break;
    case ICQ_MSGxURL:{
            ICQUrl *url = static_cast<ICQUrl*>(msg);
            s += quoteText(url->URL);
            if (*url->Message.c_str()){
                s += "<br>";
                s += ParseText(url->Message);
            }
            break;
        }
    case ICQ_MSGxAUTHxREQUEST:{
            ICQAuthRequest *req = static_cast<ICQAuthRequest*>(msg);
            s += i18n("Authorization request");
            if (*req->Message.c_str()){
                s += "<br>";
                s += ParseText(req->Message);
            }
            break;
        }
    case ICQ_MSGxAUTHxREFUSED:{
            ICQAuthRefused *req = static_cast<ICQAuthRefused*>(msg);
            s += i18n("Authorization refused");
            if (*req->Message.c_str()){
                s += "<br>";
                s += ParseText(req->Message);
            }
            break;
        }
    case ICQ_MSGxAUTHxGRANTED:
        s += i18n("Authorization granted");
        break;
    case ICQ_MSGxADDEDxTOxLIST:
        s += i18n("Added to contact list");
        break;
    case ICQ_MSGxCONTACTxREQUEST:{
            ICQContactRequest *req = static_cast<ICQContactRequest*>(msg);
            s += i18n("Contact request");
            if (*req->Message.c_str()){
                s += "<br>";
                s += ParseText(req->Message);
            }
            break;
        }
    case ICQ_MSGxFILE:{
            ICQFile *file = static_cast<ICQFile*>(msg);
            s += i18n("File: ");
            s += file->Name.c_str();
            s += " (";
            s += QString::number(file->Size);
            s += " ";
            s += i18n("bytes");
            s += ")";
            s += "<br>";
            s += ParseText(file->Description);
            break;
        }
    case ICQ_MSGxCHAT:{
            ICQChat *chat = static_cast<ICQChat*>(msg);
            s += ParseText(chat->Reason);
            break;
        }
    case ICQ_MSGxCONTACTxLIST:{
            ICQContacts *contacts = static_cast<ICQContacts*>(msg);
            s += msg->Received ? i18n("Contact list received") : i18n("Contact list sent");
            s += "<br>";
            s += "<table>";
            for (ContactList::iterator it = contacts->Contacts.begin(); it != contacts->Contacts.end(); it++){
                Contact *contact = static_cast<Contact*>(*it);
                s += "<tr><td align=right>" + QString::number(contact->Uin) + "</td><td>";
                s += quoteText(contact->Alias.c_str());
                s += "</td></tr>";
            }
            s += "</table>";
            break;
        }
    case ICQ_MSGxSMS:{
            ICQSMS *sms = static_cast<ICQSMS*>(msg);
            s += ParseText(sms->Message);
            if (*sms->Phone.c_str()){
                s += "<br>";
                s += quoteText(sms->Phone);
                if (*sms->Network.c_str())
                    s += " (" + quoteText(sms->Network) + ")";
            }
            break;
        }
    case ICQ_MSGxWEBxPANEL:{
            ICQWebPanel *m = static_cast<ICQWebPanel*>(msg);
            s += ParseText(m->Message);
            break;
        }
    case ICQ_MSGxEMAILxPAGER:{
            ICQEmailPager *m = static_cast<ICQEmailPager*>(msg);
            s += ParseText(m->Message);
            break;
        }
    default:
        log(L_WARN, "Unknown message type %u", msg->Type());
        s += "???";
    }
    return s;
}

void TextShow::addMessageText(ICQMessage *msg)
{
    setText(text() + makeMessageText(msg));
}

QString TextShow::quoteText(const char *text)
{
    string msg;
    pClient->quoteText(text, msg);
    QString s = QString::fromLocal8Bit(msg.c_str());
    return s;
}

MsgView::MsgView(QWidget *p)
        : TextShow(p)
{
    bBack = false;
    connect(pClient, SIGNAL(messageRead(ICQMessage*)), this, SLOT(messageRead(ICQMessage*)));
    connect(pMain, SIGNAL(colorsChanged()), this, SLOT(colorsChanged()));
    oldSendColor = pMain->ColorSend();
    oldReceiveColor = pMain->ColorReceive();
}

static char FONT_FORMAT[] = "<font color=\"#%06X\">";

void MsgView::colorsChanged()
{
    char FONT_SEND[] = "<##FontSend##>";
    char FONT_RECEIVE[] = "<##FontReceive##>";
    viewport()->setUpdatesEnabled(false);
    QString t = text();
    QString c;
    c.sprintf(FONT_FORMAT, oldSendColor);
    t.replace(QRegExp(c), FONT_SEND);
    c.sprintf(FONT_FORMAT, oldReceiveColor);
    t.replace(QRegExp(c), FONT_RECEIVE);
    c.sprintf(FONT_FORMAT, pMain->ColorSend());
    t.replace(QRegExp(FONT_SEND), c);
    c.sprintf(FONT_FORMAT, pMain->ColorReceive());
    t.replace(QRegExp(FONT_RECEIVE), c);
    setText(t);
    scrollToAnchor(curAnchor);
    viewport()->setUpdatesEnabled(true);
    viewport()->repaint();
}

void MsgView::messageRead(ICQMessage *msg)
{
    QString pat;
    pat.sprintf("<p><a name=\"%lu.%lu\"></a></p>", msg->Uin(), msg->Id);
    QString res;
    QString t = text();
    int pos = t.find(pat);
    if (pos < 0) return;
    bool isUpdates = viewport()->isUpdatesEnabled();
    viewport()->setUpdatesEnabled(false);
    res = t.left(pos);
    if (msg){
        bool bSaveBack = bBack;
        bBack = false;
        res += makeMessage(msg, false);
        bBack = bSaveBack;
    }
    t = t.mid(pos+1);
    pos = t.find("<p><a name=");
    if (pos >= 0) res += t.mid(pos);
    setText(res);
    curAnchor = QString::number(msg->Uin()) + "." + QString::number(msg->Id);
    scrollToAnchor(curAnchor);
    if (!isUpdates) return;
    viewport()->setUpdatesEnabled(true);
    viewport()->repaint();
}

void MsgView::deleteUser(unsigned long uin)
{
    QString pat;
    pat.sprintf("<p><a name=\"%lu.", uin);
    QString res;
    QString t = text();
    for (;;){
        int pos = t.find(pat);
        if (pos < 0){
            res += t;
            break;
        }
        res += t.left(pos);
        t = t.mid(pos+1);
        pos = t.find("<p><a name=");
        if (pos >= 0){
            t = t.mid(pos);
        }else{
            t = "";
        }
    }
    setText(res);
}

void MsgView::setMessage(unsigned long uin, unsigned long msgId)
{
    QString pat;
    pat.sprintf("<p><a name=\"%lu.%lu\"></a></p>", uin, msgId);
    if (text().find(pat) < 0) return;
    curAnchor = QString::number(uin) + "." + QString::number(msgId);
    scrollToAnchor(curAnchor);
}

QString MsgView::makeMessage(ICQMessage *msg, bool bUnread)
{
    QString s;
    s.sprintf("<p><a name=\"%lu.%lu\"></a></p>"
              "<table width=100%%><tr>"
              "<td><a href=\"msg://%lu.%lu\"><img src=\"icon:%s\"></a>&nbsp;",
              msg->Uin(), msg->Id, msg->Uin(), msg->Id, Client::getMessageIcon(msg->Type()));
    if (bUnread) s += "<b>";
    QString color;
    color.sprintf(FONT_FORMAT, msg->Received ? pMain->ColorReceive() : pMain->ColorSend());
    s += color;
    if (msg->Received){
        CUser u(msg->Uin);
        s += u.name(true);
    }else{
        CUser u(pClient);
        s += u.name(true);
    }
    s += "</font>";
    if (bUnread) s += "</b>";
    s += "</td><td align=right>";
    if (bUnread) s += "<b>";
    QDateTime time;
    time.setTime_t(msg->Time);
#if USE_KDE
    s += KGlobal::locale()->formatDateTime(time);
#else
    s += time.toString();
#endif
    if (bUnread) s += "</b>";
    s += "</td></tr></table>";
    unsigned long foreColor = 0;
    unsigned long backColor = 0;
    if (msg->Type() == ICQ_MSGxMSG){
        ICQMsg *m = static_cast<ICQMsg*>(msg);
        foreColor = m->ForeColor();
        backColor = m->BackColor();
    }
    s += "<table width=100%";
    if (foreColor != backColor){
        QString bg;
        bg.sprintf(" bgcolor=#%06lX", backColor);
        s += bg;
    }
    s += "><tr><td>";
    if (foreColor != backColor){
        QString fg;
        fg.sprintf("<font color=#%06lX>", foreColor);
        s += fg;
    }
    s += makeMessageText(msg);
    if (foreColor != backColor) s += "</font>";
    s += "</td></tr></table>";
    return s;
}

void MsgView::addMessage(ICQMessage *msg, bool bUnread)
{
    if (msg->Id >= MSG_PROCESS_ID) return;
    bool bUpdates = viewport()->isUpdatesEnabled();
    viewport()->setUpdatesEnabled(false);
    QString s(makeMessage(msg, bUnread));
    if (bBack){
        setText(s + text());
    }else{
        setText(text() + s);
    }
    scrollToAnchor(curAnchor);
    if (!bUpdates) return;
    viewport()->setUpdatesEnabled(true);
    viewport()->repaint();
}

void MsgView::addUnread(unsigned long uin)
{
    ICQUser *u = pClient->getUser(uin);
    if (u == NULL) return;
    History h(uin);
    for (list<unsigned long>::iterator it = u->unreadMsgs.begin(); it != u->unreadMsgs.end(); it++){
        ICQMessage *msg = h.getMessage(*it);
        if (msg == NULL) continue;
        addMessage(msg, true);
    }
}

HistoryView::HistoryView(QWidget *p, unsigned long uin)
        : MsgView(p), m_nUin(uin)
{
    bBack = true;
    ICQUser *u = pClient->getUser(uin);
    if (u == NULL) return;
    History h(uin);
    History::iterator &it = h.messages();
    list<unsigned long>::iterator unreadIt;
    QString t;
    for (++it; *it; ++it){
        for (unreadIt = u->unreadMsgs.begin(); unreadIt != u->unreadMsgs.end(); unreadIt++)
            if ((*unreadIt) == (*it)->Id) break;
        t = makeMessage(*it, unreadIt != u->unreadMsgs.end()) + t;
    }
    setText(t);
    connect(pClient, SIGNAL(messageReceived(ICQMessage*)), this, SLOT(messageReceived(ICQMessage*)));
    connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
}

void HistoryView::processEvent(ICQEvent *e)
{
    if (e->type() != EVENT_DONE) return;
    ICQMessage *msg = e->message();
    if (msg == NULL) return;
    messageReceived(msg);
}

void HistoryView::messageReceived(ICQMessage *msg)
{
    if (msg->Uin != m_nUin) return;
    if (msg->Id >= MSG_PROCESS_ID) return;
    int x = contentsX();
    int y = contentsY();
    bool bUnread = false;
    ICQUser *u = pClient->getUser(m_nUin);
    if (u){
        for (list<unsigned long>::iterator it = u->unreadMsgs.begin(); it != u->unreadMsgs.end(); it++){
            if ((*it) == msg->Id){
                bUnread = true;
                break;
            }
        }
    }
    viewport()->setUpdatesEnabled(false);
    if (bBack){
        QString saveText = text();
        setText("");
        addMessage(msg, bUnread);
        y += contentsHeight();
        setText(text() + saveText);
    }else{
        addMessage(msg, bUnread);
    }
    setContentsPos(x, y);
    viewport()->setUpdatesEnabled(true);
    viewport()->repaint();
}

#ifndef _WINDOWS
#include "msgview.moc"
#endif
