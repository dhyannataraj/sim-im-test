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
#include "socket.h"

#include <expat.h>

#include <stack>
#include <map>

using namespace std;

class JabberProtocol;
class JabberClient;

const unsigned JABBER_SIGN		= 0x0002;

const unsigned SUBSCRIBE_NONE	= 0;
const unsigned SUBSCRIBE_FROM	= 1;
const unsigned SUBSCRIBE_TO		= 2;
const unsigned SUBSCRIBE_BOTH	= (SUBSCRIBE_FROM | SUBSCRIBE_TO);

typedef struct JabberUserData
{
    clientData	base;
    char		*ID;
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
    unsigned		ProtocolIcons;
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
    char			*VHost;
    char			*ID;
    char			*Field;
    char			*Type;
    char			*Label;
    char			*Value;
    void			*Options;
    void			*OptionLabels;
    unsigned		nOptions;
} JabberAgentInfo;

typedef struct JabberSearchData
{
    char			*ID;
    char			*JID;
    char			*First;
    char			*Last;
    char			*Nick;
    char			*EMail;
} JabberSearchData;

typedef struct JabberListRequest
{
    string			jid;
    string			grp;
    string			name;
    bool			bDelete;
} JabberListRequest;

class JabberClient : public TCPClient, public EventReceiver
{
    Q_OBJECT
public:
    class ServerRequest
    {
    public:
        ServerRequest(JabberClient *client, const char *type, const char *from, const char *to);
        virtual ~ServerRequest();
        void	send();
        void	start_element(const char *name);
        void	end_element(bool bNewLevel = false);
        void	add_attribute(const char *name, const char *value);
        void	add_condition(const char *cond);
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
        string m_query;
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
        bool   m_bBody;
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
    PROP_ULONG(Port);
    PROP_BOOL(UseSSL);
    PROP_BOOL(UsePlain);
    PROP_BOOL(UseVHost);
    PROP_BOOL(Register);
    PROP_ULONG(Priority);
    PROP_UTF8(ListRequest);
    PROP_BOOL(Typing);
    PROP_BOOL(ProtocolIcons);

    string		buildId(JabberUserData *data);
    JabberUserData	*findContact(const char *jid, const char *name, bool bCreate, Contact *&contact);
    bool		add_contact(const char *id);
    void		get_agents();
    void		get_agent_info(const char *jid, const char *type);
    void		auth_request(const char *jid, unsigned type, const char *text, bool bCreate);
    string		search(const char *jid, const char *condition);
    string		register_agent(const char *jid, const char *condition);

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
    virtual void setClientInfo(void *data);

    void init();
    void sendPacket();
    void startHandshake();
    void connected();
    void handshake(const char *id);
    void rosters_request();
    void info_request(JabberUserData *user_data);
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

    list<ServerRequest*>	m_requests;
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
    bool			bOK;
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

