/***************************************************************************
                          toolbtn.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan.ru
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

#include "defs.h"

#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qcombobox.h>

#include <list>
using namespace std;

class QMainWindow;
class QPopupMenu;
class QAccel;
class ToolBarStates;

class CPushButton : public QPushButton
{
    Q_OBJECT
public:
    CPushButton(QWidget *parent, const char *name = NULL);
    void setTip(const QString &tip);
protected:
    QString  tip;
};

class CToolButton : public QToolButton
{
    Q_OBJECT
public:
    CToolButton(QWidget * parent, const char *name);
    void setPopup(QPopupMenu *popup);
    void setTextLabel(const QString &label);
    QPoint popupPos(QWidget *w);
    void setAccel(int key);
    void enableAccel(bool bState);
    bool bCtrl;
    bool bProcessCtrl;
signals:
    void showPopup(QPoint);
protected slots:
    void ctrlClick();
    void btnClicked();
protected:
    int accelKey;
    QPopupMenu *popup;
    QAccel *ctrlAccel;
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void contextMenuEvent(QContextMenuEvent*);
    void showEvent(QShowEvent*);
    void hideEvent(QHideEvent*);
};

class PictButton : public CToolButton
{
    Q_OBJECT
public:
    PictButton(QToolBar*, const char *name);
    ~PictButton();
    void setText(const QString& text);
public slots:
    void setState(const QString& icon, const QString& text);
protected slots:
    void iconChanged();
protected:
    QString text;
    QString icon;
    void paintEvent(QPaintEvent*);
    QSizePolicy sizePolicy() const;
    QSize minimumSizeHint() const;
    QSize sizeHint() const;
};

class PictPushButton : public QPushButton
{
    Q_OBJECT
public:
    PictPushButton(QWidget*, const char *name=NULL);
    void setState(const QString& icon, const QString& text);
protected slots:
    void iconChanged();
protected:
    QString text;
    QString icon;
    void paintEvent(QPaintEvent*);
    QSize minimumSizeHint() const;
    QSize sizeHint() const;
};

class CToolCombo : public QComboBox
{
    Q_OBJECT
public:
    CToolCombo(QToolBar*, const QString& toolTip, const char *name);
protected:
    virtual QSizePolicy sizePolicy() const;
signals:
    void showPopup(QPoint);
};

const unsigned long BTN_SEPARATOR	= 0xFFFE;
const unsigned long BTN_END_DEF		= 0xFFFF;

const int BTN_TOGGLE		= 0x0001;
const int BTN_PICT			= 0x0002;
const int BTN_HIDE			= 0x0004;
const int BTN_COMBO			= 0x0008;
const int BTN_CTRL			= 0x0010;

typedef struct ToolBarDef
{
    unsigned long id;
    const char *icon;
    const char *icon_on;
    const char *text;
    int	flags;
    const char *slot;
    const char *popup_slot;
} ToolBarDef;

#define SEPARATOR	{ BTN_SEPARATOR, NULL, NULL, NULL, 0, NULL, NULL }
#define END_DEF		{ BTN_END_DEF, NULL, NULL, NULL, 0, NULL, NULL }

class CToolBar : public QToolBar
{
    Q_OBJECT
public:
    CToolBar(const ToolBarDef *def, list<unsigned long> *active, QMainWindow *parent, QWidget *receiver);
    ~CToolBar();
    void setState(int id, const char *icon, const QString &text);
    void setPopup(int id, QPopupMenu *popup);
    void setEnabled(int id, bool bEnable);
    void setIcon(int id, const char *icon);
    void show(int id = 0);
    void hide(int id = 0);
    bool isVisible(int id);
    bool isEnabled(int id);
    bool isOn(int id);
    bool isCtrl(int id);
    void setOn(int id, bool bOn);
    QWidget *getWidget(int id);
    QPoint popupPos(int id, QWidget *popup);
    static void save(const ToolBarDef *def, list<unsigned long> *active);
    void enableAccel(bool bState);
public slots:
    void popupActivated(int);
protected slots:
    void toolBarChanged(const ToolBarDef *def);
    void showPopup(QPoint p);
protected:
    ToolBarStates *states;
    bool isButton(int id);
    bool isPictButton(int id);
    bool isCombo(int id);
    QWidget *m_receiver;
    const ToolBarDef *m_def;
    list<unsigned long> *m_active;
};

#endif

