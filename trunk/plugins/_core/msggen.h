/***************************************************************************
                          msggen.h  -  description
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

#ifndef _MSGGEN_H
#define _MSGGEN_H

#include "simapi.h"

#include <qtoolbutton.h>
#include <qlabel.h>

class CToolCustom;
class QToolButton;
class MsgEdit;
class QFont;

class MsgGen : public QObject, public EventReceiver
{
    Q_OBJECT
public:
    MsgGen(CToolCustom *parent, Message *msg);
protected slots:
    void init();
    void textChanged();
    void fontChanged(const QFont&);
    void bgColorChanged(QColor);
    void fgColorChanged(QColor);
    void toggleBold(bool);
    void toggleItalic(bool);
    void toggleUnderline(bool);
    void selectFont();
protected:
    void *processEvent(Event*);
    string m_client;
    MsgEdit	*m_edit;
    QToolButton *btnSend;
    QToolButton *btnBold;
    QToolButton *btnItalic;
    QToolButton *btnUnderline;
};

class ColorLabel : public QLabel
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

class ColorPopup : public QFrame
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

class ColorToolButton : public QToolButton
{
    Q_OBJECT
public:
    ColorToolButton(QWidget *parent, QColor color);
signals:
    void colorChanged(QColor color);
protected slots:
    void btnClicked();
    void selectColor(QColor);
    void selectCustom();
    void closePopup();
protected:
    ColorPopup *m_popup;
    QColor m_color;
};

#endif

