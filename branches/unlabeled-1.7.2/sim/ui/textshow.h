/***************************************************************************
                          textshow.h  -  description
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

#ifndef _TEXTSHOW_H
#define _TEXTSHOW_H

#include "simapi.h"

#include <qglobal.h>
#include <qmainwindow.h>
#include <qtoolbutton.h>
#include <qlabel.h>

#if QT_VERSION < 300
#include "qt3/qtextedit.h"
#else
#include <qtextedit.h>
#ifdef USE_KDE
#ifdef HAVE_KTEXTEDIT_H
#include <ktextedit.h>
#define QTextEdit KTextEdit
#endif
#endif
#endif

class CToolBar;

const unsigned TextCmdBase	= 0x00030000;
const unsigned CmdBgColor	= TextCmdBase;
const unsigned CmdFgColor	= TextCmdBase + 1;
const unsigned CmdBold		= TextCmdBase + 2;
const unsigned CmdItalic	= TextCmdBase + 3;
const unsigned CmdUnderline	= TextCmdBase + 4;
const unsigned CmdFont		= TextCmdBase + 5;

class UI_EXPORT TextShow : public QTextEdit
{
    Q_OBJECT
public:
    TextShow (QWidget *parent, const char *name=NULL);
    ~TextShow();
    QString quoteText(const char *text, const char *charset=NULL);
    void copy();
    QString selectedText();
    QString plainText(int paraFrom=0, int paraTo=0x7FFFFFFF, int indexFrom=0, int indexTo=0);
    void setText(const QString &text);
    void setSource(const QString &url);
    const QColor &background() const;
    const QColor &foreground() const;
    void setForeground(const QColor&);
    void setBackground(const QColor&);
    static unsigned textPosition(const QString &text, unsigned pos);
    static QString unquoteString(const QString &s, int from, int to);
signals:
    void showPopup(QPoint);
protected:
    void startDrag();
    void keyPressEvent(QKeyEvent *e);
    void resizeEvent(QResizeEvent *e);
    void emitLinkClicked(const QString&);
    bool linksEnabled() const { return true; }
};

class UI_EXPORT TextEdit : public TextShow, public EventReceiver
{
    Q_OBJECT
public:
    TextEdit(QWidget *parent, const char *name = NULL);
    ~TextEdit();
    void setCtrlMode(bool);
    void setTextFormat(QTextEdit::TextFormat);
    const QColor &foreground() const;
    void setForeground(const QColor&);
    void changeText();
    void setParam(void*);
    bool isEmpty();
signals:
    void ctrlEnterPressed();
    void lostFocus();
    void emptyChanged(bool);
    void colorsChanged();
protected slots:
    void slotTextChanged();
    void slotColorChanged(const QColor &c);
    void bgColorChanged(QColor c);
    void fgColorChanged(QColor c);
    void fontChanged(const QFont &f);
protected:
    void *processEvent(Event*);
    bool eventFilter(QObject *o, QEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void *m_param;
    bool m_bBold;
    bool m_bItalic;
    bool m_bUnderline;
    QColor curFG;
    bool m_bCtrlMode;
    bool m_bEmpty;
};

class QToolBar;

class UI_EXPORT ColorLabel : public QLabel
{
    Q_OBJECT
public:
    ColorLabel(QWidget *parent, QColor c, int id, const QString&);
signals:
    void selected(int);
protected:
    void mouseReleaseEvent(QMouseEvent*);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    unsigned m_id;
};

class UI_EXPORT ColorPopup : public QFrame
{
    Q_OBJECT
public:
    ColorPopup(QWidget *parent, QColor c);
signals:
    void colorChanged(QColor color);
protected slots:
    void colorSelected(int);
protected:
    QColor m_color;
};

class UI_EXPORT RichTextEdit : public QMainWindow
{
    Q_OBJECT
public:
    RichTextEdit(QWidget *parent, const char *name = NULL);
    void setText(const QString&);
    QString text();
    void setTextFormat(QTextEdit::TextFormat);
    QTextEdit::TextFormat textFormat();
    void setReadOnly(bool bState);
    void showBar();
protected:
    TextEdit	*m_edit;
    CToolBar	*m_bar;
};

#endif
