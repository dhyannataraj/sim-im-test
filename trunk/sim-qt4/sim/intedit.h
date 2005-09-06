/***************************************************************************
                          intedit.h  -  description
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

#ifndef _INTEDIT_H
#define _INTEDIT_H

#include "simapi.h"

#include <QLineEdit>
#include <QValidator>
#include <QGroupBox>
#include <QMouseEvent>
#include <QFocusEvent>
#include <QMoveEvent>
#include <QKeyEvent>
#include <QRegExp>


class EXPORT IntLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    IntLineEdit(QWidget *parent);
    unsigned id;
signals:
    void escape();
    void focusOut();
protected:
    void focusOutEvent(QFocusEvent*);
    void keyPressEvent(QKeyEvent*);
};

class EXPORT RegExpValidator : public QRegExpValidator
{
public:
    RegExpValidator(const char *regexp, QWidget *parent);
};

class EXPORT EMailValidator : public RegExpValidator
{
public:
    EMailValidator(QWidget *parent);
};

class EXPORT PhoneValidator : public RegExpValidator
{
public:
    PhoneValidator(QWidget *parent);
};

class QRadioButton;

class EXPORT RadioGroup : public QGroupBox
{
    Q_OBJECT
public:
    RadioGroup(QWidget *parent, const char *name);
    ~RadioGroup();
    bool isChecked();
    virtual void setTitle(const QString &);
    void show();
    void hide();
signals:
    void toggled(bool);
public slots:
    void slotToggled();
    void slotToggled(bool);
    void radioDestroyed();
protected:
    bool m_bInit;
    QRadioButton	*m_button;
    void moveEvent(QMoveEvent*);
    void mousePressEvent(QMouseEvent*);
};

#endif

