/***************************************************************************
                          core.h  -  description
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

#ifndef _CORE_H
#define _CORE_H

#include <vector>

#include <qfont.h>
#include <qobject.h>
#include <qthread.h>

#include "cmddef.h"
#include "event.h"
#include "misc.h"
#include "plugins.h"
#include "core_consts.h"

typedef std::map<QString, unsigned> MAP_TYPES;

struct msg_id
{
    unsigned    id;
    unsigned    contact;
    unsigned    type;
    QString     client;
};

class FileLock;
class QWidget;
class QTranslator;
class QMimeSource;
class Commands;
class MainWindow;
class UserView;
class SearchDialog;
class CommonStatus;
class StatusWnd;
class ConnectionManager;

struct CoreData
{
    SIM::Data	Profile;
    SIM::Data	SavePasswd;
    SIM::Data	NoShow;
    SIM::Data	ShowPanel;
    SIM::Data	ManualStatus;
    SIM::Data	StatusTime;
    SIM::Data	Invisible;
    SIM::Data	geometry[5];
    SIM::Data	toolBarState[7];
    SIM::Data	Buttons;
    SIM::Data	Menues;
    SIM::Data	ShowOnLine;
    SIM::Data	GroupMode;
    SIM::Data	UseDblClick;
    SIM::Data	UseSysColors;
    SIM::Data	ColorOnline;
    SIM::Data	ColorOffline;
    SIM::Data	ColorAway;
    SIM::Data	ColorNA;
    SIM::Data	ColorDND;
    SIM::Data	ColorGroup;
    SIM::Data	GroupSeparator;
    SIM::Data	Lang;
    SIM::Data	ContainerMode;
    SIM::Data	SendOnEnter;
    SIM::Data	ShowOwnerName;
    SIM::Data	ContainerGeometry[5];
    SIM::Data	ContainerBar[7];
    SIM::Data	ContainerStatusSize;
    SIM::Data	Containers;
    SIM::Data	Container;
    SIM::Data	CopyMessages;
    SIM::Data	EditHeight;
    SIM::Data	EditBar[7];
    SIM::Data	EditBackground;
    SIM::Data	EditForeground;
    SIM::Data	EditFont;
    SIM::Data	EditSaveFont;
    SIM::Data	OwnColors;
    SIM::Data	UseSmiles;
    SIM::Data	UseExtViewer;
    SIM::Data	ExtViewer;
    SIM::Data	CloseSend;
    SIM::Data	HistoryPage;
    SIM::Data	HistoryDirection;
    SIM::Data	HistorySize[2];
    SIM::Data	HistoryBar[7];
    SIM::Data	HistoryAvatarBar[7];
    SIM::Data	HistorySearch;
    SIM::Data	Unread;
    SIM::Data	NoShowAutoReply;
    SIM::Data	SortMode;
    SIM::Data	CloseTransfer;
    SIM::Data	HistoryStyle;
    SIM::Data	AuthStyle;
    SIM::Data	VisibleStyle;
    SIM::Data	InvisibleStyle;
    SIM::Data	SmallGroupFont;
    SIM::Data	ShowAllEncodings;
    SIM::Data	ShowEmptyGroup;
    SIM::Data	NoJoinAlert;
    SIM::Data	EnableSpell;
    SIM::Data	RemoveHistory;
    SIM::Data	SearchGeometry[5];
    SIM::Data	SearchClient;
    SIM::Data	NoScroller;
    SIM::Data	CfgGeometry[5];
    SIM::Data	ShowAvatarInHistory;
    SIM::Data	ShowAvatarInContainer;
};

const unsigned CONTAINER_SIMPLE	= 0;
const unsigned CONTAINER_NEW	= 1;
const unsigned CONTAINER_GROUP	= 2;
const unsigned CONTAINER_ALL	= 3;

const unsigned CONTAINER_GRP	= 0x80000000;

const unsigned char SORT_NONE	= 0;
const unsigned char SORT_STATUS	= 1;
const unsigned char SORT_ACTIVE	= 2;
const unsigned char SORT_NAME	= 3;

const unsigned NEW_MSG_NOOPEN	= 0;
const unsigned NEW_MSG_MINIMIZE	= 1;
const unsigned NEW_MSG_RAISE	= 2;

struct CoreUserData
{
    SIM::Data	LogStatus;
    SIM::Data	LogMessage;
    SIM::Data	OpenNewMessage;
    SIM::Data	OpenOnOnline;
    SIM::Data	IncomingPath;
    SIM::Data	AcceptMode;
    SIM::Data	OverwriteFiles;
    SIM::Data	DeclineMessage;
};

struct SMSUserData
{
    SIM::Data	SMSSignatureBefore;
    SIM::Data	SMSSignatureAfter;
};

struct ARUserData
{
    SIM::Data	AutoReply;
};

struct ListUserData
{
    SIM::Data	OfflineOpen;
    SIM::Data	OnlineOpen;
    SIM::Data	ShowAlways;
};

struct TranslitUserData
{
    SIM::Data	Translit;
};

struct HistoryUserData
{
    SIM::Data	CutSize;
    SIM::Data	MaxSize;
    SIM::Data	CutDays;
    SIM::Data	Days;
};

class ClientList : public std::vector<SIM::Client*>
{
public:
    ClientList();
    ~ClientList();
    void addToContacts();
};


#include "core_events.h"

const unsigned	MESSAGE_DEFAULT		= 0x0000;
const unsigned	MESSAGE_SILENT		= 0x0001;
const unsigned	MESSAGE_HIDDEN		= 0x0002;
const unsigned	MESSAGE_SENDONLY	= 0x0004;
const unsigned	MESSAGE_INFO		= 0x0008;
const unsigned  MESSAGE_SYSTEM		= 0x0010;
const unsigned  MESSAGE_ERROR		= 0x0020;
const unsigned	MESSAGE_CHILD		= 0x0040;

const unsigned  MIN_INPUT_BAR_ID = 0x1010;
const unsigned  MAX_INPUT_BAR_ID = 0x1500;

const unsigned  STYLE_UNDER		= 1;
const unsigned  STYLE_ITALIC	= 2;
const unsigned  STYLE_STRIKE	= 4;

class MsgEdit;

struct MessageDef
{
    const SIM::CommandDef	*cmdReceived;
    const SIM::CommandDef	*cmdSent;
    unsigned			flags;
    const char			*singular;
    const char			*plural;
    SIM::Message*		(*create)(Buffer *cfg);
    QObject*			(*generate)(MsgEdit *edit, SIM::Message *msg);
    SIM::Message*		(*drag)(QMimeSource*);
};

struct clientContact
{
    SIM::clientData	*data;
    SIM::Client	*client;
    bool		bNew;
};


class Tmpl;
class XSL;
class BalloonMsg;

class HistoryThread : public QThread
{
public:
    virtual void run();
    void set_id(unsigned id) {m_id=id;}
    void set_Viewer(const QString &Viewer) {m_Viewer=Viewer;}
protected:
    unsigned m_id;
    QString m_Viewer;
};

class CorePlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
    Q_OBJECT
public:
    CorePlugin(unsigned, Buffer*);
    virtual ~CorePlugin();
    PROP_STR(Profile)
    PROP_BOOL(SavePasswd)
    PROP_BOOL(NoShow)
    PROP_BOOL(ShowPanel)
    void setManualStatus(unsigned long status);
    unsigned long getManualStatus() { return data.ManualStatus.toULong(); }
    PROP_ULONG(StatusTime)
    PROP_BOOL(Invisible)
    PROP_STRLIST(Buttons)
    PROP_STRLIST(Menues)
    PROP_BOOL(ShowOnLine)
    PROP_ULONG(GroupMode)
    PROP_BOOL(UseDblClick)
    PROP_BOOL(UseSysColors)
    PROP_ULONG(ColorOnline)
    PROP_ULONG(ColorOffline)
    PROP_ULONG(ColorAway)
    PROP_ULONG(ColorNA)
    PROP_ULONG(ColorDND)
    PROP_ULONG(ColorGroup)
    PROP_BOOL(GroupSeparator)
    PROP_STR(Lang);
    //    PROP_ULONG(ContainerMode);
    unsigned getContainerMode();
    void setContainerMode(unsigned);
    PROP_BOOL(SendOnEnter);
    PROP_BOOL(ShowOwnerName);
    PROP_ULONG(ContainerStatusSize);
    PROP_STR(Containers);
    PROP_STRLIST(Container);
    PROP_ULONG(CopyMessages);
    PROP_ULONG(EditHeight);
    PROP_ULONG(EditForeground);
    PROP_ULONG(EditBackground);
    PROP_STR(EditFont);
    PROP_BOOL(EditSaveFont);
    PROP_BOOL(OwnColors);
    PROP_BOOL(UseSmiles);
    PROP_BOOL(UseExtViewer);
    PROP_STR(ExtViewer);
    PROP_BOOL(CloseSend);
    PROP_ULONG(HistoryPage);
    PROP_BOOL(HistoryDirection);
    PROP_UTF8(HistorySearch);
    PROP_STR(Unread);
    PROP_STRLIST(NoShowAutoReply);
    PROP_ULONG(SortMode);
    PROP_BOOL(CloseTransfer);
    PROP_STR(HistoryStyle);
    PROP_ULONG(AuthStyle);
    PROP_ULONG(VisibleStyle);
    PROP_ULONG(InvisibleStyle);
    PROP_BOOL(SmallGroupFont);
    PROP_BOOL(ShowAllEncodings);
    PROP_BOOL(ShowEmptyGroup);
    PROP_BOOL(NoJoinAlert);
    PROP_BOOL(EnableSpell);
    PROP_BOOL(RemoveHistory);
    PROP_STR(SearchClient);
    PROP_BOOL(NoScroller);
    PROP_BOOL(ShowAvatarInHistory);
    PROP_BOOL(ShowAvatarInContainer);
    void setRegNew(bool p_new) {m_RegNew=p_new;}
    bool getRegNew() const {return m_RegNew;}
    void setICQUIN(QString p_uin) {m_ICQUIN=p_uin;}
    QString getICQUIN() const {return m_ICQUIN;}
    void setICQPassword(QString p_password) {m_ICQPassword=p_password;}
    QString getICQPassword() const {return m_ICQPassword;}

    unsigned user_data_id;
    unsigned sms_data_id;
    unsigned ar_data_id;
    unsigned list_data_id;
    unsigned translit_data_id;
    unsigned history_data_id;

    SIM::CommandsMap	preferences;
    SIM::CommandsMap	messageTypes;
    MAP_TYPES	types;

    std::list<msg_id>	unread;

    QFont editFont;
    static CorePlugin	*m_plugin;
    SIM::Message *createMessage(const char *type, Buffer *cfg);
    QString clientName(SIM::Client *client);

    XSL	*historyXSL;
    CoreData	data;
    bool m_bIgnoreEvents;
signals:
    void modeChanged();
protected slots:
    void dialogFinished();
    void dialogDestroy();
    void managerFinished();
    void destroyManager();
    void selectProfile();
    void checkHistory();
    void alertFinished();
    void focusDestroyed();
    void showMain();
protected:
    virtual bool processEvent(SIM::Event*);
    virtual QCString getConfig();
    virtual QWidget *createConfigWindow(QWidget *parent);
    void showInfo(SIM::CommandDef *cmd);
    bool init(bool bFirst);
    void destroy();
    void loadDir();
    void loadClients(ClientList&);
    void loadMenu();
    QString poFile(const char *lang);
    SIM::Client *loadClient(const QString &name, Buffer *cfg);
    bool adjustClientItem(unsigned id, SIM::CommandDef *cmd);
    void showPanel();
    void hideWindows();
    void changeProfile();
    void installTranslator();
    void removeTranslator();
    void initData();
    void loadUnread();
    void clearUnread(unsigned contact_id);
    void getWays(std::vector<clientContact> &ways, SIM::Contact *contact);
    QString typeName(const QString &name);
    void setAutoReplies();
    bool lockProfile(const QString &profile, bool bSend = false);

    void createMainToolbar();
    bool updateMainToolbar(unsigned long commandID);
    void createHistoryToolbar();
    void createContainerToolbar();
    void createMsgEditToolbar();

    bool                m_bInit;
    QStringList         m_profiles;
    QWidget             *m_cfg;
    QWidget             *m_focus;
    UserView            *m_view;
    SearchDialog        *m_search;
    QTranslator         *m_translator;
    ConnectionManager   *m_manager;
    CommonStatus        *m_status;
    StatusWnd           *m_statusWnd;
    unsigned            m_nClients;
    unsigned            m_nClientsMenu;
    unsigned            m_nResourceMenu;
    Commands            *m_cmds;
    MainWindow          *m_main;
    Tmpl                *m_tmpl;
    BalloonMsg          *m_alert;
    FileLock            *m_lock;
    bool                m_RegNew;
    QString             m_ICQUIN;
    QString             m_ICQPassword;
    HistoryThread      *m_HistoryThread;

    friend class MainWindow;
    friend class UserView;
    friend class UserConfig;
    friend class SearchDialog;
    friend class LangConfig;
    friend class CommonStatus;
    friend class LoginDialog;
    friend class StatusWnd;
    friend class InterfaceConfig;
    friend class ConnectionManager;
    friend class Container;
    friend class UserWnd;
    friend class MsgEdit;
    friend class HistoryWindow;
    friend class Tmpl;
};

#endif

