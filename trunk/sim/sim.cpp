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

#include "defs.h"
#include "icons.h"
#include "mainwin.h"
#include "splash.h"
#include "about.h"
#include "log.h"

#ifdef USE_KDE
#include "sim.h"
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kglobal.h>
#else
#include <qapplication.h>
#include <qstringlist.h>
#define I18N_NOOP(A)	(A)
#endif

MainWindow *pMain   = NULL;
Splash     *pSplash = NULL;

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

static bool bPluralInit = false;
static int plural_form = -1;

static void initPlural()
{
    if (bPluralInit) return;
    bPluralInit = true;
    QString pf = i18n("_: Dear translator, please do not translate this string "
                    "in any form, but pick the _right_ value out of "
                    "NoPlural/TwoForms/French... If not sure what to do mail "
                    "thd@kde.org and coolo@kde.org, they will tell you. "
                    "Better leave that out if unsure, the programs will "
                    "crash!!\nDefinition of PluralForm - to be set by the "
                    "translator of kdelibs.po"));
    if ( pf == "NoPlural" )
        plural_form = 0;
    else if ( pf == "TwoForms" )
        plural_form = 1;
    else if ( pf == "French" )
        plural_form = 2;
    else if ( pf == "Gaeilge" )
        plural_form = 3;
    else if ( pf == "Russian" )
        plural_form = 4;
    else if ( pf == "Polish" )
        plural_form = 5;
    else if ( pf == "Slovenian" )
        plural_form = 6;
    else if ( pf == "Lithuanian" )
        plural_form = 7;
    else if ( pf == "Czech" )
        plural_form = 8;
    else if ( pf == "Slovak" )
        plural_form = 9;
    else if ( pf == "Maltese" )
        plural_form = 10;
}

QString put_n_in(const QString &orig, unsigned long n)
{
    QString ret = orig;
    int index = ret.find("%n");
    if (index == -1)
        return ret;
    ret.replace(index, 2, QString::number(n));
    return ret;
}

#define EXPECT_LENGTH(x) \
   if (forms.count() != x) \
	return QString( "BROKEN TRANSLATION %1" ).arg( singular ); 

QString i18n(const char *singular, const char *plural, unsigned long n)
{
    if (!singular || !singular[0] || !plural || !plural[0])
        return QString::null;
    char *newstring = new char[strlen(singular) + strlen(plural) + 6];
    sprintf(newstring, "_n: %s\n%s", singular, plural);
    QString r = i18n(newstring);
    delete [] newstring;
    initPlural();
    if ( r.isEmpty() || plural_form == -1) {
        if ( n == 1 )
            return put_n_in( QString::fromUtf8( singular ),  n );
        else
            return put_n_in( QString::fromUtf8( plural ),  n );
    }
    QStringList forms = QStringList::split( "\n", r, false );
    switch ( plural_form ) {
    case 0: // NoPlural
        EXPECT_LENGTH( 1 );
        return put_n_in( forms[0], n);
    case 1: // TwoForms
        EXPECT_LENGTH( 2 );
        if ( n == 1 )
            return put_n_in( forms[0], n);
        else
            return put_n_in( forms[1], n);
    case 2: // French
        EXPECT_LENGTH( 2 );
        if ( n == 1 || n == 0 )
            return put_n_in( forms[0], n);
        else
            return put_n_in( forms[1], n);
    case 3: // Gaeilge
        EXPECT_LENGTH( 3 );
        if ( n == 1 )
            return put_n_in( forms[0], n);
        else if ( n == 2 )
            return put_n_in( forms[1], n);
        else
            return put_n_in( forms[2], n);
    case 4: // Russian, corrected by mok
        EXPECT_LENGTH( 3 );
        if ( n%10 == 1  &&  n%100 != 11)
            return put_n_in( forms[0], n); // odin fail
        else if (( n%10 >= 2 && n%10 <=4 ) && (n%100<10 || n%100>20))
            return put_n_in( forms[1], n); // dva faila
        else
            return put_n_in( forms[2], n); // desyat' failov
    case 5: // Polish
        EXPECT_LENGTH( 3 );
        if ( n == 1 )
            return put_n_in( forms[0], n);
        else if ( n%10 >= 2 && n%10 <=4 && (n%100<10 || n%100>=20) )
            return put_n_in( forms[1], n);
        else
            return put_n_in( forms[2], n);
    case 6: // Slovenian
        EXPECT_LENGTH( 4 );
        if ( n%100 == 1 )
            return put_n_in( forms[1], n); // ena datoteka
        else if ( n%100 == 2 )
            return put_n_in( forms[2], n); // dve datoteki
        else if ( n%100 == 3 || n%100 == 4 )
            return put_n_in( forms[3], n); // tri datoteke
        else
            return put_n_in( forms[0], n); // sto datotek
    case 7: // Lithuanian
        EXPECT_LENGTH( 3 );
        if ( n%10 == 0 || (n%100>=11 && n%100<=19) )
            return put_n_in( forms[2], n);
        else if ( n%10 == 1 )
            return put_n_in( forms[0], n);
        else
            return put_n_in( forms[1], n);
    case 8: // Czech
        EXPECT_LENGTH( 3 );
        if ( n%100 == 1 )
            return put_n_in( forms[0], n);
        else if (( n%100 >= 2 ) && ( n%100 <= 4 ))
            return put_n_in( forms[1], n);
        else
            return put_n_in( forms[2], n);
    case 9: // Slovak
        EXPECT_LENGTH( 3 );
        if ( n == 1 )
            return put_n_in( forms[0], n);
        else if (( n >= 2 ) && ( n <= 4 ))
            return put_n_in( forms[1], n);
        else
            return put_n_in( forms[2], n);
    case 10: // Maltese
        EXPECT_LENGTH( 4 );
        if ( n == 1 )
            return put_n_in( forms[0], n );
        else if ( ( n == 0 ) || ( n%100 > 0 && n%100 <= 10 ) )
            return put_n_in( forms[1], n );
        else if ( n%100 > 10 && n%100 < 20 )
            return put_n_in( forms[2], n );
        else
            return put_n_in( forms[3], n );
    }
    return QString::null;
}

#endif

void SimApp::saveState(QSessionManager &sm)
{
    QApplication::saveState(sm);
    pMain->saveState();
}

int _argc;
char **_argv;

KAboutData *appAboutData = NULL;

int main(int argc, char *argv[])
{
    _argc = argc;
    _argv = argv;

    QApplication::setColorSpec( QApplication::ManyColor );

    KAboutData aboutData(PACKAGE,
                         I18N_NOOP("SIM"),
                         VERSION,
                         I18N_NOOP("ICQ client"),
                         KAboutData::License_GPL,
                         "Copyright (C) 2002, Vladimir Shutoff",
                         0,
                         "http://sim-icq.sourceforge.net/",
                         "sim-icq-main@lists.sourceforge.net");

    aboutData.addAuthor("Vladimir Shutoff",I18N_NOOP("Maintainer"),"shutoff@mail.ru");
    appAboutData = &aboutData;

#ifdef USE_KDE
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
    pSplash = new Splash;
    initIcons("");
    pMain = new MainWindow;
#ifdef USE_KDE
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
    pSplash->hide();

    return app.exec();
}

#ifdef USE_KDE
#include "sim.moc"
#endif


