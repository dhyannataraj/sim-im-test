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

#include "plugins.h"

#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QLibrary>
#include <QMessageBox>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QByteArray>
#include <QVector>
#include <memory>

#include "socket/simsockets.h"
#include "fetch.h"
#include "exec.h"
#include "misc.h"
#include "xsl.h"
#include "builtinlogger.h"
#include "profilemanager.h"

#include <ctype.h>
#include <errno.h>

#ifndef  LTDL_SHLIB_EXT
# if defined(Q_OS_MAC) /* MacOS needs .a */
#  define  LTDL_SHLIB_EXT ".dylib"
# else
#  if defined(_WIN32) || defined(_WIN64)
#   define  LTDL_SHLIB_EXT ".dll"
#  else
#   define  LTDL_SHLIB_EXT ".so"
#   include <dlfcn.h>
#  endif
# endif
#endif

namespace SIM
{

    Plugin::Plugin(unsigned base)
        : m_current(base)
          , m_base(base)
    {}

    unsigned Plugin::registerType()
    {
        return m_current++;
    }

    void Plugin::boundTypes()
    {
        m_current = (m_current | 0x3F) + 1;
    }

    class PluginManagerPrivate : public EventReceiver
    {
        public:
            PluginManagerPrivate(int argc, char **argv);
            ~PluginManagerPrivate();
        protected:
            virtual bool processEvent(Event *e);

            bool findParam(EventArg *a);
            void usage(const QString &);

            bool create(pluginInfo&);
            bool createPlugin(pluginInfo&);

            void release(pluginInfo&, bool bFree = true);
            void release(const QString &name);
            void release_all(Plugin *to);

            void load(pluginInfo&);
            void load(const QString &name);
            void load_all(EventPluginsLoad *p);

            void saveState();
            void loadState();
            void reloadState();

            pluginInfo *getInfo(const QString &name);
            pluginInfo *getInfo(int n);
            bool setInfo(const QString &name);

            bool arePluginsInBuildDirectory(QStringList& pluginsList);
            void loadPlugins(const QStringList& pluginsList);
            QString getPluginName(pluginInfo &info);

#ifndef WIN32
            Q_PID execute(const QString &prg, const QStringList &args);
#endif

            QString app_name;
            QStringList args;
            QVector<pluginInfo> plugins;
            QStringList cmds;
            QStringList descrs;

            unsigned m_base;
            bool m_bLoaded;
            bool m_bInInit;
            bool m_bAbort;
            bool m_bPluginsInBuildDir;  // plugins in build dir -> full path in pluginInfo.filePath

            ExecManager	 *m_exec;
            std::auto_ptr<BuiltinLogger> builtinLogger;

            friend class PluginManager;
    };

    static bool cmp_plugin(pluginInfo p1, pluginInfo p2)
    {
        return QString::compare(p1.name, p2.name, Qt::CaseInsensitive) < 0;
    }

    static bool findPluginsInBuildDir(const QDir &appDir, const QString &subdir, QStringList &pluginsList)
    {
        QString pluginsDir(appDir.absoluteFilePath("plugins"));
        log(L_DEBUG, "Searching for plugins in build directory '%s'...", qPrintable(pluginsDir));
        int count = 0;
        // trunk/plugins/*
        QDirIterator it(pluginsDir, QDir::Dirs|QDir::NoDotAndDotDot);
        while (it.hasNext()) {
            const QString dir = it.next();
            // trunk/plugins/$plugin_name/$plugin_name.so
            const QString pluginFilename = dir + '/' + subdir + '/' + it.fileName() + LTDL_SHLIB_EXT;
            if (QFile::exists(pluginFilename)) {
                log(L_DEBUG, "Found '%s'...", qPrintable(pluginFilename));
                pluginsList.append(pluginFilename);
                count++;
            }
        }
        log(L_DEBUG, "%i plugins found.", count);
        return count > 0;
    }

    bool PluginManagerPrivate::arePluginsInBuildDirectory(QStringList& pluginsList)
    {
        QDir appDir(qApp->applicationDirPath());
        if(findPluginsInBuildDir(appDir, ".", pluginsList)                    // cmake location is source dir itself
                || findPluginsInBuildDir(appDir.path() + "/..", ".", pluginsList)  // 
                || findPluginsInBuildDir(appDir, ".libs", pluginsList)             // autotools location is .libs subdur
                || findPluginsInBuildDir(appDir.path() + "/..", ".libs", pluginsList) 
                || findPluginsInBuildDir(appDir.path() + "/..", "debug", pluginsList)   // msvc + cmake
                || findPluginsInBuildDir(appDir.path() + "/..", "release", pluginsList) // msvc + cmake
                || findPluginsInBuildDir(appDir.path() + "/..", "relwithdebinfo", pluginsList) // msvc + cmake
          )
            return true;
        return false;
    }

    QString PluginManagerPrivate::getPluginName(pluginInfo &info)
    {
#if defined( WIN32 ) || defined( __OS2__ )
        QString pluginName = info.filePath;
        if(!m_bPluginsInBuildDir)
            pluginName = "plugins\\" + info.name;
#else
        QString pluginName = info.filePath;
        if(pluginName[0] != '/')
        {
            pluginName = PLUGIN_PATH;
            pluginName += '/';
            pluginName += info.name;
            pluginName += LTDL_SHLIB_EXT;
        }
#endif
        return pluginName;
    }

    void PluginManagerPrivate::loadPlugins(const QStringList& pluginsList)
    {
        for(QStringList::const_iterator it = pluginsList.begin(); it != pluginsList.end(); ++it)
        {
            QString f = QFileInfo(*it).fileName();
            int p = f.lastIndexOf('.');
            if (p > 0)
                f = f.left(p);
            pluginInfo info;
            info.plugin		 = NULL;
#if defined( WIN32 ) || defined( __OS2__ )
            info.name        = f.toLower();
#else
            info.name		 = f;
#endif
            info.filePath    = *it;
            info.cfg		 = NULL;
            info.bDisabled	 = false;
            info.bNoCreate	 = false;
            info.bFromCfg	 = false;
            info.module		 = NULL;
            info.info		 = NULL;
            info.base		 = 0;
            plugins.push_back(info);
            log(L_DEBUG, "Found plugin %s", qPrintable(info.name));
            fprintf(stderr,"Found plugin %s\n", qPrintable(info.name));
        }
    }

    PluginManagerPrivate::PluginManagerPrivate(int argc, char **argv)
        : EventReceiver(LowPriority), m_bPluginsInBuildDir(false)
    {
        m_bAbort = false;
        unsigned logLevel = L_ERROR | L_WARN;
        // #ifdef DEBUG // zowers: commented out ifdef to be able to get some output from users even on production systems
        logLevel |= L_DEBUG;
        // #endif
        builtinLogger.reset(new BuiltinLogger(logLevel));

        m_exec = new ExecManager;

        app_name = QString::fromLocal8Bit(*argv);
        for (argv++, argc--; argc > 0; argv++, argc--)
            args.push_back(QString::fromLocal8Bit(*argv));

        m_base = 0;
        m_bLoaded = false;
        m_bInInit = true;

        QStringList pluginsList;
        if(arePluginsInBuildDirectory(pluginsList))
        {
            log(L_DEBUG,"Loading plugins from build directory!");
            m_bPluginsInBuildDir = true;
        }
        else
        {
#if defined( WIN32 ) || defined( __OS2__ )
            QString pluginDir(app_file("plugins"));
#else
            QString pluginDir(PLUGIN_PATH);
#endif
            QDirIterator it(pluginDir, QStringList("*"LTDL_SHLIB_EXT), QDir::Files);
            while (it.hasNext())
                pluginsList.append(it.next());

            if(pluginsList.count() == 0) {
                log(L_ERROR,
                        "Can't access %s or directory contains no plugins!",
                        qPrintable(pluginDir));
                m_bAbort = true;
                return;
            }
            m_bAbort = false;

            log(L_DEBUG,"Loading plugins from %s",qPrintable(pluginDir));
        }
        qSort(pluginsList);
        loadPlugins(pluginsList);

        EventGetPluginInfo ePlugin("_core");
        ePlugin.process();
        const pluginInfo *coreInfo = ePlugin.info();
        if (!coreInfo)
        {
            log(L_ERROR,"Fatal error: Core plugin failed to load. Aborting!");
            m_bAbort = true;
            return;
        }
        qSort(plugins.begin(), plugins.end(), cmp_plugin);
        const int size = plugins.count();
        for(int i = 0; i < size; ++i) {
            create(plugins[i]);
            if (m_bAbort)
                return;
        }

        EventInit eStart;
        eStart.process();
        if(eStart.abortLoading())
        {
            log(L_ERROR,"EventInit failed - aborting!");
            m_bAbort = true;
            return;
        }
        for (QStringList::iterator it_args = args.begin(); it_args != args.end(); ++it_args){
            if ((*it_args).length()){
                usage(*it_args);
                break;
            }
        }
        m_bInInit = false;
    }

    PluginManagerPrivate::~PluginManagerPrivate()
    {
        release_all(NULL);
        delete m_exec;
        setLogEnable(false);
        XSL::cleanup();
    }

    bool PluginManagerPrivate::processEvent(Event *e)
    {
        switch (e->type()){
            case eEventArg:
                {
                    EventArg *a = static_cast<EventArg*>(e);
                    return findParam(a);
                }
            case eEventGetPluginInfo:
                {
                    EventGetPluginInfo *info = static_cast<EventGetPluginInfo*>(e);
                    if(info->pluginName().isEmpty())
                        info->setInfo(getInfo(info->idx()));
                    else
                        info->setInfo(getInfo(info->pluginName()));
                    return true;
                }
            case eEventApplyPlugin:
                {
                    EventApplyPlugin *p = static_cast<EventApplyPlugin*>(e);
                    return setInfo(p->pluginName());
                }
            case eEventPluginsUnload:
                {
                    EventPluginsUnload *p = static_cast<EventPluginsUnload*>(e);
                    release_all(p->plugin());
                    return true;
                }
            case eEventPluginsLoad:
                {
                    EventPluginsLoad *p = static_cast<EventPluginsLoad*>(e);
                    load_all(p);
                    return true;
                }
            case eEventUnloadPlugin:
                {
                    EventUnloadPlugin *p = static_cast<EventUnloadPlugin*>(e);
                    release(p->pluginName());
                    return true;
                }
            case eEventLoadPlugin:
                {
                    EventLoadPlugin *p = static_cast<EventLoadPlugin*>(e);
                    load(p->pluginName());
                    return true;
                }
            case eEventSaveState:
                saveState();
                break;
            case eEventGetArgs:
                {
                    EventGetArgs *ga = static_cast<EventGetArgs*>(e);
                    ga->setArgs(qApp->argc(), qApp->argv());
                    return true;
                }
#ifndef WIN32
            case eEventExec:
                {
                    EventExec *exec = static_cast<EventExec*>(e);
                    exec->setPid(execute(exec->cmd(), exec->args()));
                    return true;
                }
#endif
            default:
                break;
        }
        return false;
    }

    pluginInfo *PluginManagerPrivate::getInfo(const QString &name)
    {
        if (name.isEmpty())
            return NULL;
        for (int n = 0; n < plugins.size(); n++){
            pluginInfo &info = plugins[n];
            if (info.name == name)
                return &info;
        }
        return NULL;
    }

    void PluginManagerPrivate::release_all(Plugin *to)
    {
        if(!plugins.size())
            return;
        for(int n = plugins.size() - 1; n > 0; n--)
        {
            pluginInfo &info = plugins[n];
            if (to && (info.plugin == to))
                continue; // Were: break;
                          // Now unload all plugins exept one passed as arg

///////////////////////////////////////////////////
// I do not know why Shutov ignored protocol plugins here
// but on changing profile we should release protocol that no longer in use
///////////////////////////////////////////////////
//        if (to && info.info && (info.info->flags & (PLUGIN_PROTOCOL & ~PLUGIN_NOLOAD_DEFAULT)))  //FIXME Compleatly remove these two lines
//            continue;
            release(info, to != NULL);
            info.bDisabled = false;
            info.bFromCfg  = false;
            if (info.cfg){
                delete info.cfg;
                info.cfg = NULL;
            }
        }
    }

    void PluginManagerPrivate::load(const QString &name)
    {
        pluginInfo *info = getInfo(name);
        if (info)
            load(*info);
    }

    void PluginManagerPrivate::load(pluginInfo &info)
    {
        if (info.module == NULL){
            QString fullName = app_file(getPluginName(info));
            info.module = new QLibrary(fullName);
            if (info.module == NULL)
                fprintf(stderr, "Can't load plugin %s\n", qPrintable(info.name));

        }
        if (info.module == NULL)
            return;
        if (info.info == NULL){
            PluginInfo* (*getInfo)() = NULL;
            getInfo = (PluginInfo* (*)()) info.module->resolve("GetPluginInfo");
            if (getInfo == NULL)
            {
                fprintf(stderr, "Plugin %s doesn't have the GetPluginInfo entry (%s)\n", qPrintable(info.name), info.module->errorString().toUtf8().data());
                release(info);
                return;
            }
            info.info = getInfo();
#ifndef WIN32
#ifdef USE_KDE
            if (!(info.info->flags & PLUGIN_KDE_COMPILE)){
                fprintf(stderr, "Plugin %s is compiled without KDE support!\n", qPrintable(info.name));
                release(info);
                return;
            }
#else
            if (info.info->flags & PLUGIN_KDE_COMPILE){
                fprintf(stderr, "Plugin %s is compiled with KDE support!\n", qPrintable(info.name));
                release(info);
                return;
            }
#endif
#endif
        }
    }

    bool PluginManagerPrivate::create(pluginInfo &info)
    {
        Config* profile = ProfileManager::instance()->currentProfile();
        if(profile)
            info.bDisabled = !profile->value(info.name + "/enabled").toBool();
        if (info.plugin)
            return true;
        EventArg a1("--enable-" + info.name);
        if (a1.process())
        {
            info.bDisabled = false;
            info.bFromCfg = true;
        }
        EventArg a2("--disable-" + info.name);
        if (a2.process()){
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
        log(L_DEBUG, "[1]Load plugin %s", qPrintable(info.name));
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
        info.plugin = info.info->create(info.base, m_bInInit, info.cfg);
        if (info.plugin == NULL){
            info.bNoCreate = true;
            info.bDisabled = true;
            return false;
        }
        if (info.cfg){
            delete info.cfg;
            info.cfg = NULL;
        }
        if (info.info->flags & PLUGIN_RELOAD){
            reloadState();
            loadState();
        }
        EventPluginChanged e(&info);
        e.process();
        return true;
    }

    void PluginManagerPrivate::load_all(EventPluginsLoad *p)
    {
        if (p->abortLoading()){
            m_bAbort = true;
            qApp->quit();
            return;
        }
        Plugin *from = p->plugin();
        reloadState();
        int i;
        for (i = 0; i < plugins.size(); i++){
            pluginInfo &info = plugins[i];
            if (info.plugin == from)
                break;
        }
        for (; i < plugins.size(); i++)
            create(plugins[i]);
    }

    void PluginManagerPrivate::release(const QString &name)
    {
        pluginInfo *info = getInfo(name);
        if (info)
            release(*info);
    }

    void PluginManagerPrivate::release(pluginInfo &info, bool bFree)
    {
        if (info.plugin){
            log(L_DEBUG, "Unload plugin %s", qPrintable(info.name));
            delete info.plugin;
            info.plugin = NULL;
            EventPluginChanged e(&info);
            e.process();
        }
        if (info.module){
            if (bFree)
                delete info.module;
            info.module = NULL;
        }
        info.info = NULL;
    }

    pluginInfo *PluginManagerPrivate::getInfo(int n)
    {
        if (n >= plugins.size())
            return NULL;
        pluginInfo &info = plugins[n];
        return &info;
    }

    bool PluginManagerPrivate::setInfo(const QString &name)
    {
        pluginInfo *info = getInfo(name);
        if (info == NULL)
            return false;
        if (info->bDisabled)
        {
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

    void PluginManagerPrivate::saveState()
    {
        if(!ProfileManager::instance()->currentProfile())
        {
            // if current profile name is empty then should not write config for it.
            log(L_DEBUG, "Refusing writing plugins.conf for empty profile");
            return;
        }

        if (m_bAbort)
            return;
        getContacts()->save();
        for (int i = 0; i < plugins.size(); i++)
        {
            pluginInfo &info = plugins[i];
            ProfileManager::instance()->currentProfile()->setValue(info.name + "/enabled", !info.bDisabled);
        }
        ProfileManager::instance()->sync();
    }

    const unsigned long NO_PLUGIN = (unsigned long)(-1);

    void PluginManagerPrivate::reloadState()
    {
        m_bLoaded = false;
        for (int i = 0; i < plugins.size(); i++){
            pluginInfo &info = plugins[i];
            if (info.cfg){
                delete info.cfg;
                info.cfg = NULL;
            }
        }
    }

    void PluginManagerPrivate::loadState()
    {
        if (m_bLoaded)
            return;
        m_bLoaded = true;
        return;
    }

    bool PluginManagerPrivate::findParam(EventArg *a)
    {
        bool bRet = false;
        if (!a->desc().isEmpty()){
            cmds.push_back(a->arg());
            descrs.push_back(a->desc());
        }
        QString value = QString::null;
        if (a->arg().endsWith(":")){
            unsigned size = a->arg().length();
            QString arg = a->arg().left(size - 1);
            for (QStringList::iterator it = args.begin(); it != args.end(); ++it){
                if (!(*it).startsWith(arg))
                    continue;
                value = (*it).mid(size);
                if (value.length()){
                    *it = QString::null;
                    bRet = true;
                    break;
                }
                ++it;
                if (it != args.end()){
                    value = (*it);
                    *it = QString::null;
                    --it;
                    *it = QString::null;
                }
                bRet = true;
                break;
            }
        }else{
            int idx = args.indexOf(a->arg());
            if(idx >= 0) {
                value = args[idx];
                args[idx].clear();
                bRet = true;
            }
        }
        a->setValue(value);
        return bRet;
    }

    void PluginManagerPrivate::usage(const QString &err)
    {
        QString title = i18n("Bad option %1").arg(err);
        QString text = i18n("Usage: %1 ").arg(app_name);
        QString comment;
        QStringList::iterator itc = cmds.begin();
        QStringList::iterator itd = descrs.begin();
        for (; itc != cmds.end(); ++itc, ++itd){
            QString p = *itc;
            bool bParam = false;
            if (p.endsWith(":")){
                bParam = true;
                p = p.left(p.length() - 1);
            }
            text += '[' + p;
            if (bParam)
                text += "<arg>";
            text += "] ";
            comment += '\t' + p;
            if (bParam)
                comment += "<arg>";
            comment += "\t - ";
            comment += i18n((*itd));
            comment += '\n';
        }
        text += '\n';
        text += comment;
        QMessageBox::critical(NULL, title, text, "Quit", 0);
    }

#ifndef WIN32
    Q_PID PluginManagerPrivate::execute(const QString &prg, const QStringList &args)
    {
        if (prg.isEmpty())
            return (Q_PID)-1;  /* return  invalid pid */

        log(L_DEBUG, "Exec: %s", qPrintable(prg));

        Q_ULONG child;
        QStringList arglist;

        // split prg to program name and optional arguments
        const QStringList s = prg.split(" ");

        arglist = s + args;
        const QString prog = arglist[0];
        arglist.pop_front();

        QProcess *proc = new QProcess();
        proc->start(prog, arglist, QIODevice::NotOpen);
        if( proc->waitForStarted(100) ) {
            child = proc->pid();
            QObject::connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), proc, SLOT(deleteLater()));
        } else {
            child = (Q_PID)-1;
            log(L_DEBUG, "can't execute %s: %d", qPrintable(arglist[0]), proc->error());
            delete proc;
        }

        return child;
    }
#endif

    PluginManager::PluginManager(int argc, char **argv)
    {
        EventReceiver::initList();
        factory = new SIMSockets(qApp);
        contacts = new ContactList;
        p = new PluginManagerPrivate(argc, argv);
    }

    void deleteResolver();

    PluginManager::~PluginManager()
    {
        EventQuit().process();
        contacts->clearClients();
        delete p;
        delete contacts;
        delete factory;
        EventReceiver::destroyList();
        deleteResolver();
        ProfileManager::instance()->sync();
    }

    bool PluginManager::isLoaded()
    {
        return !p->m_bAbort;
    }

    ContactList *PluginManager::contacts = NULL;
    SocketFactory *PluginManager::factory = NULL;
}

// vim: set expandtab:
