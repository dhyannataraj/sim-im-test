/***************************************************************************
                          plugins.cpp  -  description
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

#include "simapi.h"
#include "sockfactory.h"
#include "fetch.h"
#include "ltdl.h"
#include "exec.h"

#ifdef WIN32
#include <windows.h>
#else
#include <ctype.h>
#endif

#include <errno.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qregexp.h>

#include <algorithm>
using namespace std;

namespace SIM
{

Plugin::Plugin(unsigned base)
{
    m_base = base;
    m_current = base;
}

unsigned Plugin::registerType()
{
    return m_current++;
}

void Plugin::boundTypes()
{
    m_current = (m_current | 0x3F) + 1;
}

QWidget *Plugin::createConfigWindow(QWidget*)
{
    return NULL;
}

string Plugin::getConfig()
{
    return "";
}

typedef struct CmdDescr
{
    string arg;
    string descr;
} CmdDescr;

class PluginManagerPrivate : public EventReceiver
{
public:
    PluginManagerPrivate(int argc, char **argv);
    ~PluginManagerPrivate();
protected:
    void *processEvent(Event *e);

    bool findParam(const char *param, const char *descr, string *res);
    void usage(const char*);

    bool create(pluginInfo&);
    bool createPlugin(pluginInfo&);

    void release(pluginInfo&, bool bFree = true);
    void release(const char *name);
    void release_all(Plugin *to);

    void load(pluginInfo&);
    void load(const char *name);
    void load_all(Plugin *to);

    void saveState();
    void loadState();
    void reloadState();

    pluginInfo *getInfo(const char *name);
    pluginInfo *getInfo(unsigned n);
    bool setInfo(const char *name);

#ifndef WIN32
    void execute(const char *prg, const char *arg);
#endif

    int m_argc;
    char **m_argv;

    string app_name;
    list<string> args;
    vector<pluginInfo> plugins;
    list<string> cmds;
    list<string> descrs;

    unsigned m_base;
    bool m_bLoaded;
    bool m_bInInit;
    bool m_bAbort;

	ExecManager	 *m_exec;

    friend class PluginManager;
};

static bool cmp_plugin(pluginInfo p1, pluginInfo p2)
{
    const char *n1 = p1.name;
    const char *n2 = p2.name;
    for (; *n1 && *n2; n1++, n2++){
        if (tolower(*n1) < tolower(*n2))
            return true;
        if (tolower(*n1) > tolower(*n2))
            return false;
    }
    return (*n1 == 0) && (*n2 != 0);
}

PluginManagerPrivate::PluginManagerPrivate(int argc, char **argv)
        : EventReceiver(LowPriority)
{
    m_argc = argc;
    m_argv = argv;
	m_exec = new ExecManager;

    app_name = *argv;
    for (argv++, argc--; argc > 0; argv++, argc--)
        args.push_back(string(*argv));

    m_base = 0;
    m_bLoaded = false;
    m_bInInit = true;

    lt_dlinit();

    QStringList pluginsList;
    QDir pluginDir(app_file("plugins").c_str());
    pluginsList = pluginDir.entryList("*" LTDL_SHLIB_EXT);
    m_bAbort = false;

    for (QStringList::Iterator it = pluginsList.begin(); it != pluginsList.end(); ++it){
        QString f = *it;
        int p = f.findRev('.');
        if (p > 0) f = f.left(p);
        pluginInfo info;
        info.plugin		 = NULL;
#ifdef WIN32
        info.name		 = strdup(QFile::encodeName(f.lower()));
#else
        info.name		 = strdup(QFile::encodeName(f));
#endif
        info.config		 = NULL;
        info.bDisabled	 = false;
        info.bNoCreate	 = false;
        info.bFromCfg	 = false;
        info.module		 = NULL;
        info.info		 = NULL;
        info.base		 = 0;
        plugins.push_back(info);
    }
    sort(plugins.begin(), plugins.end(), cmp_plugin);
    for (vector<pluginInfo>::iterator itp = plugins.begin(); itp != plugins.end(); ++itp){
        create((*itp));
        if (m_bAbort)
            return;
    }
    m_bInInit = false;
    Event eStart(EventInit);
    eStart.process();
    for (list<string>::iterator it_args = args.begin(); it_args != args.end(); ++it_args){
        if ((*it_args).length()){
            usage((*it_args).c_str());
            break;
        }
    }
}

PluginManagerPrivate::~PluginManagerPrivate()
{
    release_all(NULL);
    for (vector<pluginInfo>::iterator itp = plugins.begin(); itp != plugins.end(); ++itp){
        free((*itp).name);
    }
	delete m_exec;
}

void *PluginManagerPrivate::processEvent(Event *e)
{
    CmdParam *p;
#ifndef WIN32
    ExecParam *exec;
#endif
    switch (e->type()){
    case EventArg:
        p = (CmdParam*)(e->param());
        return (void*)findParam(p->arg, p->descr, p->value);
    case EventPluginGetInfo:
        return getInfo((unsigned)(e->param()));
    case EventApplyPlugin:
        return (void*)setInfo((const char*)(e->param()));
    case EventPluginsUnload:
        release_all((Plugin*)(e->param()));
        return e->param();
    case EventPluginsLoad:
        load_all((Plugin*)(e->param()));
        return e->param();
    case EventUnloadPlugin:
        release((const char*)(e->param()));
        return e->param();
    case EventLoadPlugin:
        load((const char*)(e->param()));
        return e->param();
    case EventSaveState:
        saveState();
        break;
    case EventGetPluginInfo:
        return getInfo((const char*)(e->param()));
    case EventArgc:
        return (void*)(m_argc);
    case EventArgv:
        return (void*)(m_argv);
#ifndef WIN32
    case EventExec:
        exec = (ExecParam*)(e->param());
        execute(exec->cmd, exec->arg);
        return e->param();
#endif
    default:
        break;
    }
    return NULL;
}

pluginInfo *PluginManagerPrivate::getInfo(const char *name)
{
    for (unsigned n = 0; n < plugins.size(); n++){
        pluginInfo &info = plugins[n];
        if (!strcmp(name, info.name))
            return &info;
    }
    return NULL;
}

void PluginManagerPrivate::release_all(Plugin *to)
{
    for (int n = plugins.size() - 1; n >= 0; n--){
        pluginInfo &info = plugins[n];
        if (to && (info.plugin == to))
            break;
        if (to && info.info && (info.info->flags & (PLUGIN_PROTOCOL & ~PLUGIN_NOLOAD_DEFAULT)))
            continue;
        release(info, to != NULL);
        info.bDisabled = false;
        info.bFromCfg  = false;
    }
}

void PluginManagerPrivate::load(const char *name)
{
    pluginInfo *info = getInfo(name);
    if (info)
        load(*info);
}

void PluginManagerPrivate::load(pluginInfo &info)
{
    if (info.module == NULL){
#ifdef WIN32
        string pluginName = "plugins\\";
#else
        string pluginName = "plugins/";
#endif
        pluginName += info.name;
        pluginName += LTDL_SHLIB_EXT;
        string fullName = app_file(pluginName.c_str());
        info.module = (void*)lt_dlopen(fullName.c_str());
        if (info.module == NULL){
            log(L_WARN, "Can't load plugin %s: %s", fullName.c_str(), lt_dlerror());
            return;
        }
    }
    if (info.module == NULL)
        return;
    if (info.info == NULL){
        PluginInfo* (*getInfo)() = NULL;
        (lt_ptr&)getInfo = lt_dlsym((lt_dlhandle)info.module, "GetPluginInfo");
        if (getInfo == NULL){
            log(L_WARN, "Plugin %s haven't entry GetInfo", info.name);
            release(info);
            return;
        }
        info.info = getInfo();
#ifndef WIN32
#ifdef USE_KDE
        if (!(info.info->flags & PLUGIN_KDE_COMPILE)){
            log(L_WARN, "Plugin %s compile without KDE support", info.name);
            release(info);
            return;
        }
#else
        if (info.info->flags & PLUGIN_KDE_COMPILE){
            log(L_WARN, "Plugin %s compile with KDE support", info.name);
            release(info);
            return;
        }
#endif
#endif
    }
}

bool PluginManagerPrivate::create(pluginInfo &info)
{
    if (info.plugin)
        return true;
    string param;
    string descr;
    const char *short_name = info.name;
    for (; *short_name; short_name++){
        char c = *short_name;
        if ((c >= '0') && (c <= '9')) continue;
        break;
    }
    string value;
    param = "--enable-";
    param += short_name;
    if (findParam(param.c_str(), NULL, &value)){
        info.bDisabled = false;
        info.bFromCfg = true;
    }
    param = "--disable-";
    param += short_name;
    if (findParam(param.c_str(), NULL, &value)){
        info.bDisabled = true;
        info.bFromCfg = true;
    }
    if (info.bDisabled)
        return false;
    load(info);
    if (info.info == NULL)
        return false;
    if (m_bInInit && (info.info->flags & (PLUGIN_PROTOCOL & ~PLUGIN_NOLOAD_DEFAULT))){
        info.bDisabled = true;
        release(info);
        return false;
    }
    return createPlugin(info);
}

bool PluginManagerPrivate::createPlugin(pluginInfo &info)
{
    if (!info.bFromCfg && (info.info->flags & (PLUGIN_NOLOAD_DEFAULT & ~PLUGIN_DEFAULT))){
        info.bDisabled = true;
        release(info);
        return false;
    }
    log(L_DEBUG, "Load plugin %s", info.name);
    if (!m_bLoaded && !(info.info->flags & (PLUGIN_NO_CONFIG_PATH & ~PLUGIN_DEFAULT))){
        loadState();
        if (info.bDisabled || (!info.bFromCfg && (info.info->flags & (PLUGIN_NOLOAD_DEFAULT & ~PLUGIN_DEFAULT)))){
            release(info);
            return false;
        }
    }
    if (info.base == 0){
        m_base += 0x1000;
        info.base = m_base;
    }
    info.plugin = info.info->create(info.base, m_bInInit, info.config);
    if ((unsigned)(info.plugin) == ABORT_LOADING){
        m_bAbort = true;
        info.plugin = NULL;
    }
    if (info.plugin == NULL){
        info.bNoCreate = true;
        info.bDisabled = true;
        return false;
    }
    if (info.config){
        free(info.config);
        info.config = NULL;
    }
    if (info.info->flags & PLUGIN_RELOAD){
        reloadState();
        loadState();
    }
    Event e(EventPluginChanged, &info);
    e.process();
    return true;
}

void PluginManagerPrivate::load_all(Plugin *from)
{
    if (from == (Plugin*)ABORT_LOADING){
        m_bAbort = true;
        qApp->quit();
        return;
    }
    reloadState();
    unsigned i;
    for (i = 0; i < plugins.size(); i++){
        pluginInfo &info = plugins[i];
        if (info.plugin == from)
            break;
    }
    for (; i < plugins.size(); i++)
        create(plugins[i]);
}

void PluginManagerPrivate::release(const char *name)
{
    pluginInfo *info = getInfo(name);
    if (info)
        release(*info);
}

void PluginManagerPrivate::release(pluginInfo &info, bool bFree)
{
    if (info.plugin){
        log(L_DEBUG, "Unload plugin %s", info.name);
        delete info.plugin;
        info.plugin = NULL;
        Event e(EventPluginChanged, &info);
        e.process();
    }
    if (info.module){
        if (bFree)
            lt_dlclose((lt_dlhandle)info.module);
        info.module = NULL;
    }
    info.info = NULL;
}

pluginInfo *PluginManagerPrivate::getInfo(unsigned n)
{
    if (n >= plugins.size())
        return NULL;
    pluginInfo &info = plugins[n];
    return &info;
}

bool PluginManagerPrivate::setInfo(const char *name)
{
    pluginInfo *info = getInfo(name);
    if (info == NULL)
        return false;
    if (info->bDisabled){
        if (info->plugin == NULL)
            return false;
        release(*info);
        load(*info);
        return true;
    }
    if (info->plugin)
        return false;
    info->bFromCfg = true;
    load(*info);
    createPlugin(*info);
    return true;
}

static char PLUGINS_CONF[] = "plugins.conf";
static char ENABLE[] = "enable";
static char DISABLE[] = "disable";

void PluginManagerPrivate::saveState()
{
    if (m_bAbort)
        return;
    getContacts()->save();
    string cfgName = user_file(PLUGINS_CONF);
    QFile f(QFile::decodeName(cfgName.c_str()));
    if (!f.open(IO_WriteOnly | IO_Truncate)){
        log(L_ERROR, "Can't create %s", cfgName.c_str());
        return;
    }
    for (unsigned i = 0; i < plugins.size(); i++){
        pluginInfo &info = plugins[i];
        string line = "[";
        line += info.name;
        line += "]\n";
        line += info.bDisabled ? DISABLE : ENABLE;
        line += ",";
        line += number(info.base);
        line += "\n";
        f.writeBlock(line.c_str(), line.length());
        if (info.plugin){
            string cfg = info.plugin->getConfig();
            if (cfg.length()){
                f.writeBlock(cfg.c_str(), cfg.length());
                f.writeBlock("\n", 1);
            }
        }
    }
}

const unsigned NO_PLUGIN = (unsigned)(-1);

void PluginManagerPrivate::reloadState()
{
    m_bLoaded = false;
    for (unsigned i = 0; i < plugins.size(); i++){
        pluginInfo &info = plugins[i];
        if (info.config){
            free(info.config);
            info.config = NULL;
        }
    }
}

void PluginManagerPrivate::loadState()
{
    if (m_bLoaded)
        return;
    m_bLoaded = true;
    string cfgName = user_file(PLUGINS_CONF);
    QFile f(QFile::decodeName(cfgName.c_str()));
    if (!f.open(IO_ReadOnly)){
        log(L_ERROR, "Can't create %s", cfgName.c_str());
        return;
    }
    unsigned i = NO_PLUGIN;
    string cfg;
    string s;
    while (getLine(f, s)){
        if (s[0] != '['){
            if (i != NO_PLUGIN){
                cfg += s;
                cfg += "\n";
            }
            continue;
        }

        if (cfg.length() && (i != NO_PLUGIN))
            plugins[i].config = strdup(cfg.c_str());
        cfg = "";
        s = s.substr(1);
        string name = getToken(s, ']');
        i = NO_PLUGIN;
        for (unsigned n = 0; n < plugins.size(); n++){
            if (!strcmp(name.c_str(), plugins[n].name)){
                i = n;
                break;
            }
        }
        if (!getLine(f, s))
            break;
        if (i == NO_PLUGIN)
            continue;
        pluginInfo &info = plugins[i];
        string token = getToken(s, ',');
        if (!strcmp(token.c_str(), ENABLE)){
            info.bDisabled = false;
            info.bFromCfg  = true;
        }else if (!strcmp(token.c_str(), DISABLE)){
            info.bDisabled = true;
            info.bFromCfg  = true;
        }else{
            continue;
        }
        token = getToken(s, ',');
        info.base = atol(token.c_str());
        if (info.base > m_base)
            m_base = info.base;
    }
    if (cfg.length() && (i != NO_PLUGIN))
        plugins[i].config = strdup(cfg.c_str());
}

bool PluginManagerPrivate::findParam(const char *p, const char *descriptor, string *value)
{
    if (descriptor){
        cmds.push_back(string(p));
        descrs.push_back(string(descriptor));
    }
    (*value) = "";
    if (*p && p[strlen(p) - 1] == ':'){
        unsigned size = strlen(p) - 1;
        for (list<string>::iterator it = args.begin(); it != args.end(); ++it){
            if ((*it).length() < size) continue;
            if (memcmp((*it).c_str(), p, size)) continue;
            *value = (*it).c_str() + size;
            if (value->length()){
                *it = "";
                return true;
            }
            ++it;
            if (it != args.end()){
                *value = (*it);
                *it = "";
                --it;
                *it = "";
                return true;
            } else {
                return true;
            }
        }
    }else{
        for (list<string>::iterator it = args.begin(); it != args.end(); ++it){
            if (strcmp((*it).c_str(), p)) continue;
            *value = (*it);
            *it = "";
            return true;
        }
    }
    return false;
}

void PluginManagerPrivate::usage(const char *err)
{
    QString title = i18n("Bad option %1") .arg(err);
    QString text = i18n("Usage: %1 ") .arg(app_name.c_str());
    QString comment;
    list<string>::iterator itc = cmds.begin();
    list<string>::iterator itd = descrs.begin();
    for (; itc != cmds.end(); ++itc, ++itd){
        string p = *itc;
        bool bParam = false;
        if (p[p.length() - 1] == ':'){
            bParam = true;
            p = p.substr(0, p.length() - 1);
        }
        text += "[";
        text += p.c_str();
        if (bParam) text += "<arg>";
        text += "] ";
        comment += "\t";
        comment += p.c_str();
        if (bParam) comment += "<arg>";
        comment += "\t - ";
        comment += i18n((*itd).c_str());
        comment += "\n";
    }
    text += "\n";
    text += comment;
    QMessageBox::critical(NULL, title, text, "Quit", 0);
}

#ifndef WIN32
void PluginManagerPrivate::execute(const char *prg, const char *arg)
{
    if (*prg == 0) return;
    QString p = QString::fromLocal8Bit(prg);
    if (p.find("%s") >= 0){
        p.replace(QRegExp("%s"), arg);
    }else{
        p += " ";
        p += QString::fromLocal8Bit(arg);
    }
    log(L_DEBUG, "Exec: %s", (const char*)p.local8Bit());
    QStringList s = QStringList::split(" ", p);
    char **arglist = new char*[s.count()+1];
    unsigned i = 0;
    for ( QStringList::Iterator it = s.begin(); it != s.end(); ++it, i++ ) {
        string arg;
        arg = (*it).local8Bit();
        arglist[i] = strdup(arg.c_str());
    }
    arglist[i] = NULL;
    if(!fork()) {
        execvp(arglist[0], arglist);
        // when we're here an error occured ...
        // a write to the logoutput isn't possible because we haven't
        // these descriptors anymore - so we need printf
        printf("can't execute %s: %s", arglist[0], strerror(errno));
        _exit(-1);
    }
    for (char **p = arglist; *p != NULL; p++)
        free(*p);
    delete[] arglist;
}
#endif

PluginManager::PluginManager(int argc, char **argv)
{
    EventReceiver::initList();
    factory = new SIMSockets;
    contacts = new ContactList;
    FetchManager::manager = new FetchManager;
    p = new PluginManagerPrivate(argc, argv);
}

void deleteResolver();

PluginManager::~PluginManager()
{
    save_state();
    Event e(EventQuit);
    e.process();
    contacts->clearClients();
    delete p;
    delete FetchManager::manager;
    delete contacts;
    delete factory;
    EventReceiver::destroyList();
    setSmiles(NULL);
    deleteResolver();
}

bool PluginManager::isLoaded()
{
    return !p->m_bAbort;
}

ContactList *PluginManager::contacts = NULL;
SocketFactory *PluginManager::factory = NULL;

};
