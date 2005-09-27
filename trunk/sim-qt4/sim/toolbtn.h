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
#include "stl.h"

#include <Q3ToolBar>
#include <QToolButton>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPaintEvent>
#include <QHideEvent>
#include <QShowEvent>
#include <QContextMenuEvent>
#include <Q3PopupMenu>
#include <QMouseEvent>

class Q3MainWindow;
class Q3PopupMenu;
class Q3Accel;
class ToolBarStates;

class EXPORT CToolItem
{
public:
    CToolItem(CommandDef *def);
    virtual ~CToolItem() {}
    virtual void setState();
    virtual QWidget *widget() = 0;
    void checkState();
    void setCommand(CommandDef *def);
    void setChecked(CommandDef *def);
    void setDisabled(CommandDef *def);
    void setShow(CommandDef *def);
    CommandDef *def();
protected:
    CommandDef m_def;
    QString m_text;
};

class EXPORT CToolButton : public QToolButton, public CToolItem
{
    Q_OBJECT
public:
    CToolButton(QWidget * parent, CommandDef *def);
    ~CToolButton();
    virtual void setState();
    virtual QWidget *widget() { return this; }
    static QPoint popupPos(QWidget*, QWidget*);
    void setTextLabel(const QString &text);
signals:
    void showPopup(QPoint);
    void buttonDestroyed();
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
    Q3Accel *accel;
    QPoint popupPos(QWidget *p);
};

class EXPORT PictButton : public CToolButton
{
    Q_OBJECT
public:
    PictButton(Q3ToolBar*, CommandDef *def);
    ~PictButton();
protected:
    virtual void setState();
    void paintEvent(QPaintEvent*);
    QSizePolicy sizePolicy() const;
    QSize minimumSizeHint() const;
    QSize sizeHint() const;
};

class EXPORT CToolCombo : public QComboBox, public CToolItem
{
    Q_OBJECT
public:
    CToolCombo(Q3ToolBar*, CommandDef *def, bool bCheck);
    ~CToolCombo();
    void setText(const QString&);
protected slots:
    void slotTextChanged(const QString &str);
    void btnDestroyed();
protected:
    QSizePolicy sizePolicy() const;
    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    virtual void setState();
    CToolButton	*m_btn;
    bool m_bCheck;
    virtual QWidget *widget() { return this; }
};

class EXPORT CToolEdit : public QLineEdit, public CToolItem
{
    Q_OBJECT
public:
    CToolEdit(Q3ToolBar*, CommandDef *def);
    ~CToolEdit();
protected slots:
    void btnDestroyed();
protected:
    virtual void setState();
    virtual QWidget *widget() { return this; }
    CToolButton	*m_btn;
};

class ButtonsMap;

class EXPORT CToolBar : public Q3ToolBar, public EventReceiver
{
    Q_OBJECT
public:
    CToolBar(CommandsDef *def, Q3MainWindow *parent);
    ~CToolBar();
    CommandsDef *m_def;
    void *param()				{ return m_param; }
    void setParam(void *param)	{ m_param = param; }
    void *processEvent(Event*);
public slots:
    void showPopup(QPoint p);
    void toolBarChanged();
    void checkState();
protected:
    void	mousePressEvent(QMouseEvent *e);
    void	contextMenuEvent(QContextMenuEvent *e);
    bool	bChanged;
    void	*m_param;
    ButtonsMap *buttons;
};

#endif

