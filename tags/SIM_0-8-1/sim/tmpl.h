/***************************************************************************
                          tmpl.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#ifndef _TMPL_H
#define _TMPL_H

#include "defs.h"
#include <qobject.h>

class Exec;

class Tmpl : public QObject
{
    Q_OBJECT
public:
    Tmpl(QObject *parent);
    ~Tmpl();
public slots:
    void expand(const QString &tmpl, unsigned long uin);
signals:
    void ready(Tmpl *tmpl, const QString &res);
protected slots:
    void expand();
    void execReady(Exec*, int, const char*);
protected:
    Exec *exec;
    unsigned long m_uin;
    QString res;
    QString t;
    void expand(bool bExt);
};

#endif

