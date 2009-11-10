/***************************************************************************
                          sim.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; e<ither version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "simapi.h"
#include "log.h"
#include "misc.h"
#include "profilemanager.h"
#include "simfs.h"
#include "paths.h"

#include <QDir>

#include <QLibrary>
#include <QSettings>
#include <QMessageBox>

#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifdef USE_KDE
#include <QWidget>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kuniqueapplication.h>
#include "simapp/kdesimapp.h"
#else
#include "aboutdata.h"
#include "simapp/simapp.h"
#endif
#include <QApplication>

#if !defined(WIN32) && !defined(Q_OS_MAC) && !defined(__OS2__)
#include <X11/Xlib.h>
#endif

#include <QSettings>

using namespace SIM;

void simMessageOutput(QtMsgType, const char *msg)
{
    if (logEnabled())
        log(SIM::L_DEBUG, "QT: %s", msg);
}

#ifndef WIN32

static const char *qt_args[] =
    {
#ifdef USE_KDE
        "caption:",
        "icon:",
        "miniicon:",
        "config:",
        "dcopserver:",
        "nocrashhandler",
        "waitforwm",
        "style:",
        "geometry:",
        "smkey:",
        "nofork",
        "help",
        "help-kde",
        "help-qt",
        "help-all",
        "author",
        "version",
        "license",
#endif
        "display:",
        "session:",
        "cmap"
        "ncols:",
        "nograb",
        "dograb",
        "sync",
        "fn",
        "font:",
        "bg",
        "background:",
        "fg",
        "foreground:",
        "btn",
        "button:",
        "name:",
        "title:",
        "reverse",
        "screen:",
        NULL
    };

#if !defined(Q_OS_MAC) && !defined(__OS2__)
extern "C" {
    static int (*old_errhandler)(Display*, XErrorEvent*) = NULL;
    static int x_errhandler( Display *dpy, XErrorEvent *err )
    {
        if (err->error_code == BadMatch)
            return 0;
        if (old_errhandler)
            return old_errhandler(dpy, err);
        return 0;
    }
}
#endif
#endif

#ifndef REVISION_NUMBER
	#define REVISION_NUMBER 
#endif

#ifdef CVS_BUILD
#define _VERSION	VERSION " SVN " __DATE__
#else
#define _VERSION	VERSION
#endif

#ifdef WIN32
#ifdef _DEBUG

class Debug
{
public:
    Debug()		{}
    ~Debug()	{} //causes crash on close in win32 by noragen
};

Debug d;

#endif

static BOOL (WINAPI *_SHGetSpecialFolderPathA)(HWND hwndOwner, LPSTR lpszPath, int nFolder, BOOL fCreate) = NULL;
static BOOL (WINAPI *_SHGetSpecialFolderPathW)(HWND hwndOwner, LPSTR lpszPath, int nFolder, BOOL fCreate) = NULL;
#endif


int main(int argc, char *argv[])
{
    SimFileEngineHandler simfs;

    int res = 1;
	QCoreApplication::setOrganizationDomain("sim-im.org");
	QCoreApplication::setApplicationName("Sim-IM");
    new SIM::ProfileManager(SIM::PathManager::configRoot());
    qInstallMsgHandler(simMessageOutput);

#ifdef USE_KDE
    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions options[] =
        {
            { 0, 0, 0 }
        };
    KCmdLineArgs::addCmdLineOptions( options );
    KUniqueApplication::addCmdLineOptions();
    if (!KUniqueApplication::start())
        exit(-1);
#endif
    SimApp app(argc, argv);
#ifdef Q_OS_MAC
    QString sPluginPath = app.applicationDirPath() + "/../";
#else
    QString sPluginPath = app.applicationDirPath() + "/plugins";
#endif
    QApplication::addLibraryPath(sPluginPath);
    PluginManager p(argc, argv);
    app.setQuitOnLastWindowClosed( false );
    if (p.isLoaded())
        res = app.exec();
    return res;
}

// vim: set expandtab:

