/***************************************************************************
                          jabber_rosters.cpp  -  description
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
#include "jabber.h"
#include "jabbermessage.h"

#include <time.h>

class RostersRequest : public JabberClient::ServerRequest
{
public:
    RostersRequest(JabberClient *client);
    ~RostersRequest();
protected:
    virtual void element_start(const char *el, const char **attr);
    virtual void element_end(const char *el);
    virtual void char_data(const char *str, int len);
    string		m_jid;
    string		m_name;
    string		m_grp;
    string		m_subscription;
    unsigned	m_subscribe;
    unsigned	m_bSubscription;
    string		*m_data;
};

RostersRequest::RostersRequest(JabberClient *client)
        : JabberClient::ServerRequest(client, _GET, NULL, NULL)
{
    m_data	= NULL;
    ContactList::ContactIterator itc;
    Contact *contact;
    while ((contact = ++itc) != NULL){
        ClientDataIterator it(contact->clientData, client);
        JabberUserData *data;
        while ((data = (JabberUserData*)(++it)) != NULL)
            data->bChecked = false;
    }
}

RostersRequest::~RostersRequest()
{
    ContactList::ContactIterator itc;
    Contact *contact;
    list<Contact*> contactRemoved;
    while ((contact = ++itc) != NULL){
        ClientDataIterator it(contact->clientData, m_client);
        JabberUserData *data;
        list<void*> dataRemoved;
        while ((data = (JabberUserData*)(++it)) != NULL){
            if (!data->bChecked){
                string jid;
                jid = data->ID;
                JabberListRequest *lr = m_client->findRequest(jid.c_str(), false);
                if (lr && lr->bDelete)
                    m_client->findRequest(jid.c_str(), true);
                dataRemoved.push_back(data);
            }
        }
        if (dataRemoved.empty())
            continue;
        for (list<void*>::iterator itr = dataRemoved.begin(); itr != dataRemoved.end(); ++itr)
            contact->clientData.freeData(*itr);
        if (contact->clientData.size() == 0)
            contactRemoved.push_back(contact);
    }
    for (list<Contact*>::iterator itr = contactRemoved.begin(); itr != contactRemoved.end(); ++itr)
        delete *itr;
    m_client->processList();
}

void RostersRequest::element_start(const char *el, const char **attr)
{
    if (strcmp(el, "item") == 0){
        m_subscribe = SUBSCRIBE_NONE;
        m_grp = "";
        m_jid = JabberClient::get_attr("jid", attr);
        if (m_jid.length() == 0)
            return;
        m_name = JabberClient::get_attr("name", attr);
        m_subscription  = "";
        m_bSubscription = false;
        string subscribe = JabberClient::get_attr("subscription", attr);
        if (subscribe == "none"){
            m_subscribe = SUBSCRIBE_NONE;
        }else if (subscribe == "from"){
            m_subscribe = SUBSCRIBE_FROM;
        }else if (subscribe == "to"){
            m_subscribe = SUBSCRIBE_TO;
        }else if (subscribe == "both"){
            m_subscribe = SUBSCRIBE_BOTH;
        }else{
            log(L_WARN, "Unknown attr subscribe=%s", subscribe.c_str());
        }
        return;
    }
    if (strcmp(el, "group") == 0){
        m_grp = "";
        m_data = &m_grp;
        return;
    }
    if (strcmp(el, "subscription") == 0){
        m_bSubscription = true;
        m_subscription = "";
        m_data = &m_subscription;
        return;
    }
}

void RostersRequest::element_end(const char *el)
{
    if (strcmp(el, "group") == 0){
        m_data = NULL;
        return;
    }
    if (strcmp(el, "item") == 0){
        bool bChanged = false;
        JabberListRequest *lr = m_client->findRequest(m_jid.c_str(), false);
        Contact *contact;
        JabberUserData *data = m_client->findContact(m_jid.c_str(), m_name.c_str(), false, contact);
        if (data == NULL){
            if (lr && lr->bDelete){
                m_client->findRequest(m_jid.c_str(), true);
            }else{
                bChanged = true;
                data = m_client->findContact(m_jid.c_str(), m_name.c_str(), true, contact);
                if (m_bSubscription){
                    contact->setTemporary(CONTACT_TEMP);
                    Event eContact(EventContactChanged, contact);
                    eContact.process();
                    m_client->auth_request(m_jid.c_str(), MessageAuthRequest, m_subscription.c_str(), true);
                    data = m_client->findContact(m_jid.c_str(), m_name.c_str(), false, contact);
                }
            }
        }
        if (data == NULL)
            return;
        if (data->Subscribe != m_subscribe){
            bChanged = true;
            data->Subscribe = m_subscribe;
        }
        set_str(&data->Group, m_grp.c_str());
        data->bChecked = true;
        if (lr == NULL){
            unsigned grp = 0;
            if (!m_grp.empty()){
                Group *group = NULL;
                ContactList::GroupIterator it;
                while ((group = ++it) != NULL){
                    if (m_grp == (const char*)(group->getName().utf8())){
                        grp = group->id();
                        break;
                    }
                }
                if (group == NULL){
                    group = getContacts()->group(0, true);
                    group->setName(QString::fromUtf8(m_grp.c_str()));
                    grp = group->id();
                    Event e(EventGroupChanged, group);
                    e.process();
                }
            }
            if (contact->getGroup() != grp){
                if (grp == 0){
                    void *d = NULL;
                    ClientDataIterator it_d(contact->clientData);
                    while ((d = ++it_d) != NULL){
                        if (d != data)
                            break;
                    }
                    if (d){
                        grp = contact->getGroup();
                        Group *group = getContacts()->group(grp);
                        if (group)
                            m_client->listRequest(data, contact->getName().utf8(), group->getName().utf8(), false);
                    }
                }
                contact->setGroup(grp);
                bChanged = true;
            }
        }
        if (bChanged){
            Event e(EventContactChanged, contact);
            e.process();
        }
    }
}

void RostersRequest::char_data(const char *str, int len)
{
    if (m_data == NULL)
        return;
    m_data->append(str, len);
}

void JabberClient::rosters_request()
{
    RostersRequest *req = new RostersRequest(this);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:roster");
    req->send();
    m_requests.push_back(req);
}

class InfoRequest : public JabberClient::ServerRequest
{
public:
    InfoRequest(JabberClient *client, JabberUserData *data);
    ~InfoRequest();
protected:
    virtual void element_start(const char *el, const char **attr);
    virtual void element_end(const char *el);
    virtual void char_data(const char *str, int len);
    string	m_jid;
    string	m_host;
    bool	m_bStarted;
    string  m_firstName;
    string	m_nick;
    string	m_desc;
    string	m_email;
    string	m_bday;
    string	m_url;
    string	m_orgName;
    string	m_orgUnit;
    string	m_title;
    string	m_role;
    string	m_phone;
    string	m_street;
    string  m_ext;
    string	m_city;
    string	m_region;
    string	m_pcode;
    string	m_country;
    string	*m_data;
};

InfoRequest::InfoRequest(JabberClient *client, JabberUserData *data)
        : JabberClient::ServerRequest(client, _GET, NULL, client->buildId(data).c_str())
{
    m_jid	= data->ID;
    m_bStarted = false;
    m_data = NULL;
}

InfoRequest::~InfoRequest()
{
    if (m_bStarted){
        Contact *contact = NULL;
        JabberUserData *data;
        if (m_jid == m_client->data.owner.ID){
            data = &m_client->data.owner;
        }else{
            string jid = m_jid;
            if (strchr(jid.c_str(), '@') == NULL){
                jid += "@";
                jid += m_host;
            }
            data = m_client->findContact(m_jid.c_str(), NULL, false, contact);
            if (data == NULL)
                return;
        }
        bool bChanged = false;
        bChanged |= set_str(&data->FirstName, m_firstName.c_str());
        bChanged |= set_str(&data->Nick, m_nick.c_str());
        bChanged |= set_str(&data->Desc, m_desc.c_str());
        bChanged |= set_str(&data->Bday, m_bday.c_str());
        bChanged |= set_str(&data->Url, m_url.c_str());
        bChanged |= set_str(&data->OrgName, m_orgName.c_str());
        bChanged |= set_str(&data->OrgUnit, m_orgUnit.c_str());
        bChanged |= set_str(&data->Title, m_title.c_str());
        bChanged |= set_str(&data->Role, m_role.c_str());
        bChanged |= set_str(&data->Street, m_street.c_str());
        bChanged |= set_str(&data->ExtAddr, m_ext.c_str());
        bChanged |= set_str(&data->City, m_city.c_str());
        bChanged |= set_str(&data->Region, m_region.c_str());
        bChanged |= set_str(&data->PCode, m_pcode.c_str());
        bChanged |= set_str(&data->Country, m_country.c_str());
        bChanged |= set_str(&data->EMail, m_email.c_str());
        bChanged |= set_str(&data->Phone, m_phone.c_str());
        if (bChanged){
            if (contact){
                m_client->setupContact(contact, data);
                Event e(EventContactChanged, (Client*)contact);
                e.process();
            }else{
                Event e(EventClientChanged, (Client*)m_client);
                e.process();
            }
        }
    }
}

void JabberClient::setupContact(Contact *contact, void *_data)
{
    JabberUserData *data = (JabberUserData*)_data;
    QString mail;
    if (data->EMail && *data->EMail)
        mail = QString::fromUtf8(data->EMail);
    contact->setEMails(mail, name().c_str());
    QString phones;
    if (data->Phone && *data->Phone){
        phones = QString::fromUtf8(data->Phone);
        phones += ",Home Phone,";
        phones += number(PHONE).c_str();
    }
    contact->setPhones(phones, name().c_str());

    if (contact->getFirstName().isEmpty() && data->FirstName && *data->FirstName)
        contact->setFirstName(QString::fromUtf8(data->FirstName), name().c_str());

    if (contact->getName().isEmpty())
        contact->setName(QString::fromUtf8(data->ID));
}

void InfoRequest::element_start(const char *el, const char**)
{
    m_data = NULL;
    if (!strcmp(el, "vcard")){
        m_bStarted = true;
        return;
    }
    if (!strcmp(el, "nick")){
        m_data = &m_nick;
        return;
    }
    if (!strcmp(el, "fn")){
        m_data = &m_firstName;
        return;
    }
    if (!strcmp(el, "desc")){
        m_data = &m_desc;
        return;
    }
    if (!strcmp(el, "email")){
        m_data = &m_email;
        return;
    }
    if (!strcmp(el, "bday")){
        m_data = &m_bday;
        return;
    }
    if (!strcmp(el, "url")){
        m_data = &m_url;
        return;
    }
    if (!strcmp(el, "orgname")){
        m_data = &m_orgName;
        return;
    }
    if (!strcmp(el, "orgunit")){
        m_data = &m_orgUnit;
        return;
    }
    if (!strcmp(el, "title")){
        m_data = &m_title;
        return;
    }
    if (!strcmp(el, "role")){
        m_data = &m_role;
        return;
    }
    if (!strcmp(el, "voice")){
        m_data = &m_phone;
        return;
    }
    if (!strcmp(el, "street")){
        m_data = &m_street;
        return;
    }
    if (!strcmp(el, "extadd")){
        m_data = &m_ext;
        return;
    }
    if (!strcmp(el, "city")){
        m_data = &m_city;
        return;
    }
    if (!strcmp(el, "region")){
        m_data = &m_region;
        return;
    }
    if (!strcmp(el, "pcode")){
        m_data = &m_pcode;
        return;
    }
    if (!strcmp(el, "country")){
        m_data = &m_country;
        return;
    }
}

void InfoRequest::element_end(const char*)
{
    m_data = NULL;
}

void InfoRequest::char_data(const char *str, int len)
{
    if (m_data)
        m_data->append(str, len);
}

void JabberClient::info_request(JabberUserData *user_data)
{
    if (getState() != Connected)
        return;
    if (user_data == NULL)
        user_data = &data.owner;
    /*
        InfoRequest *req = new InfoRequest(this, user_data);
        req->start_element("vCard");
        req->add_attribute("prodid", "-//HandGen//NONSGML vGen v1.0//EN");
        req->add_attribute("xmlns", "vcard-temp");
        req->add_attribute("version", "2.0");
        req->send();
        m_requests.push_back(req);
    */
}

class SetInfoRequest : public JabberClient::ServerRequest
{
public:
    SetInfoRequest(JabberClient *client, JabberUserData *data);
protected:
    virtual void element_start(const char *el, const char **attr);
    string	m_jid;
    string  m_firstName;
    string	m_nick;
    string	m_desc;
    string	m_bday;
    string	m_url;
    string	m_orgName;
    string	m_orgUnit;
    string	m_title;
    string	m_role;
    string	m_street;
    string	m_ext;
    string	m_city;
    string	m_region;
    string	m_pcode;
    string	m_country;
};

SetInfoRequest::SetInfoRequest(JabberClient *client, JabberUserData *data)
        : JabberClient::ServerRequest(client, _SET, NULL, NULL)
{
    m_jid	= data->ID;
    if (data->FirstName)
        m_firstName = data->FirstName;
    if (data->Nick)
        m_nick = data->Nick;
    if (data->Desc)
        m_desc = data->Desc;
    if (data->Bday)
        m_bday = data->Bday;
    if (data->Url)
        m_url = data->Url;
    if (data->OrgName)
        m_orgName = data->OrgName;
    if (data->OrgUnit)
        m_orgUnit = data->OrgUnit;
    if (data->Title)
        m_title = data->Title;
    if (data->Role)
        m_role = data->Role;
    if (data->Street)
        m_street = data->Street;
    if (data->ExtAddr)
        m_ext = data->ExtAddr;
    if (data->City)
        m_city = data->City;
    if (data->Region)
        m_region = data->Region;
    if (data->PCode)
        m_pcode = data->PCode;
    if (data->Country)
        m_country = data->Country;
}

void SetInfoRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "iq")){
        string type = JabberClient::get_attr("type", attr);
        if (type == "result"){
            set_str(&m_client->data.owner.FirstName, m_firstName.c_str());
            set_str(&m_client->data.owner.Nick, m_nick.c_str());
            set_str(&m_client->data.owner.Desc, m_desc.c_str());
            set_str(&m_client->data.owner.Bday, m_bday.c_str());
            set_str(&m_client->data.owner.Url, m_url.c_str());
            set_str(&m_client->data.owner.OrgName, m_orgName.c_str());
            set_str(&m_client->data.owner.OrgUnit, m_orgUnit.c_str());
            set_str(&m_client->data.owner.Title, m_title.c_str());
            set_str(&m_client->data.owner.Role, m_role.c_str());
            set_str(&m_client->data.owner.Street, m_street.c_str());
            set_str(&m_client->data.owner.ExtAddr, m_ext.c_str());
            set_str(&m_client->data.owner.City, m_city.c_str());
            set_str(&m_client->data.owner.Region, m_region.c_str());
            set_str(&m_client->data.owner.PCode, m_pcode.c_str());
            set_str(&m_client->data.owner.Country, m_country.c_str());
        }
    }
}

void JabberClient::setClientInfo(void *_data)
{
    JabberUserData *data = (JabberUserData*)_data;
    if (getState() != Connected)
        return;
    SetInfoRequest *req = new SetInfoRequest(this, data);
    req->start_element("vCard");
    req->add_attribute("prodid", "-//HandGen//NONSGML vGen v1.0//EN");
    req->add_attribute("xmlns", "vcard-temp");
    req->add_attribute("version", "2.0");
    req->text_tag("FN", data->FirstName);
    req->text_tag("NICKNAME", data->Nick);
    req->text_tag("DESC", data->Desc);
    QString mails = getContacts()->owner()->getEMails();
    while (mails.length()){
        QString mailItem = getToken(mails, ';', false);
        QString mail = getToken(mailItem, '/');
        if (mailItem.length())
            continue;
        req->text_tag("EMAIL", mail.utf8());
        break;
    }
    req->text_tag("BDAY", data->Bday);
    req->text_tag("URL", data->Url);
    req->start_element("ORG");
    req->text_tag("ORGNAME", data->OrgName);
    req->text_tag("ORGUNIT", data->OrgUnit);
    req->end_element();
    req->text_tag("TITLE", data->Title);
    req->text_tag("ROLE", data->Role);
    QString phone;
    QString phones = getContacts()->owner()->getPhones();
    while (phones.length()){
        QString phoneItem = getToken(phones, ';', false);
        QString phoneValue = getToken(phoneItem, '/', false);
        if (phoneItem.length())
            continue;
        QString number = getToken(phoneValue, ',');
        QString type = getToken(phoneValue, ',');
        if (type == "Hone Phone"){
            phone = number;
            break;
        }
    }
    if (phone.length()){
        req->start_element("TEL");
        req->start_element("HOME");
        req->end_element();
        req->text_tag("VOICE", phone);
        req->end_element();
    }
    req->start_element("ADDR");
    req->start_element("HOME");
    req->end_element();
    req->text_tag("STREET", data->Street);
    req->text_tag("EXTADD", data->ExtAddr);
    req->text_tag("LOCALITY", data->City);
    req->text_tag("REGION", data->Region);
    req->text_tag("PCODE", data->PCode);
    req->text_tag("COUNTRY", data->Country);
    req->send();
    m_requests.push_back(req);
}

class AddRequest : public JabberClient::ServerRequest
{
public:
    AddRequest(JabberClient *client, const char *jid);
protected:
    virtual void element_start(const char *el, const char **attr);
    string m_jid;
};

AddRequest::AddRequest(JabberClient *client, const char *jid)
        : JabberClient::ServerRequest(client, _SET, NULL, NULL)
{
    m_jid  = jid;
}

void AddRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "iq")){
        string type = JabberClient::get_attr("type", attr);
        if (type == "result"){
            Contact *contact;
            m_client->findContact(m_jid.c_str(), NULL, true, contact);
        }
    }
}

bool JabberClient::add_contact(const char *jid)
{
    Contact *contact;
    if (findContact(jid, NULL, false, contact)){
        Event e(EventContactChanged, contact);
        e.process();
        return false;
    }
    AddRequest *req = new AddRequest(this, jid);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:roster");
    req->start_element("item");
    req->add_attribute("jid", jid);
    req->send();
    m_requests.push_back(req);
    return true;
}

JabberClient::PresenceRequest::PresenceRequest(JabberClient *client)
        : ServerRequest(client, NULL, NULL, NULL)
{
}

JabberClient::PresenceRequest::~PresenceRequest()
{
    unsigned status = STATUS_UNKNOWN;
    /*
        m_type - flags after draft-ietf-xmpp-core-16 / 8.4.1
    */
    if (m_type == "unavailable"){
        status = STATUS_OFFLINE;
    }else if (m_type == "subscribe"){
        m_client->auth_request(m_from.c_str(), MessageAuthRequest, m_status.c_str(), true);
    }else if (m_type == "subscribed"){
        m_client->auth_request(m_from.c_str(), MessageAuthGranted, m_status.c_str(), true);
    }else if (m_type == "unsubscribe"){
        m_client->auth_request(m_from.c_str(), MessageRemoved, m_status.c_str(), true);
    }else if (m_type == "unsubscribed"){
        m_client->auth_request(m_from.c_str(), MessageAuthRefused, m_status.c_str(), true);
    }else if (m_type == "probe"){
        // server want to to know if we're living
        m_client->ping();
    }else if (m_type == "error"){
        log(L_DEBUG, "An error has occurred regarding processing or delivery of a previously-sent presence stanza");
    }else if (m_type.length() == 0){
        // m_show - flags after draft-ietf-xmpp-im-15 / 4.2
        status = STATUS_ONLINE;
        if (m_show == "away"){
            status = STATUS_AWAY;
        }else if (m_show == "chat"){
            status = STATUS_FFC;
        }else if (m_show == "xa"){
            status = STATUS_NA;
        }else if (m_show == "dnd"){
            status = STATUS_DND;
        }else if (m_show == "online"){
            status = STATUS_ONLINE;
        }else if (m_show.empty()){
            status = STATUS_ONLINE;
            if (m_status == "Online"){
                status = STATUS_ONLINE;
            }else if (m_status == "Disconnected"){
                status = STATUS_OFFLINE;
            }else if (m_status == "Connected"){
                status = STATUS_ONLINE;
            }else if (!m_status.empty()){
                log(L_DEBUG, "Unsupported status %s", m_status.c_str());
            }
        }else{
            log(L_DEBUG, "Unsupported available status %s", m_show.c_str());
        }
    }else{
        log(L_DEBUG, "Unsupported presence type %s", m_type.c_str());
    }
    if (status != STATUS_UNKNOWN){
        Contact *contact;
        JabberUserData *data = m_client->findContact(m_from.c_str(), NULL, false, contact);
        if (data){
            bool bOnLine = false;
            bool bChanged = set_str(&data->AutoReply, m_status.c_str());
            if (data->Status != status){
                time_t now;
                time(&now);
                bChanged = true;
                if ((status == STATUS_ONLINE) &&
                        ((now - m_client->data.owner.OnlineTime > 60) ||
                         (data->Status != STATUS_OFFLINE)))
                    bOnLine = true;
                if (data->Status == STATUS_OFFLINE)
                    data->OnlineTime = now;
                data->Status = status;
                data->StatusTime = now;
            }
            if (bChanged){
                StatusMessage m;
                m.setContact(contact->id());
                m.setClient(m_client->dataName(data).c_str());
                m.setFlags(MESSAGE_RECEIVED);
                m.setStatus(status);
                Event e(EventMessageReceived, &m);
                e.process();
            }
            if (bOnLine && !contact->getIgnore() && !m_client->isAgent(data->ID)){
                Event e(EventContactOnline, contact);
                e.process();
            }
        }
    }
}

void JabberClient::PresenceRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "presence")){
        m_from = JabberClient::get_attr("from", attr);
        m_type = JabberClient::get_attr("type", attr);
    }
    m_data = "";
}

void JabberClient::PresenceRequest::element_end(const char *el)
{
    if (!strcmp(el, "show")){
        m_show = m_data;
    }else if (!strcmp(el, "status")){
        m_status = m_data;
    }
}

void JabberClient::PresenceRequest::char_data(const char *str, int len)
{
    m_data.append(str, len);
}

JabberClient::IqRequest::IqRequest(JabberClient *client)
        : ServerRequest(client, NULL, NULL, NULL)
{
}

JabberClient::IqRequest::~IqRequest()
{
}

void JabberClient::IqRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "query"))
        m_query = JabberClient::get_attr("xmlns", attr);
    if (m_query == "jabber:iq:roster"){
        if (!strcmp(el, "item")){
            string jid = JabberClient::get_attr("jid", attr);
            string subscription = JabberClient::get_attr("subscription", attr);
            string name = JabberClient::get_attr("name", attr);
            if (!subscription.empty()){
                unsigned subscribe = SUBSCRIBE_NONE;
                if (subscription == "none"){
                    subscribe = SUBSCRIBE_NONE;
                }else if (subscription == "to"){
                    subscribe = SUBSCRIBE_TO;
                }else if (subscription == "from"){
                    subscribe = SUBSCRIBE_FROM;
                }else if (subscription == "both"){
                    subscribe = SUBSCRIBE_BOTH;
                }else if (subscription == "remove"){
                }else{
                    log(L_DEBUG, "Unknown value subscription=%s", subscription.c_str());
                }
                Contact *contact;
                JabberUserData *data = m_client->findContact(jid.c_str(), name.c_str(), false, contact);
                if ((data == NULL) && (subscribe != SUBSCRIBE_NONE))
                    data = m_client->findContact(jid.c_str(), name.c_str(), true, contact);
                if (data && (data->Subscribe != subscribe)){
                    data->Subscribe = subscribe;
                    Event e(EventContactChanged, contact);
                    e.process();
                }
            }
        }
    }
}

JabberClient::MessageRequest::MessageRequest(JabberClient *client)
        : ServerRequest(client, NULL, NULL, NULL)
{
    m_data = NULL;
}

JabberClient::MessageRequest::~MessageRequest()
{
    if (m_from.empty() || m_body.empty())
        return;
    Contact *contact;
    JabberUserData *data = m_client->findContact(m_from.c_str(), NULL, false, contact);
    if (data == NULL){
        data = m_client->findContact(m_from.c_str(), NULL, true, contact);
        if (data == NULL)
            return;
        contact->setTemporary(CONTACT_TEMP);
    }
    Message *msg;
    if (m_subj.empty()){
        msg = new Message(MessageGeneric);
    }else{
        JabberMessage *m = new JabberMessage;
        m->setSubject(QString::fromUtf8(m_subj.c_str()));
        msg = m;
    }
    msg->setText(QString::fromUtf8(m_body.c_str()));
    msg->setFlags(MESSAGE_RECEIVED);
    msg->setClient(m_client->dataName(data).c_str());
    msg->setContact(contact->id());
    Event e(EventMessageReceived, msg);
    if (!e.process())
        delete msg;
}

void JabberClient::MessageRequest::element_start(const char *el, const char **attr)
{
    m_data = NULL;
    if (!strcmp(el, "message")){
        m_from = JabberClient::get_attr("from", attr);
        return;
    }
    if (!strcmp(el, "body")){
        m_data = &m_body;
        return;
    }
    if (!strcmp(el, "subject")){
        m_data = &m_subj;
        return;
    }
}

void JabberClient::MessageRequest::element_end(const char*)
{
    m_data = NULL;
}

void JabberClient::MessageRequest::char_data(const char *str, int len)
{
    if (m_data)
        m_data->append(str, len);
}

class AgentRequest : public JabberClient::ServerRequest
{
public:
    AgentRequest(JabberClient *client);
    ~AgentRequest();
protected:
    JabberAgentsInfo	data;
    string m_data;
    virtual void element_start(const char *el, const char **attr);
    virtual void element_end(const char *el);
    virtual void char_data(const char *el, int len);
};

/*

typedef struct JabberAgentsInfo
{
	char			*VHost;
	char			*ID;
	char			*Name;
	unsigned		Search;
	unsigned		Register;
} JabberAgentsInfo;

*/

static DataDef jabberAgentsInfo[] =
    {
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_BOOL, 1, 0 },
        { "", DATA_BOOL, 1, 0 },
        { "", DATA_ULONG, 1, 0 },
        { NULL, 0, 0, 0 }
    };


AgentRequest::AgentRequest(JabberClient *client)
        : ServerRequest(client, _GET, NULL, client->VHost().c_str())
{
    load_data(jabberAgentsInfo, &data, NULL);
}

AgentRequest::~AgentRequest()
{
    free_data(jabberAgentsInfo, &data);
}

void AgentRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "agent")){
        free_data(jabberAgentsInfo, &data);
        load_data(jabberAgentsInfo, &data, NULL);
        m_data = JabberClient::get_attr("jid", attr);
        set_str(&data.ID, m_data.c_str());
    }else if (!strcmp(el, "search")){
        data.Search = (unsigned)(-1);
    }else if (!strcmp(el, "register")){
        data.Register = (unsigned)(-1);
    }
    m_data = "";
}

void AgentRequest::element_end(const char *el)
{
    if (!strcmp(el, "agent")){
        if (data.ID && *data.ID){
            set_str(&data.VHost, m_client->VHost().c_str());
            data.Client = m_client;
            Event e(static_cast<JabberPlugin*>(m_client->protocol()->plugin())->EventAgentFound, &data);
            e.process();
        }
    }else if (!strcmp(el, "name")){
        set_str(&data.Name, m_data.c_str());
    }
}

void AgentRequest::char_data(const char *el, int len)
{
    m_data.append(el, len);
}

void JabberClient::get_agents()
{
    AgentRequest *req = new AgentRequest(this);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:agents");
    req->send();
    m_requests.push_back(req);
}

class AgentInfoRequest : public JabberClient::ServerRequest
{
public:
    AgentInfoRequest(JabberClient *client, const char *jid);
    ~AgentInfoRequest();
protected:
    JabberAgentInfo		data;
    bool   m_bOption;
    string m_data;
    string m_jid;
    virtual void element_start(const char *el, const char **attr);
    virtual void element_end(const char *el);
    virtual void char_data(const char *el, int len);
};


/*

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

*/

static DataDef jabberAgentInfo[] =
    {
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRLIST, 1, 0 },
        { "", DATA_STRLIST, 1, 0 },
        { "", DATA_ULONG, 1, 0 },
        { NULL, 0, 0, 0 }
    };


AgentInfoRequest::AgentInfoRequest(JabberClient *client, const char *jid)
        : ServerRequest(client, _GET, NULL, jid)
{
    m_jid = jid;
    m_bOption = false;
    load_data(jabberAgentInfo, &data, NULL);
}

AgentInfoRequest::~AgentInfoRequest()
{
    free_data(jabberAgentInfo, &data);
}

void AgentInfoRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "field")){
        free_data(jabberAgentInfo, &data);
        load_data(jabberAgentInfo, &data, NULL);
        set_str(&data.ID, m_jid.c_str());
        m_data = JabberClient::get_attr("var", attr);
        set_str(&data.Field, m_data.c_str());
        m_data = JabberClient::get_attr("type", attr);
        set_str(&data.Type, m_data.c_str());
        m_data = JabberClient::get_attr("label", attr);
        set_str(&data.Label, m_data.c_str());
    }
    if (!strcmp(el, "option")){
        m_bOption = true;
        m_data = JabberClient::get_attr("label", attr);
        set_str(&data.OptionLabels, data.nOptions, m_data.c_str());
    }
    m_data = "";
}

void AgentInfoRequest::element_end(const char *el)
{
    if (!strcmp(el, "field")){
        if (data.Field && *data.Field){
            set_str(&data.VHost, m_client->VHost().c_str());
            Event e(static_cast<JabberPlugin*>(m_client->protocol()->plugin())->EventAgentInfo, &data);
            e.process();
        }
    }else if (!strcmp(el, "option")){
        m_bOption = false;
        const char *str = get_str(data.Options, data.nOptions);
        if (str && *str)
            data.nOptions++;
    }else if (!strcmp(el, "value")){
        if (m_bOption){
            set_str(&data.Options, data.nOptions, m_data.c_str());
        }else{
            set_str(&data.Value, m_data.c_str());
        }
    }else if (!strcmp(el, "key") || !strcmp(el, "instructions")){
        set_str(&data.Value, m_data.c_str());
        set_str(&data.ID, m_jid.c_str());
        set_str(&data.Type, el);
        Event e(static_cast<JabberPlugin*>(m_client->protocol()->plugin())->EventAgentInfo, &data);
        e.process();
    }else if (!strcmp(el, "username") || !strcmp(el, "password") || !strcmp(el, "nick")){
        set_str(&data.Value, m_data.c_str());
        set_str(&data.ID, m_jid.c_str());
        set_str(&data.Type, el);
        Event e(static_cast<JabberPlugin*>(m_client->protocol()->plugin())->EventAgentInfo, &data);
        e.process();
    }
}

void AgentInfoRequest::char_data(const char *el, int len)
{
    m_data.append(el, len);
}

void JabberClient::get_agent_info(const char *jid, const char *type)
{
    AgentInfoRequest *req = new AgentInfoRequest(this, jid);
    req->start_element("query");
    string xmlns = "jabber:iq:";
    xmlns += type;
    req->add_attribute("xmlns", xmlns.c_str());
    req->send();
    m_requests.push_back(req);
}

class SearchRequest : public JabberClient::ServerRequest
{
public:
    SearchRequest(JabberClient *client, const char *jid);
    ~SearchRequest();
protected:
    JabberSearchData data;
    string m_data;
    virtual void element_start(const char *el, const char **attr);
    virtual void element_end(const char *el);
    virtual void char_data(const char *el, int len);
};

/*

typedef struct JabberSearchData
{
	char			*ID;
	char			*JID;
	char			*First;
	char			*Last;
	char			*Nick;
	char			*EMail;
} JabberSearchData;

*/

static DataDef jabberSearchData[] =
    {
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { NULL, 0, 0, 0 }
    };


SearchRequest::SearchRequest(JabberClient *client, const char *jid)
        : ServerRequest(client, _SET, NULL, jid)
{
    load_data(jabberSearchData, &data, NULL);
}

SearchRequest::~SearchRequest()
{
    Event e(static_cast<JabberPlugin*>(m_client->protocol()->plugin())->EventSearchDone, (void*)m_id.c_str());
    e.process();
    free_data(jabberSearchData, &data);
}

void SearchRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "item")){
        free_data(jabberSearchData, &data);
        load_data(jabberSearchData, &data, NULL);
        m_data = JabberClient::get_attr("jid", attr);
        set_str(&data.JID, m_data.c_str());
    }
    m_data = "";
}

void SearchRequest::element_end(const char *el)
{
    if (!strcmp(el, "item")){
        if (data.JID && *data.JID){
            set_str(&data.ID, m_id.c_str());
            Event e(static_cast<JabberPlugin*>(m_client->protocol()->plugin())->EventSearch, &data);
            e.process();
        }
    }else if (!strcmp(el, "first")){
        set_str(&data.First, m_data.c_str());
    }else if (!strcmp(el, "last")){
        set_str(&data.Last, m_data.c_str());
    }else if (!strcmp(el, "nick")){
        set_str(&data.Nick, m_data.c_str());
    }else if (!strcmp(el, "email")){
        set_str(&data.EMail, m_data.c_str());
    }
}

void SearchRequest::char_data(const char *el, int len)
{
    m_data.append(el, len);
}

string JabberClient::search(const char *jid, const char *condition)
{
    SearchRequest *req = new SearchRequest(this, jid);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:search");
    req->add_condition(condition);
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

#if 0
I18N_NOOP("Password does not match");
I18N_NOOP("Low level network error");
#endif

class RegisterRequest : public JabberClient::ServerRequest
{
public:
    RegisterRequest(JabberClient *client, const char *jid);
    ~RegisterRequest();
protected:
    bool   m_bOK;
    string m_error;
    string *m_data;
    virtual void element_start(const char *el, const char **attr);
    virtual void element_end(const char *el);
    virtual void char_data(const char *el, int len);
};

RegisterRequest::RegisterRequest(JabberClient *client, const char *jid)
        : ServerRequest(client, _SET, NULL, jid)
{
    m_data = NULL;
    m_bOK  = false;
}

RegisterRequest::~RegisterRequest()
{
    agentRegisterInfo ai;
    ai.id = m_id.c_str();
    ai.bOK = m_bOK;
    ai.error = m_error.c_str();
    Event e(static_cast<JabberPlugin*>(m_client->protocol()->plugin())->EventAgentRegister, &ai);
    e.process();
}

void RegisterRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "error")){
        m_data = &m_error;
        return;
    }
    if (!strcmp(el, "iq")){
        string type = JabberClient::get_attr("type", attr);
        if (type == "result")
            m_bOK = true;
    }
}

void RegisterRequest::element_end(const char*)
{
    m_data = NULL;
}

void RegisterRequest::char_data(const char *el, int len)
{
    if (m_data == NULL)
        return;
    m_data->append(el, len);
}

string JabberClient::register_agent(const char *jid, const char *condition)
{
    RegisterRequest *req = new RegisterRequest(this, jid);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:register");
    req->add_condition(condition);
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

void JabberClient::processList()
{
    if (getState() != Connected)
        return;
    for (list<JabberListRequest>::iterator it = m_listRequests.begin(); it != m_listRequests.end(); ++it){
		JabberListRequest &r = (*it);
        JabberClient::ServerRequest *req = new JabberClient::ServerRequest(this, JabberClient::ServerRequest::_SET, NULL, NULL);
        req->start_element("query");
        req->add_attribute("xmlns", "jabber:iq:roster");
        req->start_element("item");
        req->add_attribute("jid", r.jid.c_str());
        if ((*it).bDelete)
            req->add_attribute("subscription", "remove");
        if (!(*it).name.empty())
            req->add_attribute("name", r.name.c_str());
        if (!(*it).bDelete)
            req->text_tag("group", r.grp.c_str());
        req->send();
        m_requests.push_back(req);
    }
    m_listRequests.clear();
}


