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
#define _FECTH_H

#include "simapi.h"
#include "buffer.h"
#include "socket.h"

#include <list>
#include <map>

using namespace std;

class my_string : public string
{
public:
    my_string(const char *str) : string(str) {}
    bool operator < (const my_string &str) const;
};

typedef map<my_string, string> HEADERS_MAP;

class FetchClient : public ClientSocketNotify
{
public:
	FetchClient(TCPClient *client, const char *url, Buffer *postData, const char *headers);
	~FetchClient();
	unsigned id() { return m_id; }
	bool	 done()	{ return m_bDone; }
	void	 fail();
	TCPClient	 *m_client;
protected:
    virtual bool error_state(const char *err, unsigned code);
    virtual void connect_ready();
    virtual void packet_ready();
	bool read_line(string&);
	ClientSocket *m_socket;
	unsigned	m_id;
	HEADERS_MAP	m_hOut;
	string		m_hIn;
	bool		m_bDone;
	unsigned	m_port;
	unsigned	m_size;
	string		m_host;
	string		m_uri;
	void		addHeader(const char *key, const char *value);
	Buffer		*m_post;
	Buffer		m_res;
	enum State{
#ifdef USE_OPENSSL
		SSLConnect,
#endif
		None,
		Header,
		Data
	};
	State		m_state;
	unsigned	m_code;
#ifdef USE_OPENSSL
	bool		m_bHTTPS;
#endif
};

class FetchManager : public EventReceiver
{
public:
	FetchManager();
	~FetchManager();
	static FetchManager *manager;
protected:
	void *processEvent(Event*);
	list<FetchClient*> m_clients;
	unsigned m_id;
	friend class FetchClient;
};

#endif

