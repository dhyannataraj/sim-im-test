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

#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlineedit.h>

class QMainWindow;
class QPopupMenu;
class QAccel;
class ToolBarStates;

class UI_EXPORT CToolItem
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

class UI_EXPORT CToolCombo : public QComboBox, public CToolItem
{
    Q_OBJECT
public:
    CToolCombo(QToolBar*, CommandDef *def, bool bCheck);
    ~CToolCombo();
    void setText(const QString&);
protected slots:
    void slotTextChanged(const QString &str);
    void btnDestroyed();
protected:
    virtual void setState();
    CToolButton	*m_btn;
    bool m_bCheck;
    virtual QWidget *widget() { return this; }
};

class UI_EXPORT CToolEdit : public QLineEdit, public CToolItem
{
    Q_OBJECT
public:
    CToolEdit(QToolBar*, CommandDef *def);
    ~CToolEdit();
protected slots:
    void btnDestroyed();
protected:
    virtual void setState();
    virtual QWidget *widget() { return this; }
    CToolButton	*m_btn;
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

