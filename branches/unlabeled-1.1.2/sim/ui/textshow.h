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

#if QT_VERSION < 300
#include "qt3/qtextbrowser.h"
#else
#include <qtextbrowser.h>
#ifdef USE_KDE
#ifdef HAVE_KTEXTBROWSER_H
#include <ktextbrowser.h>
#define QTextBrowser KTextBrowser
#endif
#endif
#endif

class UI_EXPORT TextShow : public QTextBrowser
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
    static unsigned textPosition(const QString &text, unsigned pos);
    static QString unquoteString(const QString &s, int from, int to);
signals:
    void showPopup(QPoint);
protected:
    void startDrag();
    void keyPressEvent(QKeyEvent *e);
    void resizeEvent(QResizeEvent *e);
    void setSource(const QString&);
};

class UI_EXPORT TextEdit : public TextShow
{
    Q_OBJECT
public:
    TextEdit(QWidget *parent, const char *name = NULL);
    ~TextEdit();
    void setCtrlMode(bool);
    const QColor &background() const;
    const QColor &foreground() const;
    void setForeground(const QColor&);
    void setBackground(const QColor&);
    void setTextFormat(QTextEdit::TextFormat);
    void changeText();
signals:
    void ctrlEnterPressed();
    void lostFocus();
protected:
    bool eventFilter(QObject *o, QEvent *e);
    void keyPressEvent(QKeyEvent *e);
    QColor curFG;
    bool m_bCtrlMode;
};

#endif
