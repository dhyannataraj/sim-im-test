/***************************************************************************
                          searchdlg.h  -  description
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

#ifndef _SEARCHDLG_H
#define _SEARCHDLG_H

#include "defs.h"
#include "searchbase.h"

class ICQEvent;
class TransparentTop;

class SearchDialog : public SearchDlgBase
{
    Q_OBJECT
public:
    SearchDialog(QWidget *p);
protected slots:
    void processEvent(ICQEvent*);
    void textChanged(const QString&);
    void currentChanged(QWidget*);
    void setState();
    void newSearch();
    void search();
protected:
    virtual void closeEvent(QCloseEvent*);
    virtual void setBackgroundPixmap(const QPixmap&);
    ICQEvent *event;
    TransparentTop *transparent;
};

#endif

