/***************************************************************************
                          plugins.h  -  description
                             -------------------
    begin                : Sat Oct 28 2006
    copyright            : (C) 2006 by Christian Ehrlicher
    email                : ch.ehrlicher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PLUGINS_H
#define _PLUGINS_H

#include <qstring.h>
#include "sim_export.h"
#include "simapi.h"	// COPY_RESTRICTED

class QFile;
class QLibrary;
class QWidget;
class Buffer;

namespace SIM {

/* PluginManager - base class for main application */

class Client;
class ContactList;
class SocketFactory;

class EXPORT PluginManager
{
public:
    PluginManager(int argc, char **argv);
    ~PluginManager();
    bool isLoaded();
    static ContactList          *contacts;
    static SocketFactory        *factory;
private:
    class PluginManagerPrivate *p;

    COPY_RESTRICTED(PluginManager)
};

/* Plugin prototype */

class EXPORT Plugin
{
public:
    Plugin(unsigned base);
    virtual ~Plugin() {}
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual std::string getConfig();
    unsigned registerType();
    void boundTypes();
protected:
    unsigned m_current;
    unsigned m_base;
};

typedef Plugin *createPlugin(unsigned base, bool bStart, Buffer *cfg);

const unsigned PLUGIN_KDE_COMPILE    = 0x0001;
#ifdef USE_KDE
const unsigned PLUGIN_DEFAULT        = PLUGIN_KDE_COMPILE;
#else
const unsigned PLUGIN_DEFAULT        = 0x0000;
#endif
const unsigned PLUGIN_NO_CONFIG_PATH = 0x0002 | PLUGIN_DEFAULT;
const unsigned PLUGIN_NOLOAD_DEFAULT = 0x0004 | PLUGIN_DEFAULT;

const unsigned PLUGIN_PROTOCOL       = 0x0008 | PLUGIN_NOLOAD_DEFAULT;
const unsigned PLUGIN_NODISABLE      = 0x0010;
const unsigned PLUGIN_RELOAD         = 0x0020;

struct PluginInfo           // Information in plugin
{
    const char      *title;         // User title
    const char      *description;   // Description
    const char      *version;       // Version
    createPlugin    *create;        // create proc
    unsigned        flags;          // plugin flags
};

struct pluginInfo
{
    Plugin          *plugin;
    QString         name;
    QString         filePath;
    Buffer          *cfg;           // configuration data
    bool            bDisabled;      // no load this plugin
    bool            bNoCreate;      // can't create plugin
    bool            bFromCfg;       // init state from config
    QLibrary        *module;        // so or dll handle
    PluginInfo      *info;
    unsigned        base;           // base for plugin types
};

SIM_EXPORT PluginInfo *GetPluginInfo();

} // namespace SIM

#endif
