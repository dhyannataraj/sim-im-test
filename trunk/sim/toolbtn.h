/***************************************************************************
                          toolbtn.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
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

#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qcombobox.h>

class QPopupMenu;
class QAccel;

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
    CToolButton( const QIconSet & p, const QString & textLabel,
                 const QString & grouptext, QObject * receiver, const char * slot, QToolBar * parent,
                 const char * name = 0 );
    CToolButton( const QIconSet & pOff, const QPixmap & pOn,  const QString & textLabel,
                 const QString & grouptext, QObject * receiver, const char * slot, QToolBar * parent,
                 const char * name = 0 );
    CToolButton ( QWidget * parent, const char * name = 0 );
    void setPopup(QPopupMenu *popup);
    void setTextLabel(const QString &label);
    QPoint popupPos(QWidget *w);
    void setAccel(int key);
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
    PictButton(QToolBar*);
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
    CToolCombo(QToolBar*, const QString& toolTip);
protected:
    virtual QSizePolicy sizePolicy() const;
};

#endif

