/***************************************************************************
                          autoreply.h  -  description
                             -------------------
    begin                : Sun Mar 24 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _AUTOREPLY_H
#define _AUTOREPLY_H

#include "defs.h"
#include "autoreplybase.h"

class QTimer;

class AutoReplyDlg : public AutoreplyBase
{
    Q_OBJECT
public:
    AutoReplyDlg(QWidget *p, unsigned long status);
protected slots:
    void apply();
    void textChanged();
    void toggled(bool);
    void tick();
protected:
    QTimer *timer;
    unsigned long status;
    int timeLeft;
};

#endif

