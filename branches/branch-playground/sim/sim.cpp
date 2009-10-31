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

void parseArgs(int* l_argc, char*** l_argv, int argc, char** argv)
{
	int _argc = *l_argc;
	char** _argv = *l_argv;
#ifdef Q_OS_MAC
	for (char **p = argv; *p; ++p){
		if( strncmp( "-psn_", *p, 4 ) != 0 ) {
			_argv[_argc++] = *p;
		}
	}
#else
	_argv[_argc++] = argv[0];
	char **to = argv + 1;
	// check all parameters and sort them
	// _argc/v: parameter for KUnqiueApplication
	//  argc/v: plugin parameter
	for (char **p = argv + 1; *p; ++p){
		char *arg = *p;
		// check if "-" or "--"
		if (arg[0] != '-') {
			*(to++) = *p;
			continue;
		}
		arg++;
		if (arg[0] == '-')
			arg++;
		// if they are parameters with variable params we need
		// to skip the next param
		bool bSkip = false;
		const char **q;
		// check for qt or kde - parameters
		for (q = qt_args; *q; ++q){
			unsigned len = strlen(*q);
			bSkip = false;
			// variable parameter?
			if ((*q)[len-1] == ':'){
				len--;
				bSkip = true;
			}
			// copy them for KUnqiueApplication-args
			if ((strlen(arg) == len) && !memcmp(arg, *q, len))
				break;
		}
		// dunno know what to do here
		if (*q){
			_argv[_argc++] = *p;
			argc--;
			if (bSkip){
				++p;
				if (*p == NULL) break;
				_argv[_argc++] = *p;
				argc--;
			}
		}else{
			*(to++) = *p;
		}
	}
	*to = NULL;
	_argv[_argc] = NULL;
#endif
	*l_argc = _argc;
	*l_argv = _argv;
}

int main(int argc, char *argv[])
{
    SimFileEngineHandler simfs;

    int res = 1;
	QCoreApplication::setOrganizationDomain("sim-im.org");
	QCoreApplication::setApplicationName("Sim-IM");
	new SIM::ProfileManager(SIM::PathManager::configRoot());
#ifdef WIN32
    Qt::HANDLE hMutex = CreateMutexA(NULL, FALSE, "SIM_Mutex");
#elif defined(__OS2__)    
    HMTX hMutex = NULLHANDLE;
    if ( DosCreateMutexSem("\\SEM32\\SIM_Mutex", &hMutex, 0, FALSE) != 0 ) {
        // prevent running another instance
        return 1;
    }
#endif
    qInstallMsgHandler(simMessageOutput);

#ifndef WIN32
    int _argc = 0;
    char **_argv = new char*[argc + 1];
	parseArgs(&_argc, &_argv, argc, argv);
#ifdef USE_KDE
    KCmdLineArgs::init( _argc, _argv, &aboutData );
    KCmdLineOptions options[] =
        {
            { 0, 0, 0 }
        };
    KCmdLineArgs::addCmdLineOptions( options );
    KUniqueApplication::addCmdLineOptions();
    if (!KUniqueApplication::start())
        exit(-1);
    SimApp app;
#else
    SimApp app(_argc, _argv);
#endif
#if !defined(Q_OS_MAC) && !defined(__OS2__)
    old_errhandler = XSetErrorHandler(x_errhandler);
#endif
#else
    for (int i = 0; i < argc; i++){
        QByteArray arg = argv[i];
        if ((arg[0] == '/') || (arg[0] == '-'))
            arg = arg.mid(1);
        if ((arg == "reinstall") || (arg == "showicons") || (arg == "hideicons"))
            return 0;
    }
    SimApp app(argc, argv);
#endif
    QApplication::addLibraryPath( app.applicationDirPath() + "/plugins" );
#ifdef Q_OS_MAC
    PluginManager p(_argc, _argv);
#else
    PluginManager p(argc, argv);
#endif
    app.setQuitOnLastWindowClosed( false );
    if (p.isLoaded())
        res = app.exec();
#ifdef WIN32
    CloseHandle(hMutex);
#elif defined(__OS2__)    
    DosCloseMutexSem(hMutex);
#endif
	return res;
}
