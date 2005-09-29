/***************************************************************************
                          qkeybutton.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#include "qkeybutton.h"

#include <q3accel.h>
#include <QCursor>
#include <QStringList>

#include <QMouseEvent>
#include <QFocusEvent>
#include <QKeyEvent>

#ifdef USE_KDE
#include <kglobalaccel.h>
#endif

QKeyButton::QKeyButton(QWidget *parent, const char *name)
        :	QPushButton( parent )
{
    m_bGrab = false;
    connect(this, SIGNAL(clicked()), this, SLOT(click()));
}

void QKeyButton::focusInEvent(QFocusEvent *e)
{
    QPushButton::focusInEvent(e);
    startGrab();
}

void QKeyButton::focusOutEvent(QFocusEvent *e)
{
    endGrab();
    QPushButton::focusOutEvent(e);
}

void QKeyButton::startGrab()
{
    if (m_bGrab) return;
    m_bGrab = true;
    grabKeyboard();
    grabMouse(QCursor(Qt::IBeamCursor));
}

void QKeyButton::endGrab()
{
    if (!m_bGrab) return;
    m_bGrab = false;
    releaseKeyboard();
    releaseMouse();
}

void QKeyButton::click()
{
    if (hasFocus()) {
        clearFocus();
    } else {
        setFocus();
    }
}

void QKeyButton::mousePressEvent(QMouseEvent *e)
{
    QPushButton::mousePressEvent(e);
    endGrab();
}

void QKeyButton::keyPressEvent(QKeyEvent *e)
{
    setKey(e, true);
}

void QKeyButton::keyReleaseEvent(QKeyEvent *e)
{
    setKey(e, false);
}

void QKeyButton::setKey(QKeyEvent *e, bool bPress)
{
    if (!m_bGrab) return;
    QStringList btns;
    unsigned state = e->modifiers();
    unsigned key_state = 0;
    QString keyName;
    QString name;
    log(L_DEBUG, "-> %X %X", e->key(), e->modifiers());
    switch (e->key()){
    case Qt::Key_Shift:
        key_state = Qt::ShiftModifier;
        break;
    case Qt::Key_Control:
        key_state = Qt::ControlModifier;
        break;
    case Qt::Key_Alt:
        key_state = Qt::AltModifier;
        break;
#if COMPAT_QT_VERSION >= 0x030000
    case Qt::Key_Meta:
        key_state = Qt::MetaModifier;
        break;
#endif
    default:
        name = Q3Accel::keyToString(e->key());
        if ((name[0] == '<') && (name[(int)(name.length()-1)] == '>'))
            return;
    }
    if (bPress){
        state |= key_state;
    }else{
        state &= ~key_state;
    }
    if (state & Qt::AltModifier) keyName += "Alt+";
    if (state & Qt::ControlModifier) keyName += "Ctrl+";
    if (state & Qt::ShiftModifier) keyName += "Shift+";
#if COMPAT_QT_VERSION >= 0x030000
    if (state & Qt::MetaModifier) keyName += "Meta+";
#endif
    setText(keyName + name);
    if (name.length()){
        endGrab();
        emit changed();
    }
}

#ifndef _WINDOWS
#include "qkeybutton.moc"
#endif

