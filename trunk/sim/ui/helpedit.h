/***************************************************************************
                          helpedit.h  -  description
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

#ifndef _HELPEDIT_H
#define _HELPEDIT_H

#include "defs.h"
#include <qmultilineedit.h>

class BalloonMsg;

class HelpEdit : public QMultiLineEdit
{
    Q_OBJECT
public:
    HelpEdit(QWidget *p, const char *name=NULL);
    QString helpText;
    virtual void setEnabled(bool);
public slots:
    void showHelp();
    void closeHelp();
protected:
    BalloonMsg *help;
    virtual void mousePressEvent(QMouseEvent*);
    virtual void focusInEvent(QFocusEvent*);
    virtual void focusOutEvent(QFocusEvent*);
};

#endif

