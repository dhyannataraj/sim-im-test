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

#include "cfgdlgbase.h"

class ConfigureDialog : public ConfigureDialogBase, public SIM::EventReceiver
{
    Q_OBJECT
public:
    ConfigureDialog();
    ~ConfigureDialog();
    void raisePage(SIM::Client *client);
    void raisePhoneBook();
    void showUpdate(bool bShow);
signals:
    void applyChanges();
    void applyChanges(SIM::Client*, void*);
    void finished();
protected slots:
    void apply();
    void updateInfo();
    void itemSelected(QListViewItem*);
    void raisePage(QWidget*);
    void repaintCurrent();
protected:
    void accept();
    void reject();
    void apply(QListViewItem *item);
    virtual bool processEvent(SIM::Event*);
    void fill(unsigned id);
    void setTitle();
    bool setCurrentItem(QListViewItem *parent, unsigned id);
    QListViewItem *findItem(QWidget *w);
    QListViewItem *findItem(QWidget *w, QListViewItem *parent);
    unsigned m_nUpdates;
    bool m_bAccept;
    void closeEvent(QCloseEvent*);
    bool bLanguageChanged;
};

#endif

