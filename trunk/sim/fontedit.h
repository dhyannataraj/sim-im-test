/***************************************************************************
                          fontedit.h  -  description
                             -------------------
    begin                : Sun Mar 17 2002
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

#ifndef _FONTEDIT_H
#define _FONTEDIT_H

#include "defs.h"
#include <qframe.h>
#include <qfont.h>

class QLabel;

class FontEdit : public QFrame
{
    Q_OBJECT
public:
    FontEdit(QWidget *parent, const char *name=NULL);
    void setWinFont(const QFont&);
    const QFont &winFont() { return f; }
protected slots:
    void chooseFont();
protected:
    QFont f;
    QLabel *lblFont;
};

#endif

