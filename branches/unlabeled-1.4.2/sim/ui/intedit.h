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

#include <qlineedit.h>
#include <qvalidator.h>
#include <qgroupbox.h>

#if COMPAT_QT_VERSION < 0x030000
#include "qt3/qregexp.h"
#else
#include <qregexp.h>
#endif

class UI_EXPORT IntLineEdit : public QLineEdit
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

#if COMPAT_QT_VERSION < 0x030000

class UI_EXPORT QRegExpValidator : public QValidator
{
public:
    QRegExpValidator(const Qt3::QRegExp& rx, QWidget *parent);
    virtual State validate(QString &, int&) const;
protected:
    Qt3::QRegExp r;
};

#endif

class UI_EXPORT RegExpValidator : public QRegExpValidator
{
public:
    RegExpValidator(const char *regexp, QWidget *parent);
};

class UI_EXPORT EMailValidator : public RegExpValidator
{
public:
    EMailValidator(QWidget *parent);
};

class UI_EXPORT PhoneValidator : public RegExpValidator
{
public:
    PhoneValidator(QWidget *parent);
};

class QRadioButton;

class UI_EXPORT RadioGroup : public QGroupBox
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

