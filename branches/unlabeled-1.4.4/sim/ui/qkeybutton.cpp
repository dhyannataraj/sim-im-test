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

#include <qaccel.h>
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
    QString keyName;
    if (state & AltButton) keyName += "Alt+";
    if (state & ControlButton) keyName += "Ctrl+";
    if (state & ShiftButton) keyName += "Shift+";
    QString name = QAccel::keyToString(e->key());
    if ((name[0] == '<') && (name[(int)(name.length()-1)] == '>'))
        return;
    setText(keyName + name);
    if (name.length()){
        endGrab();
        emit changed();
    }
}

#ifndef _WINDOWS
#include "qkeybutton.moc"
#endif

