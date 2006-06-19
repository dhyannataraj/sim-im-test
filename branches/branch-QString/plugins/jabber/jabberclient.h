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

#include "sax.h"
#include "stl.h"
#include "socket.h"

class JabberProtocol;
class JabberClient;

const unsigned JABBER_SIGN		= 0x0002;

const unsigned SUBSCRIBE_NONE	= 0;
const unsigned SUBSCRIBE_FROM	= 1;
const unsigned SUBSCRIBE_TO		= 2;
const unsigned SUBSCRIBE_BOTH	= (SUBSCRIBE_FROM | SUBSCRIBE_TO);

struct JabberUserData : public SIM::clientData
{
    SIM::Data		ID;
    SIM::Data		Node;
    SIM::Data		Resource;
    SIM::Data		Name;
    SIM::Data		Status;
    SIM::Data		FirstName;
    SIM::Data		Nick;
    SIM::Data		Desc;
    SIM::Data		Bday;
    SIM::Data		Url;
    SIM::Data		OrgName;
    SIM::Data		OrgUnit;
    SIM::Data		Title;
    SIM::Data		Role;
    SIM::Data		Street;
    SIM::Data		ExtAddr;
    SIM::Data		City;
    SIM::Data		Region;
    SIM::Data		PCode;
    SIM::Data		Country;
    SIM::Data		EMail;
    SIM::Data		Phone;
    SIM::Data		StatusTime;
    SIM::Data		OnlineTime;
    SIM::Data		Subscribe;
    SIM::Data		Group;
    SIM::Data		bChecked;
    SIM::Data		TypingId;
    SIM::Data		composeId;
    SIM::Data		richText;
    SIM::Data		invisible;
    SIM::Data		PhotoWidth;
    SIM::Data		PhotoHeight;
    SIM::Data		LogoWidth;
    SIM::Data		LogoHeight;
    SIM::Data		nResources;
    SIM::Data		Resources;
    SIM::Data		ResourceStatus;
    SIM::Data		ResourceReply;
    SIM::Data		ResourceStatusTime;
    SIM::Data		ResourceOnlineTime;
    SIM::Data		AutoReply;
};

typedef struct JabberClientData
{
    SIM::Data		Server;
    SIM::Data		Port;
    SIM::Data		UseSSL;
    SIM::Data		UsePlain;
    SIM::Data		UseVHost;
    SIM::Data		Register;
    SIM::Data		Priority;
    SIM::Data		ListRequest;
    SIM::Data		VHost;
    SIM::Data		Typing;
    SIM::Data		RichText;
    SIM::Data		ProtocolIcons;
    SIM::Data		MinPort;
    SIM::Data		MaxPort;
    SIM::Data		Photo;
    SIM::Data		Logo;
    SIM::Data		AutoSubscribe;
    SIM::Data		AutoAccept;
    SIM::Data		UseHTTP;
    SIM::Data		URL;
    SIM::Data		InfoUpdated;
    JabberUserData	owner;
} JabberClientData;

typedef struct JabberAgentsInfo
{
    SIM::Data		VHost;
    SIM::Data		ID;
    SIM::Data		Name;
    SIM::Data		Search;
    SIM::Data		Register;
    JabberClient	*Client;
} JabberAgentsInfo;

typedef struct JabberAgentInfo
{
    SIM::Data		ReqID;
    SIM::Data		VHost;
    SIM::Data		ID;
    SIM::Data		Field;
    SIM::Data		Type;
    SIM::Data		Label;
    SIM::Data		Value;
    SIM::Data		Desc;
    SIM::Data		Options;
    SIM::Data		OptionLabels;
    SIM::Data		nOptions;
    SIM::Data		bRequired;
} JabberAgentInfo;

typedef struct JabberSearchData
{
    SIM::Data		ID;
    SIM::Data		JID;
    SIM::Data		First;
    SIM::Data		Last;
    SIM::Data		Nick;
    SIM::Data		EMail;
    SIM::Data		Status;
    SIM::Data		Fields;
    SIM::Data		nFields;
} JabberSearchData;

typedef struct JabberListRequest
{
    QString		    jid;
    QString		    grp;
    QString		    name;
    bool			bDelete;
} JabberListRequest;

typedef struct DiscoItem
{
    std::string			id;
    std::string			jid;
    std::string			node;
    std::string			name;
    std::string			type;
    std::string			category;
    std::string			features;
} DiscoItem;

class JabberClient : public SIM::TCPClient, public SAXParser
{
    Q_OBJECT
public:
    class ServerRequest
    {
    public:
        ServerRequest(JabberClient *client, const char *type, const char *from, const char *to, const char *id=NULL);
        virtual ~ServerRequest();
        void	send();
        void	start_element(const QString &name);
        void	end_element(bool bNewLevel = false);
        void	add_attribute(const QString &name, const QString &value);
        void	add_condition(const QString &cond, bool bXData);
        void	add_text(const QString &text);
        void	text_tag(const QString &name, const QString &value);
        static const char *_GET;
        static const char *_SET;
        static const char *_RESULT;
    protected:
        virtual void	element_start(const char *el, const char **attr);
        virtual void	element_end(const char *el);
        virtual	void	char_data(const char *str, int len);
        QString  		m_element;
        std::stack<QString>	m_els;
        std::string		m_id;
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
        std::string		*m_data;
        std::string		m_url;
        std::string		m_descr;
        std::string		m_query;
        std::string		m_from;
        std::string		m_id;
        std::string		m_file_name;
        unsigned		m_file_size;
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
        std::string m_from;
        std::string m_data;
        std::string m_type;
        std::string m_status;
        std::string m_show;
        std::string m_stamp1;
        std::string m_stamp2;
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
        std::string m_from;
        std::string *m_data;
        std::string m_body;
        std::string m_richText;
        std::string m_subj;
        std::string m_error;
        std::string m_contacts;
        std::string m_target;
        std::string m_desc;
        std::vector<std::string> m_targets;
        std::vector<std::string> m_descs;

        bool   m_bBody;
        bool   m_bRosters;
        bool   m_bError;
        std::string m_id;
        bool   m_bCompose;
        bool   m_bEvent;
        bool   m_bRichText;
        unsigned m_errorCode;
    };

    JabberClient(JabberProtocol*, ConfigBuffer *cfg);
    ~JabberClient();
    virtual QString name();
    virtual QString dataName(void*);
    virtual QWidget	*setupWnd();
    virtual QString getConfig();

    void setID(const QString &id);
    QString getID()
    {
        return data.owner.ID.str();
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
    PROP_BOOL(AutoSubscribe);
    PROP_BOOL(AutoAccept);
    PROP_BOOL(UseHTTP);
    PROP_STR(URL);
    PROP_BOOL(InfoUpdated);

    QString		    buildId(JabberUserData *data);
    JabberUserData	*findContact(const char *jid, const char *name, bool bCreate, SIM::Contact *&contact, std::string &resource, bool bJoin=true);
    bool			add_contact(const char *id, unsigned grp);
    std::string		get_agents(const char *jid);
    std::string		get_agent_info(const char *jid, const char *node, const char *type);
    void			auth_request(const char *jid, unsigned type, const char *text, bool bCreate);
    std::string		search(const char *jid, const char *node, const char *condition);
    std::string		process(const char *jid, const char *node, const char *condition, const char *type);

    static std::string	to_lower(const char *s);
    static std::string	get_attr(const char *name, const char **attrs);
    virtual void setupContact(SIM::Contact*, void *data);
    virtual void updateInfo(SIM::Contact *contact, void *data);

    JabberClientData	data;

    JabberListRequest *findRequest(const char *jid, bool bRemove);

    QString VHost();
    bool isAgent(const QString &jid);
    virtual bool send(SIM::Message*, void*);
    void    listRequest(JabberUserData *data, const char *name, const char *grp, bool bDelete);
    void	sendFileRequest(SIM::FileMessage *msg, unsigned short port, JabberUserData *data, const char *url, unsigned size);
    void	sendFileAccept(SIM::FileMessage *msg, JabberUserData *data);

    std::list<SIM::Message*> m_ackMsg;
    std::list<SIM::Message*> m_waitMsg;

    QString photoFile(JabberUserData*);
    QString logoFile(JabberUserData*);
    std::list<ServerRequest*>	m_requests;

    std::string discoItems(const char *jid, const char *node);
    std::string discoInfo(const char *jid, const char *node);
    std::string browse(const char *jid);
    std::string versionInfo(const char *jid, const char *node);
    std::string timeInfo(const char *jid, const char *node);
    std::string lastInfo(const char *jid, const char *node);
    std::string statInfo(const char *jid, const char *node);
    void addLang(ServerRequest *req);
    void info_request(JabberUserData *user_data, bool bVCard);
    virtual void setClientInfo(void *data);
    void changePassword(const char *pass);

protected slots:
    void	ping();
    void	auth_failed();
    void	auth_ok();
protected:
    virtual void *processEvent(SIM::Event *e);
    SIM::Socket *createSocket();

    virtual QString contactName(void *clientData);
    virtual void setStatus(unsigned status);
    void setStatus(unsigned status, const char *ar);
    virtual void disconnected();
    virtual void connect_ready();
    virtual void packet_ready();
    virtual void setInvisible(bool bState);
    virtual bool isMyData(SIM::clientData*&, SIM::Contact*&);
    virtual bool createData(SIM::clientData*&, SIM::Contact*);
    virtual bool compareData(void*, void*);
    virtual bool canSend(unsigned, void*);
    virtual void contactInfo(void *data, unsigned long &curStatus, unsigned &style, QString &statusIcon, QString *icons = NULL);
    virtual QString resources(void *data);
    virtual QString contactTip(void *data);
    virtual QWidget *searchWindow(QWidget *parent);
    virtual SIM::CommandDef *infoWindows(SIM::Contact *contact, void *data);
    virtual QWidget *infoWindow(QWidget *parent, SIM::Contact *contact, void *data, unsigned id);
    virtual SIM::CommandDef *configWindows();
    virtual QWidget *configWindow(QWidget *parent, unsigned id);

    void init();
    void sendPacket();
    void startHandshake();
    void connected();
    void handshake(const char *id);
    void rosters_request();
    void setOffline(JabberUserData *data);

    static	QString encodeXML(const QString &str);
    QString		m_id;
    unsigned	m_depth;

    QString		get_unique_id();
    unsigned	m_id_seed;
    unsigned	m_msg_id;

    bool		m_bHTTP;
    void		element_start(const char *el, const char **attr);
    void		element_end(const char *el);
    void		char_data(const char *str, int len);

    std::list<JabberListRequest>	m_listRequests;
    ServerRequest			*m_curRequest;

    const char *get_icon(JabberUserData *data, unsigned status, bool invisible);

    void		processList();

    void		auth_plain();
#ifdef USE_OPENSSL
    void		auth_digest();
#endif
    void		auth_register();
    bool		m_bSSL;
    bool		m_bJoin;

    friend class ServerRequest;
    friend class RostersRequest;
    friend class PresenceRequest;
    friend class JabberBrowser;
};

class JabberFileTransfer : public SIM::FileTransfer, public SIM::ClientSocketNotify, public SIM::ServerSocketNotify
{
public:
    JabberFileTransfer(SIM::FileMessage *msg, JabberUserData *data, JabberClient *client);
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
    virtual bool    error_state(const QString &err, unsigned code);
    virtual void	packet_ready();
    virtual void	connect_ready();
    virtual void	write_ready();
    virtual void	startReceive(unsigned pos);
    virtual void	bind_ready(unsigned short port);
    virtual bool	error(const char *err);
    virtual bool	accept(SIM::Socket *s, unsigned long ip);
    bool get_line(const char *str);
    void send_line(const char *str);
    unsigned m_startPos;
    unsigned m_endPos;
    unsigned m_answer;
    std::string			m_url;
    SIM::ClientSocket	*m_socket;
};

class JabberSearch;

typedef struct agentRegisterInfo
{
    const char		*id;
    unsigned		err_code;
    const char		*error;
} agentRegisterInfo;

#endif

