/***************************************************************************
                          sim.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "icons.h"
#include "mainwin.h"
#include "log.h"

#if USE_KDE
#include "sim.h"
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kglobal.h>
#else
#include <qapplication.h>
#endif

MainWindow *pMain = NULL;

#ifdef USE_KDE

KApplication *kApp = NULL;

SimApp::SimApp() : KUniqueApplication()
{
    firstInstance = true;
    connect(this, SIGNAL(lastWindowClosed()), this, SLOT(quit()));
}

SimApp::~SimApp()
{
}

int SimApp::newInstance()
{
    if (firstInstance){
        firstInstance = false;
    }else{
        pMain->setShow(true);
    }
    return 0;
}

#else

QString i18n(const char *text)
{
    return QObject::tr(text);
}

class SimApp : public QApplication
{
	public:
		    SimApp(int &argc, char **argv)
			                : QApplication(argc, argv) {}
	protected:
		        void saveState(QSessionManager&);
};

#endif

#if !defined(USE_KDE) || (QT_VERSION < 300)

QString put_n_in(const QString &orig, unsigned long n)
{
	    QString ret = orig;
	        int index = ret.find("%n");
		    if (index == -1)
			            return ret;
		        ret.replace(index, 2, QString::number(n));
			    return ret;
}

QString i18n(const char *singular, const char *plural, unsigned long n)
{
    if (n == 1)
        return put_n_in(QObject::tr(singular), n);
    return put_n_in(QObject::tr(plural), n);
}

#endif

void SimApp::saveState(QSessionManager &sm)
{
    QApplication::saveState(sm);
    pMain->saveState();
}

int _argc;
char **_argv;

int main(int argc, char *argv[])
{
    _argc = argc;
    _argv = argv;

    QApplication::setColorSpec( QApplication::ManyColor );
#if USE_KDE
    KAboutData aboutData(PACKAGE,
                         I18N_NOOP(PACKAGE),
                         VERSION,
                         I18N_NOOP("ICQ client"),
                         KAboutData::License_GPL,
                         "Copyright (C) 2002, Vladimir Shutoff",
                         0,
                         "http://sim.shutoff.spb.ru/",
                         "shutoff@mail.ru");

    aboutData.addAuthor("Vladimir Shutoff",I18N_NOOP("Maintainer"),"shutoff@mail.ru");

    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineOptions options[] =
        {
            { "b <dir>", i18n("Directory for files"), 0 },
            { "d <loglevel>", i18n("Loglevel"), 0 },
            { 0, 0, 0 }
        };
    KCmdLineArgs::addCmdLineOptions( options );
    KUniqueApplication::addCmdLineOptions();

    if (!KUniqueApplication::start())
        exit(-1);
    SimApp app;
    kApp = &app;
#else
    SimApp app(argc, argv);
#endif
    initIcons("");
    pMain = new MainWindow;
#if USE_KDE
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->isSet("b"))
        pMain->homeDir = strdup(args->getOption("b"));
    if (args->isSet("d"))
        log_level = atoi(args->getOption("d"));

#else
    for (int i = 0; i < argc; i++){
        if (!strcmp(argv[i], "-b") && argv[i+1])
            pMain->homeDir = argv[++i];
        if (!strcmp(argv[i], "-d") && argv[i+1])
            log_level = atoi(argv[++i]);
    }
#endif
    if (!pMain->init())
        return 0;
    app.setMainWidget(pMain);

    return app.exec();
}

#if USE_KDE
#include "sim.moc"
#endif


