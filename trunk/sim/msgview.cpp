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
#include "ui/ballonmsg.h"

#ifdef USE_KDE
#include <keditcl.h>
#include <kstdaccel.h>
#include <kglobal.h>
#ifdef HAVE_KROOTPIXMAP_H
#include <krootpixmap.h>
#endif
#else
#include "ui/finddlg.h"
#endif

#include <qdatetime.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qpainter.h>
#include <qregexp.h>
#include <qobjectlist.h>

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4786)
#endif
#endif

QString ParseText(const char *text);

TextShow::TextShow(QWidget *p, const char *name)
        : QTextBrowser(p, name)
{
    srchdialog = NULL;
    bg = new TransparentBg(this);
    baseBG = colorGroup().color(QColorGroup::Base);
    baseFG = colorGroup().color(QColorGroup::Text);
    setTextFormat(RichText);
}

void TextShow::resizeEvent(QResizeEvent *e)
{
    QTextBrowser::resizeEvent(e);
    scrollToAnchor(curAnchor);
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
}

void TextShow::setBackground(const QColor& c)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Active, QColorGroup::Base, c);
    pal.setColor(QPalette::Inactive, QColorGroup::Base, c);
    pal.setColor(QPalette::Disabled, QColorGroup::Base, c);
    setPalette(pal);
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
    bDirty = false;
    connect(pClient, SIGNAL(messageRead(ICQMessage*)), this, SLOT(messageRead(ICQMessage*)));
    connect(pClient, SIGNAL(markFinished()), this, SLOT(markFinished()));
    connect(pMain, SIGNAL(colorsChanged()), this, SLOT(colorsChanged()));
    oldSendColor = pMain->ColorSend();
    oldReceiveColor = pMain->ColorReceive();
}

QPopupMenu *TextShow::createPopupMenu(const QPoint &p)
{
    QPopupMenu *popup = QTextEdit::createPopupMenu(p);
    if (popup){
        popup->insertSeparator();
    }else{
        popup = new QPopupMenu(this);
    }
    popup->insertItem(Icon("find"), i18n("&Find"), this, SLOT(search()), CTRL+Key_F);
    popup->insertItem(Icon("find_next"), i18n("Find &next"), this, SLOT(repeatSearch()), Key_F3);
    return popup;
}

void TextShow::search(){

    if( srchdialog == 0 )
    {
        srchdialog = new KEdFind( this, "searchdialog", false);
        connect(srchdialog,SIGNAL(search()),this,SLOT(search_slot()));
        connect(srchdialog,SIGNAL(done()),this,SLOT(searchdone_slot()));
    }
    srchdialog->show();
    srchdialog->result();
}

void TextShow::search_slot()
{
    if (!srchdialog) return;
    int parag, index;
    getCursorPosition(&parag, &index);
    startSearch(parag, index);
}

void TextShow::startSearch(int parag, int index)
{
    QString to_find_string = srchdialog->getText();
    if (doSearch(to_find_string, srchdialog->case_sensitive(), (!srchdialog->get_direction()), &parag, &index)){
        setSelection(parag, index, parag, index + to_find_string.length());
        setCursorPosition(parag, index + to_find_string.length());
        ensureCursorVisible();
        return;
    }
    if (!srchdialog->isVisible()){
        QApplication::beep();
        return;
    }
    QStringList btns;
    btns.append(i18n("&Yes"));
    btns.append(i18n("&No"));

    QObjectList *l = srchdialog->queryList("QPushButton");
    QObjectListIt it( *l );
    if (it.current() == 0){
        delete l;
        QApplication::beep();
        return;
    }
    QPushButton *btnOK = static_cast<QPushButton*>(it.current());
    delete l;
    QRect rc = btnOK->rect();
    rc.moveTopLeft(btnOK->mapToGlobal(rc.topLeft()));
    BalloonMsg *msg = new BalloonMsg(!srchdialog->get_direction() ?
                                     i18n("End of document reached.\nContinue from the beginning?") :
                                     i18n("Beginning of document reached.\nContinue from the end?"),
                                     rc, btns, this);
    connect(msg, SIGNAL(action(int)), this, SLOT(searchAgain(int)));
    msg->show();
}

void TextShow::searchAgain(int n)
{
    if (n) return;
    if (!srchdialog) return;
    if (srchdialog->get_direction()){
        startSearch(paragraphs(), 0);
    }else{
        startSearch(0, 0);
    }
}

void TextShow::searchdone_slot()
{
    if (!srchdialog)
        return;
    srchdialog->hide();
    setFocus();
}

bool TextShow::doSearch(QString s_pattern, bool case_sensitive, bool forward, int *parag, int *index)
{
    return QTextEdit::find(s_pattern, case_sensitive, false, forward, parag, index);
}

void TextShow::repeatSearch()
{
    if(!srchdialog) return;
    search_slot();
    setFocus();
}

void TextShow::keyPressEvent( QKeyEvent *e )
{
#ifdef USE_KDE
    KKey key( e );
    if ( KStdAccel::find().contains( key ) ) {
        search();
        e->accept();
        return;
    }
    if ( KStdAccel::findNext().contains( key ) ) {
        repeatSearch();
        e->accept();
        return;
    }
#endif
    if (e->key() == Key_F3){
        repeatSearch();
        e->accept();
        return;
    }
    if ((e->key() == Key_F) && (e->state() & ControlButton)){
        search();
        e->accept();
        return;
    }
    QTextEdit::keyPressEvent(e);
}

static char FONT_FORMAT[] = "<font color=\"#%06X\">";

void MsgView::colorsChanged()
{
    int x = contentsX();
    int y = contentsY();
    char FONT_SEND[] = "<##FontSend##>";
    char FONT_RECEIVE[] = "<##FontReceive##>";
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
    setContentsPos(x, y);
}

void MsgView::messageRead(ICQMessage *msg)
{
    QString pat;
    pat.sprintf("<p><a name=\"%lu.%lu\"></a>", msg->getUin(), msg->Id);
    if (!bDirty){
        bDirty = true;
        newText = text();
    }
    int pos = newText.find(pat);
    if (pos < 0) return;
    QString res;
    res = newText.left(pos);
    if (msg){
        bool bSaveBack = bBack;
        bBack = false;
        res += makeMessage(msg, false);
        bBack = bSaveBack;
    }
    newText = newText.mid(pos+1);
    pos = newText.find("<p><a name=");
    if (pos >= 0) res += newText.mid(pos);
    newText = res;
    curAnchor = QString::number(msg->getUin()) + "." + QString::number(msg->Id);
    if (!pClient->bMarkMode)
        markFinished();
}

void MsgView::markFinished()
{
    if (!bDirty) return;
    setText(newText, curAnchor);
    scrollToAnchor(curAnchor);
    bDirty = false;
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
    setText(res, curAnchor);
    scrollToAnchor(curAnchor);
}

void MsgView::setMessage(unsigned long uin, unsigned long msgId)
{
    QString pat;
    pat.sprintf("<p><a name=\"%lu.%lu\"></a>", uin, msgId);
    if (text().find(pat) < 0) return;
    curAnchor = QString::number(uin) + "." + QString::number(msgId);
    scrollToAnchor(curAnchor);
}

QString MsgView::makeMessage(ICQMessage *msg, bool bUnread)
{
    QString s;
    s.sprintf("<p><a name=\"%lu.%lu\"></a>"
              "<table width=100%%><tr>"
              "<td><a href=\"msg://%lu.%lu\"><img src=\"icon:%s\"></a>&nbsp;",
              msg->getUin(), msg->Id, msg->getUin(), msg->Id, Client::getMessageIcon(msg->Type()));
    if (bUnread) s += "<b>";
    QString color;
    color.sprintf(FONT_FORMAT, msg->Received ? pMain->ColorReceive() : pMain->ColorSend());
    s += color;
    if (msg->Received){
        CUser u(msg->getUin());
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
    /*
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
        s += "</td></tr></table></p>";
    */
    s += "<p";
    if (foreColor != backColor){
        QString bg;
        bg.sprintf(" bgcolor=#%06lX", backColor);
        s += bg;
    }
    s += ">";
    if (foreColor != backColor){
        QString fg;
        fg.sprintf("<font color=#%06lX>", foreColor);
        s += fg;
    }
    s += makeMessageText(msg);
    if (foreColor != backColor) s += "</font></p>";
    return s;
}

void MsgView::addMessage(ICQMessage *msg, bool bUnread, bool bSet)
{
    if (msg->Id >= MSG_PROCESS_ID) return;
    int x = contentsX();
    int y = contentsY();
    QString s(makeMessage(msg, bUnread));
    if (bSet) curAnchor = QString::number(msg->getUin()) + "." + QString::number(msg->Id);
    if (bBack){
        setText(s + text(), curAnchor);
    }else{
        setText(text() + s, curAnchor);
    }
    if (bSet){
        scrollToBottom();
    }else{
        setContentsPos(x, y);
    }
}

void MsgView::addUnread(unsigned long uin)
{
    ICQUser *u = pClient->getUser(uin);
    if (u == NULL) return;
    History h(uin);
    for (list<unsigned long>::iterator it = u->unreadMsgs.begin(); it != u->unreadMsgs.end(); it++){
        ICQMessage *msg = h.getMessage(*it);
        if (msg == NULL) continue;
        addMessage(msg, true, false);
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
    if (msg->getUin() != m_nUin) return;
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
    if (bBack){
        QString saveText = text();
        setText("");
        addMessage(msg, bUnread, false);
        y += contentsHeight();
        setText(text() + saveText);
    }else{
        addMessage(msg, bUnread, false);
    }
    setContentsPos(x, y);
}

#ifndef _WINDOWS
#include "msgview.moc"
#endif
