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

#include <qobject.h>
#include <qstring.h>

#include "event.h"
#include "plugins.h"

struct ShortcutsData
{
    SIM::Data	Key;
    SIM::Data	Global;
    SIM::Data	Mouse;
};

typedef std::map<unsigned, const char*>	MAP_STR;
typedef std::map<unsigned, bool>		MAP_BOOL;
typedef std::map<unsigned, SIM::CommandDef>	MAP_CMDS;

class KGlobalAccel;

class GlobalKey : public QObject
{
    Q_OBJECT
public:
    GlobalKey(SIM::CommandDef *cmd);
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
    SIM::CommandDef m_cmd;
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

class ShortcutsPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
    Q_OBJECT
public:
    ShortcutsPlugin(unsigned, Buffer*);
    virtual ~ShortcutsPlugin();
    PROP_STRLIST(Key);
    PROP_STRLIST(Global);
    PROP_STRLIST(Mouse);
    void applyKeys();
    void releaseKeys();
    const char *getOldKey(SIM::CommandDef *cmd);
    bool getOldGlobal(SIM::CommandDef *cmd);
    static QWidget *getMainWindow();
    static unsigned stringToButton(const QString &cfg);
    static QString buttonToString(unsigned button);
protected:
    virtual bool eventFilter(QObject*, QEvent*);
    virtual void *processEvent(SIM::Event*);
    virtual std::string getConfig();
    virtual QWidget *createConfigWindow(QWidget *parent);
    void applyKeys(unsigned long);
    void applyKey(SIM::CommandDef*);
    void releaseKeys(unsigned long);
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

