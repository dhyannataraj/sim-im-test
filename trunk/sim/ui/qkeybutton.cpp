/***************************************************************************
                          qkeybutton.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#include "defs.h"
#include "qkeybutton.h"
#include "keys.h"

#include <qcursor.h>
#include <qstringlist.h>

#ifdef USE_KDE
#include <kglobalaccel.h>
#endif

QKeyButton::QKeyButton(QWidget *parent, const char *name)
        :	QPushButton( parent, name )
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
    grabMouse(QCursor(IbeamCursor));
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
    setFocus();
}

void QKeyButton::mousePressEvent(QMouseEvent *e)
{
    QPushButton::mousePressEvent(e);
    endGrab();
}

void QKeyButton::keyPressEvent(QKeyEvent *e)
{
    setKey(e);
}

void QKeyButton::keyReleaseEvent(QKeyEvent *e)
{
    setKey(e);
}

void QKeyButton::setKey(QKeyEvent *e)
{
    if (!m_bGrab) return;
    QStringList btns;
    ButtonState state = e->state();
    if (state & AltButton) btns.append("ALT");
    if (state & ControlButton) btns.append("CTRL");
    if (state & ShiftButton) btns.append("SHIFT");
    const char *name = NULL;
    if (e->key() &&
            (e->key() != Key_Alt) &&
            (e->key() != Key_Control) &&
            (e->key() != Key_Shift))
        name = HotKeys::keyToString(e->key());
    if (name) btns.append(name);
    setText(btns.join("-"));
    if (name){
        endGrab();
        emit changed();
    }
}

#ifndef _WINDOWS
#include "qkeybutton.moc"
#endif

