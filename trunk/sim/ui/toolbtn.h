/***************************************************************************
                          toolbtn.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#ifndef _TOOLBTN_H
#define _TOOLBTN_H

#include "simapi.h"

#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include <vector>
using namespace std;

class QMainWindow;
class QPopupMenu;
class QAccel;
class ToolBarStates;

class UI_EXPORT CToolItem
{
public:
    CToolItem(CommandDef *def);
    virtual ~CToolItem() {}
    virtual void setState() = 0;
    virtual QWidget *widget() = 0;
    void checkState();
    void setCommand(CommandDef *def);
    CommandDef *def();
protected:
    CommandDef m_def;
    QString m_text;
};

class UI_EXPORT CToolButton : public QToolButton, public CToolItem
{
    Q_OBJECT
public:
    CToolButton(QWidget * parent, CommandDef *def);
    virtual void setState();
    virtual QWidget *widget() { return this; }
    static QPoint popupPos(QToolButton*, QWidget*);
signals:
    void showPopup(QPoint);
protected slots:
    void btnClicked();
    void btnToggled(bool);
    void accelActivated(int);
protected:
    void setTextLabel();
    void mousePressEvent(QMouseEvent*);
    void contextMenuEvent(QContextMenuEvent*);
    void showEvent(QShowEvent*);
    void hideEvent(QHideEvent*);
    void setAccel(int key);
    void enableAccel(bool bState);
    unsigned accelKey;
    QAccel *accel;
    QPoint popupPos(QWidget *p);
};

class UI_EXPORT PictButton : public CToolButton
{
    Q_OBJECT
public:
    PictButton(QToolBar*, CommandDef *def);
    ~PictButton();
protected:
    virtual void setState();
    void paintEvent(QPaintEvent*);
    QSizePolicy sizePolicy() const;
    QSize minimumSizeHint() const;
    QSize sizeHint() const;
};

class QHBoxLayout;

class UI_EXPORT CToolCustom : public QWidget, public CToolItem
{
    Q_OBJECT
public:
    CToolCustom(QToolBar*, CommandDef *def);
    void addWidget(QWidget*);
    void removeWidgets();
    void setText(const QString &text);
protected:
    virtual QWidget *widget() { return this; }
    virtual void setState();
    QSizePolicy sizePolicy() const;
    QLabel		*m_label;
    QHBoxLayout	*m_lay;
};

class ButtonsMap;

class UI_EXPORT CToolBar : public QToolBar, public EventReceiver
{
    Q_OBJECT
public:
    CToolBar(CommandsDef *def, QMainWindow *parent);
    ~CToolBar();
    CommandsDef *m_def;
    void *param()				{ return m_param; }
    void setParam(void *param)	{ m_param = param; }
    void checkState();
    void	*processEvent(Event*);
public slots:
    void showPopup(QPoint p);
    void toolBarChanged();
protected:
    void	mousePressEvent(QMouseEvent *e);
    void	contextMenuEvent(QContextMenuEvent *e);
    bool	bChanged;
    void	*m_param;
    ButtonsMap *buttons;
};

#endif

