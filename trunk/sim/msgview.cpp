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
#include <qvaluelist.h>
#include <qtimer.h>
#include <qstringlist.h>
#include <qtextcodec.h>

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4786)
#endif
#endif

TextShow::TextShow(QWidget *p, const char *name)
        : QTextBrowser(p, name)
{
    m_nUin = 0;
    codec = QTextCodec::codecForLocale();
    srchdialog = NULL;
    bg = new TransparentBg(this);
    baseBG = colorGroup().color(QColorGroup::Base);
    baseFG = colorGroup().color(QColorGroup::Text);
    setTextFormat(RichText);
    connect(pClient, SIGNAL(encodingChanged(unsigned long)), this, SLOT(encodingChanged(unsigned long)));
}

void TextShow::setUin(unsigned long uin)
{
    codec = pClient->codecForUser(uin);
    m_nUin = uin;
}

void TextShow::resizeEvent(QResizeEvent *e)
{
    QTextBrowser::resizeEvent(e);
    scrollToAnchor(curAnchor);
}

void TextShow::copy()
{
    if (!hasSelectedText()) return;
    UTFstring text(selectedText().utf8());
    text = pClient->clearHTML(text);
    QString msgText = QString::fromUtf8(text.c_str());
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

QString TextShow::makeMessageText(ICQMessage *msg, bool bIgnore)
{
    QString s;
    const char *encoding = msg->Charset.c_str();
    if (*encoding == 0)
        encoding = codec->name();
    UTFstring msg_text;
    switch (msg->Type()){
    case ICQ_MSGxMSG:
        msg_text = (static_cast<ICQMsg*>(msg))->Message.c_str();
        Client::toUTF(msg_text, encoding);
        s += MainWindow::ParseText(msg_text, bIgnore);
        break;
    case ICQ_MSGxURL:{
            ICQUrl *url = static_cast<ICQUrl*>(msg);
            s += "<a href=\"";
            s += url->URL.c_str();
            s += "\">";
            s += quoteText(url->URL, msg->Charset.c_str());
            s += "</a>";
            if (*url->Message.c_str()){
                msg_text = url->Message.c_str();
                Client::toUTF(msg_text, encoding);
                s += "<br>";
                s += MainWindow::ParseText(msg_text, true);
            }
            break;
        }
    case ICQ_MSGxAUTHxREQUEST:{
            ICQAuthRequest *req = static_cast<ICQAuthRequest*>(msg);
            s += i18n("Authorization request");
            if (*req->Message.c_str()){
                msg_text = req->Message.c_str();
                Client::toUTF(msg_text, encoding);
                s += "<br>";
                s += MainWindow::ParseText(msg_text, true);
            }
            break;
        }
    case ICQ_MSGxAUTHxREFUSED:{
            ICQAuthRefused *req = static_cast<ICQAuthRefused*>(msg);
            s += i18n("Authorization refused");
            if (*req->Message.c_str()){
                msg_text = req->Message.c_str();
                Client::toUTF(msg_text, encoding);
                s += "<br>";
                s += MainWindow::ParseText(msg_text, true);
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
                msg_text = req->Message.c_str();
                Client::toUTF(msg_text, encoding);
                s += "<br>";
                s += MainWindow::ParseText(msg_text, true);
            }
            break;
        }
    case ICQ_MSGxFILE:{
            ICQFile *file = static_cast<ICQFile*>(msg);
            QString name = QString::fromLocal8Bit(file->Name.c_str());
            if (name.find(QRegExp("^[0-9]+ Files$")) >= 0){
                s += i18n("File", "%n files", name.toInt());
            }else{
                s += i18n("File");
                s += ": ";
                s += file->Name.c_str();
            }
            s += " (";
            s += QString::number(file->Size);
            s += " ";
            s += i18n("bytes");
            s += ")";
            s += "<br>";
            msg_text = file->Description.c_str();
            Client::toUTF(msg_text, encoding);
            s += MainWindow::ParseText(msg_text, true);
            break;
        }
    case ICQ_MSGxCHAT:{
            ICQChat *chat = static_cast<ICQChat*>(msg);
            msg_text = chat->Reason.c_str();
            Client::toUTF(msg_text, encoding);
            s += MainWindow::ParseText(msg_text, true);
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
                s += quoteText(contact->Alias.c_str(), msg->Charset.c_str());
                s += "</td></tr>";
            }
            s += "</table>";
            break;
        }
    case ICQ_MSGxSMS:{
            ICQSMS *sms = static_cast<ICQSMS*>(msg);
            msg_text = sms->Message.c_str();
            Client::toUTF(msg_text, encoding);
            s += MainWindow::ParseText(msg_text, true);
            if (*sms->Phone.c_str()){
                s += "<br>";
                s += quoteText(sms->Phone, msg->Charset.c_str());
                if (*sms->Network.c_str())
                    s += " (" + quoteText(sms->Network, msg->Charset.c_str()) + ")";
            }
            break;
        }
    case ICQ_MSGxWEBxPANEL:{
            ICQWebPanel *m = static_cast<ICQWebPanel*>(msg);
            msg_text = m->Message.c_str();
            Client::toUTF(msg_text, encoding);
            s += MainWindow::ParseText(msg_text, true);
            break;
        }
    case ICQ_MSGxEMAILxPAGER:{
            ICQEmailPager *m = static_cast<ICQEmailPager*>(msg);
            msg_text = m->Message.c_str();
            Client::toUTF(msg_text, encoding);
            s += MainWindow::ParseText(msg_text, true);
            break;
        }
    default:
        log(L_WARN, "Unknown message type %u", msg->Type());
        s += "???";
    }
    return s;
}

QString TextShow::quoteText(const char *text, const char *charset)
{
    string msg = ICQClient::quoteText(text);
    QString s = Client::from8Bit(codec, msg, charset);
    return s;
}

void TextShow::encodingChanged(unsigned long _uin)
{
    if (_uin && (m_nUin != _uin)) return;
    QTextCodec *newCodec = pClient->codecForUser(m_nUin);
    string s = Client::to8Bit(codec, text());
    codec = newCodec;
    setText(Client::from8Bit(codec, s));
    scrollToBottom();
}

MsgView::MsgView(QWidget *p)
        : TextShow(p)
{
    bBack = false;
    connect(pClient, SIGNAL(messageRead(ICQMessage*)), this, SLOT(messageRead(ICQMessage*)));
    connect(pMain, SIGNAL(colorsChanged()), this, SLOT(colorsChanged()));
    connect(pMain, SIGNAL(ownColorsChanged()), this, SLOT(ownColorsChanged()));
    oldSendColor = pMain->ColorSend();
    oldReceiveColor = pMain->ColorReceive();
}

void MsgView::setUin(unsigned long uin)
{
    bool bAdd = (m_nUin == 0);
    TextShow::setUin(uin);
    if (bAdd) addUnread(uin);
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
#if QT_VERSION < 300
    if ( KStdAccel::isEqual( e, KStdAccel::find()) ) {
        search();
        e->accept();
        return;
    }
    else if ( KStdAccel::isEqual( e, KStdAccel::findNext()) ) {
        repeatSearch();
        e->accept();
        return;
    }
#else
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
    pat.sprintf("<a href=\"msg://%lu.%lu", msg->getUin(), msg->Id);
    for (int i = 0; i < paragraphs();){
        if (text(i).find(pat) < 0){
            for (i++; i < paragraphs(); i++)
                if (text(i).find("<a href=\"msg://") >= 0) break;
            continue;
        }
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
}

void MsgView::deleteUser(unsigned long uin)
{
    QString pat;
    pat.sprintf("<a href=\"msg://%lu.", uin);
    for (int i = 0; i < paragraphs();){
        if (text(i).find(pat) < 0){
            for (i++; i < paragraphs(); i++)
                if (text(i).find("<a href=\"msg://") >= 0) break;
            continue;
        }
        removeParagraph(i);
        for (; i < paragraphs(); ){
            if (text(i).find("<a href=\"msg://") >= 0) break;
            int n = paragraphs();
            removeParagraph(i);
            if (n == paragraphs()){
                i++;
                break;
            }
        }
    }
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
    s.sprintf("<p><nobr><a name=\"%lu.%lu\"></a>"
              "<a href=\"msg://%lu.%lu\"><img src=\"icon:%s\"></a>&nbsp;",
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
    s += "</font>&nbsp;&nbsp;";
    QDateTime time;
    time.setTime_t(msg->Time);
    s += "<font size=-1>";
#ifdef USE_KDE
    s += KGlobal::locale()->formatDateTime(time);
#else
    s += time.toString();
#endif
    s += "</font>";
    if (bUnread) s += "</b>";
    s += "</nobr></p>";
    unsigned long foreColor = 0;
    unsigned long backColor = 0;
    if (!pMain->UseOwnColors() && (msg->Type() == ICQ_MSGxMSG)){
        ICQMsg *m = static_cast<ICQMsg*>(msg);
        foreColor = m->ForeColor();
        backColor = m->BackColor();
    }
    s += "<p>";
    if (foreColor != backColor){
        QString fg;
        fg.sprintf("<font color=#%06lX>", foreColor);
        s += fg;
    }
    s += makeMessageText(msg, pMain->UseOwnColors());
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
    unsigned long foreColor = 0;
    unsigned long backColor = 0;
    if (!pMain->UseOwnColors() && (msg->Type() == ICQ_MSGxMSG)){
        ICQMsg *m = static_cast<ICQMsg*>(msg);
        foreColor = m->ForeColor();
        backColor = m->BackColor();
    }
    if (bBack){
        setText(s + text(), curAnchor);
        if (foreColor != backColor)
            setMsgBgColor(msg->getUin(), msg->Id, backColor, 0);
    }else{
        int n = paragraphs();
        if (n > 0) n--;
        append(s);
        if (foreColor != backColor)
            setMsgBgColor(msg->getUin(), msg->Id, backColor, n);
    }
    if (bSet){
        scrollToBottom();
    }else{
        setContentsPos(x, y);
    }
}

int MsgView::setMsgBgColor(unsigned long uin, unsigned long id, unsigned long rgb, int start)
{
    QString pat;
    pat.sprintf("<a href=\"msg://%lu.%lu", uin, id);
    for (int n = start; n < paragraphs(); n++){
        if (text(n).find(pat) < 0) continue;
        pat = "<a href=\"msg://";
        for (n++; n < paragraphs(); n++){
            if (text(n).isEmpty()) break;
            if (text(n).find(pat) >= 0) break;
            setParagraphBackgroundColor(n, QColor(rgb));
        }
        return n;
    }
    log(L_WARN, "Message bg color not found");
    return paragraphs();
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

void MsgView::ownColorsChanged()
{
    setText("");
}

HistoryView::HistoryView(QWidget *p, unsigned long uin)
        : MsgView(p)
{
    setUin(uin);
    bFill = false;
    bBack = true;
    h = NULL;
    connect(pClient, SIGNAL(messageReceived(ICQMessage*)), this, SLOT(messageReceived(ICQMessage*)));
    connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    QTimer::singleShot(100, this, SLOT(fill()));
}

HistoryView::~HistoryView()
{
    if (h) delete h;
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

void HistoryView::fill()
{
    if (!bFill){
        bFill = true;
        u = pClient->getUser(m_nUin);
        if (u == NULL) return;
        h = new History(m_nUin);
        showProgress(0);
    }
    if ((u == NULL) || (h == NULL)) return;
    History::iterator &it = h->messages();
    list<unsigned long>::iterator unreadIt;
    ++it;
    if (*it){
        showProgress(it.progress());
        for (unreadIt = u->unreadMsgs.begin(); unreadIt != u->unreadMsgs.end(); unreadIt++)
            if ((*unreadIt) == (*it)->Id) break;
        bBack = false;
        addMessage(*it, unreadIt != u->unreadMsgs.end(), false);
        bBack = true;
        QTimer::singleShot(0, this, SLOT(fill()));
        return;
    }

    delete h;
    h = NULL;
    u = NULL;
    showProgress(101);
}

void HistoryView::ownColorsChanged()
{
    MsgView::ownColorsChanged();
    bFill = false;
    if (h){
        delete h;
        h = NULL;
    }
    QTimer::singleShot(100, this, SLOT(fill()));
}

#ifndef _WINDOWS
#include "msgview.moc"
#endif
