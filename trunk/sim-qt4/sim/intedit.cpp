/***************************************************************************
                          intedit.cpp  -  description
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

#include "intedit.h"

#include <QGroupBox>
#include <QLayout>
#include <QLabel>
#include <QObject>
#include <QRadioButton>
#include <QPushButton>
#include <QTimer>
#include <QFocusEvent>
#include <QListIterator>
#include <QMoveEvent>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QMouseEvent>

IntLineEdit::IntLineEdit(QWidget *parent) : QLineEdit(parent)
{
    id = 0;
}

void IntLineEdit::focusOutEvent(QFocusEvent *e)
{
    QLineEdit::focusOutEvent(e);
    emit focusOut();
}

void IntLineEdit::keyPressEvent(QKeyEvent *e)
{
    QLineEdit::keyPressEvent(e);
    if (e->key() == Qt::Key_Escape) emit escape();
}

RegExpValidator::RegExpValidator(const char *reg_exp, QWidget *parent)
        : QRegExpValidator(QRegExp(reg_exp), parent)
{
}

EMailValidator::EMailValidator(QWidget *parent)
        : RegExpValidator("[A-Za-z0-9\\.\\-_\\+]+@[A-Za-z0-9\\-_]+\\.[A-Za-z0-9\\.\\-_]+", parent)
{
}

PhoneValidator::PhoneValidator(QWidget *parent)
        : RegExpValidator("\\+?[0-9 ]+(\\([0-9]+\\))?([0-9 ]+\\-)*[0-9 ]+", parent)
{
}

const unsigned GRP_RADIO_POS = 10;

class GrpRadioButton : public QRadioButton
{
public:
    GrpRadioButton(QWidget *parent);
protected:
    void keyPressEvent(QKeyEvent *e);
};

GrpRadioButton::GrpRadioButton(QWidget *parent)
        : QRadioButton(parent)
{
}

void GrpRadioButton::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()){
    case Qt::Key_Down:{
            QRadioButton *first = NULL;
            QRadioButton *next  = NULL;
            QObjectList l = parentWidget()->queryList("QRadioButton");
            QObject *obj;
            for (int i = 0; i < l.size(); ++i){
                if (first == NULL)
                    first = static_cast<QRadioButton*>(obj);
                if (obj == this){
                    if ((obj = l[i]) == NULL){
                        next = first;
                    }else{
                        next = static_cast<QRadioButton*>(obj);
                    }
                    break;
                }
            }
            if (next){
                next->setFocus();
                next->setChecked(true);
            }
            return;
        }
    case Qt::Key_Up:{
            QRadioButton *prev  = NULL;
            QObjectList l = parentWidget()->queryList("QRadioButton");
            QObject *obj;
            for ( int i = 0; i < l.size(); ++i){
                if ((obj == this) && prev)
                    break;
                prev = static_cast<QRadioButton*>(obj);
            }
            if (prev){
                prev->setFocus();
                prev->setChecked(true);
            }
            return;
        }
    }
    QRadioButton::keyPressEvent(e);
}

RadioGroup::RadioGroup(QWidget *parent) : QGroupBox(parent)
{
    m_bInit  = false;
    m_button = new GrpRadioButton(parent);
    QSize s = m_button->sizeHint();
    connect(m_button, SIGNAL(destroyed()), this, SLOT(radioDestroyed()));
    if (parentWidget()->layout() && parentWidget()->layout()->inherits("QVBoxLayout")){
        QVBoxLayout *lay = static_cast<QVBoxLayout*>(parentWidget()->layout());
        QLayoutIterator it = lay->iterator();
        int pos = lay->findWidget(this);
        if (pos >= 0){
            lay->insertSpacing(pos, s.height() / 2);
        }else{
            lay->addSpacing(s.height() / 2);
        }
    }
    connect(m_button, SIGNAL(toggled(bool)), this, SLOT(slotToggled(bool)));
    QObjectList l = parentWidget()->queryList("QRadioButton");
    QObject *obj;
    for ( int i = 0; i < l.size(); ++i){
        if (obj != this)
            break;
    }
    if (obj == NULL)
        m_button->setChecked(true);
    QTimer::singleShot(0, this, SLOT(slotToggled()));
}

RadioGroup::~RadioGroup()
{
    if (m_button)
        delete m_button;
}

void RadioGroup::radioDestroyed()
{
    m_button = NULL;
}

void RadioGroup::slotToggled()
{
    if (!m_bInit){
        QPushButton *btnDefault = NULL;
        QObjectList l = topLevelWidget()->queryList("QPushButton");
        QObject *obj;
        for ( int i = 0; i < l.size(); ++i){
            btnDefault = static_cast<QPushButton*>(obj);
            if (btnDefault->isDefault())
                break;
            btnDefault = NULL;
        }
        if (btnDefault){
            m_bInit = true;
            QObjectList l = parentWidget()->queryList("QLineEdit");
            QObject *obj;
            for (int i = 0; i < l.size(); ++i){
                connect(obj, SIGNAL(returnPressed()), btnDefault, SLOT(animateClick()));
            }
        }
    }
    slotToggled(m_button->isChecked());
}

void RadioGroup::slotToggled(bool bState)
{
    if (bState){
        QObjectList l = parentWidget()->queryList("QRadioButton");
        QObject *obj;
        for (int i = 0; i < l.size(); ++i){
            if (obj != m_button)
                static_cast<QRadioButton*>(obj)->setChecked(false);
        }
    }else{
        bState = true;
        QObjectList l = parentWidget()->queryList("QRadioButton");
        QObject *obj;
        for (int i = 0; i < l.size(); ++i){
            if (static_cast<QRadioButton*>(obj)->isChecked()){
                bState = false;
                break;
            }
        }
        if (bState)
            m_button->setChecked(true);
    }
    QObjectList l = queryList();
    QObject *obj;
    for ( int i = 0; i < l.size(); ++i){
        if (obj->inherits("QLabel") || obj->inherits("QLineEdit") || obj->inherits("QComboBox")){
            static_cast<QWidget*>(obj)->setEnabled(bState);
        }
    }
    emit toggled(bState);
}

void RadioGroup::show()
{
    QGroupBox::show();
    m_button->show();
}

void RadioGroup::hide()
{
    QGroupBox::hide();
    m_button->hide();
}

bool RadioGroup::isChecked()
{
    return m_button->isChecked();
}

void RadioGroup::setTitle(const QString &text)
{
    m_button->setText(text);
}

void RadioGroup::moveEvent(QMoveEvent *e)
{
    QGroupBox::moveEvent(e);
    QSize s = m_button->sizeHint();
    m_button->move(x() + GRP_RADIO_POS, y() - s.height() / 2);
    m_button->resize(s);
}

void RadioGroup::mousePressEvent(QMouseEvent *e)
{
    QGroupBox::mousePressEvent(e);
    if (e->button() == Qt::LeftButton)
        m_button->setChecked(true);
}

#ifndef _WINDOWS
#include "intedit.moc"
#endif
