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
#include "qobject.h"

#include <map>
#include <list>
#include <vector>
using namespace std;

typedef map<string, unsigned>				MAP_TYPES;

typedef struct msg_id
{
    unsigned	id;
    unsigned	contact;
    unsigned	type;
    string		client;
} msg_id;

class QWidget;
class QTranslator;
class QMimeSource;
class Commands;
class MainWindow;
class UserView;
class SearchDialog;
class ExecManager;
class CommonStatus;
class StatusWnd;
class ConnectionManager;
class Icons;

typedef struct CoreData
{
    char		*Profile;
    unsigned	SavePasswd;
    unsigned	NoShow;
    unsigned	ShowPanel;
    unsigned	ManualStatus;
    unsigned	Invisible;
    long		geometry[4];
    long		toolBarState[7];
    void		*Buttons;
    void		*Menues;
    unsigned	ShowOnLine;
    unsigned	GroupMode;
    unsigned	UseDblClick;
    unsigned	UseSysColors;
    unsigned	ColorOnline;
    unsigned	ColorOffline;
    unsigned	ColorAway;
    unsigned	ColorNA;
    unsigned	ColorDND;
    unsigned	ColorGroup;
    unsigned	GroupSeparator;
    char		*Lang;
    unsigned	ContainerMode;
    unsigned	SendOnEnter;
    unsigned	containerSize[2];
    unsigned	containerBar[7];
    unsigned	ContainerStatusSize;
    char		*Containers;
    void		*Container;
    unsigned	CopyMessages;
    unsigned	EditHeight;
    unsigned	editBar[7];
    unsigned	EditBackground;
    unsigned	EditForeground;
    char		*EditFont;
    unsigned	OwnColors;
    unsigned	ColorSend;
    unsigned	ColorReceive;
    unsigned	ColorSender;
    unsigned	ColorReceiver;
    unsigned	UseSmiles;
    unsigned	CloseSend;
    unsigned	HistoryPage;
    unsigned	HistoryDirection;
    unsigned	historySize[2];
    long		historyBar[7];
    char		*HistorySearch;
    char		*Unread;
    void		*NoShowAutoReply;
    unsigned	SortLexic;
    unsigned	CloseTransfer;
} CoreData;

const unsigned CONTAINER_SIMPLE	= 0;
const unsigned CONTAINER_NEW	= 1;
const unsigned CONTAINER_GROUP	= 2;
const unsigned CONTAINER_ALL	= 3;

const unsigned CONTAINER_GRP	= 0x80000000;

typedef struct CoreUserData
{
    unsigned	LogStatus;
    unsigned	LogMessage;
    unsigned	OpenOnReceive;
    unsigned	OpenOnOnline;
    char		*IncomingPath;
    unsigned	AcceptMode;
    unsigned	OverwriteFiles;
    char		*DeclineMessage;
} CoreUserData;

typedef struct SMSUserData
{
    unsigned	SMSTranslit;
    char		*SMSSignatureBefore;
    char		*SMSSignatureAfter;
} SMSUserData;

typedef struct ARUserData
{
    void		*AutoReply;
} ARUserData;

typedef struct ListUserData
{
    unsigned	OfflineOpen;
    unsigned	OnlineOpen;
    unsigned	ShowAllways;
} ListUserData;

class ClientList : public vector<Client*>
{
public:
    ClientList();
    ~ClientList();
    void addToContacts();
};

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
const unsigned	CmdCustomInput			= (CmdBase + 19);
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
const unsigned	CmdHistoryFind			= (CmdBase + 50);
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
const unsigned	CmdCustomReceive		= (CmdBase + 64);
const unsigned	CmdNextMessage			= (CmdBase + 65);
const unsigned	CmdGrantAuth			= (CmdBase + 66);
const unsigned	CmdRefuseAuth			= (CmdBase + 67);
const unsigned	CmdSort					= (CmdBase + 68);
const unsigned	CmdSortStatus			= (CmdBase + 69);
const unsigned	CmdSortLexic			= (CmdBase + 70);
const unsigned	CmdPhones				= (CmdBase + 71);
const unsigned	CmdLocation				= (CmdBase + 72);
const unsigned	CmdPhoneState			= (CmdBase + 73);
const unsigned	CmdPhoneNoShow			= (CmdBase + 74);
const unsigned	CmdPhoneAvailable		= (CmdBase + 75);
const unsigned	CmdPhoneBusy			= (CmdBase + 76);
const unsigned  CmdPhoneBook			= (CmdBase + 77);
const unsigned	CmdShowAllways			= (CmdBase + 78);

const unsigned	CmdContactGroup			= (CmdBase + 0x100);
const unsigned	CmdUnread				= (CmdBase + 0x200);
const unsigned	CmdContactClients		= (CmdBase + 0x300);
const unsigned	CmdMsgSpecial			= (CmdBase + 0x400);
const unsigned	CmdClient				= (CmdBase + 0x500);

const unsigned	MenuConnections			= (CmdBase + 1);
const unsigned	MenuContainerContact	= (CmdBase + 2);
const unsigned  MenuStatus				= (CmdBase + 3);
const unsigned	MenuGroups				= (CmdBase + 4);
const unsigned	MenuMsgView				= (CmdBase + 5);
const unsigned	MenuTextEdit			= (CmdBase + 6);
const unsigned	MenuMsgCommand			= (CmdBase + 7);
const unsigned	MenuSort				= (CmdBase + 8);
const unsigned	MenuPhones				= (CmdBase + 9);
const unsigned	MenuLocation			= (CmdBase + 10);
const unsigned	MenuPhoneState			= (CmdBase + 11);

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

const unsigned	BarHistory				= (CmdBase + 1);
const unsigned	BarReceived				= (CmdBase + 2);

class MsgEdit;
class Tmpl;

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

const unsigned	MESSAGE_DEFAULT	= 0;
const unsigned	MESSAGE_SILENT	= 1;
const unsigned	MESSAGE_HIDDEN	= 2;

typedef struct MessageDef
{
    const CommandDef	*cmd;
    unsigned			flags;
    unsigned			base_type;
    const char			*singular;
    const char			*plural;
    Message*			(*create)(const char *cfg);
    QObject*			(*show)(QWidget *custom, Message *msg);
    QObject*			(*generate)(QWidget *custom, Message *msg);
    Message*			(*drag)(QMimeSource*);
} MessageDef;

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
    PROP_ULONG(ManualStatus)
    PROP_BOOL(Invisible)
    PROP_STRLIST(Buttons);
    PROP_STRLIST(Menues);
    PROP_BOOL(ShowOnLine);
    PROP_ULONG(GroupMode);
    PROP_BOOL(UseDblClick);
    PROP_BOOL(UseSysColors);
    PROP_ULONG(ColorOnline);
    PROP_ULONG(ColorOffline);
    PROP_ULONG(ColorAway);
    PROP_ULONG(ColorNA);
    PROP_ULONG(ColorDND);
    PROP_ULONG(ColorGroup);
    PROP_BOOL(GroupSeparator);
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
    PROP_BOOL(OwnColors);
    PROP_ULONG(ColorSend);
    PROP_ULONG(ColorReceive);
    PROP_ULONG(ColorSender);
    PROP_ULONG(ColorReceiver);
    PROP_BOOL(UseSmiles);
    PROP_BOOL(CloseSend);
    PROP_ULONG(HistoryPage);
    PROP_BOOL(HistoryDirection);
    PROP_UTF8(HistorySearch);
    PROP_STR(Unread);
    PROP_STRLIST(NoShowAutoReply);
    PROP_BOOL(SortLexic);
    PROP_BOOL(CloseTransfer);

    unsigned user_data_id;
    unsigned sms_data_id;
    unsigned ar_data_id;
    unsigned list_data_id;

    CommandsMap	preferences;
    CommandsMap	messageTypes;
    MAP_TYPES	types;

    list<msg_id>	unread;

    QFont editFont;
    static CorePlugin	*m_plugin;
    Message *createMessage(const char *type, const char *cfg);
    QString clientName(Client *client);
signals:
    void modeChanged();
protected slots:
    void dialogFinished();
    void dialogDestroy();
    void managerFinished();
    void destroyManager();
    void selectProfile();
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
    string typeName(const char *name);
    void setAutoReplies();

    CoreData	data;

    bool				m_bInit;
    vector<string>		m_profiles;
    QWidget				*m_cfg;
    UserView			*m_view;
    SearchDialog		*m_search;
    QTranslator			*m_translator;
    ConnectionManager	*m_manager;
    CommonStatus		*m_status;
    StatusWnd			*m_statusWnd;
    char				*m_profile;
    unsigned			m_nClients;
    unsigned			m_nClientsMenu;
    ExecManager			*m_exec;
    Commands			*m_cmds;
    MainWindow			*m_main;
    Icons				*m_icons;
    Tmpl				*m_tmpl;

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

