/***************************************************************************
                          fetch.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#ifndef _FETCH_H
#define _FETCH_H

#include "simapi.h"
#include "buffer.h"
#include "socket.h"
#include "stl.h"

typedef map<my_string, string> HEADERS_MAP;

class FetchClient : public ClientSocketNotify
{
public:
    FetchClient(const char *url, Buffer *postData, const char *headers, bool bRedirect);
    ~FetchClient();
    unsigned id() { return m_id; }
    bool	 done()	{ return m_bDone; }
    void	 fail();
    string		m_uri;
    string		m_hIn;
    HEADERS_MAP	m_hOut;
    unsigned	m_code;
    Buffer		m_res;
    Buffer		*m_post;
	bool		m_bRedirect;
protected:
    virtual bool error_state(const char *err, unsigned code);
    virtual void connect_ready();
    virtual void packet_ready();
    bool read_line(string&);
    ClientSocket *m_socket;
    unsigned	m_id;
    bool		m_bDone;
	bool		crackUrl(const char *url, string &proto, string &host, unsigned short &port, string &user, string &pass, string &uri, string &extra);
	void		fetch();
    unsigned	m_size;
    void		addHeader(const char *key, const char *value);
	bool		findHeader(const char *key);
    enum State{
#ifdef USE_OPENSSL
        SSLConnect,
#endif
        None,
        Header,
        Data,
		Redirect
    };
    State		m_state;
#ifdef USE_OPENSSL
    bool		m_bHTTPS;
#endif
};

class FetchManager : public QObject, public EventReceiver
{
	Q_OBJECT
public:
    FetchManager();
    ~FetchManager();
    static FetchManager *manager;
	bool useWinInet();
	void remove(FetchClient*);
protected slots:
	void remove();
protected:
    void *processEvent(Event*);
    list<FetchClient*> m_clients;
	list<FetchClient*> m_remove;
    unsigned m_id;
    friend class FetchClient;
};

EXPORT string basic_auth(const char *user, const char *pass);
EXPORT bool get_connection_state(bool &state);
EXPORT bool get_connection_proxy();

#endif

