/***************************************************************************
                          helpedit.cpp  -  description
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

#include "helpedit.h"
#include "ballonmsg.h"

#include <qtimer.h>

HelpEdit::HelpEdit(QWidget *parent, const char *name)
        : QMultiLineEdit(parent, name)
{
    help = NULL;
}

void HelpEdit::focusInEvent(QFocusEvent *e)
{
    showHelp();
    QMultiLineEdit::focusInEvent(e);
}

void HelpEdit::focusOutEvent(QFocusEvent *e)
{
    QMultiLineEdit::focusOutEvent(e);
    QTimer::singleShot(0, this, SLOT(closeHelp()));
}

void HelpEdit::setEnabled(bool bEnabled)
{
    if (!bEnabled)
        QTimer::singleShot(0, this, SLOT(closeHelp()));
    QMultiLineEdit::setEnabled(bEnabled);
}

void HelpEdit::mousePressEvent(QMouseEvent *e)
{
    showHelp();
    QMultiLineEdit::mousePressEvent(e);
}

void HelpEdit::showHelp()
{
    if (help) return;
    QStringList btn;
    QPoint p = mapToGlobal(rect().topLeft());
    QRect rc(p.x(), p.y(), width(), height());
    help = new BalloonMsg(helpText, rc, btn, this, false, false);
    help->show();
}

void HelpEdit::closeHelp()
{
    if (help == NULL) return;
    delete help;
    help = NULL;
}

#ifndef _WINDOWS
#include "helpedit.moc"
#endif

