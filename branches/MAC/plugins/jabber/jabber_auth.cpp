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

#ifdef USE_OPENSSL
#include <openssl/sha.h>
#endif

#include <qtimer.h>

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
    string username = data.owner.ID;
    username = getToken(username, '@');
    req->text_tag("username", username.c_str());
    req->text_tag("password", getPassword());
    req->text_tag("resource", data.owner.Resource);
    req->send();
    m_requests.push_back(req);
}

void JabberClient::auth_register()
{
    AuthRequest *req = new AuthRequest(this);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:register");
    string username = data.owner.ID;
    username = getToken(username, '@');
    req->text_tag("username", username.c_str());
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
    string username = data.owner.ID;
    username = getToken(username, '@');
    req->text_tag("username", username.c_str());

    string digest = m_id;
    digest += getPassword().utf8();
    SHA_CTX c;
    unsigned char md[SHA_DIGEST_LENGTH];
    SHA1_Init(&c);
    SHA1_Update(&c, digest.c_str(),(unsigned long)digest.length());
    SHA1_Final(md, &c);
    digest = "";
    for (unsigned i = 0; i < SHA_DIGEST_LENGTH; i++){
        char b[3];
        sprintf(b, "%02x",md[i]);
        digest += b;
    }

    req->text_tag("digest", digest.c_str());
    req->text_tag("resource", data.owner.Resource);
    req->send();
    m_requests.push_back(req);
}

#endif
