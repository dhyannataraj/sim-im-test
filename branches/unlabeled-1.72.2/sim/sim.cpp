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
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "simapi.h"

#ifdef WIN32
#include <windows.h>
#else
#ifndef QT_MACOSX_VERSION
#include <X11/X.h>
#include <X11/Xlib.h>
#endif
#endif

#ifdef USE_KDE
#include <qwidgetlist.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kuniqueapplication.h>
#else
#include <aboutdata.h>
#include <qapplication.h>
#endif

#ifdef USE_KDE
class SimApp : public KUniqueApplication
{
public:
    SimApp();
    ~SimApp();
    int newInstance();
    void saveState(QSessionManager&);
protected:
    bool firstInstance;
};

SimApp::SimApp() : KUniqueApplication()
{
    firstInstance = true;
}

SimApp::~SimApp()
{
}

int SimApp::newInstance()
{
    if (firstInstance){
        firstInstance = false;
    }else{
        QWidgetList  *list = QApplication::topLevelWidgets();
        QWidgetListIt it( *list );
        QWidget *w;
        while ( (w=it.current()) != 0 ) {
            ++it;
            if (w->inherits("MainWindow")){
                raiseWindow(w);
            }
        }
        delete list;
    }
    return 0;
}

#else

class SimApp : public QApplication
{
public:
SimApp(int argc, char **argv) : QApplication(argc, argv) {}
    ~SimApp();
protected:
    void saveState(QSessionManager&);
};

SimApp::~SimApp()
{
}

#endif

void SimApp::saveState(QSessionManager &sm)
{
    QApplication::saveState(sm);
}

void simMessageOutput( QtMsgType, const char *msg )
{
    log(L_DEBUG, "QT: %s", msg);
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

#ifndef QT_MACOSX_VERSION
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

#ifdef CVS_BUILD
#define _VERSION	VERSION " CVS " __DATE__
#else
#define _VERSION	VERSION
#endif

int main(int argc, char *argv[])
{
    int res = 1;
#ifdef WIN32
    HANDLE hMutex = CreateMutexA(NULL, FALSE, "SIM_Mutex");
#endif
    QApplication::setColorSpec( QApplication::ManyColor );
    qInstallMsgHandler(simMessageOutput);
    KAboutData aboutData(PACKAGE,
                         I18N_NOOP("SIM"),
                         _VERSION,
                         I18N_NOOP("Multiprotcol instant messenger"),
                         KAboutData::License_GPL,
                         "Copyright (C) 2002-2004, Vladimir Shutoff",
                         0,
                         "http://sim-icq.sourceforge.net/",
                         "sim-icq-main@lists.sourceforge.net");
    aboutData.addAuthor("Vladimir Shutoff",I18N_NOOP("Maintainer"),"vovan@shutoff.ru");
    aboutData.addAuthor("Christian Ehrlicher",I18N_NOOP("Developer"),"Ch.Ehrlicher@gmx.de");
    setAboutData(&aboutData);
#ifndef WIN32
    int _argc = 0;
    char **_argv = new char*[argc + 1];
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
#ifndef QT_MACOSX_VERSION
    old_errhandler = XSetErrorHandler(x_errhandler);
#endif
#else
    for (int i = 0; i < argc; i++){
        string arg = argv[i];
        if ((arg[0] == '/') || (arg[0] == '-'))
            arg = arg.substr(1);
        if ((arg == "reinstall") || (arg == "showicons") || (arg == "hideicons"))
            return 0;
    }
    SimApp app(argc, argv);
    StyleInfo*  (*getStyleInfo)() = NULL;
    HINSTANCE hLib = LoadLibraryA("UxTheme.dll");
    if (hLib != NULL)
        hLib = LoadLibraryA(app_file("plugins\\styles\\xpstyle.dll").c_str());
    if (hLib != NULL)
        (DWORD&)getStyleInfo = (DWORD)GetProcAddress(hLib,"GetStyleInfo");
    if (getStyleInfo){
        StyleInfo *info = getStyleInfo();
        if (info)
            qApp->setStyle(info->create());
    }
#endif
    PluginManager p(argc, argv);
    if (p.isLoaded())
        res = app.exec();
#ifdef WIN32
    CloseHandle(hMutex);
#endif
    return res;
};



