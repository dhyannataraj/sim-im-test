/***************************************************************************
                          mainwin.cpp  -  description
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

#include "mainwin.h"
#include "client.h"
#include "cuser.h"
#include "toolbtn.h"
#include "userview.h"
#include "icons.h"
#include "dock.h"
#include "log.h"
#include "userbox.h"
#include "themes.h"
#include "xosd.h"
#include "logindlg.h"
#include "passwddlg.h"
#include "transparent.h"
#include "ui/searchdlg.h"
#include "ui/setupdlg.h"
#include "ui/autoreply.h"
#include "ui/alertmsg.h"
#include "ui/ballonmsg.h"
#include "ui/filetransfer.h"
#include "chatwnd.h"
#include "about.h"

#ifndef _WINDOWS
#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifdef WIN32
#include <windowsx.h>
#include <shellapi.h>
#include <mmsystem.h>
#include <shlobj.h>
#include <winreg.h>
#include "../idle/IdleTracker.h"
#endif

#include <errno.h>
#include <fcntl.h>
#include <time.h>

#include <fstream>

#include <qframe.h>
#include <qlayout.h>
#include <qtoolbar.h>
#include <qapplication.h>
#include <qdns.h>
#include <qfile.h>
#include <qstyle.h>
#include <qwidgetlist.h>
#include <qobjectlist.h>
#include <qtranslator.h>
#include <qregexp.h>
#include <qpopupmenu.h>

#ifdef USE_KDE
#include <kwin.h>
#include <kwinmodule.h>
#include <kpopupmenu.h>
#include <kaudioplayer.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kapp.h>
#include <kipc.h>
#include <kaboutapplication.h>
#include <kaboutkde.h>
#endif

#ifdef USE_SCRNSAVER
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/scrnsaver.h>
#endif

static char ICQ_CONF[] = "icq.conf";
static char SIM_CONF[] = "sim.conf";

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

static const char *sound(const char *vaw)
{
    string s = "sounds";
#ifdef WIN32
    s += "\\";
#else
    s += "/";
#endif
    s += vaw;
    return app_file(s.c_str());
}

MainWindow::MainWindow(const char *name)
        : QMainWindow(NULL, name, WType_TopLevel | WStyle_Customize | WStyle_Title | WStyle_NormalBorder| WStyle_SysMenu),
        Show(this, "Show", true),
        OnTop(this, "OnTop", true),
        ShowOffline(this, "ShowOffline", true),
        GroupMode(this, "GroupMode", true),
        mLeft(this, "Left"),
        mTop(this, "Top"),
        mWidth(this, "Width"),
        mHeight(this, "Height"),
        UseStyle(this, "Style"),
        AutoAwayTime(this, "AutoAwayTime", 300),
        AutoNATime(this, "AutoNATime", 900),
        ManualStatus(this, "ManualStatus", ICQ_STATUS_OFFLINE),
        DivPos(this, "DivPos"),
        SpellOnSend(this, "SpellOnSend", true),
        ToolbarDock(this, "ToolbarDock", "Top"),
        ToolbarOffset(this, "ToolbarOffset"),
        ToolbarY(this, "ToolbarY"),
        IncomingMessage(this, "IncomingMessage", sound("message.wav")),
        IncomingURL(this, "IncomingURL", sound("url.wav")),
        IncomingSMS(this, "IncomingSMS", sound("sms.wav")),
        IncomingAuth(this, "IncomingAuth", sound("auth.wav")),
        IncomingFile(this, "IncomingFile", sound("file.wav")),
        IncomingChat(this, "IncomingChat", sound("chat.wav")),
        FileDone(this, "FileDone", sound("filedone.wav")),
        OnlineAlert(this, "OnlineAlert", sound("alert.wav")),
        BirthdayReminder(this, "BirthdayReminder", sound("birthday.wav")),
        UrlViewer(this, "URLViewer",
#ifdef USE_KDE
                  "konqueror"
#else
                  "netscape"
#endif
                 ),
        MailClient(this, "MailClient",
#ifdef USE_KDE
                   "kmail"
#else
                   "netscape mailto:%s"
#endif
                  ),
        SoundPlayer(this, "SoundPlayer"),
        UseTransparent(this, "TransparentMain"),
        Transparent(this, "TransparencyMain", 80),
        UseTransparentContainer(this, "TransparentContainer"),
        TransparentContainer(this, "TransparencyContainer", 80),
        NoShowAway(this, "NoShowAway"),
        NoShowNA(this, "NoShowNA"),
        NoShowOccupied(this, "NoShowOccupied"),
        NoShowDND(this, "NoShowDND"),
        NoShowFFC(this, "NoShowFFC"),
        UseSystemFonts(this, "UseSystemFonts"),
        FontFamily(this, "FontFamily"),
        FontSize(this, "FontSize"),
        FontWeight(this, "FontWeight"),
        FontItalic(this, "FontItalic"),
        FontMenuFamily(this, "FontMenuFanily"),
        FontMenuSize(this, "FontMenuSize"),
        FontMenuWeight(this, "FontMenuWeight"),
        FontMenuItalic(this, "FontMenuItalic"),
        ColorSend(this, "ColorSend", 0x0000B0),
        ColorReceive(this, "ColorReceive", 0xB00000),
        ChatWidth(this, "ChatWidth"),
        ChatHeight(this, "Chatheight"),
        UserBoxX(this, "UserBoxX"),
        UserBoxY(this, "UserBoxY"),
        UserBoxWidth(this, "UserBoxWidth"),
        UserBoxHeight(this, "UserBoxHeight"),
        UserBoxToolbarDock(this, "UserBoxToolbarDock", "Top"),
        UserBoxToolbarOffset(this, "UserBoxToolbarOffset"),
        UserBoxToolbarY(this, "UserBoxToolbarY"),
        CloseAfterSend(this, "CloseAfterSend"),
        UserWindowInTaskManager(this, "UserWindowInTaskManager", true),
        Icons(this, "Icons"),
        XOSD_on(this, "XOSD_on", true),
        XOSD_pos(this, "XOSD_pos"),
        XOSD_offset(this, "XOSD_offset", 30),
        XOSD_color(this, "XOSD_color", 0x00E000),
        XOSD_FontFamily(this, "XOSD_FontFamily"),
        XOSD_FontSize(this, "XOSD_FontSize"),
        XOSD_FontWeight(this, "XOSD_FontWeight"),
        XOSD_FontItalic(this, "XOSD_FontItalic"),
        XOSD_timeout(this, "XOSD_timeout", 7),
        XOSD_Shadow(this, "XOSD_Shadow", true),
        XOSD_Background(this, "XOSD_Background", false),
        XOSD_BgColor(this, "XOSD_BgColor", colorGroup().background().rgb()),
        ContainerMode(this, "ContainerMode", ContainerModeGroup),
        MessageBgColor(this, "MessageBgColor"),
        MessageFgColor(this, "MessageFgColor"),
        SimpleMode(this, "SimpleMode"),
        UseOwnColors(this, "UseOwnColors")
{
    pMain = this;
    bQuit = false;
    dock = NULL;
    noToggle = false;
    bInLogin = false;
    lockFile = -1;
    translator = NULL;
    mAboutApp = NULL;

#ifdef HAVE_UMASK
    umask(0077);
#endif

    initTranslator();

#ifdef USE_KDE
    mAboutKDE = NULL;
#endif

    int tz;
#ifndef HAVE_TM_GMTOFF
    tz = _timezone / 2;
#else
    time_t now;
    time(&now);
    struct tm *tm = localtime(&now);
    tz = tm->tm_gmtoff;
    if (tm->tm_isdst) tz -= (60 * 60);
#endif
    realTZ = - tz / (30 * 60);

    menuUser = new KPopupMenu(this);
    menuGroup = new QPopupMenu(this);
    menuGroup->setCheckable(true);
    connect(menuGroup, SIGNAL(activated(int)), this, SLOT(moveUser(int)));
    connect(menuUser, SIGNAL(activated(int)), this, SLOT(userFunction(int)));

    m_autoStatus = ICQ_STATUS_OFFLINE;
    m_bAutoAway = m_bAutoNA = false;
    autoAwayTimer = new QTimer(this);
    connect(autoAwayTimer, SIGNAL(timeout()), this, SLOT(autoAway()));
    autoAwayTimer->start(10000);

    QDns resolver;
    resolver.setRecordType(QDns::A);
    resolver.setLabel("localhost");

    pClient = new Client(this);
    connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    connect(pClient, SIGNAL(messageReceived(ICQMessage*)), this, SLOT(messageReceived(ICQMessage*)));

    menuStatus = new QPopupMenu(this);
    menuStatus->setCheckable(true);
    connect(menuStatus, SIGNAL(activated(int)), this, SLOT(setStatus(int)));

    themes = new Themes(this);
    xosd = new XOSD(this);

    menuPhoneLocation = new QPopupMenu(this);
    connect(menuPhoneLocation, SIGNAL(activated(int)), this, SLOT(setPhoneLocation(int)));
    menuPhoneLocation->setCheckable(true);
    menuPhoneStatus = new QPopupMenu(this);
    connect(menuPhoneStatus, SIGNAL(activated(int)), this, SLOT(setPhoneStatus(int)));
    menuPhoneStatus->setCheckable(true);

    menuPhone = new QPopupMenu(this);
    menuPhone->insertItem(i18n("Current location"), menuPhoneLocation);
    menuPhone->insertItem(i18n("Phone status"), menuPhoneStatus);
    menuPhone->insertSeparator();
    menuPhone->insertItem(i18n("Phone Book"), this, SLOT(phonebook()));

    menuGroups = new QPopupMenu(this);

    menuFunction = new KPopupMenu(this);
    menuFunction->setCheckable(true);

    menuContainers = new QPopupMenu(this);
    connect(menuContainers, SIGNAL(activated(int)), this, SLOT(toContainer(int)));

    toolbar = new QToolBar(this);
    toolbar->setHorizontalStretchable(true);
    toolbar->setVerticalStretchable(true);

    btnShowOffline = new CToolButton(Icon("online_on"), Pict("online_off"), i18n("Show offline"), "",
                                     this, SLOT(toggleShowOffline()), toolbar);
    btnShowOffline->setToggleButton(true);

    btnGroupMode = new CToolButton(Icon("grp_off"), Pict("grp_on"), i18n("Group mode"), "",
                                   this, SLOT(toggleGroupMode()), toolbar);
    btnGroupMode->setToggleButton(true);

    toolbar->addSeparator();

    btnStatus = new PictButton(toolbar);
    btnStatus->setState(pClient->getStatusIcon(), pClient->getStatusText());
    btnStatus->setPopup(menuStatus);
    btnStatus->setPopupDelay(0);

    CToolButton *btnSetup = new CToolButton(toolbar);
    btnSetup->setTextLabel(i18n("Menu"));
    btnSetup->setOffIconSet(Icon("2downarrow"));
    btnSetup->setPopup(menuFunction);
    btnSetup->setPopupDelay(0);

    users = new UserView(this);
    setCentralWidget(users);

    connect(btnGroupMode, SIGNAL(showPopup(QPoint)), this, SLOT(showGroupPopup(QPoint)));
    connect(menuGroups, SIGNAL(activated(int)), users, SLOT(grpFunction(int)));

    searchDlg = NULL;
    setupDlg = NULL;

    bBlinkState = false;
    blinkTimer = new QTimer(this);
    connect(blinkTimer, SIGNAL(timeout()), this, SLOT(blink()));
    blinkTimer->start(800);
#ifdef WIN32
    IdleTrackerInit();
#endif
#ifdef USE_KDE
    connect(kapp, SIGNAL(iconChanged(int)), this, SLOT(changeIcons(int)));
    kapp->addKipcEventMask(KIPC::IconChanged);
#endif
    transparent = new TransparentTop(this, UseTransparent, Transparent);
    setOnTop();
    loadMenu();
}

void MainWindow::changeMode(bool bSimple)
{
    if (SimpleMode() == bSimple) return;
    SimpleMode = bSimple;
    emit modeChanged(bSimple);
}

void MainWindow::setOnTop()
{
    if (OnTop()){
        setWFlags(WStyle_StaysOnTop);
    }else{
        clearWFlags(WStyle_StaysOnTop);
    }
#ifdef WIN32
    menuFunction->setItemChecked(mnuOnTop, OnTop());
    SetWindowPos(winId(), (HWND)(OnTop() ? HWND_TOPMOST : HWND_NOTOPMOST),
                 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
#else
#ifdef USE_KDE
    if (OnTop()){
        KWin::setState(winId(), NET::StaysOnTop);
    }else{
        KWin::clearState(winId(), NET::StaysOnTop);
    }
    KWinModule *kwin = new KWinModule(this);
    connect(kwin, SIGNAL(currentDesktopChanged(int)),
            this, SLOT(currentDesktopChanged(int)));
#endif
#endif
}

void MainWindow::setBackgroundPixmap(const QPixmap &pm)
{
    if (transparent) transparent->updateBackground(pm);
}

void MainWindow::changeColors()
{
    emit colorsChanged();
}

void MainWindow::changeOwnColors(bool bUse)
{
    if (bUse == UseOwnColors()) return;
    UseOwnColors = bUse;
    emit ownColorsChanged();
}

void MainWindow::toggleOnTop()
{
    OnTop = !OnTop();
    setOnTop();
}

void MainWindow::showGroupPopup(QPoint p)
{
    menuGroups->setItemEnabled(mnuGrpCreate, pClient->m_state == ICQClient::Logged);
    menuGroups->setItemEnabled(mnuGrpCollapseAll, GroupMode());
    menuGroups->setItemEnabled(mnuGrpExpandAll, GroupMode());
    menuGroups->popup(p);
}

MainWindow::~MainWindow()
{
    transparent = NULL;
    while (containers.size()){
        delete containers.front();
    }
    for (list<UserFloat*>::iterator it = floating.begin(); it != floating.end(); ++it)
        delete *it;
#ifndef WIN32
    if (lockFile != -1) ::close(lockFile);
#endif
    if (dock) delete dock;
#ifdef WIN32
    IdleTrackerTerm();
#endif
    pMain = NULL;
}

void MainWindow::changeTransparent()
{
    emit transparentChanged();
}

void MainWindow::setPhoneLocation(int location)
{
    unsigned n;
    for (n = 0; n < menuPhoneLocation->count(); n++){
        if (menuPhoneLocation->idAt(n) == location)
            break;
    }
    for (PhoneBook::iterator it = pClient->Phones.begin(); it != pClient->Phones.end(); it++){
        PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
        phone->Active = false;
        if ((phone->Type() == PHONE) || (phone->Type() == MOBILE) || (phone->Type() == SMS)){
            if (n == 0) phone->Active = true;
            n--;
        }
    }
    pClient->updatePhoneBook();
}

void MainWindow::setPhoneStatus(int status)
{
    pClient->PhoneState = (unsigned short)status;
    pClient->updatePhoneStatus();
}

void MainWindow::showPopup(QPoint p)
{
    menuFunction->popup(p);
}

void MainWindow::addStatusItem(int status)
{
    menuStatus->insertItem(Icon(Client::getStatusIcon(status)), Client::getStatusText(status), status & 0xFF);
    setStatusItem(status);
}

void MainWindow::setStatusItem(int status)
{
    menuStatus->setItemChecked(status & 0xFF, (unsigned)(pClient->uStatus & 0xFF) == (unsigned)(status & 0xFF));
}

#ifdef WIN32

static bool makedir(char *p)
{
    char *r = strrchr(p, '\\');
    if (r == NULL) return true;
    *r = 0;
    CreateDirectoryA(p, NULL);
    *r = '\\';
    return true;
}

#else

static bool makedir(char *p)
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

void MainWindow::toggleGroupMode()
{
    setGroupMode(btnGroupMode->isOn());
}

void MainWindow::toggleShowOffline()
{
    setShowOffline(btnShowOffline->isOn());
}

void MainWindow::setGroupMode(bool bState)
{
    btnGroupMode->setOn(bState);
    users->setGroupMode(bState);
}

void MainWindow::setShowOffline(bool bState)
{
    btnShowOffline->setOn(bState);
    users->setShowOffline(bState);
}

void MainWindow::buildFileName(string &s, const char *name, bool bUseKDE, bool bCreate)
{
    s = homeDir;
    if (s.length() == 0){
#ifndef _WINDOWS
        struct passwd *pwd = getpwuid(getuid());
        if (pwd){
            s = pwd->pw_dir;
        }else{
            log(L_ERROR, "Can't get pwd");
        }
        if (s[s.size() - 1] != '/') s += '/';
#ifndef USE_KDE
        bUseKDE = false;
#endif
        if (bUseKDE){
            char *kdehome = getenv("KDEHOME");
            if (kdehome){
                s = kdehome;
            }else{
                s += ".kde/";
            }
            if (s.length() == 0) s += '/';
            if (s[s.length()-1] != '/') s += '/';
            s += "share/apps/sim/";
        }else{
            s += ".sim/";
        }
#else
        char szPath[MAX_PATH];
        if (SHGetSpecialFolderPathA(NULL, szPath, CSIDL_APPDATA, true)){
            s = szPath;
            if (s.length()  == 0) s = "c:\\";
            if (s[s.length() - 1] != '\\') s += '\\';
            s += "sim\\";
        }else{
            s = "c:\\";
        }
#endif
    }
#ifndef _WINDOWS
    if (s[s.length() - 1] != '/') s += '/';
#else
    if (s[s.length() - 1] != '\\') s += '\\';
#endif
    s += name;
    s += '\x00';
    if (bCreate) makedir((char*)s.c_str());
}

bool MainWindow::init()
{
    string file;
#ifndef WIN32
#ifdef USE_KDE
    string kdeDir;
    buildFileName(kdeDir, "", true, false);
    if (kdeDir.length()) kdeDir = kdeDir.substr(0, kdeDir.length()-1);
    struct stat st;
    if (stat(kdeDir.c_str(), &st) < 0){
        string mainDir;
        buildFileName(mainDir, "", false, false);
        if (mainDir.length()) mainDir = mainDir.substr(0, mainDir.length()-1);
        if (stat(mainDir.c_str(), &st) >= 0){
            if (rename(mainDir.c_str(), kdeDir.c_str()) < 0)
                log(L_WARN, "Rename error %s %s [%s]", mainDir.c_str(),
                    kdeDir.c_str(), strerror(errno));
        }
    }
#endif
    buildFileName(file, "lock");
    if ((lockFile = ::open(file.c_str(), O_RDWR | O_CREAT, 0600)) == -1){
        log(L_ERROR, "Can't open %s: %s", file.c_str(), strerror(errno));
        return false;
    }
    struct flock fl;
    fl.l_type   = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_len    = 1;
    fl.l_start  = 0;
    fl.l_pid    = getpid();
    if ((fcntl(lockFile, F_SETLK, &fl) == -1) && (errno != ENOLCK)){
        log(L_ERROR, "Can't lock %s: %s", file.c_str(), strerror(errno));
        return false;
    }
#endif

    string part;
    buildFileName(file, SIM_CONF);
    std::ifstream fs(file.c_str(), ios::in);
    load(fs, part);

    setDock(true);

    if (mLeft < 5) mLeft = 5;
    if (mTop < 5) mTop = 5;
    if (mLeft > QApplication::desktop()->width() - 5) mLeft = QApplication::desktop()->width() - 5;
    if (mTop > QApplication::desktop()->height() - 5) mTop = QApplication::desktop()->height() - 5;
    if (mWidth > QApplication::desktop()->width() - 5) mWidth = QApplication::desktop()->width() - 5;
    if (mHeight > QApplication::desktop()->height() - 5) mHeight = QApplication::desktop()->height() - 5;
    move(mLeft, mTop);
    if (mWidth() && mHeight()) resize(mWidth, mHeight);
    if (*Icons.c_str()) changeIcons(0);
    themes->setTheme(QString::fromLocal8Bit(UseStyle.c_str()));
    setFonts();
    {
        string file, part;
        buildFileName(file, ICQ_CONF);
        ifstream fs(file.c_str(), ios::in);
        pClient->load(fs, part);
    }
    for (;;){
        if (part.size() == 0) break;
        if (part == string("[Floaty]")){
            UserFloat *uFloat = new UserFloat;
            if (!uFloat->load(fs, part)){
                delete uFloat;
                continue;
            }
            floating.push_back(uFloat);
            uFloat->show();
#ifdef USE_KDE
            KWin::setOnAllDesktops(uFloat->winId(), true);
            KWin::setState(uFloat->winId(),  NET::SkipTaskbar | NET::StaysOnTop);
#endif
            continue;
        }
        if (part == string("[UserBox]")){
            UserBox *box = new UserBox;
            if (!box->load(fs, part)){
                delete box;
                continue;
            }
            containers.push_back(box);
            continue;
        }
        break;
    }
    menuStatus->setItemChecked(ICQ_STATUS_FxPRIVATE, pClient->inInvisible());

    if ((pClient->Uin == 0) || (*pClient->EncryptedPassword.c_str() == 0)){
        bInLogin = true;
        LoginDialog dlg;
        dlg.exec();
        bInLogin = false;
        ManualStatus = ICQ_STATUS_ONLINE;
    }
    if (pClient->Uin == 0)
        return false;

    ToolBarDock tDock = Top;
    if (ToolbarDock == "Minimized"){
        tDock = Minimized;
    }else if (ToolbarDock == "Bottom"){
        tDock = Bottom;
    }else if (ToolbarDock == "Left"){
        tDock = Left;
    }else if (ToolbarDock == "Right"){
        tDock = Right;
    }else if (ToolbarDock == "TornOff"){
        tDock = TornOff;
    }
    moveToolBar(toolbar, tDock, false, 0, ToolbarOffset);
    if (tDock == TornOff){
        toolbar->move(ToolbarOffset, ToolbarY);
        toolbar->show();
    }

    xosd->init();
    transparentChanged();
    setShow(Show());
    setOnTop();

    setShowOffline(ShowOffline);
    setGroupMode(GroupMode);

    realSetStatus();
    return true;
}

void MainWindow::messageReceived(ICQMessage *msg)
{
    ICQUser *u = pClient->getUser(msg->getUin());
    if (u == NULL) return;
    CUser user(u);
    xosd->setMessage(i18n("%1 from %2 received")
                     .arg(pClient->getMessageText(msg->Type()))
                     .arg(user.name()), u->Uin());
}

void MainWindow::processEvent(ICQEvent *e)
{
    switch (e->type()){
    case EVENT_AUTH_REQUIRED:
        userFunction(e->Uin(), mnuAuth, 0);
        return;
    case EVENT_USER_DELETED:{
            UserFloat *floaty = findFloating(e->Uin(), true);
            if (floaty == NULL) return;
            delete floaty;
            saveContacts();
            return;
        }
    case EVENT_MESSAGE_RECEIVED:{
            if (e->state == ICQEvent::Fail) return;
            ICQMessage *msg = e->message();
            if (msg == NULL) return;
            const char *wav;
            switch (msg->Type()){
            case ICQ_MSGxURL:
                wav = IncomingURL.c_str();
                break;
            case ICQ_MSGxSMS:
                wav = IncomingSMS.c_str();
                break;
            case ICQ_MSGxFILE:
                wav = IncomingFile.c_str();
                break;
            case ICQ_MSGxCHAT:
                wav = IncomingChat.c_str();
                break;
            case ICQ_MSGxAUTHxREQUEST:
            case ICQ_MSGxAUTHxREFUSED:
            case ICQ_MSGxAUTHxGRANTED:
            case ICQ_MSGxADDEDxTOxLIST:
                wav = IncomingAuth.c_str();
                break;
            default:
                wav = IncomingMessage.c_str();
            }
            if (((pClient->uStatus & 0xFF) != ICQ_STATUS_AWAY) && ((pClient->uStatus & 0xFF) != ICQ_STATUS_NA))
                playSound(wav);
            return;
        }
    case EVENT_BAD_PASSWORD:
        if (!bInLogin){
            PasswdDialog dlg;
            bInLogin = true;
            if (!dlg.exec()){
                quit();
                return;
            }
            bInLogin = false;
        }
        return;
    case EVENT_ANOTHER_LOCATION:
        setShow(true);
        BalloonMsg::message(i18n("Your UIN used from another location"), btnStatus);
        break;
    case EVENT_INFO_CHANGED:
        saveContacts();
        if ((e->Uin() == pClient->Uin()) || (e->Uin() == 0)){
            if ((realTZ != pClient->TimeZone()) && (e->subType() == EVENT_SUBTYPE_FULLINFO)){
                ICQUser u;
                u = *static_cast<ICQUser*>(pClient);
                u.TimeZone = realTZ;
                pClient->setInfo(&u);
            }
            menuPhoneLocation->clear();
            unsigned long n = 0;
            for (PhoneBook::iterator it = pClient->Phones.begin(); it != pClient->Phones.end(); it++){
                PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
                if ((phone->Type == PHONE) || (phone->Type == MOBILE) || (phone->Type == SMS)){
                    menuPhoneLocation->insertItem(phone->getNumber().c_str());
                    if (phone->Active)
                        menuPhoneLocation->setItemChecked(menuPhoneLocation->idAt(n), true);
                    n++;
                }
            }
            menuPhone->setItemEnabled(menuPhone->idAt(0), n != 0);
            menuPhoneStatus->setItemChecked(0, pClient->PhoneState() == 0);
            menuPhoneStatus->setItemChecked(1, pClient->PhoneState() == 1);
            menuPhoneStatus->setItemChecked(2, pClient->PhoneState() == 2);
        }
    case EVENT_STATUS_CHANGED:
        if ((e->Uin() == pClient->Uin) || (e->Uin() == 0)){
            ownerChanged();
            if (realTZ != pClient->TimeZone())
                pClient->addInfoRequest(pClient->Uin, true);
        }else{
            ICQUser *u = pClient->getUser(e->Uin());
            if (u && !u->inIgnore() && ((u->uStatus & 0xFF) == ICQ_STATUS_ONLINE) &&
                    ((u->prevStatus & 0xFF) != ICQ_STATUS_ONLINE) &&
                    ((u->prevStatus & 0xFF) != ICQ_STATUS_FREEFORCHAT) &&
                    (((pClient->uStatus & 0xFF) == ICQ_STATUS_ONLINE) ||
                     ((pClient->uStatus & 0xFF) == ICQ_STATUS_FREEFORCHAT)) &&
                    ((u->prevStatus == ICQ_STATUS_OFFLINE) || pClient->AlertAway()) &&
                    ((u->OnlineTime() > pClient->OnlineTime()) || (u->prevStatus  != ICQ_STATUS_OFFLINE))){
                if (!u->AlertOverride()) u = pClient;
                if (u->AlertSound()) playSound(OnlineAlert.c_str());
                if (u->AlertOnScreen()){
                    CUser user(e->Uin());
                    xosd->setMessage(i18n("User %1 is online") .arg(user.name()), e->Uin());
                }
                if (u->AlertPopup()){
                    AlertMsgDlg *dlg = new AlertMsgDlg(this, e->Uin());
                    dlg->show();
                    dlg->raise();
                }
                if (u->AlertWindow())
                    userFunction(e->Uin(), mnuAction);
            }
        }
        return;
    }
}

void MainWindow::saveState()
{
    if (m_bAutoAway || m_bAutoNA) ManualStatus = (unsigned long)m_autoStatus;
    ShowOffline = btnShowOffline->isOn();
    GroupMode = btnGroupMode->isOn();
    Show = isShow();
    mLeft = pos().x();
    mTop = pos().y();
    mWidth = size().width();
    mHeight = size().height();
    ToolBarDock tDock;
    int index;
    bool nl;
    int extraOffset;
    getLocation(toolbar, tDock, index, nl, extraOffset);
    ToolbarOffset = (short)extraOffset;
    switch (tDock){
    case Minimized:
        ToolbarDock = "Minimized";
        break;
    case Bottom:
        ToolbarDock = "Bottom";
        break;
    case Left:
        ToolbarDock = "Left";
        break;
    case Right:
        ToolbarDock = "Right";
        break;
    case TornOff:
        ToolbarDock = "TornOff";
        ToolbarOffset = toolbar->geometry().left();
        ToolbarY = toolbar->geometry().top();
        break;
    default:
        ToolbarDock = "Top";
    }
    OnTop = testWFlags(WStyle_StaysOnTop);
#ifdef USE_KDE
    OnTop = KWin::info(winId()).state & NET::StaysOnTop;
#endif
    UseStyle = themes->getTheme().local8Bit();
    string file;
    buildFileName(file, SIM_CONF);
#ifndef WIN32
    struct stat st;
    if ((stat(file.c_str(), &st) >= 0) && (st.st_mode != 0600))
        unlink(file.c_str());
#endif
    std::ofstream fs(file.c_str(), ios::out);
    save(fs);
    for (list<UserFloat*>::iterator itFloat = floating.begin(); itFloat != floating.end(); itFloat++){
        fs << "[Floaty]\n";
        (*itFloat)->save(fs);
    }
    for (list<UserBox*>::iterator itBox = containers.begin(); itBox != containers.end(); itBox++){
        fs << "[UserBox]\n";
        (*itBox)->save(fs);
    }
    saveContacts();
}

void MainWindow::saveContacts()
{
    if (pClient->Uin() == 0) return;
    string file;
    buildFileName(file, ICQ_CONF);
#ifndef WIN32
    struct stat st;
    if ((stat(file.c_str(), &st) >= 0) && (st.st_mode != 0600))
        unlink(file.c_str());
#endif
    ofstream fs(file.c_str(), ios::out);
    pClient->save(fs);
    fs.close();
}

void MainWindow::setDock(bool bUseDock)
{
    log(L_DEBUG, "Set dock %u", bUseDock);
    if (bUseDock){
        if (dock == NULL){
            dock = new DockWnd(this);
            connect(dock, SIGNAL(showPopup(QPoint)), this, SLOT(showPopup(QPoint)));
            connect(dock, SIGNAL(toggleWin()), this, SLOT(toggleShow()));
            connect(dock, SIGNAL(doubleClicked()), this, SLOT(dockDblClicked()));
        }
    }else{
        if (dock){
            delete dock;
            dock = NULL;
        }
        setShow(true);
    }
}

void MainWindow::dockDblClicked()
{
    for (list<UserBox*>::iterator itBox = containers.begin(); itBox != containers.end(); ++itBox){
        if (!(*itBox)->isActiveWindow()) continue;
        ICQUser *u = pClient->getUser((*itBox)->currentUser());
        if (u && u->unreadMsgs.size()){
            (*itBox)->showUser(u->Uin, mnuAction, 0);
            return;
        }
    }
    list<ICQUser*>::iterator it;
    for (it = pClient->contacts.users.begin(); it != pClient->contacts.users.end(); ++it){
        if ((*it)->unreadMsgs.size()){
            showUser((*it)->Uin, mnuAction, 0);
            return;
        }
    }
    setShow(true);
}

void MainWindow::destroyBox(UserBox *box)
{
    containers.remove(box);
}

void MainWindow::closeUser(unsigned long uin)
{
    for (list<UserBox*>::iterator it = containers.begin(); it != containers.end(); ++it)
        if ((*it)->closeUser(uin)) return;
}

void MainWindow::showUser(unsigned long uin, int function, unsigned long param)
{
    ICQUser *u = pClient->getUser(uin);
    if (uin && (u == NULL)) return;
    list<UserBox*>::iterator it;
    if (uin){
        for (it = containers.begin(); it != containers.end(); ++it){
            if (!(*it)->haveUser(uin)) continue;
            (*it)->showUser(uin, function, param);
            return;
        }
    }
    UserBox *box;
    unsigned long grpId = u ? u->GrpId() : 0;
    switch (ContainerMode()){
    case ContainerModeAll:
        for (it = containers.begin(); it != containers.end(); ++it){
            if ((*it)->GrpId != ContainerAllUsers) continue;
            (*it)->showUser(uin, function, param);
            return;
        }
        box = new UserBox(ContainerAllUsers);
        break;
    case ContainerModeGroup:
        for (it = containers.begin(); it != containers.end(); ++it){
            if ((*it)->GrpId != (unsigned long)grpId) continue;
            (*it)->showUser(uin, function, param);
            return;
        }
        box = new UserBox(grpId);
        break;
    default:
        grpId = 0x7FFFFFFF;
        for (it = containers.begin(); it != containers.end(); ++it){

            if ((*it)->GrpId < 0x10000L) continue;
            if ((*it)->GrpId < grpId) grpId = (*it)->GrpId - 1;
        }
        box = new UserBox(grpId);
    }
    containers.push_back(box);
    box->showUser(uin, function, param);
}

void MainWindow::setShow(bool bShow)
{
    log(L_DEBUG, "Set show %u", bShow);
    if (!bShow){
        hide();
        return;
    }
    show();
    showNormal();
#ifdef USE_KDE
    KWin::setOnDesktop(winId(), KWin::currentDesktop());
#endif
    setActiveWindow();
    raise();
#ifdef USE_KDE
    KWin::setActiveWindow(winId());
#endif
}

bool MainWindow::isShow()
{
    if (!isVisible()) return false;
#ifdef USE_KDE
    return (KWin::info(winId()).desktop == KWin::currentDesktop())
           || KWin::info(winId()).onAllDesktops;
#else
    return true;
#endif
}

void MainWindow::toggleShow()
{
    if (noToggle) return;
    setShow(!isShow());
    noToggle = true;
    QTimer::singleShot(1000, this, SLOT(setToggle()));
}

void MainWindow::setToggle()
{
    noToggle = false;
}

void MainWindow::setup()
{
    if (setupDlg == NULL)
        setupDlg = new SetupDialog(this, 0);
    emit setupInit();
    setupDlg->show();
#ifdef USE_KDE
    KWin::setOnDesktop(setupDlg->winId(), KWin::currentDesktop());
#endif
    setupDlg->raise();
#ifdef USE_KDE
    KWin::setActiveWindow(setupDlg->winId());
#endif

}

void MainWindow::phonebook()
{
    if (setupDlg == NULL)
        setupDlg = new SetupDialog(this, 108);
    setupDlg->show();
#ifdef USE_KDE
    KWin::setOnDesktop(setupDlg->winId(), KWin::currentDesktop());
#endif
    setupDlg->raise();
#ifdef USE_KDE
    KWin::setActiveWindow(setupDlg->winId());
#endif

}

void MainWindow::quit()
{
    bQuit = true;
    saveState();
    close();
}

void MainWindow::search()
{
    if (searchDlg == NULL) searchDlg = new SearchDialog(NULL);
    searchDlg->show();
    searchDlg->raise();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (!bQuit && (dock != NULL)){
        e->ignore();
        hide();
        return;
    }
    pClient->setStatus(ICQ_STATUS_OFFLINE);
    saveState();
    if (dock){
        delete dock;
        dock = NULL;
    }
    QMainWindow::closeEvent(e);
}

void MainWindow::autoAway()
{
#ifdef WIN32
    unsigned long idle_time = (GetTickCount() - IdleTrackerGetLastTickCount()) / 1000;
#endif
#ifdef USE_SCRNSAVER
    static XScreenSaverInfo *mit_info = NULL;
    if (mit_info == NULL) {
        int event_base, error_base;
        if(XScreenSaverQueryExtension(x11Display(), &event_base, &error_base)) {
            mit_info = XScreenSaverAllocInfo ();
        }
    }
    if (mit_info == NULL){
        log(L_WARN, "No XScreenSaver extension found on current XServer, disabling auto-away.");
        autoAwayTimer->stop();
        return;
    }
    if (!XScreenSaverQueryInfo(x11Display(), qt_xrootwin(), mit_info)) {
        log(L_WARN, "XScreenSaverQueryInfo failed, disabling auto-away.");
        autoAwayTimer->stop();
        return;
    }
    unsigned long idle_time = (mit_info->idle / 1000);
#endif
#if defined(USE_SCRNSAVER) || defined(WIN32)
    if ((m_bAutoAway && (idle_time < AutoAwayTime)) ||
            (m_bAutoNA && (idle_time < AutoNATime))){
        m_bAutoAway = m_bAutoNA = false;
        ManualStatus = (unsigned long)m_autoStatus;
        realSetStatus();
        return;
    }
    if (!m_bAutoNA && AutoNATime() && (idle_time > AutoNATime())){
        m_bAutoNA = true;
        if (ManualStatus == ICQ_STATUS_OFFLINE) return;
        if (!m_bAutoAway){
            m_autoStatus = ManualStatus;
            m_bAutoAway = true;
        }
        ManualStatus = ICQ_STATUS_NA;
        realSetStatus();
        return;
    }
    if (!m_bAutoAway && AutoAwayTime() && (idle_time > AutoAwayTime())){
        m_bAutoAway = true;
        if (ManualStatus == ICQ_STATUS_OFFLINE) return;
        if (ManualStatus == ICQ_STATUS_NA) return;
        m_autoStatus = ManualStatus;
        ManualStatus = ICQ_STATUS_AWAY;
        realSetStatus();
        return;
    }
#endif
}

void MainWindow::setStatus(int status)
{
    if ((unsigned long)status == ICQ_STATUS_FxPRIVATE){
        pClient->setInvisible(!pClient->inInvisible());
        menuStatus->setItemChecked(ICQ_STATUS_FxPRIVATE, pClient->inInvisible());
        return;
    }
    AutoReplyDlg *autoDlg = NULL;
    switch (status & 0xFF){
    case ICQ_STATUS_AWAY:
        if (NoShowAway()) break;
        autoDlg = new AutoReplyDlg(this, ICQ_STATUS_AWAY);
        break;
    case ICQ_STATUS_NA:
        if (NoShowNA()) break;
        autoDlg = new AutoReplyDlg(this, ICQ_STATUS_NA);
        break;
    case ICQ_STATUS_OCCUPIED:
        if (NoShowOccupied()) break;
        autoDlg = new AutoReplyDlg(this, ICQ_STATUS_OCCUPIED);
        break;
    case ICQ_STATUS_DND:
        if (NoShowDND()) break;
        autoDlg = new AutoReplyDlg(this, ICQ_STATUS_DND);
        break;
    case ICQ_STATUS_FREEFORCHAT:
        if (NoShowFFC()) break;
        autoDlg = new AutoReplyDlg(this, ICQ_STATUS_FREEFORCHAT);
        break;
    }
    if (autoDlg){
        autoDlg->show();
        autoDlg->raise();
    }
    if (status == (ICQ_STATUS_OFFLINE & 0xFF)) status = ICQ_STATUS_OFFLINE;
    ManualStatus = (unsigned long)status;
    QTimer::singleShot(800, this, SLOT(realSetStatus()));
}

void MainWindow::realSetStatus()
{
    pClient->setStatus(ManualStatus);
}

void MainWindow::moveUser(int grp)
{
    ICQUser *u = pClient->getUser(m_uin);
    if (u == NULL) return;
    switch (grp){
    case mnuGroupVisible:
        pClient->setInVisible(u, !u->inVisible());
        break;
    case mnuGroupInvisible:
        pClient->setInInvisible(u, !u->inInvisible());
        break;
    case mnuGroupIgnore:
        if (!u->inIgnore()){
            CUser user(u);
            QStringList btns;
            btns.append(i18n("&Yes"));
            btns.append(i18n("&No"));
            BalloonMsg *msg = new BalloonMsg(i18n("Add user %1 to ignore list ?") .arg(user.name()),
                                             m_rc, btns, this);
            connect(msg, SIGNAL(action(int)), this, SLOT(ignoreUser(int)));
            msg->show();
            break;
        }
        pClient->setInIgnore(u, !u->inIgnore());
        break;
    default:
        ICQGroup *g = pClient->getGroup(grp);
        if (grp) pClient->moveUser(u, g);
    }
}

void MainWindow::setIcons()
{
    QPixmap icon = Pict(pClient->getStatusIcon());
    switch (pClient->m_state){
    case ICQClient::Logoff:
    case ICQClient::Logged:
        btnStatus->setState(pClient->getStatusIcon(), pClient->getStatusText());
        break;
    default:
        btnStatus->setState(Client::getStatusIcon(bBlinkState ? ICQ_STATUS_OFFLINE : ICQ_STATUS_ONLINE), i18n("Connecting"));
        icon = Pict(Client::getStatusIcon(bBlinkState ? ICQ_STATUS_OFFLINE : ICQ_STATUS_ONLINE));
    }
    setIcon(icon);
}

void MainWindow::blink()
{
    if ((pClient->m_state == ICQClient::Logoff) || (pClient->m_state == ICQClient::Logged)) return;
    bBlinkState = !bBlinkState;
    setIcons();
}

void MainWindow::ownerChanged()
{
    setIcons();
    setStatusItem(ICQ_STATUS_ONLINE);
    setStatusItem(ICQ_STATUS_AWAY);
    setStatusItem(ICQ_STATUS_NA);
    setStatusItem(ICQ_STATUS_DND);
    setStatusItem(ICQ_STATUS_OCCUPIED);
    setStatusItem(ICQ_STATUS_FREEFORCHAT);
    setStatusItem(ICQ_STATUS_OFFLINE);
    CUser owner(pClient);
    setCaption(owner.name());
#ifdef USE_KDE
    menuFunction->changeTitle(1, owner.name());
#endif
}

void MainWindow::adjustGroupMenu(QPopupMenu *menuGroup, unsigned long uin)
{
    menuGroup->clear();
    ICQUser *u = pClient->getUser(uin);
    if (u == NULL) return;
    vector<ICQGroup*>::iterator it;
    for (it = pClient->contacts.groups.begin(); it != pClient->contacts.groups.end(); it++){
        CGroup grp(*it);
        menuGroup->insertItem(grp.name(), (*it)->Id);
        if ((*it)->Id == u->GrpId) menuGroup->setItemChecked((*it)->Id, true);
    }
    menuGroup->insertSeparator();
    if (u->Type == USER_TYPE_ICQ){
        menuGroup->insertItem(i18n("In visible list"), mnuGroupVisible);
        menuGroup->setItemChecked(mnuGroupVisible, u->inVisible());
        menuGroup->insertItem(i18n("In invisible list"), mnuGroupInvisible);
        menuGroup->setItemChecked(mnuGroupInvisible, u->inInvisible());
    }
    menuGroup->insertItem(i18n("In ignore list"), mnuGroupIgnore);
    menuGroup->setItemChecked(mnuGroupIgnore, u->inIgnore());
}

void MainWindow::showUserPopup(unsigned long uin, QPoint p, QPopupMenu *popup, const QRect &rc)
{
    m_rc  = rc;
    m_uin = uin;
    ICQUser *u = pClient->getUser(uin);
    if (u == NULL) return;
    adjustGroupMenu(menuGroup, uin);
    menuUser->clear();
#ifdef USE_KDE
    CUser user(u);
    menuUser->insertTitle(user.name());
#endif
    menuUser->insertSeparator();
    menuUser->insertItem(i18n("Groups"), menuGroup, mnuGroups);
    menuUser->insertItem(Pict("remove"), i18n("Delete"), mnuDelete);
    menuUser->setItemEnabled(mnuGroups, (pClient->m_state == ICQClient::Logged) || (u->Type() != USER_TYPE_ICQ));
    menuUser->setItemEnabled(mnuDelete, (pClient->m_state == ICQClient::Logged) || (u->Type() != USER_TYPE_ICQ) || (u->GrpId() == 0));
    menuUser->insertSeparator();
    menuUser->insertItem(Icon("info"), i18n("User info"), mnuInfo);
    menuUser->insertItem(Icon("history"), i18n("History"), mnuHistory);
    menuUser->insertSeparator();
    menuUser->insertItem(Icon("floating"),
                         findFloating(uin) ? i18n("Floating off") : i18n("Floating on"), mnuFloating);
    if (popup){
        menuUser->insertSeparator();
        menuUser->insertItem(i18n("Users"), popup);
        menuUser->insertItem(Icon("exit"), i18n("Close"), mnuClose);
    }
    adjustUserMenu(menuUser, u, true);
    menuUser->popup(p);
}

void MainWindow::addMessageType(QPopupMenu *menuUser, int type, int id, bool bAdd,
#ifdef USE_KDE
                                bool bHaveTitle
#else
                                bool
#endif
                               )
{
    if (bAdd){
        int pos = 0;
#ifdef USE_KDE
        if (bHaveTitle) pos = 1;
#endif
        if (menuUser->findItem(id)){
            if (menuUser->indexOf(id) == pos) return;
            menuUser->removeItem(id);
        }
        menuUser->insertItem(Icon(Client::getMessageIcon(type)), Client::getMessageText(type), id, pos);
        return;
    }
    if (menuUser->findItem(id) == NULL) return;
    menuUser->removeItem(id);
}

void MainWindow::userFunction(int function)
{
    userFunction(m_uin, function);
}

void MainWindow::deleteUser(int n)
{
    if (n) return;
    ICQUser *u = pClient->getUser(m_uin);
    if (u) pClient->deleteUser(u);
}

void MainWindow::ignoreUser(int n)
{
    if (n) return;
    ICQUser *u = pClient->getUser(m_uin);
    if (u) pClient->setInIgnore(u, true);
}

void MainWindow::currentDesktopChanged(int)
{
#ifdef USE_KDE
    list<UserFloat*>::iterator it;
    for (it = floating.begin(); it != floating.end(); it++){
        KWin::setOnAllDesktops((*it)->winId(), true);
        KWin::setState((*it)->winId(),
                       /* NET::SkipTaskbar | */ NET::StaysOnTop);
    }
#endif
}

void MainWindow::userFunction(unsigned long uin, int function, unsigned long param)
{
    switch (function){
    case mnuFloating:
        {
            UserFloat *uFloat = findFloating(uin, true);
            if (uFloat){
                delete uFloat;
            }else{
                uFloat = new UserFloat;
                if (!uFloat->setUin(m_uin)){
                    delete uFloat;
                    return;
                }
                floating.push_back(uFloat);
                uFloat->show();
#ifdef USE_KDE
                KWin::setOnAllDesktops(uFloat->winId(), true);
                KWin::setState(uFloat->winId(),
                               NET::SkipTaskbar | NET::StaysOnTop | NET::Sticky);
#endif
            }
            return;
        }
    case mnuClose:
        closeUser(uin);
        return;
    case mnuAction: {
            ICQUser *u = pClient->getUser(uin);
            if (u && (u->unreadMsgs.size() == 0) && (u->Type == USER_TYPE_EXT)){
                PhoneBook::iterator it;
                for (it = u->Phones.begin(); it != u->Phones.end(); ++it){
                    PhoneInfo *info = static_cast<PhoneInfo*>(*it);
                    if (info->Type == SMS) break;
                }
                if (it == u->Phones.end()){
                    if (u->EMails.size())
                        pMain->sendMail(uin);
                    return;
                }
            }
            showUser(uin, mnuAction, param);
            return;
        }
    case mnuChat:{
            QWidget *chat = chatWindow(uin);
            if (chat){
                chat->show();
                chat->showNormal();
#ifdef USE_KDE
                KWin::setOnDesktop(chat->winId(), KWin::currentDesktop());
#endif
                chat->setActiveWindow();
                chat->raise();
#ifdef USE_KDE
                KWin::setActiveWindow(chat->winId());
#endif
                return;
            }
            showUser(uin, function, param);
            return;
        }
    case mnuMessage:
    case mnuURL:
    case mnuSMS:
    case mnuFile:
    case mnuAuth:
    case mnuContacts:
    case mnuHistory:
    case mnuInfo:
        showUser(uin, function, param);
        return;
    case mnuDelete:
        {
            ICQUser *u = pClient->getUser(uin);
            if (u){
                CUser user(u);
                m_uin = uin;
                QStringList btns;
                btns.append(i18n("&Yes"));
                btns.append(i18n("&No"));
                BalloonMsg *msg = new BalloonMsg(i18n("Delete user %1?") .arg(user.name()),
                                                 m_rc, btns, this);
                connect(msg, SIGNAL(action(int)), this, SLOT(deleteUser(int)));
                msg->show();
            }
            return;
        }
    case mnuMail:
        sendMail(uin);
        return;
    default:
        log(L_WARN, "No user function %u for %lu", function, uin);
    }
}

UserFloat *MainWindow::findFloating(unsigned long uin, bool bDelete)
{
    list<UserFloat*>::iterator it;
    for (it = floating.begin(); it != floating.end(); it++){
        if ((*it)->Uin == uin){
            UserFloat *res = *it;
            if (bDelete) floating.remove(res);
            return res;
        }
    }
    return NULL;
}

void MainWindow::toContainer(int containerId)
{
    list<UserBox*>::iterator it;
    for (it = containers.begin(); it != containers.end(); ++it){
        if (!(*it)->haveUser(uinMenu)) continue;
        if ((*it)->GrpId == (unsigned long)containerId) return;
        (*it)->closeUser(uinMenu);
    }
    for (it = containers.begin(); it != containers.end(); ++it)
        if ((*it)->GrpId == (unsigned long)containerId) break;
    UserBox *box = NULL;
    if (it != containers.end()) box = *it;
    if (box == NULL){
        box = new UserBox(containerId);
        containers.push_back(box);
    }
    box->showUser(uinMenu, mnuAction, 0);
}

void MainWindow::adjustUserMenu(QPopupMenu *menu, ICQUser *u, bool haveTitle)
{
    if (haveTitle){
        uinMenu = u->Uin();
        menuContainers->clear();
        bool canNew = true;
        list<UserBox*>::iterator it;
        for (it = containers.begin(); it != containers.end(); ++it){
            menuContainers->insertItem((*it)->containerName(), (*it)->GrpId());
            if (!(*it)->haveUser(uinMenu)) continue;
            menuContainers->setItemChecked((*it)->GrpId, true);
            if (((*it)->GrpId >= 0x10000L) && ((*it)->count() == 1))
                canNew = false;
        }
        if (canNew){
            unsigned long newId = 0x7FFFFFFF;
            for (it = containers.begin(); it != containers.end(); ++it){
                if ((*it)->GrpId < 0x10000L) continue;
                if ((*it)->GrpId < newId) newId = (*it)->GrpId - 1;
            }
            menuContainers->insertItem(i18n("New"), newId);
        }
        if (menu->findItem(mnuContainers) == NULL)
            menu->insertItem(i18n("To container"), menuContainers, mnuContainers);
    }
    bool haveEmail = false;
    for (EMailList::iterator it = u->EMails.begin(); it != u->EMails.end(); ++it){
        EMailInfo *mailInfo = static_cast<EMailInfo*>(*it);
        if (*mailInfo->Email.c_str()){
            haveEmail = true;
            break;
        }
    }
    addMessageType(menu, ICQ_MSGxMAIL, mnuMail, haveEmail, haveTitle);
    addMessageType(menu, ICQ_MSGxAUTHxREQUEST, mnuAuth, u->WaitAuth, haveTitle);
    addMessageType(menu, ICQ_MSGxCHAT, mnuChat, (u->Type() == USER_TYPE_ICQ) && u->isOnline(), haveTitle);
    addMessageType(menu, ICQ_MSGxFILE, mnuFile, (u->Type() == USER_TYPE_ICQ) && u->isOnline(), haveTitle);
    addMessageType(menu, ICQ_MSGxCONTACTxLIST, mnuContacts, u->Type() == USER_TYPE_ICQ, haveTitle);
    bool havePhone = (u->Type() == USER_TYPE_ICQ);
    if (!havePhone){
        for (PhoneBook::iterator it = u->Phones.begin(); it != u->Phones.end(); ++it){
            PhoneInfo *info = static_cast<PhoneInfo*>(*it);
            if (info->Type == SMS){
                havePhone = true;
                break;
            }
        }
    }
    addMessageType(menu, ICQ_MSGxSMS, mnuSMS, havePhone, haveTitle);
    addMessageType(menu, ICQ_MSGxURL, mnuURL, u->Type() == USER_TYPE_ICQ, haveTitle);
    addMessageType(menu, ICQ_MSGxMSG, mnuMessage, u->Type() == USER_TYPE_ICQ, haveTitle);
}

void MainWindow::goURL(const char *url)
{
#ifdef WIN32
    ShellExecuteA(winId(), NULL, url, NULL, NULL, SW_SHOWNORMAL);
#else
    exec(UrlViewer.c_str(), url);
#endif
}

void MainWindow::sendMail(unsigned long uin)
{
    const char *mail = NULL;
    ICQUser *u = pClient->getUser(uin);
    if (u){
        for (EMailList::iterator it = u->EMails.begin(); it != u->EMails.end(); ++it){
            EMailInfo *mailInfo = static_cast<EMailInfo*>(*it);
            if (*mailInfo->Email.c_str()){
                mail = mailInfo->Email.c_str();
                break;
            }
        }
    }
    sendMail(mail);
}

void MainWindow::sendMail(const char *mail)
{
    if (mail == NULL) return;

#ifdef WIN32
    string s = "mailto:";
    s += mail;
    ShellExecuteA(winId(), NULL, s.c_str(), NULL, NULL, SW_SHOWNORMAL);
#else
    exec(MailClient.c_str(), mail);
#endif
}

void MainWindow::exec(const char *prg, const char *arg)
{
#ifndef WIN32
    if (*prg == 0) return;
    QString p = QString::fromLocal8Bit(prg);
    if (p.find("%s") >= 0){
        p.replace(QRegExp("%s"), arg);
    }else{
        p += " ";
        p += QString::fromLocal8Bit(arg);
    }
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
        if (execvp(arglist[0], arglist))
            log(L_DEBUG, "can't execute %s: %s", arglist[0], strerror(errno));
        _exit(-1);
    }
    for (char **p = arglist; *p != NULL; p++)
        free(*p);
    delete[] arglist;
#endif
}

void MainWindow::playSound(const char *wav)
{
#ifdef WIN32
    sndPlaySoundA(wav, SND_ASYNC | SND_NODEFAULT);
#else
#ifdef USE_KDE
    KAudioPlayer::play(wav);
#else
    if (*(SoundPlayer.c_str()) == 0) return;

    const char *arglist[3];
    arglist[0] = SoundPlayer.c_str();
    arglist[1] = wav;
    arglist[2] = NULL;

    if(!fork()) {
        execvp(arglist[0], (char**)arglist);
        _exit(-1);
    }
#endif
#endif
}

void MainWindow::addNonIM()
{
    showUser(0, mnuInfo);
}

void MainWindow::setFonts()
{
#ifdef USE_KDE
    if (UseSystemFonts()) return;
#endif
    if (FontSize() > 128) FontSize = 0;
    if (FontSize()){
        QFont fontWnd(FontFamily.c_str(), FontSize(), FontWeight(), FontItalic());
        qApp->setFont(fontWnd, true);
    }
    if (FontMenuSize() > 128) FontMenuSize = 0;
    if (FontMenuSize()){
        QFont fontMenu(FontMenuFamily.c_str(), FontMenuSize(), FontMenuWeight(), FontMenuItalic());
        qApp->setFont(fontMenu, true, "QPopupMenu");
    }
}

extern KAboutData *appAboutData;

void MainWindow::bug_report()
{
    goURL("http://sourceforge.net/tracker/?group_id=56866");
}

void MainWindow::about()
{
    if( mAboutApp == 0 )
    {
        mAboutApp = new KAboutApplication( appAboutData, this, "about", false );
        connect( mAboutApp, SIGNAL(finished()), this, SLOT( dialogFinished()) );
    }
    mAboutApp->show();
}

void MainWindow::about_kde()
{
#ifdef USE_KDE
    if( mAboutKDE == 0 )
    {
        mAboutKDE = new KAboutKDE( this, "aboutkde", false );
        connect( mAboutKDE, SIGNAL(finished()), this, SLOT( dialogFinished()) );
    }
    mAboutKDE->show();
#endif
}

void MainWindow::dialogFinished()
{
    QTimer::singleShot( 0, this, SLOT(timerExpired()) );
}


void MainWindow::timerExpired()
{
#ifdef USE_KDE
    if( mAboutKDE != 0 && mAboutKDE->isVisible() == false )
    {
        delete mAboutKDE; mAboutKDE = NULL;
    }
#endif
    if( mAboutApp != 0 && mAboutApp->isVisible() == false )
    {
        delete mAboutApp; mAboutApp = NULL;
    }
}

void MainWindow::changeIcons(int)
{
    initIcons(Icons.c_str());
    loadMenu();
    setIcons();
    emit iconChanged();
}

void MainWindow::loadMenu()
{
    menuStatus->clear();
    addStatusItem(ICQ_STATUS_ONLINE);
    addStatusItem(ICQ_STATUS_AWAY);
    addStatusItem(ICQ_STATUS_NA);
    addStatusItem(ICQ_STATUS_DND);
    addStatusItem(ICQ_STATUS_OCCUPIED);
    addStatusItem(ICQ_STATUS_FREEFORCHAT);
    addStatusItem(ICQ_STATUS_OFFLINE);
    menuStatus->insertSeparator();
    menuStatus->insertItem(Icon("invisible"), i18n("Invisible"), ICQ_STATUS_FxPRIVATE);

    menuPhoneStatus->clear();
    menuPhoneStatus->insertItem(i18n("Don't show"), 0);
    menuPhoneStatus->insertItem(Icon("phone"), i18n("Available"), 1);
    menuPhoneStatus->insertItem(Icon("nophone"), i18n("Busy"), 2);

    menuGroups->clear();
    menuGroups->insertItem(Pict("grp_create"), i18n("Create group"), mnuGrpCreate);
    menuGroups->insertSeparator();
    menuGroups->insertItem(Pict("grp_expand"), i18n("Expand all"), mnuGrpExpandAll);
    menuGroups->insertItem(Pict("grp_collapse"), i18n("Collapse all"), mnuGrpCollapseAll);

    menuFunction->clear();
#ifdef USE_KDE
    menuFunction->insertTitle(i18n("ICQ"), 1);
#endif
    menuFunction->setCheckable(true);
    menuFunction->insertItem(Icon("find"), i18n("Find User"), this, SLOT(search()));
    menuFunction->insertItem(Icon("nonim"), i18n("Add Non-IM contact"), this, SLOT(addNonIM()));
    menuFunction->insertSeparator();
    menuFunction->insertItem(i18n("Status"), menuStatus);
    menuFunction->insertSeparator();
    menuFunction->insertItem(Icon("configure"), i18n("Setup"), this, SLOT(setup()));
    menuFunction->insertItem(Icon("grp_on"), i18n("Groups"), menuGroups);
    menuFunction->insertItem(Icon("phone"), i18n("Phone \"Follow Me\""), menuPhone);
#ifdef WIN32
    menuFunction->insertSeparator();
    menuFunction->insertItem(i18n("Always on top"), this, SLOT(toggleOnTop()), 0, mnuOnTop);
#endif
    menuFunction->insertSeparator();
    menuFunction->insertItem(i18n("&Bug report"), this, SLOT(bug_report()));
    menuFunction->insertItem(Icon("licq"), i18n("&About SIM"), this, SLOT(about()));
#ifdef USE_KDE
    menuFunction->insertItem(Icon("about_kde"), i18n("About &KDE"), this, SLOT(about_kde()));
#endif
    menuFunction->insertSeparator();
    menuFunction->insertItem(Icon("exit"), i18n("Quit"), this, SLOT(quit()));
}

void MainWindow::changeWm()
{
    emit wmChanged();
}

QWidget *MainWindow::chatWindow(unsigned long uin)
{
    QWidget *res = NULL;
    QWidgetList *list = QApplication::topLevelWidgets();
    QWidgetListIt it(*list);
    QWidget *w;
    while ( (w=it.current()) != NULL) {
        ++it;
        if (w->inherits("ChatWindow")){
            ChatWindow *chat = static_cast<ChatWindow*>(w);
            if (chat->chat && (chat->chat->getUin() == uin))
                res = chat;
        }else{
            QObjectList *l = w->queryList("ChatWindow");
            QObjectListIt it(*l);
            QObject *obj;
            while ((obj=it.current()) != NULL){
                ++it;
                ChatWindow *chat = static_cast<ChatWindow*>(obj);
                if (chat->chat && (chat->chat->getUin() == uin))
                    res = chat;
            }
            delete l;
        }
    }
    delete list;
    return res;
}

void MainWindow::chatClose()
{
    emit chatChanged();
}

QWidget *MainWindow::ftWindow(unsigned long uin, const string &fileName)
{
    QWidget *res = NULL;
    QWidgetList *list = QApplication::topLevelWidgets();
    QWidgetListIt it(*list);
    QWidget *w;
    while ( (w=it.current()) != NULL) {
        ++it;
        if (w->inherits("FileTransferDlg")){
            FileTransferDlg *ft = static_cast<FileTransferDlg*>(w);
            if (ft->file && (ft->file->getUin() == uin) && (ft->file->shortName() == fileName))
                res = ft;
        }else{
            QObjectList *l = w->queryList("FileTransferDlg");
            QObjectListIt it(*l);
            QObject *obj;
            while ((obj=it.current()) != NULL){
                ++it;
                FileTransferDlg *ft = static_cast<FileTransferDlg*>(obj);
                if (ft->file && (ft->file->getUin() == uin) && (ft->file->shortName() == fileName))
                    res = ft;
            }
            delete l;
        }
    }
    delete list;
    return res;
}

void MainWindow::ftClose()
{
    emit ftChanged();
}

void MainWindow::initTranslator()
{
    if (translator)
        qApp->removeTranslator(translator);
    translator = NULL;
    string lang;
#ifdef WIN32
    char buff[256];
    int res = GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SABBREVLANGNAME, buff, sizeof(buff));
    if (res){
        lang += tolower(buff[0]);
        lang += tolower(buff[1]);
    }
#else
    char *p = getenv("LANG");
    if (p){
        for (; *p; p++){
            if (*p == '.') break;
            lang += *p;
        }
    }
#endif
    if (lang.size() == 0) return;
    string s = "po";
#ifdef WIN32
    s += "\\";
#else
    s += "/";
#endif
    for (const char *p = lang.c_str(); *p; p++)
        s += tolower(*p);
    s += ".qm";
    QFile f(QString::fromLocal8Bit(app_file(s.c_str())));
    if (!f.exists()) return;
    translator = new QTranslator(this);
    translator->load(f.name());
    qApp->installTranslator(translator);
    appAboutData->setTranslator(I18N_NOOP("_: NAME OF TRANSLATORS\nYour names"),
                                I18N_NOOP("_: EMAIL OF TRANSLATORS\nYour emails"));
}

#ifndef _WINDOWS
#include "mainwin.moc"
#endif

