/***************************************************************************
                          cfgdlg.h  -  description
                             -------------------
    begin                : Sun Mar 17 2002
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

#ifndef _CFGDLG_H
#define _CFGDLG_H

#include "simapi.h"
#include "cfgdlgbase.h"

class ConfigureDialog : public ConfigureDialogBase, public EventReceiver
{
    Q_OBJECT
public:
    ConfigureDialog();
    ~ConfigureDialog();
    void raisePage(Client *client);
    void raisePhoneBook();
    void showUpdate(bool bShow);
signals:
    void applyChanges();
    void applyChanges(Client*, void*);
    void finished();
protected slots:
    void apply();
    void updateInfo();
    void itemSelected(QListViewItem*);
    void raisePage(QWidget*);
protected:
    void apply(QListViewItem *item);
    virtual void accept();
    virtual void *processEvent(Event*);
    void fill(unsigned id);
    void setTitle();
    unsigned m_nUpdates;
    bool m_bAccept;
    void closeEvent(QCloseEvent*);
    bool bLanguageChanged;
};

#endif

