/***************************************************************************
                          textshow.cpp  -  description
                             -------------------
    begin                : Sat Mar 16 2002
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

#include "textshow.h"
#include "simapi.h"

#ifdef USE_KDE
#include <keditcl.h>
#include <kstdaccel.h>
#include <kglobal.h>
#include <kfiledialog.h>
#define QFileDialog	KFileDialog
#ifdef HAVE_KROOTPIXMAP_H
#include <krootpixmap.h>
#endif
#else
#include <qfiledialog.h>
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
#include <qtoolbar.h>
#include <qlineedit.h>
#include <qaccel.h>
#include <qdragobject.h>

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4786)
#endif
#endif

#define MAX_HISTORY	100

TextEdit::TextEdit(QWidget *p, const char *name)
        : TextShow(p, name)
{
    setReadOnly(false);
    curFG = colorGroup().color(QColorGroup::Text);
    m_bCtrlMode = true;
    setWordWrap(WidgetWidth);
#if QT_VERSION >= 0x030100
    setAutoFormatting(0);
#endif
    viewport()->installEventFilter(this);
}

TextEdit::~TextEdit()
{
}

bool TextEdit::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::FocusOut){
        emit lostFocus();
    }
    return QTextEdit::eventFilter(o, e);
}

void TextEdit::changeText()
{
    emit textChanged();
}

void TextEdit::setCtrlMode(bool mode)
{
    m_bCtrlMode = mode;
}

void TextEdit::keyPressEvent(QKeyEvent *e)
{
    if (((e->key() == Key_Enter) || (e->key() == Key_Return))){
        if (!m_bCtrlMode || (e->state() == ControlButton)){
            emit ctrlEnterPressed();
            return;
        }
    }
    if (!isReadOnly()){
        if ((e->state() == ShiftButton) && (e->key() == Key_Insert)){
            paste();
            return;
        }
        if ((e->state() == ControlButton) && (e->key() == Key_Delete)){
            cut();
            return;
        }
    }
#if (QT_VERSION >= 300) && (QT_VERSION < 0x030100)
    // Workaround about autoformat feature in qt 3.0.x
    if ((e->text()[0] == '-') || (e->text()[0] == '*')){
        if (isOverwriteMode() && !hasSelectedText())
            moveCursor(MoveForward, true);
        insert( e->text(), TRUE, FALSE );
        return;
    }
#endif
#if QT_VERSION >= 300
    if ((e->key() == Key_Return) || (e->key() == Key_Enter)){
        QKeyEvent e1(QEvent::KeyPress, e->key(), e->ascii(), e->state() | ControlButton, e->text(), e->count());
        QTextEdit::keyPressEvent(&e1);
        return;
    }
#endif
    TextShow::keyPressEvent(e);
}

void TextEdit::setBackground(const QColor& c)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Active, QColorGroup::Base, c);
    pal.setColor(QPalette::Inactive, QColorGroup::Base, c);
    pal.setColor(QPalette::Disabled, QColorGroup::Base, c);
    setPalette(pal);
}

void TextEdit::setForeground(const QColor& c)
{
    curFG = c;
    setColor(c);
    QPalette pal = palette();
    pal.setColor(QPalette::Active, QColorGroup::Text, c);
    setPalette(pal);
    setColor(c);
}

const QColor &TextEdit::background() const
{
    return palette().color(QPalette::Active, QColorGroup::Base);
}

const QColor &TextEdit::foreground() const
{
    return curFG;
}

void TextEdit::setTextFormat(QTextEdit::TextFormat format)
{
    if (format == textFormat())
        return;
    if (format == RichText){
        QTextEdit::setTextFormat(format);
        return;
    }
    QString t = plainText(0, paragraphs(), 0, 0);
    QTextEdit::setTextFormat(format);
    setText(t);
}

TextShow::TextShow(QWidget *p, const char *name)
        : QTextBrowser(p, name)
{
    setTextFormat(RichText);
    setReadOnly(true);
}

TextShow::~TextShow()
{
}

void TextShow::setSource(const QString &href)
{
    QCString s = href.local8Bit();
    Event e(EventGoURL, (void*)(const char*)s);
    e.process();
}

void TextShow::resizeEvent(QResizeEvent *e)
{
    QPoint p = QPoint(0, height());
    p = mapToGlobal(p);
    p = viewport()->mapFromGlobal(p);
    int x, y;
    viewportToContents(p.x(), p.y(), x, y);
    int para;
    int pos = charAt(QPoint(x, y), &para);
    QTextEdit::resizeEvent(e);
    if (pos == -1){
        scrollToBottom();
    }else{
        setCursorPosition(para, pos);
        ensureCursorVisible();
    }
}

void TextShow::keyPressEvent(QKeyEvent *e)
{
    if (((e->state() == Qt::ControlButton) && (e->key() == Qt::Key_C)) ||
            ((e->state() == ControlButton) && (e->key() == Key_Insert))){
        copy();
        return;
    }
    QTextBrowser::keyPressEvent(e);
}

void TextShow::copy()
{
#if QT_VERSION <= 0x030100
    QApplication::clipboard()->setText(selectedText());
#else
    QApplication::clipboard()->setText(selectedText(),QClipboard::Selection);
#endif
}

void TextShow::startDrag()
{
    QDragObject *drag = new QTextDrag(selectedText(), viewport());
    if ( isReadOnly() ) {
        drag->dragCopy();
    } else {
        if ( drag->drag() && QDragObject::target() != this && QDragObject::target() != viewport() )
            removeSelectedText();
    }
}

QString TextShow::selectedText()
{
    QString res;
    int paraFrom, paraTo, indexFrom, indexTo;
    getSelection(&paraFrom, &indexFrom, &paraTo, &indexTo);
    return plainText(paraFrom, paraTo, indexFrom, indexTo);
}

QString TextShow::plainText(int paraFrom, int paraTo, int indexFrom, int indexTo)
{
    QString res;
    if ((paraFrom > paraTo) || ((paraFrom == paraTo) && (indexFrom >= indexTo)))
        return res;
    for (int i = paraFrom; i <= paraTo; i++){
        if (i >= paragraphs())
            break;
        res += unquoteString(text(i), (i == paraFrom) ? indexFrom : 0, (i == paraTo) ? indexTo : -1);
        if ((i < paraTo) && (i < paragraphs()))
            res += "\n";
    }
    return res;
}

QString TextShow::unquoteString(const QString &s, int from, int to)
{
    string text;
    unsigned startPos = textPosition(s, from);
    unsigned endPos = textPosition(s, to);
    text = s.mid(startPos, endPos - startPos).utf8();
    Event e(EventDecodeText, &text);
    e.process();
    return QString::fromUtf8(SIM::unquoteText(text.c_str()).c_str());
}

unsigned TextShow::textPosition(const QString &text, unsigned pos)
{
    if (pos >= text.length())
        return text.length();
    unsigned i;
    for (i = 0; i < text.length(); i++){
        QChar c = text[(int)i];
        if (c == '<'){
            QString tag = text.mid(i + 1, 3).lower();
            if (tag == "img"){
                if (pos == 0)
                    return i;
                pos--;
            }
            for (; i < text.length(); i++){
                c = text[(int)i];
                if (c == '>')
                    break;
            }
            continue;
        }
        if (pos == 0)
            return i;
        pos--;
    }
    return i;
}

QString TextShow::quoteText(const char *t, const char *charset)
{
    if (t == NULL)
        t = "";
    QString text;
    QTextCodec *codec = NULL;
    if (charset)
        codec = QTextCodec::codecForName(charset);
    if (codec){
        text = codec->makeDecoder()->toUnicode(t, strlen(t));
    }else{
        text = QString::fromLocal8Bit(t);
    }
    return quoteString(text);
}

void TextShow::setText(const QString &text)
{
    QTextBrowser::setText(text, "");
}

#ifndef _WINDOWS
#include "textshow.moc"
#endif
