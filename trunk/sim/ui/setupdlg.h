/***************************************************************************
                          setupdlg.h  -  description
                             -------------------
    begin                : Sun Mar 17 2002
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

#ifndef _SETUPDLG_H
#define _SETUPDLG_H

#include "defs.h"
#include "setupdlgbase.h"

class QListViewItem;
class TransparentTop;
class ICQUser;

class SetupDialog : public SetupDialogBase
{
    Q_OBJECT
public:
    SetupDialog(QWidget *parent, int nWin);
signals:
    void applyChanges(ICQUser*);
    void backgroundUpdated();
protected slots:
    void selectionChanged();
    void update();
    void apply();
    void ok();
    void iconChanged();
protected:
    void iconChanged(QListViewItem*);
    void addPage(QWidget *page, int id, const QString &name, const char *icon);
    QListViewItem *itemMain;
    TransparentTop *transparent;
    void setBackgroundPixmap(const QPixmap &pm);
};

#endif

