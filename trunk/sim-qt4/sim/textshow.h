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
#include <q3mainwindow.h>
#include <qtoolbutton.h>
#include <qlabel.h>
//Added by qt3to4:
#include <QKeyEvent>
#include <QEvent>
#include <Q3Frame>
#include <QResizeEvent>
#include <Q3PopupMenu>
#include <QMouseEvent>

#if COMPAT_QT_VERSION < 0x030000
#include "qt3/q3textedit.h"
#else
#include <q3textedit.h>
#ifdef USE_KDE
#ifdef HAVE_KTEXTEDIT_H
#include <ktextedit.h>
#define Q3TextEdit KTextEdit
#endif
#endif
#endif

class CToolBar;
class Q3TextDrag;

const unsigned TextCmdBase	= 0x00030000;
const unsigned CmdBgColor	= TextCmdBase;
const unsigned CmdFgColor	= TextCmdBase + 1;
const unsigned CmdBold		= TextCmdBase + 2;
const unsigned CmdItalic	= TextCmdBase + 3;
const unsigned CmdUnderline	= TextCmdBase + 4;
const unsigned CmdFont		= TextCmdBase + 5;

class EXPORT TextShow : public Q3TextEdit
{
    Q_OBJECT
public:
    TextShow (QWidget *parent, const char *name=NULL);
    ~TextShow();
    QString quoteText(const char *text, const char *charset=NULL);
    void copy();
    void cut();
    void setText(const QString &text);
    virtual void setSource(const QString &url);
    const QColor &background() const;
    const QColor &foreground() const;
    void setForeground(const QColor&);
    void setBackground(const QColor&);
signals:
    void finished();
protected slots:
    void slotSelectionChanged();
    void slotResizeTimer();
protected:
    Q3TextDrag *dragObject(QWidget *parent) const;
    QTimer	  *m_timer;
    void startDrag();
    void keyPressEvent(QKeyEvent *e);
    void resizeEvent(QResizeEvent *e);
    void emitLinkClicked(const QString&);
    bool linksEnabled() const { return true; }
};

class EXPORT TextEdit : public TextShow, public EventReceiver
{
    Q_OBJECT
public:
    TextEdit(QWidget *parent, const char *name = NULL);
    ~TextEdit();
    void setCtrlMode(bool);
    void setTextFormat(Qt::TextFormat);
    const QColor &foreground() const;
    const QColor &defForeground() const;
    void setForeground(const QColor&, bool bDef);
    void changeText();
    void setParam(void*);
    void setFont(const QFont&);
    bool isEmpty();
    QPoint m_popupPos;
signals:
    void ctrlEnterPressed();
    void lostFocus();
    void emptyChanged(bool);
    void colorsChanged();
    void fontSelected(const QFont &font);
    void finished(TextEdit*);
protected slots:
    void slotClicked(int parag, int index);
    void slotTextChanged();
    void slotColorChanged(const QColor &c);
    void bgColorChanged(QColor c);
    void fgColorChanged(QColor c);
    void fontChanged(const QFont &f);
protected:
    void *processEvent(Event*);
    bool eventFilter(QObject *o, QEvent *e);
    void keyPressEvent(QKeyEvent *e);
    Q3PopupMenu *createPopupMenu(const QPoint& pos);
    void *m_param;
    bool m_bBold;
    bool m_bItalic;
    bool m_bUnderline;
    bool m_bChanged;
    QColor curFG;
    QColor defFG;
    bool m_bCtrlMode;
    bool m_bEmpty;
    bool m_bSelected;
    bool m_bNoSelected;
    bool m_bInClick;
};

class Q3ToolBar;

class EXPORT ColorLabel : public QLabel
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

class EXPORT ColorPopup : public Q3Frame
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

class EXPORT RichTextEdit : public Q3MainWindow
{
    Q_OBJECT
public:
    RichTextEdit(QWidget *parent, const char *name = NULL);
    void setText(const QString&);
    QString text();
    void setTextFormat(Qt::TextFormat);
    Qt::TextFormat textFormat();
    void setReadOnly(bool bState);
    void showBar();
protected:
    TextEdit	*m_edit;
    CToolBar	*m_bar;
};

#endif
