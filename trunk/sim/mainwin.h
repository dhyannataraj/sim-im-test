/***************************************************************************
                          mainwin.h  -  description
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

#ifndef _MAINWIN_H
#define _MAINWIN_H

#include "defs.h"

#include <fstream>

#include <qmainwindow.h>
#include <qtimer.h>

#include "cfg.h"

class Skin;
class DockWnd;
class KPopupMenu;
class UserView;
class CToolButton;
class PictButton;
class QTimer;
class UserFloat;
class UserBox;
class QPopupMenu;
class QToolBar;
class Themes;
class SearchDialog;
class SetupDialog;
class ICQMessage;
class ICQUser;
class TransparentTop;
class QTranslator;
class QTextCodec;

const int mnuAction = 0;
const int mnuMessage = 1;
const int mnuURL = 2;
const int mnuSMS = 3;
const int mnuFile = 4;
const int mnuContacts = 5;
const int mnuAuth = 6;
const int mnuChat = 7;
const int mnuFloating = 10;
const int mnuClose = 11;
const int mnuInfo = 12;
const int mnuHistory = 13;
const int mnuSearch = 14;
const int mnuDelete = 15;
const int mnuGroups = 16;
const int mnuMail = 17;
const int mnuContainers = 18;
const int mnuActionInt = 19;
const int mnuGrpCreate = 21;
const int mnuGrpCollapseAll = 22;
const int mnuGrpExpandAll = 23;
const int mnuGrpRename = 24;
const int mnuGrpDelete = 25;
const int mnuGrpUp = 26;
const int mnuGrpDown = 27;
const int mnuAbout = 30;
const int mnuAboutKDE = 31;
const int mnuBugSend = 32;
const int mnuAlert = 33;
const int mnuAccept = 34;
const int mnuSound = 35;
const int mnuSecureOn = 36;
const int mnuSecureOff = 37;
const int mnuAutoResponse = 38;
const int mnuGrpTitle = 0x10000;
const int mnuPopupStatus = 0x20000;

const unsigned long mnuGroupVisible = 0x10001;
const unsigned long mnuGroupInvisible = 0x10002;
const unsigned long mnuGroupIgnore = 0x10003;
const unsigned long mnuOnTop = 0x10004;

const unsigned short ContainerModeUser = 0;
const unsigned short ContainerModeGroup = 1;
const unsigned short ContainerModeAll = 2;

const unsigned long ContainerAllUsers = 0xFFFFFFFF;

class XOSD;
class ICQEvent;
class QDialog;
class KAboutKDE;
class HotKeys;

class unread_msg
{
public:
    unread_msg(ICQMessage *msg);
    unsigned short type() { return m_type; }
    unsigned long uin() { return m_uin; }
    bool operator == (const unread_msg &msg) const { return (m_uin == msg.m_uin) && (m_id == msg.m_id); }
protected:
    unsigned long  m_uin;
    unsigned long  m_id;
    unsigned short m_type;
};

typedef struct msgInfo
{
    unsigned long  uin;
    unsigned short type;
    unsigned	   count;
    int			   menuId;
    bool operator < (const msgInfo &m) const;
} msgInfo;

class MainWindow : public QMainWindow, public ConfigArray
{
    Q_OBJECT
public:
    MainWindow(const char *name = NULL);
    ~MainWindow();

    ConfigBool   Show;
    ConfigBool	 OnTop;

    ConfigBool   ShowOffline;
    ConfigBool   GroupMode;

    ConfigShort  mLeft;
    ConfigShort  mTop;
    ConfigShort  mWidth;
    ConfigShort  mHeight;

    ConfigString UseStyle;

    ConfigULong  AutoAwayTime;
    ConfigULong  AutoNATime;
	ConfigBool	 NoAlertAway;

    ConfigULong  ManualStatus;
    ConfigShort  DivPos;

    ConfigBool   SpellOnSend;

    ConfigString ToolbarDock;
    ConfigShort	 ToolbarOffset;
    ConfigShort  ToolbarY;

    ConfigString UrlViewer;
    ConfigString MailClient;

    ConfigBool   UseTransparent;
    ConfigULong  Transparent;
    ConfigBool	 UseTransparentContainer;
    ConfigULong	 TransparentContainer;
    ConfigBool	 TransparentIfInactive;

    ConfigBool	 NoShowAway;
    ConfigBool	 NoShowNA;
    ConfigBool	 NoShowOccupied;
    ConfigBool	 NoShowDND;
    ConfigBool	 NoShowFFC;

    ConfigBool	 UseSystemFonts;

    ConfigString FontFamily;
    ConfigUShort FontSize;
    ConfigUShort FontWeight;
    ConfigBool   FontItalic;

    ConfigString FontMenuFamily;
    ConfigUShort FontMenuSize;
    ConfigUShort FontMenuWeight;
    ConfigBool   FontMenuItalic;

    ConfigULong	 ColorSend;
    ConfigULong	 ColorReceive;

    ConfigUShort ChatWidth;
    ConfigUShort ChatHeight;

    ConfigShort	 UserBoxX;
    ConfigShort	 UserBoxY;
    ConfigUShort UserBoxWidth;
    ConfigUShort UserBoxHeight;
    ConfigString UserBoxToolbarDock;
    ConfigShort	 UserBoxToolbarOffset;
    ConfigShort  UserBoxToolbarY;

    ConfigString UserBoxFontFamily;
    ConfigUShort UserBoxFontSize;
    ConfigUShort UserBoxFontWeight;
    ConfigBool   UserBoxFontItalic;

    ConfigBool	 CloseAfterSend;
    ConfigBool	 CloseAfterFileTransfer;
    ConfigBool	 MainWindowInTaskManager;
    ConfigBool	 UserWindowInTaskManager;

    ConfigString Icons;

    ConfigBool   XOSD_on;
    ConfigShort  XOSD_pos;
    ConfigShort  XOSD_offset;
    ConfigULong  XOSD_color;
    ConfigString XOSD_FontFamily;
    ConfigUShort XOSD_FontSize;
    ConfigUShort XOSD_FontWeight;
    ConfigBool   XOSD_FontItalic;
    ConfigUShort XOSD_timeout;
    ConfigBool	 XOSD_Shadow;
    ConfigBool	 XOSD_Background;
    ConfigULong	 XOSD_BgColor;

    ConfigUShort ContainerMode;
    ConfigULong	 MessageBgColor;
    ConfigULong	 MessageFgColor;
    ConfigBool	 SimpleMode;
    ConfigBool	 UseOwnColors;
    ConfigBool	 UserWndOnTop;

    ConfigString KeyWindow;
    ConfigString KeyDblClick;
    ConfigString KeySearch;

    ConfigBool	 UseEmotional;
    ConfigULong	 AutoHideTime;

    bool 	     init();

    QPopupMenu   *menuStatus;
    QPopupMenu	 *menuPhone;
    QPopupMenu	 *menuPhoneLocation;
    QPopupMenu	 *menuPhoneStatus;
    KPopupMenu   *menuFunction;
    KPopupMenu   *menuUser;
    QPopupMenu	 *menuGroup;
    QPopupMenu   *menuContainers;
    QPopupMenu   *menuGroups;

    UserView     *users;

    CToolButton  *btnShowOffline;
    CToolButton  *btnGroupMode;
    PictButton   *btnStatus;
    Themes *themes;
    XOSD	*xosd;

    void setShow(bool bState);
    bool isShow();
    static void buildFileName(string &s, const char *name, bool bSystem=true, bool bCreate=true);
    static void playSound(const char *wav);

    static string homeDir;

    void adjustUserMenu(QPopupMenu *menu, ICQUser *u, bool bHaveTitle, bool bShort);
    void adjustGroupMenu(QPopupMenu *menu, unsigned long uin);
    void destroyBox(UserBox*);
    unsigned long m_uin;
    QRect m_rc;

    void setFonts();
    void changeColors();
    void changeOwnColors(bool bUse);
    void changeWm();
    void changeMode(bool bSample);
    void setKeys(const char *kWindow, const char *kDblClick, const char *kSearch);

    QWidget *chatWindow(unsigned long uin);
    QWidget *ftWindow(unsigned long uin, const string &fileName);

    virtual void setBackgroundPixmap(const QPixmap&);

    static QString ParseText(const UTFstring &t, bool bIgnoreColors);

    static const char *sound(const char *wav);

    void fillUnread(list<msgInfo> &msgs);
    list<unread_msg> messages;
signals:
    void modeChanged(bool);
    void transparentChanged();
    void colorsChanged();
    void ownColorsChanged();
    void setupInit();
    void iconChanged();
    void wmChanged();
    void chatChanged();
    void ftChanged();
public slots:
    void quit();
    void setup();
    void phonebook();
    void search();
    void toggleShow();
    void showPopup(QPoint);
    void saveState();
    void toggleGroupMode();
    void toggleShowOffline();
    void setGroupMode(bool);
    void setShowOffline(bool);
    void setStatus(int);
    void showUserPopup(unsigned long uin, QPoint, QPopupMenu*, const QRect&);
    void userFunction(int);
    void userFunction(unsigned long uin, int, unsigned long param=0);
    void showUser(int);
    void goURL(const char*);
    void sendMail(unsigned long);
    void sendMail(const char*);
    void toggleOnTop();
    void moveUser(int);
    void changeTransparent();
    void changeIcons(int);
    void chatClose();
    void ftClose();
protected slots:
    void toggleWindow();
    void realSetStatus();
    void autoAway();
    void setToggle();
    void blink();
    void processEvent(ICQEvent*);
    void messageReceived(ICQMessage *msg);
    void messageRead(ICQMessage*);
    void dockDblClicked();
    void setPhoneLocation(int);
    void setPhoneStatus(int);
    void toContainer(int);
    void addNonIM();
    void sendSMS();
    void showGroupPopup(QPoint);
    void deleteUser(int);
    void ignoreUser(int);
    void about();
    void about_kde();
    void bug_report();
    void dialogFinished();
    void timerExpired();
    void currentDesktopChanged(int);
    void adjustGroupsMenu();
    void adjustFucntionMenu();
protected:
    list<msgInfo> menuMsgs;
    void loadUnread();

    char realTZ;
    int lockFile;
    bool bBlinkState;

    list<UserFloat*> floating;
    UserFloat *findFloating(unsigned long uin, bool bDelete = false);

    bool m_bAutoAway, m_bAutoNA;
    int  m_autoStatus;
    bool noToggle;
    QTimer *autoAwayTimer;
    QTimer *blinkTimer;
    void setIcons();
    void setDock(bool bState);
    unsigned long uinMenu;
    void setOnTop();
    void ownerChanged();
    void saveContacts();
    void setStatusIcon(QPixmap *p);
    void addStatusItem(int status);
    void setStatusItem(int status);
    bool	bQuit;
    DockWnd	*dock;
    HotKeys *keys;
    virtual bool event(QEvent*);
    virtual void closeEvent(QCloseEvent*);
    void addMessageType(QPopupMenu *menu, int type, int id, bool bAdd, bool bHaveTitle);
    void addMenuItem(QPopupMenu *menuUser, const char *icon, const QString &n, int id, bool bAdd, bool bHaveTitle);
    QToolBar *toolbar;
    list<UserBox*> containers;
    SearchDialog *searchDlg;
    SetupDialog *setupDlg;
    bool bInLogin;

    unsigned hideTime;

    void exec(const char *prg, const char *arg);
    void loadMenu();
    void showUser(unsigned long uin, int function, unsigned long param=0);
    void closeUser(unsigned long uin);

    TransparentTop *transparent;
    QDialog      *mAboutApp;
#ifdef USE_KDE
    KAboutKDE    *mAboutKDE;
#endif
    QTranslator	 *translator;
    void initTranslator();
};

extern MainWindow *pMain;

#endif

