/***************************************************************************
                          jabberclient.h  -  description
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

#ifndef _JABBERCLIENT_H
#define _JABBERCLIENT_H

#include "simapi.h"
#include "stl.h"
#include "socket.h"

#include <expat.h>

class JabberProtocol;
class JabberClient;
class JabberBrowser;

const unsigned JABBER_SIGN		= 0x0002;

const unsigned SUBSCRIBE_NONE	= 0;
const unsigned SUBSCRIBE_FROM	= 1;
const unsigned SUBSCRIBE_TO		= 2;
const unsigned SUBSCRIBE_BOTH	= (SUBSCRIBE_FROM | SUBSCRIBE_TO);

typedef struct JabberUserData
{
    clientData	base;
    char		*ID;
    char		*Node;
    char		*Resource;
    char		*Name;
    unsigned	Status;
    char		*FirstName;
    char		*Nick;
    char		*Desc;
    char		*Bday;
    char		*Url;
    char		*OrgName;
    char		*OrgUnit;
    char		*Title;
    char		*Role;
    char		*Street;
    char		*ExtAddr;
    char		*City;
    char		*Region;
    char		*PCode;
    char		*Country;
    char		*EMail;
    char		*Phone;
    char		*AutoReply;
    unsigned	StatusTime;
    unsigned	OnlineTime;
    unsigned	Subscribe;
    char		*Group;
    unsigned	bChecked;
    char		*TypingId;
    unsigned	composeId;
    unsigned	richText;
    unsigned	invisible;
    unsigned long    PhotoWidth;
    unsigned long    PhotoHeight;
    unsigned long    LogoWidth;
    unsigned long    LogoHeight;
} JabberUserData;

typedef struct JabberClientData
{
    char			*Server;
    unsigned		Port;
    unsigned		UseSSL;
    unsigned		UsePlain;
    unsigned		UseVHost;
    unsigned		Register;
    unsigned		Priority;
    char			*ListRequest;
    char			*VHost;
    unsigned		Typing;
    unsigned		RichText;
    unsigned		ProtocolIcons;
    unsigned		MinPort;
    unsigned		MaxPort;
    char			*Photo;
    char			*Logo;
    long			browser_geo[5];
    long			browser_bar[7];
    char			*BrowserHistory;
    JabberUserData	owner;
} JabberClientData;

typedef struct JabberAgentsInfo
{
    char			*VHost;
    char			*ID;
    char			*Name;
    unsigned		Search;
    unsigned		Register;
    JabberClient	*Client;
} JabberAgentsInfo;

typedef struct JabberAgentInfo
{
    char			*ReqID;
    char			*VHost;
    char			*ID;
    char			*Field;
    char			*Type;
    char			*Label;
    char			*Value;
    void			*Options;
    void			*OptionLabels;
    unsigned		nOptions;
    unsigned		bRequired;
} JabberAgentInfo;

typedef struct JabberSearchData
{
    char			*ID;
    char			*JID;
    char			*First;
    char			*Last;
    char			*Nick;
    char			*EMail;
    void			*Fields;
    unsigned		nFields;
} JabberSearchData;

typedef struct JabberListRequest
{
    string			jid;
    string			grp;
    string			name;
    bool			bDelete;
} JabberListRequest;

typedef struct JabberDiscoItem
{
    string			id;
    string			jid;
    string			name;
    string			node;
} JabberDiscoItem;

class JabberClient : public TCPClient, public EventReceiver
{
    Q_OBJECT
public:
    class ServerRequest
    {
    public:
        ServerRequest(JabberClient *client, const char *type, const char *from, const char *to, const char *id=NULL);
        virtual ~ServerRequest();
        void	send();
        void	start_element(const char *name);
        void	end_element(bool bNewLevel = false);
        void	add_attribute(const char *name, const char *value);
        void	add_condition(const char *cond, bool bXData);
        void	add_text(const char *text);
        void	text_tag(const char *name, const char *value);
        static const char *_GET;
        static const char *_SET;
    protected:
        virtual void	element_start(const char *el, const char **attr);
        virtual void	element_end(const char *el);
        virtual	void	char_data(const char *str, int len);
        string			m_element;
        stack<string>	m_els;
        string			m_id;
        JabberClient	*m_client;
        friend class JabberClient;
    };

class IqRequest : public ServerRequest
    {
    public:
        IqRequest(JabberClient *client);
        ~IqRequest();
    public:
        virtual void	element_start(const char *el, const char **attr);
        virtual void	element_end(const char *el);
        virtual	void	char_data(const char *str, int len);
        string	*m_data;
        string  m_url;
        string	m_descr;
        string  m_query;
        string	m_from;
        string  m_id;
    };

class PresenceRequest : public ServerRequest
    {
    public:
        PresenceRequest(JabberClient *client);
        ~PresenceRequest();
    protected:
        virtual void element_start(const char *el, const char **attr);
        virtual void element_end(const char *el);
        virtual void char_data(const char *str, int len);
        string m_from;
        string m_data;
        string m_type;
        string m_status;
        string m_show;
    };

class MessageRequest : public ServerRequest
    {
    public:
        MessageRequest(JabberClient *client);
        ~MessageRequest();
    protected:
        virtual void element_start(const char *el, const char **attr);
        virtual void element_end(const char *el);
        virtual void char_data(const char *str, int len);
        string m_from;
        string *m_data;
        string m_body;
        string m_richText;
        string m_subj;
        string m_error;
        string m_contacts;
        string m_target;
        string m_desc;
        vector<string> m_targets;
        vector<string> m_descs;
        bool   m_bBody;
        bool   m_bRosters;
        string m_id;
        bool   m_bCompose;
        bool   m_bEvent;
        bool   m_bRichText;
        unsigned m_errorCode;
    };

    JabberClient(JabberProtocol*, const char *cfg);
    ~JabberClient();
    virtual string name();
    virtual QWidget	*setupWnd();
    virtual string getConfig();

    void setID(const QString &id);
    QString getID()
    {
        return QString::fromUtf8(data.owner.ID ? data.owner.ID : "");
    }
    PROP_STR(Server);
    PROP_STR(VHost);
    PROP_USHORT(Port);
    PROP_BOOL(UseSSL);
    PROP_BOOL(UsePlain);
    PROP_BOOL(UseVHost);
    PROP_BOOL(Register);
    PROP_ULONG(Priority);
    PROP_UTF8(ListRequest);
    PROP_BOOL(Typing);
    PROP_BOOL(RichText);
    PROP_BOOL(ProtocolIcons);
    PROP_USHORT(MinPort);
    PROP_USHORT(MaxPort);
    PROP_UTF8(Photo);
    PROP_UTF8(Logo);
    PROP_UTF8(BrowserHistory);

    string		buildId(JabberUserData *data);
    JabberUserData	*findContact(const char *jid, const char *name, bool bCreate, Contact *&contact);
    bool		add_contact(const char *id, unsigned grp);
    void		get_agents();
    string		get_agent_info(const char *jid, const char *node, const char *type);
    void		auth_request(const char *jid, unsigned type, const char *text, bool bCreate);
    string		search(const char *jid, const char *node, const char *condition);
    string		process(const char *jid, const char *node, const char *condition, const char *type);

    static string	to_lower(const char *s);
    static string	get_attr(const char *name, const char **attrs);
    virtual void setupContact(Contact*, void *data);
    virtual void updateInfo(Contact *contact, void *data);

    JabberClientData	data;
    string dataName(void*);

    JabberListRequest *findRequest(const char *jid, bool bRemove);

    string VHost();
    bool isAgent(const char *jid);
    virtual bool send(Message*, void*);
    void    listRequest(JabberUserData *data, const char *name, const char *grp, bool bDelete);
    void	sendFileRequest(FileMessage *msg, unsigned short port, JabberUserData *data, const char *url);

    list<Message*>  m_ackMsg;
    list<Message*>	m_waitMsg;

    QString photoFile(JabberUserData*);
    QString logoFile(JabberUserData*);
    list<ServerRequest*>	m_requests;
    JabberBrowser   *m_browser;

    string discoItems(const char *jid, const char *node);
    string discoInfo(const char *jid, const char *node);
    string versionInfo(const char *jid, const char *node);
    string timeInfo(const char *jid, const char *node);
    string lastInfo(const char *jid, const char *node);
    string statInfo(const char *jid, const char *node);
    void addLang(ServerRequest *req);
    void info_request(JabberUserData *user_data, bool bVCard);
    virtual void setClientInfo(void *data);

protected slots:
    void	ping();
    void	auth_failed();
    void	auth_ok();
protected:
    virtual void *processEvent(Event *e);

    virtual QString contactName(void *clientData);
    virtual void setStatus(unsigned status);
    void setStatus(unsigned status, const char *ar);
    virtual void disconnected();
    virtual void connect_ready();
    virtual void packet_ready();
    virtual void setInvisible(bool bState);
    virtual bool isMyData(clientData*&, Contact*&);
    virtual bool createData(clientData*&, Contact*);
    virtual bool compareData(void*, void*);
    virtual bool canSend(unsigned, void*);
    virtual void contactInfo(void *data, unsigned long &curStatus, unsigned &style, const char *&statusIcon, string *icons = NULL);
    virtual QString contactTip(void *data);
    virtual QWidget *searchWindow();
    virtual CommandDef *infoWindows(Contact *contact, void *data);
    virtual QWidget *infoWindow(QWidget *parent, Contact *contact, void *data, unsigned id);
    virtual CommandDef *configWindows();
    virtual QWidget *configWindow(QWidget *parent, unsigned id);

    void init();
    void sendPacket();
    void startHandshake();
    void connected();
    void handshake(const char *id);
    void rosters_request();
    void setOffline(JabberUserData *data);

    static	QCString encodeXML(const QString &str);
    XML_Parser	m_parser;
    string		m_id;
    unsigned	m_depth;

    string		get_unique_id();
    unsigned	m_id_seed;
    unsigned	m_msg_id;

    bool		m_bXML;
    void		element_start(const char *el, const char **attr);
    void		element_end(const char *el);
    void		char_data(const char *str, int len);
    static void p_element_start(void *data, const char *el, const char **attr);
    static void p_element_end(void *data, const char *el);
    static void p_char_data(void *data, const char *str, int len);

    list<JabberListRequest>	m_listRequests;
    ServerRequest			*m_curRequest;

    void		processList();

    void		auth_plain();
#ifdef USE_OPENSSL
    void		auth_digest();
#endif
    void		auth_register();
    bool		m_bSSL;

    friend class ServerRequest;
    friend class RostersRequest;
    friend class PresenceRequest;
    friend class JabberBrowser;
};

class JabberFileTransfer : public FileTransfer, public ClientSocketNotify, public ServerSocketNotify
{
public:
    JabberFileTransfer(FileMessage *msg, JabberUserData *data, JabberClient *client);
    ~JabberFileTransfer();
    void listen();
    void connect();
protected:
    JabberClient	*m_client;
    JabberUserData	*m_data;
    enum State
    {
        None,
        Listen,
        ListenWait,
        Header,
        Send,
        Wait,
        Connect,
        ReadHeader,
        Receive
    };
    State m_state;
    virtual bool    error_state(const char *err, unsigned code);
    virtual void	packet_ready();
    virtual void	connect_ready();
    virtual void	write_ready();
    virtual void	startReceive(unsigned pos);
    virtual void	bind_ready(unsigned short port);
    virtual bool	error(const char *err);
    virtual bool	accept(Socket *s, unsigned long ip);
    bool get_line(const char *str);
    void send_line(const char *str);
    unsigned m_startPos;
    unsigned m_endPos;
    unsigned m_answer;
    string   m_url;
    ClientSocket	*m_socket;
};

class JabberSearch;

typedef struct agentInfo
{
    JabberSearch	*search;
    string			name;
} agentInfo;

typedef struct agentRegisterInfo
{
    const char		*id;
    unsigned		err_code;
    const char		*error;
} agentRegisterInfo;

class my_string : public string
{
public:
my_string(const char *str) : string(str) {}
    bool operator < (const my_string &str) const;
};

typedef map<my_string, agentInfo> AGENTS_MAP;

#endif

