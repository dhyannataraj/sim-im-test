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

#include "simapi.h"
#include "stl.h"

typedef map<string, unsigned>				MAP_TYPES;

typedef struct msg_id
{
    unsigned	id;
    unsigned	contact;
    unsigned	type;
    string		client;
} msg_id;

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
class Icons;

typedef struct CoreData
{
    Data	Profile;
    Data	SavePasswd;
    Data	NoShow;
    Data	ShowPanel;
    Data	ManualStatus;
    Data	StatusTime;
    Data	Invisible;
    Data	geometry[5];
    Data	toolBarState[7];
    Data	Buttons;
    Data	Menues;
    Data	ShowOnLine;
    Data	GroupMode;
    Data	UseDblClick;
    Data	UseSysColors;
    Data	ColorOnline;
    Data	ColorOffline;
    Data	ColorAway;
    Data	ColorNA;
    Data	ColorDND;
    Data	ColorGroup;
    Data	GroupSeparator;
    Data	Lang;
    Data	ContainerMode;
    Data	SendOnEnter;
    Data	containerGeo[5];
    Data	containerBar[7];
    Data	ContainerStatusSize;
    Data	Containers;
    Data	Container;
    Data	CopyMessages;
    Data	EditHeight;
    Data	editBar[7];
    Data	EditBackground;
    Data	EditForeground;
    Data	EditFont;
    Data	EditSaveFont;
    Data	OwnColors;
    Data	UseSmiles;
    Data	CloseSend;
    Data	HistoryPage;
    Data	HistoryDirection;
    Data	historySize[2];
    Data	historyBar[7];
    Data	HistorySearch;
    Data	Unread;
    Data	NoShowAutoReply;
    Data	SortMode;
    Data	CloseTransfer;
    Data	HistoryStyle;
    Data	AuthStyle;
    Data	VisibleStyle;
    Data	InvisibleStyle;
    Data	SmallGroupFont;
    Data	ShowAllEncodings;
    Data	DefaultEncoding;
    Data	ShowEmptyGroup;
    Data	NoJoinAlert;
    Data	EnableSpell;
    Data	RemoveHistory;
} CoreData;

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

typedef struct CoreUserData
{
    Data	LogStatus;
    Data	LogMessage;
    Data	OpenNewMessage;
    Data	OpenOnOnline;
    Data	IncomingPath;
    Data	AcceptMode;
    Data	OverwriteFiles;
    Data	DeclineMessage;
} CoreUserData;

typedef struct SMSUserData
{
    Data	SMSSignatureBefore;
    Data	SMSSignatureAfter;
} SMSUserData;

typedef struct ARUserData
{
    Data	AutoReply;
} ARUserData;

typedef struct ListUserData
{
    Data	OfflineOpen;
    Data	OnlineOpen;
    Data	ShowAlways;
} ListUserData;

typedef struct TranslitUserData
{
    Data	Translit;
} TranslitUserData;

typedef struct HistoryUserData
{
    Data	CutSize;
    Data	MaxSize;
    Data	CutDays;
    Data	Days;
} HistoryUserData;

class ClientList : public vector<Client*>
{
public:
    ClientList();
    ~ClientList();
    void addToContacts();
};

typedef struct ENCODING
{
    const char *language;
    const char *codec;
    int         mib;
    int		rtf_code;
    int		cp_code;
    bool        bMain;
} ENCODING;

const unsigned  CmdBase					= 0x00020000;
const unsigned	CmdInfo					= (CmdBase + 1);
const unsigned	CmdSearch				= (CmdBase + 2);
const unsigned	CmdConnections			= (CmdBase + 3);
const unsigned	CmdCM					= (CmdBase + 4);
const unsigned	CmdChange				= (CmdBase + 5);
const unsigned	CmdShowPanel			= (CmdBase + 6);
const unsigned	CmdCommonStatus			= (CmdBase + 7);
const unsigned	CmdTitle				= (CmdBase + 8);
const unsigned	CmdSetup				= (CmdBase + 9);
const unsigned	CmdMainWindow			= (CmdBase + 11);
const unsigned	CmdUserView				= (CmdBase + 12);
const unsigned	CmdContainer			= (CmdBase + 13);
const unsigned	CmdClose				= (CmdBase + 14);
const unsigned	CmdContainerContact		= (CmdBase + 15);
const unsigned	CmdContainerContacts	= (CmdBase + 16);
const unsigned	CmdSendMessage			= (CmdBase + 17);
const unsigned	CmdSend					= (CmdBase + 18);
const unsigned	CmdStatusMenu			= (CmdBase + 20);
const unsigned	CmdStatusBar			= (CmdBase + 21);
const unsigned	CmdQuit					= (CmdBase + 23);
const unsigned	CmdMenu					= (CmdBase + 23);
const unsigned	CmdOnline				= (CmdBase + 24);
const unsigned	CmdGroup				= (CmdBase + 25);
const unsigned	CmdGrpOff				= (CmdBase + 26);
const unsigned	CmdGrpMode1				= (CmdBase + 27);
const unsigned	CmdGrpMode2				= (CmdBase + 28);
const unsigned	CmdGrpCreate			= (CmdBase + 29);
const unsigned	CmdGrpRename			= (CmdBase + 30);
const unsigned	CmdGrpDelete			= (CmdBase + 31);
const unsigned	CmdGrpTitle				= (CmdBase + 32);
const unsigned	CmdGrpUp				= (CmdBase + 33);
const unsigned	CmdGrpDown				= (CmdBase + 34);
const unsigned	CmdContactTitle			= (CmdBase + 35);
const unsigned	CmdContactRename		= (CmdBase + 36);
const unsigned	CmdContactDelete		= (CmdBase + 37);
const unsigned  CmdConfigure			= (CmdBase + 38);
const unsigned	CmdMessageType			= (CmdBase + 39);
const unsigned	CmdSendClose			= (CmdBase + 40);
const unsigned	CmdSmile				= (CmdBase + 41);
const unsigned	CmdMultiply				= (CmdBase + 42);
const unsigned  CmdSendSMS				= (CmdBase + 43);
const unsigned	CmdInvisible			= (CmdBase + 44);
const unsigned	CmdHistory				= (CmdBase + 45);
const unsigned	CmdHistorySave			= (CmdBase + 46);
const unsigned	CmdHistoryDirection		= (CmdBase + 47);
const unsigned	CmdHistoryNext			= (CmdBase + 48);
const unsigned	CmdHistoryPrev			= (CmdBase + 49);
const unsigned	CmdMsgOpen				= (CmdBase + 51);
const unsigned	CmdMsgQuote				= (CmdBase + 53);
const unsigned	CmdMsgAnswer			= (CmdBase + 54);
const unsigned	CmdMsgForward			= (CmdBase + 55);
const unsigned	CmdCopy					= (CmdBase + 56);
const unsigned	CmdCut					= (CmdBase + 57);
const unsigned	CmdPaste				= (CmdBase + 58);
const unsigned	CmdSelectAll			= (CmdBase + 59);
const unsigned	CmdUndo					= (CmdBase + 60);
const unsigned	CmdRedo					= (CmdBase + 61);
const unsigned	CmdClear				= (CmdBase + 62);
const unsigned	CmdSeparate				= (CmdBase + 63);
const unsigned	CmdNextMessage			= (CmdBase + 65);
const unsigned	CmdGrantAuth			= (CmdBase + 66);
const unsigned	CmdRefuseAuth			= (CmdBase + 67);
const unsigned	CmdPhones				= (CmdBase + 71);
const unsigned	CmdLocation				= (CmdBase + 72);
const unsigned	CmdPhoneState			= (CmdBase + 73);
const unsigned	CmdPhoneNoShow			= (CmdBase + 74);
const unsigned	CmdPhoneAvailable		= (CmdBase + 75);
const unsigned	CmdPhoneBusy			= (CmdBase + 76);
const unsigned  CmdPhoneBook			= (CmdBase + 77);
const unsigned	CmdShowAlways			= (CmdBase + 78);
const unsigned	CmdFileAccept			= (CmdBase + 79);
const unsigned	CmdFileDecline			= (CmdBase + 80);
const unsigned	CmdDeclineWithoutReason	= (CmdBase + 81);
const unsigned	CmdDeclineReasonInput	= (CmdBase + 82);
const unsigned	CmdDeclineReasonBusy	= (CmdBase + 83);
const unsigned	CmdDeclineReasonLater	= (CmdBase + 84);
const unsigned	CmdHistoryFind		= (CmdBase + 85);
const unsigned	CmdFileName		= (CmdBase + 86);
const unsigned	CmdPhoneNumber		= (CmdBase + 87);
const unsigned	CmdTranslit		= (CmdBase + 88);
const unsigned  CmdUrlInput		= (CmdBase + 89);
const unsigned	CmdCutHistory		= (CmdBase + 90);
const unsigned	CmdDeleteMessage		= (CmdBase + 91);
const unsigned	CmdEditList				= (CmdBase + 92);
const unsigned	CmdRemoveList			= (CmdBase + 93);
const unsigned	CmdStatusWnd			= (CmdBase + 94);
const unsigned	CmdEmptyGroup			= (CmdBase + 95);
const unsigned	CmdEnableSpell			= (CmdBase + 96);
const unsigned	CmdSpell			= (CmdBase + 97);

const unsigned	CmdContactGroup			= (CmdBase + 0x100);
const unsigned	CmdUnread				= (CmdBase + 0x200);
const unsigned	CmdContactClients		= (CmdBase + 0x300);
const unsigned	CmdMsgSpecial			= (CmdBase + 0x400);
const unsigned	CmdClient				= (CmdBase + 0x500);
const unsigned  CmdContactResource		= (CmdBase + 0x600);
const unsigned  CmdReceived				= 0x600;

const unsigned	MenuConnections			= (CmdBase + 1);
const unsigned	MenuContainerContact	= (CmdBase + 2);
const unsigned  MenuStatus				= (CmdBase + 3);
const unsigned	MenuGroups				= (CmdBase + 4);
const unsigned	MenuMsgView				= (CmdBase + 5);
const unsigned	MenuTextEdit			= (CmdBase + 6);
const unsigned	MenuMsgCommand			= (CmdBase + 7);
const unsigned	MenuPhones				= (CmdBase + 8);
const unsigned	MenuLocation			= (CmdBase + 9);
const unsigned	MenuPhoneState			= (CmdBase + 10);
const unsigned	MenuFileDecline			= (CmdBase + 11);
const unsigned	MenuMailList			= (CmdBase + 12);
const unsigned	MenuPhoneList			= (CmdBase + 13);
const unsigned	MenuStatusWnd			= (CmdBase + 14);

const unsigned	EventCreateMessageType	= (CmdBase + 1);
const unsigned	EventRemoveMessageType	= (CmdBase + 2);
const unsigned	EventRealSendMessage	= (CmdBase + 3);
const unsigned  EventHistoryConfig		= (CmdBase + 4);
const unsigned	EventTemplateExpand		= (CmdBase + 5);
const unsigned	EventTemplateExpanded	= (CmdBase + 6);
const unsigned	EventARRequest			= (CmdBase + 7);
const unsigned	EventClientStatus		= (CmdBase + 8);
const unsigned	EventLoadMessage		= (CmdBase + 9);
const unsigned	EventDefaultAction		= (CmdBase + 10);
const unsigned	EventContactClient      = (CmdBase + 11);
const unsigned	EventGetIcons			= (CmdBase + 12);
const unsigned	EventSortChanged		= (CmdBase + 13);
const unsigned	EventActiveContact		= (CmdBase + 14);
const unsigned	EventMessageRetry		= (CmdBase + 15);
const unsigned	EventHistoryColors		= (CmdBase + 16);
const unsigned	EventHistoryFont		= (CmdBase + 17);
const unsigned  EventCheckSend			= (CmdBase + 18);
const unsigned	EventCutHistory			= (CmdBase + 19);
const unsigned  EventTmplHelp			= (CmdBase + 20);
const unsigned  EventTmplHelpList		= (CmdBase + 21);
const unsigned	EventDeleteMessage		= (CmdBase + 22);
const unsigned	EventRewriteMessage		= (CmdBase + 23);
const unsigned	EventJoinAlert			= (CmdBase + 24);

const unsigned	BarHistory				= (CmdBase + 1);

class MsgEdit;
class Tmpl;

typedef struct CheckSend
{
    unsigned	id;
    Client		*client;
    void		*data;
} CheckSend;

typedef struct MessageID
{
    unsigned	id;
    const char	*client;
    unsigned	contact;
} MessageID;

typedef struct ARRequest
{
    Contact			*contact;
    unsigned		status;
    EventReceiver	*receiver;
    void			*param;
} ARReuest;

typedef struct TemplateExpand
{
    QString			tmpl;
    Contact			*contact;
    EventReceiver	*receiver;
    void			*param;
} TemplateExpand;

typedef struct MsgSend
{
    Message		*msg;
    MsgEdit		*edit;
} MsgSend;

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

typedef struct MessageDef
{
    const CommandDef	*cmdReceived;
    const CommandDef	*cmdSent;
    unsigned			flags;
    const char			*singular;
    const char			*plural;
    Message*			(*create)(const char *cfg);
    QObject*			(*generate)(MsgEdit *edit, Message *msg);
    Message*			(*drag)(QMimeSource*);
} MessageDef;

typedef struct clientContact
{
    clientData	*data;
    Client		*client;
    bool		bNew;
} clientContact;

class XSL;
class BalloonMsg;

class CorePlugin : public QObject, public Plugin, public EventReceiver
{
    Q_OBJECT
public:
    CorePlugin(unsigned, const char*);
    virtual ~CorePlugin();
    PROP_STR(Profile)
    PROP_BOOL(SavePasswd)
    PROP_BOOL(NoShow)
    PROP_BOOL(ShowPanel)
    void setManualStatus(unsigned long status);
    unsigned long getManualStatus() { return data.ManualStatus.value; }
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
    PROP_STR(DefaultEncoding);
    PROP_BOOL(ShowEmptyGroup);
    PROP_BOOL(NoJoinAlert);
    PROP_BOOL(EnableSpell);
    PROP_BOOL(RemoveHistory);

    unsigned user_data_id;
    unsigned sms_data_id;
    unsigned ar_data_id;
    unsigned list_data_id;
    unsigned translit_data_id;
    unsigned history_data_id;

    CommandsMap	preferences;
    CommandsMap	messageTypes;
    MAP_TYPES	types;

    list<msg_id>	unread;

    QFont editFont;
    static CorePlugin	*m_plugin;
    Message *createMessage(const char *type, const char *cfg);
    QString clientName(Client *client);

    XSL	*historyXSL;
    CoreData	data;

    bool m_bIgnoreEvents;
    const ENCODING *encodings;

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
    virtual void *processEvent(Event*);
    virtual string getConfig();
    virtual QWidget *createConfigWindow(QWidget *parent);
    void showInfo(CommandDef *cmd);
    bool init(bool bFirst);
    void destroy();
    void loadDir();
    void loadClients(ClientList&);
    void loadMenu();
    QString poFile(const char *lang);
    Client *loadClient(const char *name, const char *cfg);
    void setCurrentProfile(const char *profile);
    bool adjustClientItem(unsigned id, CommandDef *cmd);
    void showPanel();
    void hideWindows();
    void changeProfile();
    void installTranslator();
    void removeTranslator();
    void initData();
    void loadUnread();
    void clearUnread(unsigned contact_id);
    void getWays(vector<clientContact> &ways, Contact *contact);
    string typeName(const char *name);
    void setAutoReplies();
    bool lockProfile(const char*, bool bSend = false);
    bool				m_bInit;
    vector<string>		m_profiles;
    QWidget				*m_cfg;
    QWidget				*m_focus;
    UserView			*m_view;
    SearchDialog		*m_search;
    QTranslator			*m_translator;
    ConnectionManager	*m_manager;
    CommonStatus		*m_status;
    StatusWnd			*m_statusWnd;
    char				*m_profile;
    unsigned			m_nClients;
    unsigned			m_nClientsMenu;
    unsigned			m_nResourceMenu;
    Commands			*m_cmds;
    MainWindow			*m_main;
    Icons				*m_icons;
    Tmpl				*m_tmpl;
    BalloonMsg			*m_alert;
    FileLock			*m_lock;

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

