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
#define QTextBrowser KTextBrowser
#endif
#endif
#endif

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
protected slots:
	void slotColorChanged(const QColor &c);
protected:
    bool eventFilter(QObject *o, QEvent *e);
    void keyPressEvent(QKeyEvent *e);
    QColor curFG;
    bool m_bCtrlMode;
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
    void colorCustom();
protected slots:
    void colorSelected(int);
};

class UI_EXPORT ColorToolButton : public QToolButton
{
    Q_OBJECT
public:
    ColorToolButton(QWidget *parent, QColor color);
	void setColor(QColor c);
signals:
    void colorChanged(QColor color);
	void aboutToShow();
protected slots:
    void btnClicked();
    void selectColor(QColor);
    void selectCustom();
    void closePopup();
protected:
    ColorPopup *m_popup;
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
protected slots:
    void toggleBold(bool);
    void toggleItalic(bool);
    void toggleUnderline(bool);
    void bgColorChanged(QColor);
    void fgColorChanged(QColor);
	void showFgPopup();
    void fontChanged(const QFont&);
    void selectFont();
protected:
    TextEdit	*m_edit;
    QToolBar	*m_bar;
	ColorToolButton *btnBG;
	ColorToolButton *btnFG;
    QToolButton	*btnBold;
    QToolButton	*btnItalic;
    QToolButton	*btnUnderline;
};

#endif
