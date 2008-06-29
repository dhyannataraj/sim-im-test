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

const unsigned long CmdBase				= 0x00020000;
const unsigned long CmdInfo				= (CmdBase + 0x001);
const unsigned long CmdSearch				= (CmdBase + 0x002);
const unsigned long CmdConnections			= (CmdBase + 0x003);
const unsigned long CmdCM				= (CmdBase + 0x004);
const unsigned long CmdChange				= (CmdBase + 0x005);
const unsigned long CmdShowPanel			= (CmdBase + 0x006);
const unsigned long CmdCommonStatus			= (CmdBase + 0x007);
const unsigned long CmdTitle				= (CmdBase + 0x008);
const unsigned long CmdSetup				= (CmdBase + 0x009);
const unsigned long CmdMainWindow			= (CmdBase + 0x00B);	// 11
const unsigned long CmdUserView				= (CmdBase + 0x00C);	// 12
const unsigned long CmdContainer			= (CmdBase + 0x00D);	// 13
const unsigned long CmdClose				= (CmdBase + 0x00E);	// 14
const unsigned long CmdContainerContact			= (CmdBase + 0x00F);	// 15
const unsigned long CmdContainerContacts		= (CmdBase + 0x010);	// 16
const unsigned long CmdSendMessage			= (CmdBase + 0x011);	// 17
const unsigned long CmdSend				= (CmdBase + 0x012);	// 18
const unsigned long CmdStatusMenu			= (CmdBase + 0x014);	// 20
const unsigned long CmdStatusBar			= (CmdBase + 0x015);	// 21
const unsigned long CmdQuit				= (CmdBase + 0x017);	// 23  // FIXME: Why do CmdQuit & CmdMenu has the same numbers?
const unsigned long CmdMenu				= (CmdBase + 0x017);	// 23
const unsigned long CmdOnline				= (CmdBase + 0x018);	// 24
const unsigned long CmdGroup				= (CmdBase + 0x019);	// 25
const unsigned long CmdGrpOff				= (CmdBase + 0x01A);	// 26
const unsigned long CmdGrpMode1				= (CmdBase + 0x01B);	// 27
const unsigned long CmdGrpMode2				= (CmdBase + 0x01C);	// 28
const unsigned long CmdGrpCreate			= (CmdBase + 0x01D);	// 29
const unsigned long CmdGrpRename			= (CmdBase + 0x01E);	// 30
const unsigned long CmdGrpDelete			= (CmdBase + 0x01F);	// 31
const unsigned long CmdGrpTitle				= (CmdBase + 0x020);	// 32
const unsigned long CmdGrpUp				= (CmdBase + 0x021);	// 33
const unsigned long CmdGrpDown				= (CmdBase + 0x022);	// 34
const unsigned long CmdContactTitle			= (CmdBase + 0x023);	// 35
const unsigned long CmdContactRename			= (CmdBase + 0x024);	// 36
const unsigned long CmdContactDelete			= (CmdBase + 0x025);	// 37
const unsigned long CmdConfigure			= (CmdBase + 0x026);	// 38
const unsigned long CmdMessageType			= (CmdBase + 0x027);	// 39
const unsigned long CmdSendClose			= (CmdBase + 0x028);	// 40
const unsigned long CmdSmile				= (CmdBase + 0x029);	// 41
const unsigned long CmdMultiply				= (CmdBase + 0x02A);	// 42
const unsigned long CmdSendSMS				= (CmdBase + 0x02B);	// 43
const unsigned long CmdInvisible			= (CmdBase + 0x02C);	// 44
const unsigned long CmdHistory				= (CmdBase + 0x02D);	// 45
const unsigned long CmdHistorySave			= (CmdBase + 0x02E);	// 46
const unsigned long CmdHistoryDirection			= (CmdBase + 0x02F);	// 47
const unsigned long CmdHistoryNext			= (CmdBase + 0x030);	// 48
const unsigned long CmdHistoryPrev			= (CmdBase + 0x031);	// 49
const unsigned long CmdMsgOpen				= (CmdBase + 0x033);	// 51
const unsigned long CmdMsgQuote				= (CmdBase + 0x035);	// 53
const unsigned long CmdMsgAnswer			= (CmdBase + 0x036);	// 54
const unsigned long CmdMsgForward			= (CmdBase + 0x037);	// 55
const unsigned long CmdCopy				= (CmdBase + 0x038);	// 56
const unsigned long CmdCut				= (CmdBase + 0x039);	// 57
const unsigned long CmdPaste				= (CmdBase + 0x03A);	// 58
const unsigned long CmdSelectAll			= (CmdBase + 0x03B);	// 59
const unsigned long CmdUndo				= (CmdBase + 0x03C);	// 60
const unsigned long CmdRedo				= (CmdBase + 0x03D);	// 61
const unsigned long CmdClear				= (CmdBase + 0x03E);	// 62
const unsigned long CmdSeparate				= (CmdBase + 0x03F);	// 63
const unsigned long CmdNextMessage			= (CmdBase + 0x041);	// 65
const unsigned long CmdGrantAuth			= (CmdBase + 0x042);	// 66
const unsigned long CmdRefuseAuth			= (CmdBase + 0x043);	// 67
const unsigned long CmdPhones				= (CmdBase + 0x047);	// 71
const unsigned long CmdLocation				= (CmdBase + 0x048);	// 72
const unsigned long CmdPhoneState			= (CmdBase + 0x049);	// 73
const unsigned long CmdPhoneNoShow			= (CmdBase + 0x04A);	// 74
const unsigned long CmdPhoneAvailable			= (CmdBase + 0x04B);	// 75
const unsigned long CmdPhoneBusy			= (CmdBase + 0x04C);	// 76
const unsigned long CmdPhoneBook			= (CmdBase + 0x04D);	// 77
const unsigned long CmdShowAlways			= (CmdBase + 0x04E);	// 78
const unsigned long CmdFileAccept			= (CmdBase + 0x04F);	// 79
const unsigned long CmdFileDecline			= (CmdBase + 0x050);	// 80
const unsigned long CmdDeclineWithoutReason		= (CmdBase + 0x051);	// 81
const unsigned long CmdDeclineReasonInput		= (CmdBase + 0x052);	// 82
const unsigned long CmdDeclineReasonBusy		= (CmdBase + 0x053);	// 83
const unsigned long CmdDeclineReasonLater		= (CmdBase + 0x054);	// 84
const unsigned long CmdHistoryFind			= (CmdBase + 0x055);	// 85
const unsigned long CmdFileName				= (CmdBase + 0x056);	// 86
const unsigned long CmdPhoneNumber			= (CmdBase + 0x057);	// 87
const unsigned long CmdTranslit				= (CmdBase + 0x058);	// 88
const unsigned long CmdUrlInput				= (CmdBase + 0x059);	// 89
const unsigned long CmdCutHistory			= (CmdBase + 0x05A);	// 90
const unsigned long CmdDeleteMessage			= (CmdBase + 0x05B);	// 91
const unsigned long CmdEditList				= (CmdBase + 0x05C);	// 92
const unsigned long CmdRemoveList			= (CmdBase + 0x05D);	// 93
const unsigned long CmdStatusWnd			= (CmdBase + 0x05E);	// 94
const unsigned long CmdEmptyGroup			= (CmdBase + 0x05F);	// 95
const unsigned long CmdEnableSpell			= (CmdBase + 0x060);	// 96
const unsigned long CmdSpell				= (CmdBase + 0x061);	// 97
const unsigned long CmdChangeEncoding			= (CmdBase + 0x062);	// 98
const unsigned long CmdAllEncodings			= (CmdBase + 0x063);	// 99
const unsigned long CmdSearchInfo			= (CmdBase + 0x065);	// 101
const unsigned long CmdSearchMsg			= (CmdBase + 0x066);	// 102
const unsigned long CmdSearchOptions			= (CmdBase + 0x067);	// 103
const unsigned long CmdFetchAway			= (CmdBase + 0x068);	// 104
const unsigned long CmdHistoryAvatar			= (CmdBase + 0x069);	// 105

const unsigned long CmdContactGroup			= (CmdBase + 0x100);
const unsigned long CmdUnread				= (CmdBase + 0x200);
const unsigned long CmdContactClients			= (CmdBase + 0x300);
const unsigned long CmdMsgSpecial			= (CmdBase + 0x400);
const unsigned long CmdClient				= (CmdBase + 0x500);
const unsigned long CmdContactResource			= (CmdBase + 0x600);
const unsigned long CmdReceived				= 0x600;  //FIXME: Why it does not have CmdBase in it?

const unsigned long MenuConnections			= (CmdBase + 0x001);
const unsigned long MenuContainerContact		= (CmdBase + 0x002);
const unsigned long MenuStatus				= (CmdBase + 0x003);
const unsigned long MenuGroups				= (CmdBase + 0x004);
const unsigned long MenuMsgView				= (CmdBase + 0x005);
const unsigned long MenuTextEdit			= (CmdBase + 0x006);
const unsigned long MenuMsgCommand			= (CmdBase + 0x007);
const unsigned long MenuPhones				= (CmdBase + 0x008);
const unsigned long MenuLocation			= (CmdBase + 0x009);
const unsigned long MenuPhoneState			= (CmdBase + 0x00A);	// 10
const unsigned long MenuFileDecline			= (CmdBase + 0x00B);	// 11
const unsigned long MenuMailList			= (CmdBase + 0x00C);	// 12
const unsigned long MenuPhoneList			= (CmdBase + 0x00D);	// 13
const unsigned long MenuStatusWnd			= (CmdBase + 0x00E);	// 14
const unsigned long MenuEncoding			= (CmdBase + 0x00F);	// 15
const unsigned long MenuSearchItem			= (CmdBase + 0x010);	// 16
const unsigned long MenuSearchGroups			= (CmdBase + 0x011);	// 17
const unsigned long MenuSearchOptions			= (CmdBase + 0x012);	// 18

const unsigned long BarHistory				= (CmdBase + 0x001);
const unsigned long BarHistoryAvatar			= (CmdBase + 0x002);

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

