/***************************************************************************
                          proxydlg.h  -  description
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

#ifndef _PROXYDLG_H
#define _PROXYDLG_H

#include "defs.h"
#include "proxydlgbase.h"

class ProxyDialog : public ProxyDlgBase
{
    Q_OBJECT
public:
    ProxyDialog(QWidget *p, const QString &msg = QString::null);
protected slots:
    void apply();
    void proxyChanged(bool);
    void proxyChanged(int);
};

#endif

