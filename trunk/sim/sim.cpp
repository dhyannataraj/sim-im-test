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
#include "control.h"
#include "client.h"
#include "log.h"
#include "sim.h"
#include "ui/logindlg.h"

#ifndef WIN32
#include <stdio.h>
#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>
#else
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#endif

#ifdef USE_KDE
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kglobal.h>
#include <kstddirs.h>
#else
#include <qapplication.h>
#define I18N_NOOP(A)	(A)
#endif

#include <errno.h>

#include <qfile.h>
#include <qdir.h>
#include <qstringlist.h>

MainWindow	*pMain   = NULL;
Splash		*pSplash = NULL;
LoginDialog	*pLoginDlg = NULL;

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

QString i18n(const char *text, const char *comment)
{
    return QObject::tr(text, comment);
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
                      "translator of kdelibs.po");
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

void resetPlural()
{
    bPluralInit = false;
    initPlural();
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

#ifndef WIN32

void simMessageOutput( QtMsgType, const char *msg )
{
    log(L_DEBUG, "QT: %s", msg);
}

#endif

#ifdef WIN32

bool makedir(char *p)
{
    char *r = strrchr(p, '\\');
    if (r == NULL) return true;
    *r = 0;
    CreateDirectoryA(p, NULL);
    *r = '\\';
    return true;
}

#else

bool makedir(char *p)
{
    bool res = true;
    char *r = strrchr(p, '/');
    if (r == NULL) return res;
    *r = 0;
    struct stat st;
    if (stat(p, &st)){
        if (makedir(p)){
            if (mkdir(p, 0755)){
                log(L_ERROR, "Can't create %s: %s", p, strerror(errno));
                res = false;
            }
        }else{
            res = false;
        }
    }else{
        if ((st.st_mode & S_IFMT) != S_IFDIR){
            log(L_ERROR, "%s no directory", p);
            res = false;
        }
    }
    *r = '/';
    return res;
}

#endif

#ifdef WIN32
static BOOL (WINAPI *_SHGetSpecialFolderPath)(HWND hwndOwner, LPSTR lpszPath, int nFolder, BOOL fCreate) = NULL;
#endif

static string homeDir;

string buildFileName(const char *name)
{
    string s = homeDir;
    if (s.length() == 0){
#ifndef _WINDOWS
        struct passwd *pwd = getpwuid(getuid());
        if (pwd){
            s = pwd->pw_dir;
        }else{
            log(L_ERROR, "Can't get pwd");
        }
        if (s[s.size() - 1] != '/') s += '/';
#ifdef USE_KDE
        char *kdehome = getenv("KDEHOME");
        if (kdehome){
            s = kdehome;
        }else{
            s += ".kde/";
        }
        if (s.length() == 0) s += '/';
        if (s[s.length()-1] != '/') s += '/';
        s += "share/apps/sim/";
#else
        s += ".sim/";
#endif
#else
        char szPath[MAX_PATH];
        HINSTANCE hLib = LoadLibraryA("Shell32.dll");
        if (hLib != NULL)
            (DWORD&)_SHGetSpecialFolderPath = (DWORD)GetProcAddress(hLib,"SHGetSpecialFolderPathA");
        if (_SHGetSpecialFolderPath && _SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, true)){
            s = szPath;
            if (s.length()  == 0) s = "c:\\";
            if (s[s.length() - 1] != '\\') s += '\\';
            s += "sim\\";
        }else{
            s = app_file("");
        }
#endif
    }
#ifndef _WINDOWS
    if (s[s.length() - 1] != '/') s += '/';
#else
    if (s[s.length() - 1] != '\\') s += '\\';
#endif
    homeDir = s;
    s += name;
    makedir((char*)s.c_str());
    return s;
}

static string app_file_name;

const char *app_file(const char *f)
{
    app_file_name = "";
#ifdef WIN32
    char buff[256];
    GetModuleFileNameA(NULL, buff, sizeof(buff));
    char *p = strrchr(buff, '\\');
    if (p) *p = 0;
    app_file_name = buff;
    if (app_file_name.length() && (app_file_name[app_file_name.length()-1] != '\\'))
        app_file_name += "\\";
#else
#ifdef USE_KDE
    QStringList lst = KGlobal::dirs()->findDirs("data", "sim");
    for (QStringList::Iterator it = lst.begin(); it != lst.end(); ++it){
        QFile f(*it + f);
        if (f.exists()){
            app_file_name = (const char*)f.name().local8Bit();
            return app_file_name.c_str();
        }
    }
    if (!lst.isEmpty()){
        app_file_name = (const char*)lst[0].local8Bit();
    }
#else
    app_file_name = PREFIX "/share/apps/sim/";
#endif
#endif
    app_file_name += f;
    return app_file_name.c_str();
}

char ICQ_CONF[] = "icq.conf";
char SIM_CONF[] = "sim.conf";
char HISTORY[] = "history";
char INCOMING_FILES[] = "IncomingFiles";

list<unsigned long> uins;

unsigned long loadUIN(const QString &name)
{
    QFile f(name);
    if (!f.open(IO_ReadOnly)) return 0;
    for (;;){
        QString line;
        long readn = f.readLine(line, 512);
        if (readn < 0) return 0;
        if (line.at(0) == '[') return 0;
        if (line.left(4) == "UIN=")
            return line.mid(4).toULong();
    }
}

void rename(const char *path, const QString &prefix)
{
    string from = buildFileName(path);
    string to;
    to = prefix.local8Bit();
    to += path;
    to = buildFileName(to.c_str());
    QDir d;
    if (!d.rename(QString::fromLocal8Bit(from.c_str()), QString::fromLocal8Bit(to.c_str())))
        log(L_WARN, "Can't rename %s to %s", from.c_str(), to.c_str());
}

void scanUIN()
{
    uins.clear();
    QFile f(QString::fromLocal8Bit(buildFileName(ICQ_CONF).c_str()));
    if (f.exists()){
        // Need migrate
        unsigned long uin = loadUIN(f.name());
        if (uin){
            QString prefix = QString::number(uin);
#ifdef WIN32
            prefix += "\\";
#else
            prefix += "/";
#endif
            rename(ICQ_CONF, prefix);
            rename(SIM_CONF, prefix);
            rename(HISTORY, prefix);
            rename(INCOMING_FILES, prefix);
            pSplash->LastUIN = uin;
        }
    }
    QDir d(QString::fromLocal8Bit(buildFileName("").c_str()));
    QStringList lst = d.entryList(QDir::Dirs);
    for (QStringList::Iterator it = lst.begin(); it != lst.end(); ++it){
        QString subDir = *it;
        unsigned long uin = subDir.toULong();
        if (uin == 0) continue;
#ifdef WIN32
        subDir = d.path() + "\\" + subDir + "\\";
#else
        subDir = d.path() + "/" + subDir + "/";
#endif
        subDir += ICQ_CONF;
        QFile f(subDir);
        if (f.exists()){
            unsigned u = loadUIN(f.name());
            if (u == uin){
                uins.push_back(u);
            }else{
                log(L_WARN, "Bad UIN for %u", uin);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    _argc = argc;
    _argv = argv;

#ifdef WIN32
    CreateMutexA(NULL, FALSE, "SIM_Mutex");
#endif

    QApplication::setColorSpec( QApplication::ManyColor );
#ifndef WIN32
    qInstallMsgHandler(simMessageOutput);
#endif

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
            { "s <socket>", i18n("Control socket"), 0 },
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
    string ctrlSock;
#ifdef USE_KDE
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->isSet("b"))
        homeDir = strdup(args->getOption("b"));
    if (args->isSet("d"))
        log_level = atoi(args->getOption("d"));
    if (args->isSet("s"))
        ctrlSock = strdup(args->getOption("s"));
#else
    for (int i = 0; i < argc; i++){
        if (!strcmp(argv[i], "-b") && argv[i+1])
            homeDir = argv[++i];
        if (!strcmp(argv[i], "-d") && argv[i+1])
            log_level = atoi(argv[++i]);
        if (!strcmp(argv[i], "-s") && argv[i+1])
            ctrlSock = argv[++i];
    }
#endif
    pSplash = new Splash;
    initIcons("");
    pMain = new MainWindow;
    ControlListener *cl = new ControlListener(pMain);
    if (!cl->bind(ctrlSock.c_str())){
        delete cl;
        if (ctrlSock.length())
            return 1;
    }
    scanUIN();
    unsigned startUIN = pSplash->LastUIN;
    if (!pSplash->SavePassword) startUIN = 0;
    if (startUIN){
        list<unsigned long>::iterator it;
        for (it = uins.begin(); it != uins.end(); ++it)
            if ((*it) == startUIN) break;
        if (it == uins.end()) startUIN = 0;
    }
    app.setMainWidget(pMain);
    if (startUIN){
        pClient->load(startUIN);
        if (pClient->EncryptedPassword.length() == 0) startUIN = 0;
    }
    if (startUIN && pSplash->NoShowLogin){
        pMain->init();
    }else{
        pLoginDlg = new LoginDialog;
        pSplash->hide();
        pLoginDlg->show();
    }
    pSplash->hide();
    return app.exec();
}

#ifdef USE_KDE
#include "sim.moc"
#endif


