/***************************************************************************
                          actioncfg.h  -  description
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

#ifndef _ACTIONCFG_H
#define _ACTIONCFG_H

#include "actioncfgbase.h"
#include "simapi.h"

class ActionPlugin;
class LineEdit;
class QListViewItem;
class MenuConfig;

class ActionConfig : public ActionConfigBase
{
    Q_OBJECT
public:
    ActionConfig(QWidget *parent, struct ActionUserData *data, ActionPlugin *plugin);
    virtual ~ActionConfig();
public slots:
    void apply();
    void apply(void*);
    void selectionChanged(QListViewItem*);
    void help();
protected:
    LineEdit		*m_edit;
    QListViewItem	*m_editItem;
    MenuConfig		*m_menu;
    void resizeEvent(QResizeEvent *e);
    void setEnabled (bool);
    QPixmap makePixmap(const char *src);
    struct ActionUserData	*m_data;
    ActionPlugin *m_plugin;
};

#endif

