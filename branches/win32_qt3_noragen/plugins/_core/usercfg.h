/***************************************************************************
                          usercfg.h  -  description
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

#ifndef _USERCFG_H
#define _USERCFG_H

#include "simapi.h"
#include "cfgdlgbase.h"

class CorePlugin;
class ConfigItem;

class UserConfig : public ConfigureDialogBase, public EventReceiver
{
    Q_OBJECT
public:
    UserConfig(Contact *conatct, Group *group);
    ~UserConfig();
    Contact *m_contact;
    Group   *m_group;
    bool raisePage(unsigned id);
    bool raiseDefaultPage();
    void showUpdate(bool);
signals:
    void applyChanges();
protected slots:
    void apply();
    void itemSelected(QListViewItem*);
    void updateInfo();
protected:
    virtual void accept();
    virtual void *processEvent(Event *e);
    void setTitle();
    void fill();
    unsigned m_nUpdates;
    unsigned m_defaultPage;
    bool raisePage(unsigned id, QListViewItem*);
    void removeCommand(unsigned id);
    bool removeCommand(unsigned id, QListViewItem*);
};

#endif

