/***************************************************************************
                          commands.h  -  description
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

#ifndef _COMMANDS_H
#define _COMMANDS_H

#include "simapi.h"

#include <map>
using namespace std;

class CorePlugin;
class CMenu;

typedef struct MenuDef
{
    CommandsDef		*def;
    CMenu			*menu;
    void			*param;
} MenuDef;

typedef map<unsigned, CommandsDef*> CMDS_MAP;
typedef map<unsigned, MenuDef>		MENU_MAP;

class Commands : public QObject, public EventReceiver
{
    Q_OBJECT
public:
    Commands();
    ~Commands();
    void set(CommandsDef*, const char *str);
protected slots:
    void popupActivated();
protected:
    bool eventFilter(QObject *o, QEvent *e);
    void *processEvent(Event*);
    CommandsDef *createBar(unsigned id);
    void removeBar(unsigned id);
    CommandsDef *createMenu(unsigned id);
    void removeMenu(unsigned id);
    void *show(unsigned id, QMainWindow *parent);
    CMenu *get(CommandDef *cmd);
    CommandsDef *getDef(unsigned id);
    CMenu *processMenu(unsigned id, void *param, int key);
    void customize(CommandsDef *def);
    void customizeMenu(unsigned id);
    void clear();
    unsigned cur_id;
    CMDS_MAP bars;
    MENU_MAP menues;
};

#endif

