/***************************************************************************
                          chatwnd.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#include "chatwnd.h"
#include "icons.h"
#include "client.h"
#include "cuser.h"
#include "mainwin.h"
#include "msgview.h"
#include "transparent.h"
#include "ui/ballonmsg.h"

#include <qsplitter.h>
#include <qframe.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qfontmetrics.h>
#include <qapplication.h>
#include <qregexp.h>
#include <qfile.h>

#if USE_KDE
#include <kfiledialog.h>
#define QFileDialog	KFileDialog
#else
#include <qfiledialog.h>
#endif

QString ParseText(const char *text);

class ChatUserItem : public QListBoxText
{
public:
    ChatUserItem(QListBox *parent, const QString &name, unsigned long uin);
    unsigned long uin;
};

ChatUserItem::ChatUserItem(QListBox *parent, const QString &text, unsigned long _uin)
        : QListBoxText(parent, text)
{
    uin = _uin;
}

ChatWindow::ChatWindow(ICQChat *_chat)
        : QMainWindow(NULL)
{
    logFile = NULL;

    setWFlags(WDestructiveClose);
    connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    transparent = new TransparentTop(this, pMain->UseTransparentContainer, pMain->TransparentContainer);

    QToolBar *toolbar = new QToolBar(this);

    btnSave = new QToolButton(toolbar);
    btnSave->setIconSet(Icon("save_all"));
    btnSave->setTextLabel(i18n("Save"));
    connect(btnSave, SIGNAL(clicked()), this, SLOT(openLog()));

    toolbar->addSeparator();

    btnBold = new QToolButton(toolbar);
    btnBold->setIconSet(Icon("text_bold"));
    btnBold->setTextLabel(i18n("Bold"));
    btnBold->setToggleButton(true);
    connect(btnBold, SIGNAL(toggled(bool)), this, SLOT(toggleBold(bool)));

    btnItalic = new QToolButton(toolbar);
    btnItalic->setIconSet(Icon("text_italic"));
    btnItalic->setTextLabel(i18n("Italic"));
    btnItalic->setToggleButton(true);
    connect(btnItalic, SIGNAL(toggled(bool)), this, SLOT(toggleItalic(bool)));

    btnUnderline = new QToolButton(toolbar);
    btnUnderline->setIconSet(Icon("text_under"));
    btnUnderline->setTextLabel(i18n("Underline"));
    btnUnderline->setToggleButton(true);
    connect(btnUnderline, SIGNAL(toggled(bool)), this, SLOT(toggleUnderline(bool)));

    chat = _chat;
    bInit = false;
    pClient->cancelMessage(chat, false);
    bInit = true;

    CUser u(chat->getUin());
    setCaption(u.name());
    setIcon(Pict("chat"));

    QSplitter *splitter = new QSplitter(this);
    setCentralWidget(splitter);
    QFrame *frm = new QFrame(splitter);
    lstUsers = new QListBox(splitter);
    splitter->setResizeMode(lstUsers, QSplitter::KeepSize);
    QVBoxLayout *lay = new QVBoxLayout(frm);
    txtChat = new TextShow(frm);
    txtChat->setTextFormat(QTextEdit::RichText);
    lay->addWidget(txtChat);
    lay->addSpacing(2);
    edtChat = new QTextEdit(frm);
    edtChat->setTextFormat(QTextEdit::RichText);
    edtChat->setWordWrap(QTextEdit::NoWrap);
    QFontMetrics fm(edtChat->font());
    int h = fm.height();
#if QT_VERSION < 300
    if ( style() == WindowsStyle && h < 26 )
        h = 22;
#endif
    if ( h < QApplication::globalStrut().height())
        h = QApplication::globalStrut().height();

    edtChat->setFixedHeight(h);
    edtChat->setHScrollBarMode(QScrollView::AlwaysOff);
    edtChat->setVScrollBarMode(QScrollView::AlwaysOff);
    lay->addWidget(edtChat);

    connect(edtChat, SIGNAL(returnPressed()), this, SLOT(sendLine()));

    CUser owner(pClient);
    new ChatUserItem(lstUsers, owner.name(), 0);

    if (pMain->ChatWidth() && pMain->ChatHeight())
        resize(pMain->ChatWidth(), pMain->ChatHeight());
    baseColor = txtChat->color();
    bClientMode = false;
}

ChatWindow::~ChatWindow()
{
    delete chat;
    chat = NULL;
    if (logFile) delete logFile;
    pMain->chatClose();
}

void ChatWindow::sendLine()
{
    QString s = edtChat->text();
    s.replace(QRegExp("<br>"), "");
    s.replace(QRegExp("</?p>"), "");
    QCString sLineSend = s.local8Bit();
    chat->chat->sendLine(sLineSend);
    QString clientString;
    QString br;
    if (bClientMode){
        int n = txtChat->paragraphs();
        clientString = txtChat->text(n-1);
        txtChat->removeParagraph(n-1);
        br = "<br>";
        int pos = clientString.find("&gt;");
        clientString = chatHeader(chat->getUin()) + clientString.mid(pos+4);
    }
    QString line = chatHeader(0) + ParseText(sLineSend) + "<br>\n";
    txtChat->insertParagraph(br + line, -1);
    if (bClientMode)
        txtChat->insertParagraph(clientString, -1);
    txtChat->scrollToBottom();
    txtChat->moveCursor(QTextEdit::MoveEnd, false);
    edtChat->setText("");
    edtChat->setBold(btnBold->isOn());
    edtChat->setItalic(btnItalic->isOn());
    edtChat->setUnderline(btnUnderline->isOn());
    if (logFile){
        QCString s = line.local8Bit();
        logFile->writeBlock(s, s.length());
        logFile->flush();
    }
}

QString ChatWindow::chatHeader(unsigned long uin)
{
    QString alias;
    if (uin){
        CUser u(uin);
        alias = u.name(true);
    }else{
        CUser u(pClient);
        alias = u.name(true);
    }
    QString color;
    color.sprintf("%06lX", uin ? pMain->ColorReceive() : pMain->ColorSend());
    return QString("<font color=\"#%1\">&lt;%2&gt;</font> ")
           .arg(color) .arg(alias);
}

void ChatWindow::processEvent(ICQEvent *e)
{
    if (e->message() != chat) return;
    chat->bDelete = false;
    if (e->state == ICQEvent::Fail){
        if (bInit) close();
        return;
    }
    if (e->type() != EVENT_CHAT) return;
    switch (e->subType()){
    case CHAT_CONNECT:{
            CUser u(e->Uin());
            new ChatUserItem(lstUsers, u.name(), e->Uin());
            QString line = chatHeader(e->Uin()) +
                           txtChat->quoteText(i18n("Enter to chat").local8Bit()) + "<br>\n";
            txtChat->insertParagraph(line, -1);
            txtChat->scrollToBottom();
            txtChat->moveCursor(QTextEdit::MoveEnd, false);
            if (logFile){
                QCString s = line.local8Bit();
                logFile->writeBlock(s, s.length());
                logFile->flush();
            }
            break;
        }
    case CHAT_FONT_FACE:
        if (bClientMode){
            txtChat->setBold(chat->chat->fontFace & FONT_BOLD);
            txtChat->setItalic(chat->chat->fontFace & FONT_ITALIC);
            txtChat->setUnderline(chat->chat->fontFace & FONT_UNDERLINE);
        }
        break;
    case CHAT_TEXT:
        if (!bClientMode){
            txtChat->insertParagraph(chatHeader(e->Uin()), -1);
            txtChat->scrollToBottom();
            txtChat->moveCursor(QTextEdit::MoveEnd, false);
            txtChat->setBold(chat->chat->fontFace & FONT_BOLD);
            txtChat->setItalic(chat->chat->fontFace & FONT_ITALIC);
            txtChat->setUnderline(chat->chat->fontFace & FONT_UNDERLINE);
            bClientMode = true;
        }
        txtChat->insert(QString::fromLocal8Bit(e->text.c_str()), false, false);
        break;
    case CHAT_BACKSPACE:
        if (bClientMode)
            txtChat->doKeyboardAction(QTextEdit::ActionBackspace);
        break;
    case CHAT_NEWLINE:{
            QString clientString;
            if (bClientMode){
                int n = txtChat->paragraphs();
                clientString = txtChat->text(n-1);
                txtChat->removeParagraph(n-1);
                int pos = clientString.find("&gt;");
                clientString = clientString.mid(pos+4);
            }
            txtChat->insertParagraph("<br>", -1);
            txtChat->moveCursor(QTextEdit::MoveEnd, false);
            QString line = chatHeader(chat->getUin()) + ParseText(clientString.local8Bit()) + "<br>\n";
            txtChat->append(line);
            txtChat->scrollToBottom();
            txtChat->moveCursor(QTextEdit::MoveEnd, false);
            bClientMode = false;
            if (logFile){
                QCString s = line.local8Bit();
                logFile->writeBlock(s, s.size());
                logFile->flush();
            }
            break;
        }
    }
}

void ChatWindow::toggleBold(bool bOn)
{
    edtChat->setBold(bOn);
}

void ChatWindow::toggleItalic(bool bOn)
{
    edtChat->setItalic(bOn);
}

void ChatWindow::toggleUnderline(bool bOn)
{
    edtChat->setUnderline(bOn);
}

void ChatWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    pMain->ChatWidth = width();
    pMain->ChatHeight = height();
}

void ChatWindow::openLog()
{
    QString fname;
    if (logFile){
        fname = logFile->name();
    }else{
        string name;
        pMain->buildFileName(name, "ChatLog/");
        fname = QString::fromLocal8Bit(name.c_str());
    }
#if WIN32
    fname.replace(QRegExp("\\\\"), "/");
#endif
    fname = QFileDialog::getSaveFileName(fname, QString::null, this);
#if WIN32
    fname.replace(QRegExp("/"), "\\");
#endif
    if (fname.length() == 0) return;
    if (logFile && (logFile->name() == fname)) return;
    QFile *newFile = new QFile(fname);
    if (!newFile->open(IO_Append)){
        BalloonMsg::message(i18n("Can't create %1") .arg(fname), btnSave);
        delete newFile;
        return;
    }
    if (logFile == NULL){
        logFile = newFile;
        return;
    }
    delete logFile;
    QString clientString;
    if (bClientMode){
        int n = txtChat->paragraphs();
        clientString = txtChat->text(n-1);
        txtChat->removeParagraph(n-1);
        int pos = clientString.find("&gt;");
        clientString = chatHeader(chat->getUin()) + clientString.mid(pos+4);
    }
    QString t = txtChat->text();
    QCString s = t.local8Bit();
    logFile->writeBlock(s, s.length());
    logFile->flush();
    if (bClientMode)
        txtChat->insertParagraph(clientString, -1);
    txtChat->scrollToBottom();
    txtChat->moveCursor(QTextEdit::MoveEnd, false);
    logFile = newFile;
}

#ifndef _WINDOWS
#include "chatwnd.moc"
#endif
