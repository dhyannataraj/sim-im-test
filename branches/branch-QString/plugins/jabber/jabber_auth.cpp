/***************************************************************************
                          jabber_auth.cpp  -  description
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

#include "jabberclient.h"

#include <qtimer.h>

using std::string;
using namespace SIM;

class AuthRequest : public JabberClient::ServerRequest
{
public:
    AuthRequest(JabberClient *client);
protected:
    bool m_bFail;
    virtual void element_start(const char *el, const char **attr);
    virtual void element_end(const char *el);
};

AuthRequest::AuthRequest(JabberClient *client)
        : JabberClient::ServerRequest(client, _SET, NULL, client->VHost().c_str())
{
    m_bFail = true;
}

void AuthRequest::element_end(const char *el)
{
    if (strcmp(el, "iq"))
        return;
    if (m_bFail){
        QTimer::singleShot(0, m_client, SLOT(auth_failed()));
    }else{
        QTimer::singleShot(0, m_client, SLOT(auth_ok()));
    }
}

void AuthRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "iq")){
        string value = JabberClient::to_lower(JabberClient::get_attr("type", attr).c_str());
        if (value == "result")
            m_bFail = false;
    }
}

void JabberClient::auth_plain()
{
    AuthRequest *req = new AuthRequest(this);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:auth");
    QString username = data.owner.ID.ptr;
    username = getToken(username, '@');
    req->text_tag("username", username);
    req->text_tag("password", getPassword());
    req->text_tag("resource", data.owner.Resource.ptr);
    req->send();
    m_requests.push_back(req);
}

void JabberClient::auth_register()
{
    AuthRequest *req = new AuthRequest(this);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:register");
    QString username = data.owner.ID.ptr;
    username = getToken(username, '@');
    req->text_tag("username", username);
    req->text_tag("password", getPassword());
    req->send();
    m_requests.push_back(req);
}

#ifdef USE_OPENSSL

void JabberClient::auth_digest()
{
    AuthRequest *req = new AuthRequest(this);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:auth");
    QString username = data.owner.ID.ptr;
    username = getToken(username, '@');
    req->text_tag("username", username);

    QString digest = m_id;
    digest += getPassword();
    QByteArray md = sha1(digest.utf8());
    digest = "";
    for (unsigned i = 0; i < md.size(); i++){
        char b[3];
        sprintf(b, "%02x", md[(int)i] & 0xFF);
        digest += b;
    }
    req->text_tag("digest", digest);
    req->text_tag("resource", data.owner.Resource.ptr);
    req->send();
    m_requests.push_back(req);
}

#endif
