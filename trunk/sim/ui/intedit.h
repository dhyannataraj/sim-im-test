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
#include <qradiobutton.h>

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

class QGroupBox;
class QLabel;

class UI_EXPORT GroupRadioButton : public QRadioButton
{
    Q_OBJECT
public:
    GroupRadioButton(const QString &text, QGroupBox *parent);
public slots:
    void slotToggled(bool);
protected:
    bool eventFilter(QObject*, QEvent*);
    QGroupBox	*m_grp;
};

#endif

