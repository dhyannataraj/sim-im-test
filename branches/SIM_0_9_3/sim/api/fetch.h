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

class Buffer;

const unsigned NO_POSTSIZE	= (unsigned)(-1);

class FetchClientPrivate;

class EXPORT FetchClient
{
public:
    FetchClient();
    virtual ~FetchClient();
    void fetch(const char *url, const char *headers = NULL, Buffer *postData = NULL, bool bRedirect = true);
    virtual const char *read_data(char *buf, unsigned &size);
    virtual bool     write_data(const char *buf, unsigned size);
    virtual unsigned post_size();
    virtual bool	 done(unsigned code, Buffer &data, const char *headers) = 0;
    bool	isDone();
    void    stop();
    void	set_speed(unsigned speed);
    static bool	crackUrl(const char *url, string &proto, string &host, unsigned short &port, string &user, string &pass, string &uri, string &extra);
private:
    FetchClientPrivate *p;
    friend class FetchClientPrivate;
    friend class FetchManager;
};

class FetchManager : public QObject
{
    Q_OBJECT
public:
    FetchManager();
    ~FetchManager();
    static FetchManager *manager;
    void done(FetchClient *client);
protected slots:
    void timeout();
};

EXPORT string basic_auth(const char *user, const char *pass);
EXPORT bool get_connection_state(bool &state);

#endif

