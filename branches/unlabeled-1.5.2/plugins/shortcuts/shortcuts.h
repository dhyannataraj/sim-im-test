/***************************************************************************
                          shortcuts.h  -  description
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

#ifndef _SHORTCUTS_H
#define _SHORTCUTS_H

#include "simapi.h"
#include "stl.h"

typedef struct ShortcutsData
{
    Data	Key;
    Data	Global;
    Data	Mouse;
} ShortcutsData;

typedef map<unsigned, const char*>	MAP_STR;
typedef map<unsigned, bool>			MAP_BOOL;
typedef map<unsigned, CommandDef>	MAP_CMDS;

class KGlobalAccel;

class GlobalKey : public QObject
{
    Q_OBJECT
public:
    GlobalKey(CommandDef *cmd);
    ~GlobalKey();
    unsigned id() { return m_cmd.id; }
#ifdef WIN32
    int key() { return m_key; }
#else
#ifndef USE_KDE
unsigned key() { return m_key; }
    unsigned state() { return m_state; }
#endif
#endif
public slots:
    void execute();
protected:
    CommandDef m_cmd;
#ifdef WIN32
    int m_key;
#else
#ifdef USE_KDE
    KGlobalAccel *accel;
#else
    unsigned m_key;
    unsigned m_state;
#endif
#endif
};

class ShortcutsPlugin : public QObject, public Plugin, public EventReceiver
{
    Q_OBJECT
public:
    ShortcutsPlugin(unsigned, const char*);
    virtual ~ShortcutsPlugin();
    PROP_STRLIST(Key);
    PROP_STRLIST(Global);
    PROP_STRLIST(Mouse);
    void applyKeys();
    void releaseKeys();
    const char *getOldKey(CommandDef *cmd);
    bool getOldGlobal(CommandDef *cmd);
    static QWidget *getMainWindow();
    static unsigned stringToButton(const char *cfg);
    static string buttonToString(unsigned button);
protected:
    virtual bool eventFilter(QObject*, QEvent*);
    virtual void *processEvent(Event*);
    virtual string getConfig();
    virtual QWidget *createConfigWindow(QWidget *parent);
    void applyKeys(unsigned);
    void applyKey(CommandDef*);
    void releaseKeys(unsigned);
#ifdef WIN32
    void init();
    bool m_bInit;
#endif
    ShortcutsData data;
    MAP_STR		oldKeys;
    MAP_BOOL	oldGlobals;
    MAP_CMDS	mouseCmds;
};

#endif

