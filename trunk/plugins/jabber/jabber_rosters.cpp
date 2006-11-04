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
#include "html.h"
#include "core.h"

#include <time.h>

#ifdef WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include <qimage.h>
#include <qfile.h>

using namespace std;
using namespace SIM;

class RostersRequest : public JabberClient::ServerRequest
{
public:
    RostersRequest(JabberClient *client);
    ~RostersRequest();
protected:
    virtual void element_start(const char *el, const char **attr);
    virtual void element_end(const char *el);
    virtual void char_data(const char *str, int len);
    QString     m_jid;
    QString     m_name;
    QString     m_grp;
    QString     m_subscription;
    unsigned    m_subscribe;
    unsigned    m_bSubscription;
    QString    *m_data;
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
            data->bChecked.asBool() = false;
    }
    client->m_bJoin = false;
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
            if (!data->bChecked.toBool()){
                QString jid = data->ID.str();
                JabberListRequest *lr = m_client->findRequest(jid, false);
                if (lr && lr->bDelete)
                    m_client->findRequest(jid, true);
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
    if (m_client->m_bJoin){
        Event e(EventJoinAlert, m_client);
        e.process();
    }
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
        QString subscribe = JabberClient::get_attr("subscription", attr);
        if (subscribe == "none"){
            m_subscribe = SUBSCRIBE_NONE;
        }else if (subscribe == "from"){
            m_subscribe = SUBSCRIBE_FROM;
        }else if (subscribe == "to"){
            m_subscribe = SUBSCRIBE_TO;
        }else if (subscribe == "both"){
            m_subscribe = SUBSCRIBE_BOTH;
        }else{
            log(L_WARN, "Unknown attr subscribe=%s", subscribe.latin1());
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
        JabberListRequest *lr = m_client->findRequest(m_jid, false);
        Contact *contact;
        QString resource;
        JabberUserData *data = m_client->findContact(m_jid, m_name, false, contact, resource);
        if (data == NULL){
            if (lr && lr->bDelete){
                m_client->findRequest(m_jid, true);
            }else{
                bChanged = true;
                data = m_client->findContact(m_jid, m_name, true, contact, resource);
                if (m_bSubscription){
                    contact->setFlags(CONTACT_TEMP);
                    Event eContact(EventContactChanged, contact);
                    eContact.process();
                    m_client->auth_request(m_jid, MessageAuthRequest, m_subscription, true);
                    data = m_client->findContact(m_jid, m_name, false, contact, resource);
                }
            }
        }
        if (data == NULL)
            return;
        if (data->Subscribe.toULong() != m_subscribe){
            bChanged = true;
            data->Subscribe.asULong() = m_subscribe;
        }
        data->Group.str() = m_grp;
        data->bChecked.asBool() = true;
        if (lr == NULL){
            unsigned grp = 0;
            if (!m_grp.isEmpty()){
                Group *group = NULL;
                ContactList::GroupIterator it;
                while ((group = ++it) != NULL){
                    if (m_grp == group->getName()){
                        grp = group->id();
                        break;
                    }
                }
                if (group == NULL){
                    group = getContacts()->group(0, true);
                    group->setName(m_grp);
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
                            m_client->listRequest(data, contact->getName(), group->getName(), false);
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
    m_data->append(QString::fromUtf8(str, len));
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
    InfoRequest(JabberClient *client, JabberUserData *data, bool bVCard);
    ~InfoRequest();
protected:
    virtual void element_start(const char *el, const char **attr);
    virtual void element_end(const char *el);
    virtual void char_data(const char *str, int len);
    QString	m_jid;
    QString  m_node;
    QString	m_host;
    bool	m_bStarted;
    QString  m_firstName;
    QString	m_nick;
    QString	m_desc;
    QString	m_email;
    QString	m_bday;
    QString	m_url;
    QString	m_orgName;
    QString	m_orgUnit;
    QString	m_title;
    QString	m_role;
    QString	m_phone;
    QString	m_street;
    QString  m_ext;
    QString	m_city;
    QString	m_region;
    QString	m_pcode;
    QString	m_country;
    QString	*m_data;
    Buffer	m_photo;
    Buffer	m_logo;
    Buffer	*m_cdata;
    bool	m_bPhoto;
    bool	m_bLogo;
    bool	m_bVCard;
};

extern DataDef jabberUserData[];

InfoRequest::InfoRequest(JabberClient *client, JabberUserData *data, bool bVCard)
        : JabberClient::ServerRequest(client, _GET, NULL, client->buildId(data))
{
    m_jid   = data->ID.str();
    m_node  = data->Node.str();
    m_bStarted = false;
    m_data  = NULL;
    m_cdata = NULL;
    m_bPhoto = false;
    m_bLogo  = false;
    m_bVCard = bVCard;
}

InfoRequest::~InfoRequest()
{
    if (m_bStarted){
        Contact *contact = NULL;
        JabberUserData *data;
        JabberUserData u_data;
        if (m_bVCard){
            load_data(jabberUserData, &u_data, NULL);
            data = &u_data;
            data->ID.str() = m_jid;
            data->Node.str() = m_node;
        }else{
            if (m_jid == m_client->data.owner.ID.str()){
                data = &m_client->data.owner;
            }else{
                QString jid = m_jid;
                if (jid.find('@') == -1){
                    jid += "@";
                    jid += m_host;
                }
                QString resource;
                data = m_client->findContact(jid, QString::null, false, contact, resource);
                if (data == NULL)
                    return;
            }
        }
        bool bChanged = false;
        bChanged |= data->FirstName.setStr(m_firstName);
        bChanged |= data->Nick.setStr(m_nick);
        bChanged |= data->Desc.setStr(m_desc);
        bChanged |= data->Bday.setStr(m_bday);
        bChanged |= data->Url.setStr(m_url);
        bChanged |= data->OrgName.setStr(m_orgName);
        bChanged |= data->OrgUnit.setStr(m_orgUnit);
        bChanged |= data->Title.setStr(m_title);
        bChanged |= data->Role.setStr(m_role);
        bChanged |= data->Street.setStr(m_street);
        bChanged |= data->ExtAddr.setStr(m_ext);
        bChanged |= data->City.setStr(m_city);
        bChanged |= data->Region.setStr(m_region);
        bChanged |= data->PCode.setStr(m_pcode);
        bChanged |= data->Country.setStr(m_country);
        bChanged |= data->EMail.setStr(m_email);
        bChanged |= data->Phone.setStr(m_phone);

        if (m_bVCard){
            Event e(EventVCard, data);
            e.process();
            free_data(jabberUserData, &u_data);
            return;
        }
        QImage photo;
        if (m_photo.size()){
            Buffer unpack;
            unpack.fromBase64(m_photo);
            QString fName = m_client->photoFile(data);
            QFile f(fName);
            if (f.open(IO_WriteOnly | IO_Truncate)){
                f.writeBlock(unpack.data(), unpack.size());
                f.close();
                photo.load(fName);
            }else{
                log(L_ERROR, "Can't create %s", (const char*)fName.local8Bit());
            }
        }
        if (photo.width() && photo.height()){
            if ((photo.width() != (int)(data->PhotoWidth.toLong())) ||
                    (photo.height() != (int)(data->PhotoHeight.toLong())))
                bChanged = true;
            data->PhotoWidth.asLong()  = photo.width();
            data->PhotoHeight.asLong() = photo.height();
            if (m_jid == m_client->data.owner.ID.str())
                m_client->setPhoto(m_client->photoFile(data));
        }else{
            if (data->PhotoWidth.toLong() || data->PhotoHeight.toLong())
                bChanged = true;
            data->PhotoWidth.asLong()  = 0;
            data->PhotoHeight.asLong() = 0;
        }

        QImage logo;
        if (m_logo.size()){
            Buffer unpack;
            unpack.fromBase64(m_logo);
            QString fName = m_client->logoFile(data);
            QFile f(fName);
            if (f.open(IO_WriteOnly | IO_Truncate)){
                f.writeBlock(unpack.data(), unpack.size());
                f.close();
                logo.load(fName);
            }else{
                log(L_ERROR, "Can't create %s", (const char*)fName.local8Bit());
            }
        }
        if (logo.width() && logo.height()){
            if ((logo.width() != (int)(data->LogoWidth.toLong())) ||
                    (logo.height() != (int)(data->LogoHeight.toLong())))
                bChanged = true;
            data->LogoWidth.asLong()  = logo.width();
            data->LogoHeight.asLong() = logo.height();
            if (m_jid == m_client->data.owner.ID.str())
                m_client->setLogo(m_client->logoFile(data));
        }else{
            if (data->LogoWidth.toLong() || data->LogoHeight.toLong())
                bChanged = true;
            data->LogoWidth.asLong()  = 0;
            data->LogoHeight.asLong() = 0;
        }

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

void InfoRequest::element_start(const char *element, const char**)
{
    m_data = NULL;
    QString el = element ? QString::fromUtf8(element) : "";
    if (el == "vcard"){
        m_bStarted = true;
        return;
    }
    if (el == "nickname"){
        m_data = &m_nick;
        return;
    }
    if (el == "fn"){
        m_data = &m_firstName;
        return;
    }
    if (el == "desc"){
        m_data = &m_desc;
        return;
    }
    if (el == "email"){
        m_data = &m_email;
        return;
    }
    if (el == "bday"){
        m_data = &m_bday;
        return;
    }
    if (el == "url"){
        m_data = &m_url;
        return;
    }
    if (el == "orgname"){
        m_data = &m_orgName;
        return;
    }
    if (el == "orgunit"){
        m_data = &m_orgUnit;
        return;
    }
    if (el == "title"){
        m_data = &m_title;
        return;
    }
    if (el == "role"){
        m_data = &m_role;
        return;
    }
    if (el == "voice"){
        m_data = &m_phone;
        return;
    }
    if (el == "street"){
        m_data = &m_street;
        return;
    }
    if (el == "extadd"){
        m_data = &m_ext;
        return;
    }
    if (el == "city"){
        m_data = &m_city;
        return;
    }
    if (el == "region"){
        m_data = &m_region;
        return;
    }
    if (el == "pcode"){
        m_data = &m_pcode;
        return;
    }
    if (el == "country"){
        m_data = &m_country;
        return;
    }
    if (el == "photo"){
        m_bPhoto = true;
        return;
    }
    if (el == "logo"){
        m_bLogo = true;
        return;
    }
    if (el == "binval"){
        if (m_bPhoto)
            m_cdata = &m_photo;
        if (m_bLogo)
            m_cdata = &m_logo;
    }
}

void InfoRequest::element_end(const char *element)
{
    m_data  = NULL;
    m_cdata = NULL;
    QString el = element ? QString::fromUtf8(element) : "";
    if (el == "photo"){
        m_bPhoto = false;
        return;
    }
    if (el == "logo"){
        m_bLogo = false;
        return;
    }
}

void InfoRequest::char_data(const char *str, int len)
{
    if (m_cdata){
        m_cdata->pack(str, len);
        return;
    }
    if (m_data)
        m_data->append(QString::fromUtf8(str, len));
}

void JabberClient::info_request(JabberUserData *user_data, bool bVCard)
{
    if (getState() != Connected)
        return;
    if (user_data == NULL)
        user_data = &data.owner;
    InfoRequest *req = new InfoRequest(this, user_data, bVCard);
    req->start_element("vCard");
    req->add_attribute("prodid", "-//HandGen//NONSGML vGen v1.0//EN");
    req->add_attribute("xmlns", "vcard-temp");
    req->add_attribute("version", "2.0");
    if (!user_data->Node.str().isEmpty())
        req->add_attribute("node", user_data->Node.str());
    req->send();
    m_requests.push_back(req);
}

class SetInfoRequest : public JabberClient::ServerRequest
{
public:
    SetInfoRequest(JabberClient *client, JabberUserData *data);
protected:
    virtual void element_start(const char *el, const char **attr);
};

SetInfoRequest::SetInfoRequest(JabberClient *client, JabberUserData *data)
        : JabberClient::ServerRequest(client, _SET, NULL, client->buildId(data))
{
}

void SetInfoRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "iq")){
        QString type = JabberClient::get_attr("type", attr);
        if (type == "result")
            m_client->setInfoUpdated(false);
    }
}

void JabberClient::setClientInfo(void *_data)
{
    JabberUserData *data = static_cast<JabberUserData*>(_data);
    if (data != &this->data.owner){
        this->data.owner.FirstName.str() = data->FirstName.str();
        this->data.owner.Nick.str() = data->Nick.str();
        this->data.owner.Desc.str() = data->Desc.str();
        this->data.owner.Bday.str() = data->Bday.str();
        this->data.owner.Url.str() = data->Url.str();
        this->data.owner.OrgName.str() = data->OrgName.str();
        this->data.owner.OrgUnit.str() = data->OrgUnit.str();
        this->data.owner.Title.str() = data->Title.str();
        this->data.owner.Role.str() = data->Role.str();
        this->data.owner.Street.str() = data->Street.str();
        this->data.owner.ExtAddr.str() = data->ExtAddr.str();
        this->data.owner.City.str() = data->City.str();
        this->data.owner.Region.str() = data->Region.str();
        this->data.owner.PCode.str() = data->PCode.str();
        this->data.owner.Country.str() = data->Country.str();
    }
    setInfoUpdated(true);
    if (getState() != Connected)
        return;
    SetInfoRequest *req = new SetInfoRequest(this, &this->data.owner);
    req->start_element("vCard");
    req->add_attribute("prodid", "-//HandGen//NONSGML vGen v1.0//EN");
    req->add_attribute("xmlns", "vcard-temp");
    req->add_attribute("version", "2.0");
    req->add_attribute("node", data->Node.str());
    req->text_tag("FN", data->FirstName.str());
    req->text_tag("NICKNAME", data->Nick.str());
    req->text_tag("DESC", data->Desc.str());
    QString mails = getContacts()->owner()->getEMails();
    while (mails.length()){
        QString mailItem = getToken(mails, ';', false);
        QString mail = getToken(mailItem, '/');
        if (mailItem.length())
            continue;
        req->text_tag("EMAIL", mail);
        break;
    }
    req->text_tag("BDAY", data->Bday.str());
    req->text_tag("URL", data->Url.str());
    req->start_element("ORG");
    req->text_tag("ORGNAME", data->OrgName.str());
    req->text_tag("ORGUNIT", data->OrgUnit.str());
    req->end_element();
    req->text_tag("TITLE", data->Title.str());
    req->text_tag("ROLE", data->Role.str());
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
    req->text_tag("STREET", data->Street.str());
    req->text_tag("EXTADD", data->ExtAddr.str());
    req->text_tag("LOCALITY", data->City.str());
    req->text_tag("REGION", data->Region.str());
    req->text_tag("PCODE", data->PCode.str());
    req->text_tag("COUNTRY", data->Country.str());
    req->end_element();
    if (!getPhoto().isEmpty()){
        QFile img(getPhoto());
        if (img.open(IO_ReadOnly)){
            Buffer b;
            b.init(img.size());
            img.readBlock(b.data(), b.size());
            Buffer packed;
            packed.toBase64(b);
            packed << (char)0;
            req->start_element("PHOTO");
            req->text_tag("BINVAL", packed.data());
            req->end_element();
        }
    }
    if (!getLogo().isEmpty()){
        QFile img(getLogo());
        if (img.open(IO_ReadOnly)){
            Buffer b;
            b.init(img.size());
            img.readBlock(b.data(), b.size());
            Buffer packed;
            packed.toBase64(b);
            packed << (char)0;
            req->start_element("LOGO");
            req->text_tag("BINVAL", packed.data());
            req->end_element();
        }
    }
    req->send();
    m_requests.push_back(req);
}

class AddRequest : public JabberClient::ServerRequest
{
public:
    AddRequest(JabberClient *client, const QString &jid, unsigned grp);
protected:
    virtual void element_start(const char *el, const char **attr);
    QString m_jid;
    unsigned m_grp;
};

AddRequest::AddRequest(JabberClient *client, const QString &jid, unsigned grp)
        : JabberClient::ServerRequest(client, _SET, NULL, NULL)
{
    m_jid = jid;
    m_grp = grp;
}

void AddRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "iq")){
        QString type = JabberClient::get_attr("type", attr);
        if (type == "result"){
            Contact *contact;
            QString resource;
            JabberUserData *data = m_client->findContact(m_jid, QString::null, true, contact, resource);
            if (data && (contact->getGroup() != m_grp)){
                contact->setGroup(m_grp);
                Event e(EventContactChanged, contact);
                e.process();
            }
        }
    }
}

bool JabberClient::add_contact(const char *_jid, unsigned grp)
{
    Contact *contact;
    QString resource;
    QString jid = QString::fromUtf8(_jid);
    if (findContact(jid, QString::null, false, contact, resource)){
        Event e(EventContactChanged, contact);
        e.process();
        return false;
    }
    AddRequest *req = new AddRequest(this, jid, grp);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:roster");
    req->start_element("item");
    req->add_attribute("jid", jid);
    Group *g = NULL;
    if (grp)
        g = getContacts()->group(grp);
    if (g)
        req->text_tag("group", g->getName());
    req->send();
    m_requests.push_back(req);
    return true;
}

JabberClient::PresenceRequest::PresenceRequest(JabberClient *client)
        : ServerRequest(client, NULL, NULL, NULL)
{
}

static unsigned get_number(QString &s, unsigned digits)
{
    if (s.length() < digits){
        s = "";
        return 0;
    }
    QString p = s.left(digits);
    s = s.mid(digits);
    return p.toUInt();
}

static time_t fromDelay(const QString &t)
{
    QString s = t;
    time_t now = time(NULL);
    struct tm _tm = *localtime(&now);
    _tm.tm_year = get_number(s, 4) - 1900;
    _tm.tm_mon  = get_number(s, 2) - 1;
    _tm.tm_mday = get_number(s, 2);
    get_number(s, 1);
    _tm.tm_hour = get_number(s, 2);
    get_number(s, 1);
    _tm.tm_min  = get_number(s, 2);
    get_number(s, 1);
    _tm.tm_sec  = get_number(s, 2);
    return mktime(&_tm);
}

JabberClient::PresenceRequest::~PresenceRequest()
{
    unsigned status = STATUS_UNKNOWN;
    bool bInvisible = false;
    // RFC 3921 "XMPP IM": 2.2.1. Types of Presence
    if (m_type == "unavailable"){
        status = STATUS_OFFLINE;
    }else if (m_type == "subscribe"){
        m_client->auth_request(m_from, MessageAuthRequest, m_status, true);
    }else if (m_type == "subscribed"){
        m_client->auth_request(m_from, MessageAuthGranted, m_status, true);
    }else if (m_type == "unsubscribe"){
        m_client->auth_request(m_from, MessageRemoved, m_status, true);
    }else if (m_type == "unsubscribed"){
        m_client->auth_request(m_from, MessageAuthRefused, m_status, true);
    }else if (m_type == "probe"){
        // server want to to know if we're living
        m_client->ping();
    }else if (m_type == "error"){
        log(L_DEBUG, "An error has occurred regarding processing or delivery of a previously-sent presence stanza");
    }else if (m_type.length() == 0){
        // RFC 3921 "XMPP IM": 2.2.2.1. Show
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
        }else if (m_show.isEmpty()){
            // RFC 3921 "XMPP IM": 2.2.2.2. Status
            status = STATUS_ONLINE;
            if (m_status == "Online"){
                status = STATUS_ONLINE;
            }else if (m_status == "Disconnected"){
                status = STATUS_OFFLINE;
            }else if (m_status == "Connected"){
                status = STATUS_ONLINE;
            }else if (m_status == "Invisible"){
                status = STATUS_ONLINE;
                bInvisible = true;
            }else if (!m_status.isEmpty()){
                status = STATUS_ONLINE;
            }
        }else{
            log(L_DEBUG, "Unsupported available status %s", m_show.latin1());
        }
    }else{
        log(L_DEBUG, "Unsupported presence type %s", m_type.latin1());
    }
    time_t time1 = time(NULL);
    time_t time2 = 0;
    if (!m_stamp1.isEmpty())
        time1 = fromDelay(m_stamp1);
    if (!m_stamp2.isEmpty()){
        time2 = fromDelay(m_stamp2);
        if (time2 > time1){
            time_t t = time1;
            time1 = time2;
            time2 = t;
        }
    }

    if (status != STATUS_UNKNOWN){
        Contact *contact;
        QString resource;
        JabberUserData *data = m_client->findContact(m_from, QString::null, false, contact, resource);
        if (data){
            unsigned i;
            for (i = 1; i <= data->nResources.toULong(); i++){
                if (resource == get_str(data->Resources, i))
                    break;
            }
            bool bChanged = false;
            if (status == STATUS_OFFLINE){
                if (i <= data->nResources.toULong()){
                    bChanged = true;
                    vector<QString> resources;
                    vector<QString> resourceReply;
                    vector<QString> resourceStatus;
                    vector<QString> resourceStatusTime;
                    vector<QString> resourceOnlineTime;
                    vector<QString> resourceClientName;
                    vector<QString> resourceClientVersion;
                    vector<QString> resourceClientOS;
                    for (unsigned n = 1; n <= data->nResources.toULong(); n++){
                        if (i == n)
                            continue;
                        resources.push_back(get_str(data->Resources, n));
                        resourceReply.push_back(get_str(data->ResourceReply, n));
                        resourceStatus.push_back(get_str(data->ResourceStatus, n));
                        resourceStatusTime.push_back(get_str(data->ResourceStatusTime, n));
                        resourceOnlineTime.push_back(get_str(data->ResourceOnlineTime, n));
                        resourceClientName.push_back(get_str(data->ResourceClientName, n));
                        resourceClientVersion.push_back(get_str(data->ResourceClientVersion, n));
                        resourceClientOS.push_back(get_str(data->ResourceClientOS, n));
                    }
                    data->Resources.clear();
                    data->ResourceReply.clear();
                    data->ResourceStatus.clear();
                    data->ResourceStatusTime.clear();
                    data->ResourceOnlineTime.clear();
                    data->ResourceClientName.clear();
                    data->ResourceClientVersion.clear();
                    data->ResourceClientOS.clear();
                    for (i = 0; i < resources.size(); i++){
                        set_str(&data->Resources, i + 1, resources[i]);
                        set_str(&data->ResourceReply, i + 1, resourceReply[i]);
                        set_str(&data->ResourceStatus, i + 1, resourceStatus[i]);
                        set_str(&data->ResourceStatusTime, i + 1, resourceStatusTime[i]);
                        set_str(&data->ResourceOnlineTime, i + 1, resourceOnlineTime[i]);
                        set_str(&data->ResourceClientName, i + 1, resourceClientName[i]);
                        set_str(&data->ResourceClientVersion, i + 1, resourceClientVersion[i]);
                        set_str(&data->ResourceClientOS, i + 1, resourceClientOS[i]);
                    }
                    data->nResources.asULong() = resources.size();
                }
                if (data->nResources.toULong() == 0)
                    data->AutoReply.str() = m_status;
            }else{
                if (i > data->nResources.toULong()){
                    bChanged = true;
                    data->nResources.asULong() = i;
                    set_str(&data->Resources, i, resource);
                    set_str(&data->ResourceOnlineTime, i, QString::number(time2 ? time2 : time1));
                    m_client->versionInfo(m_from);
                }
                if (QString::number(status) != get_str(data->ResourceStatus, i)){
                    bChanged = true;
                    set_str(&data->ResourceStatus, i, QString::number(status));
                    set_str(&data->ResourceStatusTime, i, QString::number(time1));
                }
                if (m_status != get_str(data->ResourceReply, i)){
                    bChanged = true;
                    set_str(&data->ResourceReply, i, m_status);
                }
            }
            bool bOnLine = false;
            status = STATUS_OFFLINE;
            for (i = 1; i <= data->nResources.toULong(); i++){
                unsigned rStatus = get_str(data->ResourceStatus, i).toUInt();
                if (rStatus > status)
                    status = rStatus;
            }
            if (data->Status.toULong() != status){
                bChanged = true;
                if ((status == STATUS_ONLINE) &&
                        (((int)(time1 - m_client->data.owner.OnlineTime.toULong()) > 60) ||
                         (data->Status.toULong() != STATUS_OFFLINE)))
                    bOnLine = true;
                if (data->Status.toULong() == STATUS_OFFLINE){
                    data->OnlineTime.asULong() = time1;
                    data->richText.asBool() = true;
                }
                if (status == STATUS_OFFLINE && data->IsTyping.toBool()){
                    data->IsTyping.asBool() = false;
                    Event e(EventContactStatus, contact);
                    e.process();
                }
                data->Status.asULong() = status;
                data->StatusTime.asULong() = time1;
            }
            if (data->invisible.toBool() != bInvisible){
                data->invisible.asBool() = bInvisible;
                bChanged = true;
            }
            if (bChanged){
                StatusMessage *m = new StatusMessage();
                m->setContact(contact->id());
                m->setClient(m_client->dataName(data));
                m->setFlags(MESSAGE_RECEIVED);
                m->setStatus(status);
                Event e(EventMessageReceived, m);
                if(!e.process())
                    delete m;
            }
            if (bOnLine && !contact->getIgnore() && !m_client->isAgent(data->ID.str())){
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
    if (!strcmp(el, "x")){
        if (JabberClient::get_attr("xmlns", attr) == "jabber:x:delay"){
            QString stamp = JabberClient::get_attr("stamp", attr);
            if (!stamp.isEmpty()){
                if (m_stamp1.isEmpty()){
                    m_stamp1 = stamp;
                }else if (m_stamp2.isEmpty()){
                    m_stamp2 = stamp;
                }
            }
        }
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
    m_data.append(QString::fromUtf8(str, len));
}

JabberClient::IqRequest::IqRequest(JabberClient *client)
        : ServerRequest(client, NULL, NULL, NULL)
{
    m_data = NULL;
    m_file_size = 0;
}

JabberClient::IqRequest::~IqRequest()
{
    JabberFileMessage *msg = NULL;
    if (m_query == "jabber:iq:oob"){
        QString proto = m_url.left(7);
        if (proto != "http://"){
            log(L_WARN, "Unknown protocol");
            return;
        }
        m_url = m_url.mid(7);
        int n = m_url.find(':');
        if (n < 0){
            log(L_WARN, "Port not found");
            return;
        }
        QString host = m_url.left(n);
        unsigned short port = (unsigned short)m_url.mid(n + 1).toLong();
        n = m_url.find('/');
        if (n < 0){
            log(L_WARN, "File not found");
            return;
        }
        QString file = m_url.mid(n + 1);
        msg = new JabberFileMessage;
        msg->setDescription(file);
        msg->setText(m_descr);
        msg->setHost(host);
        msg->setPort(port);
    }else if (!m_file_name.isEmpty()){
        msg = new JabberFileMessage;
        msg->setDescription(m_file_name);
        msg->setSize(m_file_size);
    }
    if (msg){
        Contact *contact;
        QString resource;
        JabberUserData *data = m_client->findContact(m_from, QString::null, false, contact, resource);
        if (data == NULL){
            data = m_client->findContact(m_from, QString::null, true, contact, resource);
            if (data == NULL)
                return;
            contact->setFlags(CONTACT_TEMP);
        }
        msg->setFrom(m_from);
        msg->setID(m_id);
        msg->setFlags(MESSAGE_RECEIVED | MESSAGE_TEMP);
        msg->setClient(m_client->dataName(data));
        msg->setContact(contact->id());
        m_client->m_ackMsg.push_back(msg);
        Event e(EventMessageReceived, msg);
        if (e.process()){
            for (list<Message*>::iterator it = m_client->m_ackMsg.begin(); it != m_client->m_ackMsg.end(); ++it){
                if ((*it) == msg){
                    m_client->m_ackMsg.erase(it);
                    break;
                }
            }
        }
    }
}

void JabberClient::IqRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "iq")){
        m_from = JabberClient::get_attr("from", attr);
        m_id   = JabberClient::get_attr("id", attr);
        m_type = JabberClient::get_attr("type", attr);
        return;
    }
    if (!strcmp(el, "query")){
        m_query = JabberClient::get_attr("xmlns", attr);
        if (m_query == "jabber:iq:roster"){
            if (!strcmp(el, "item")){
                QString jid = JabberClient::get_attr("jid", attr);
                QString subscription = JabberClient::get_attr("subscription", attr);
                QString name = JabberClient::get_attr("name", attr);
                if (!subscription.isEmpty()){
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
                        log(L_DEBUG, "Unknown value subscription=%s", subscription.latin1());
                    }
                    Contact *contact;
                    QString resource;
                    JabberUserData *data = m_client->findContact(jid, name, false, contact, resource);
                    if ((data == NULL) && (subscribe != SUBSCRIBE_NONE)){
                        data = m_client->findContact(jid, name, true, contact, resource);
                    }
                    if (data && (data->Subscribe.toULong() != subscribe)){
                        data->Subscribe.asULong() = subscribe;
                        Event e(EventContactChanged, contact);
                        e.process();
                        if (m_client->getAutoSubscribe() && ((subscribe & SUBSCRIBE_FROM) == 0)){
                            AuthMessage *msg = new AuthMessage(MessageAuthRequest);
                            msg->setContact(contact->id());
                            msg->setFlags(MESSAGE_NOHISTORY);
                            m_client->send(msg, data);
                        }
                    }
                }
            }
        // XEP-0092: Software Version
        }else if (m_query == "jabber:iq:version"){
            if (m_type == "get"){
                // send our version
                JabberClient::ServerRequest *req = new JabberClient::ServerRequest(m_client, JabberClient::ServerRequest::_RESULT, NULL, m_from, m_id);
                req->start_element("query");
                req->add_attribute("xmlns", "jabber:iq:version");
                req->text_tag("name", PACKAGE);
                req->text_tag("version", VERSION);
                QString version = get_os_version();
                req->text_tag("os", version);
                req->send();
                m_client->m_requests.push_back(req);
            }
        }
    }
    if (!strcmp(el, "url"))
        m_data = &m_url;
    if (!strcmp(el, "desc"))
        m_data = &m_descr;
    if (!strcmp(el, "file")){
        m_file_name = JabberClient::get_attr("name", attr);
        m_file_size = JabberClient::get_attr("size", attr).toUInt();
    }
}

void JabberClient::IqRequest::element_end(const char*)
{
    m_data = NULL;
}

void JabberClient::IqRequest::char_data(const char *data, int len)
{
    if (m_data)
        m_data->append(QString::fromUtf8(data, len));
}

class JabberBgParser : public HTMLParser
{
public:
    JabberBgParser();
    QString parse(const QString &text);
    unsigned bgColor;
protected:
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &attrs);
    virtual void tag_end(const QString &tag);
    QString res;
};

JabberBgParser::JabberBgParser()
{
    bgColor = 0xFFFFFF;
}

QString JabberBgParser::parse(const QString &text)
{
    res = "";
    HTMLParser::parse(text);
    return res;
}

void JabberBgParser::text(const QString &text)
{
    res += quoteString(text);
}

void JabberBgParser::tag_start(const QString &tag, const list<QString> &attrs)
{
    if (tag == "body"){
        for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
            QString name = *it;
            ++it;
            QString value = *it;
            if (name.lower() == "bgcolor"){
                QColor c(value);
                bgColor = c.rgb();
            }
        }
        return;
    }
    res += "<";
    res += tag;
    for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
        QString name = *it;
        ++it;
        QString value = *it;
        res += " ";
        res += name;
        if (name == "style"){
            list<QString> styles = parseStyle(value);
            for (list<QString>::iterator it = styles.begin(); it != styles.end(); ++it){
                QString name = *it;
                ++it;
                QString value = *it;
                if (name == "background-color"){
                    QColor c;
                    c.setNamedColor(value);
                    bgColor = c.rgb() & 0xFFFFFF;
                }
            }
        }
        if (!value.isEmpty()){
            res += "=\'";
            res += quoteString(value);
            res += "\'";
        }
    }
    res += ">";
}

void JabberBgParser::tag_end(const QString &tag)
{
    if (tag == "body"){
        return;
    }
    res += "</";
    res += tag;
    res += ">";
}

JabberClient::MessageRequest::MessageRequest(JabberClient *client)
        : ServerRequest(client, NULL, NULL, NULL)
{
    m_data = NULL;
    m_errorCode = 0;
    m_bBody = false;
    m_bCompose = false;
    m_bEvent = false;
    m_bRichText = false;
    m_bRosters = false;
    m_bError = false;
}

JabberClient::MessageRequest::~MessageRequest()
{
    if (m_from.isEmpty())
        return;
    Contact *contact;
    QString resource;
    JabberUserData *data = m_client->findContact(m_from, QString::null, false, contact, resource);
    if (data == NULL){
        data = m_client->findContact(m_from, QString::null, true, contact, resource);
        if (data == NULL)
            return;
        contact->setFlags(CONTACT_TEMP);
    }
    Message *msg = NULL;

    if (!m_bError){
        // JEP-0022 composing event handling
        if (m_bBody){
            // Msg contains normal message. 
            // <composing/> here means "send me composing events, please", so we should do it.
            // But if that tag is absent, we must not send them.
            data->SendTypingEvents.asBool() = m_bCompose;
            data->TypingId.str() = (m_bCompose ? m_id : QString::null);

            // also, incoming message implicitly means that user has stopped typing
            if (data->IsTyping.toBool()){
                data->IsTyping.asBool() = false;
                Event e(EventContactStatus, contact);
                e.process();
            }
        }
        else{
            // Msg has no body ==> it is event message. 
            // Presence of <composing/> here means "I'm typing", absence - "I'm not typing anymore".
            data->IsTyping.asBool() = m_bCompose;
            Event e(EventContactStatus, contact);
            e.process();
        }
    }

    if (m_errorCode || !m_error.isEmpty()){
        if (!m_bEvent){
            JabberMessageError *m = new JabberMessageError;
            m->setError(m_error);
            m->setCode(m_errorCode);
            msg = m;
        }
    }else if (m_bBody){
        if (!m_contacts.isEmpty()){
            msg = new ContactsMessage;
            static_cast<ContactsMessage*>(msg)->setContacts(m_contacts);
        }else if (m_subj.isEmpty()){
            msg = new Message(MessageGeneric);
        }else{
            JabberMessage *m = new JabberMessage;
            m->setSubject(m_subj);
            msg = m;
        }
    }
    if (msg == NULL)
        return;
    if (m_bBody && m_contacts.isEmpty()){
        if (m_richText.isEmpty()){
            data->richText.asBool() = false;
            msg->setText(m_body);
        }else{
            JabberBgParser p;
            msg->setText(p.parse(m_richText));
            msg->setFlags(MESSAGE_RICHTEXT);
            msg->setBackground(p.bgColor);
        }
        if (m_targets.size()){
            if ((msg->getFlags() & MESSAGE_RICHTEXT) == 0){
                msg->setText(quoteString(msg->getText()));
                msg->setFlags(MESSAGE_RICHTEXT);
            }
            QString text = msg->getText();
            for (unsigned i = 0; i < m_targets.size(); i++){
                text += "<br><a href=\"";
                text += quoteString(m_targets[i]);
                text += "\">";
                text += quoteString(m_descs[i]);
                text += "</a>";
            }
        }
    }else{
        msg->setText(m_body);
    }
    msg->setFlags(msg->getFlags() | MESSAGE_RECEIVED);
    msg->setClient(m_client->dataName(data));
    msg->setContact(contact->id());
    Event e(EventMessageReceived, msg);
    if (!e.process())
        delete msg;
}

void JabberClient::MessageRequest::element_start(const char *el, const char **attr)
{
    if (m_bRichText){
        *m_data += "<";
        *m_data += el;
        if (attr){
            for (const char **p = attr; *p; ){
                const char *key = *(p++);
                const char *val = *(p++);
                *m_data += " ";
                *m_data += key;
                *m_data += "=\'";
                *m_data += val;
                *m_data += "\'";
            }
        }
        *m_data += ">";
        return;
    }
    m_data = NULL;
    if (!strcmp(el, "message")){
        m_from = JabberClient::get_attr("from", attr);
        m_id = JabberClient::get_attr("id", attr);
        if (JabberClient::get_attr("type", attr) == "error")
            m_bError = true;
        return;
    }
    if (!strcmp(el, "body")){
        m_data = &m_body;
        m_bBody = true;
        return;
    }
    if (!strcmp(el, "subject")){
        m_data = &m_subj;
        return;
    }
    if (!strcmp(el, "error")){
        m_errorCode = JabberClient::get_attr("code", attr).toUInt();
        m_data = &m_error;
        return;
    }
    if (m_bEvent){
        // Parsing <x xmlns='jabber:x:event'> tag, which contains JEP-0022 event info
        if (!strcmp(el, "composing"))
            m_bCompose = true;
        return;
    }
    if (!strcmp(el, "url-data")){
        m_target = JabberClient::get_attr("target", attr);
        m_desc = "";
    }
    if (!strcmp(el, "desc")){
        m_data = &m_desc;
        return;
    }
    if (m_bRosters && !strcmp(el, "item")){
        QString jid  = JabberClient::get_attr("jid", attr);
        QString name = JabberClient::get_attr("name", attr);
        if (!jid.isEmpty()){
            if (!m_contacts.isEmpty())
                m_contacts += ";";
            m_contacts += "jabber:";
            m_contacts += jid;
            if (name.isEmpty()){
                int n = jid.find('@');
                if (n >= 0){
                    name = jid.left(n);
                }else{
                    name = jid;
                }
            }
            m_contacts += "/";
            m_contacts += name;
            m_contacts += ",";
            m_contacts += name;
            m_contacts += " (";
            m_contacts += jid;
            m_contacts += ")";
        }
    }
    if (!strcmp(el, "x")){
        if (JabberClient::get_attr("xmlns", attr) == "jabber:x:event")
            m_bEvent = true;
        if (JabberClient::get_attr("xmlns", attr) == "jabber:x:roster")
            m_bRosters = true;
    }
    if (!strcmp(el, "html")){
        QString xmlns = JabberClient::get_attr("xmlns", attr);
        if (xmlns == "http://jabber.org/protocol/xhtml-im"){
            m_bRichText = true;
            m_data = &m_richText;
        }
    }
}

void JabberClient::MessageRequest::element_end(const char *element)
{
    QString el = QString::fromUtf8(element);
    if (m_bRichText){
        if (el == "html"){
            m_bRichText = false;
            m_data = NULL;
            return;
        }
        *m_data += "</";
        *m_data += el;
        *m_data += ">";
        return;
    }
    if (el == "x")
        m_bRosters = false;
    if (el == "url-data"){
        if (!m_target.isEmpty()){
            if (m_desc.isEmpty())
                m_desc = m_target;
            m_targets.push_back(m_target);
            m_descs.push_back(m_desc);
        }
        m_target = "";
        m_desc = "";
    }
    m_data = NULL;
}

void JabberClient::MessageRequest::char_data(const char *str, int len)
{
    if (m_data)
        m_data->append(QString::fromUtf8(str, len));
}

class AgentRequest : public JabberClient::ServerRequest
{
public:
    AgentRequest(JabberClient *client, const QString &jid);
    ~AgentRequest();
protected:
    JabberAgentsInfo	data;
    QString m_data;
    QString m_jid;
    bool   m_bError;
    virtual void element_start(const char *el, const char **attr);
    virtual void element_end(const char *el);
    virtual void char_data(const char *el, int len);
};

class AgentsDiscoRequest : public JabberClient::ServerRequest
{
public:
    AgentsDiscoRequest(JabberClient *client);
protected:
    virtual void element_start(const char *el, const char **attr);
};

class AgentDiscoRequest : public JabberClient::ServerRequest
{
public:
    AgentDiscoRequest(JabberClient *client, const QString &jid);
    ~AgentDiscoRequest();
protected:
    JabberAgentsInfo	data;
    virtual void element_start(const char *el, const char **attr);
    bool m_bError;
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
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

AgentDiscoRequest::AgentDiscoRequest(JabberClient *client, const QString &jid)
        : ServerRequest(client, _GET, NULL, jid)
{
    load_data(jabberAgentsInfo, &data, NULL);
    data.ID.str() = jid;
    m_bError = false;
}

AgentDiscoRequest::~AgentDiscoRequest()
{
    if (data.Name.str().isEmpty()){
        QString jid = data.ID.str();
        int n = jid.find('.');
        if (n > 0){
            jid = jid.left(n);
            data.Name.str() = jid;
        }
    }
    if (m_bError){
        data.Register.asBool() = true;
        data.Search.asBool()   = true;
    }
    if (!data.Name.str().isEmpty()){
        data.VHost.str() = m_client->VHost();
        data.Client = m_client;
        Event e(EventAgentFound, &data);
        e.process();
    }
    free_data(jabberAgentsInfo, &data);
}

void AgentDiscoRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "error")){
        m_bError = true;
        return;
    }
    if (!strcmp(el, "identity")){
        data.Name.str() = JabberClient::get_attr("name", attr);
        return;
    }
    if (!strcmp(el, "feature")){
        QString s = JabberClient::get_attr("var", attr);
        if (s == "jabber:iq:register")
            data.Register.asBool() = true;
        if (s == "jabber:iq:search")
            data.Search.asBool()   = true;
    }
}

AgentsDiscoRequest::AgentsDiscoRequest(JabberClient *client)
        : ServerRequest(client, _GET, NULL, client->VHost())
{
}

void AgentsDiscoRequest::element_start(const char *el, const char **attr)
{
    if (strcmp(el, "item"))
        return;
    QString jid = JabberClient::get_attr("jid", attr);
    if (!jid.isEmpty()){
        AgentDiscoRequest *req = new AgentDiscoRequest(m_client, jid);
        req->start_element("query");
        req->add_attribute("xmlns", "http://jabber.org/protocol/disco#info");
        req->send();
        m_client->m_requests.push_back(req);
    }
}

AgentRequest::AgentRequest(JabberClient *client, const QString &jid)
        : ServerRequest(client, _GET, NULL, jid)
{
    load_data(jabberAgentsInfo, &data, NULL);
    m_bError = false;
    m_jid    = jid;
}

AgentRequest::~AgentRequest()
{
    free_data(jabberAgentsInfo, &data);
    if (m_bError){
        AgentsDiscoRequest *req = new AgentsDiscoRequest(m_client);
        req->start_element("query");
        req->add_attribute("xmlns", "http://jabber.org/protocol/disco#items");
        req->send();
        m_client->m_requests.push_back(req);
    }
}

void AgentRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "agent")){
        free_data(jabberAgentsInfo, &data);
        load_data(jabberAgentsInfo, &data, NULL);
        m_data = JabberClient::get_attr("jid", attr);
        data.ID.str() = m_data;
    }else if (!strcmp(el, "search")){
        data.Search.asBool() = true;
    }else if (!strcmp(el, "register")){
        data.Register.asBool() = true;
    }else if (!strcmp(el, "error")){
        m_bError = true;
    }
    m_data = "";
}

void AgentRequest::element_end(const char *el)
{
    if (!strcmp(el, "agent")){
        if (!data.ID.str().isEmpty()){
            data.VHost.str() = m_client->VHost();
            data.Client = m_client;
            Event e(EventAgentFound, &data);
            e.process();
        }
    }else if (!strcmp(el, "name")){
        data.Name.str() = m_data;
    }
}

void AgentRequest::char_data(const char *el, int len)
{
    m_data.append(QString::fromUtf8(el, len));
}

QString JabberClient::get_agents(const QString &jid)
{
    AgentRequest *req = new AgentRequest(this, jid);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:agents");
    addLang(req);
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

class AgentInfoRequest : public JabberClient::ServerRequest
{
public:
    AgentInfoRequest(JabberClient *client, const QString &jid);
    ~AgentInfoRequest();
protected:
    JabberAgentInfo		data;
    bool   m_bOption;
    QString m_data;
    QString m_jid;
    QString m_error;
    bool   m_bError;
    unsigned m_error_code;
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
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRLIST, 1, 0 },
        { "", DATA_STRLIST, 1, 0 },
        { "", DATA_ULONG, 1, 0 },
        { "", DATA_BOOL, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };


AgentInfoRequest::AgentInfoRequest(JabberClient *client, const QString &jid)
        : ServerRequest(client, _GET, NULL, jid)
{
    m_jid = jid;
    m_bOption = false;
    m_error_code = 0;
    m_bError = false;
    load_data(jabberAgentInfo, &data, NULL);
}

AgentInfoRequest::~AgentInfoRequest()
{
    free_data(jabberAgentInfo, &data);
    load_data(jabberAgentInfo, &data, NULL);
    data.ID.str() = m_jid;
    data.ReqID.str() = m_id;
    data.nOptions.asULong() = m_error_code;
    data.Label.str() = m_error;
    Event e(EventAgentInfo, &data);
    e.process();
    free_data(jabberAgentInfo, &data);
}

void AgentInfoRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "error")){
        m_bError = true;
        m_error_code = JabberClient::get_attr("code", attr).toUInt();
    }
    if (m_bError)
        return;
    if (!strcmp(el, "field")){
        free_data(jabberAgentInfo, &data);
        load_data(jabberAgentInfo, &data, NULL);
        data.ID.str() = m_jid;
        m_data = JabberClient::get_attr("var", attr);
        data.Field.str() = m_data;
        m_data = JabberClient::get_attr("type", attr);
        data.Type.str() = m_data;
        m_data = JabberClient::get_attr("label", attr);
        data.Label.str() = m_data;
    }
    if (!strcmp(el, "option")){
        m_bOption = true;
        m_data = JabberClient::get_attr("label", attr);
        set_str(&data.OptionLabels, data.nOptions.toULong(), m_data);
    }
    if (!strcmp(el, "x")){
        data.VHost.str() = m_client->VHost();
        data.Type.str() = "x";
        data.ReqID.str() = m_id;
        data.ID.str() = m_jid;
        Event e(EventAgentInfo, &data);
        e.process();
        free_data(jabberAgentInfo, &data);
        load_data(jabberAgentInfo, &data, NULL);
    }
    m_data = "";
}

void AgentInfoRequest::element_end(const char *el)
{
    if (!strcmp(el, "error")){
        m_error = m_data;
        m_data  = "";
        m_bError = false;
        return;
    }
    if (m_bError)
        return;
    if (!strcmp(el, "desc")){
        data.Desc.str() = m_data;
        return;
    }
    if (!strcmp(el, "field")){
        if (!data.Field.str().isEmpty()){
            data.VHost.str() = m_client->VHost();
            data.ReqID.str() = m_id;
            data.ID.str() = m_jid;
            Event e(EventAgentInfo, &data);
            e.process();
            free_data(jabberAgentInfo, &data);
            load_data(jabberAgentInfo, &data, NULL);
        }
    }else if (!strcmp(el, "option")){
        m_bOption = false;
        QString str = get_str(data.Options, data.nOptions.toULong());
        if (!str.isEmpty())
            data.nOptions.asULong()++;
    }else if (!strcmp(el, "value")){
        if (m_bOption){
            set_str(&data.Options, data.nOptions.toULong(), m_data);
        }else{
            data.Value.str() = m_data;
        }
    }else if (!strcmp(el, "required")){
        data.bRequired.asBool() = true;
    }else if (!strcmp(el, "key") || !strcmp(el, "instructions")){
        data.Value.str() = m_data;
        data.ID.str() = m_jid;
        data.ReqID.str() = m_id;
        data.Type.str() = QString::fromUtf8(el);
        Event e(EventAgentInfo, &data);
        e.process();
        free_data(jabberAgentInfo, &data);
        load_data(jabberAgentInfo, &data, NULL);
    }else if (strcmp(el, "error") && strcmp(el, "iq") && strcmp(el, "query") && strcmp(el, "x")){
        data.Value.str() = m_data;
        data.ID.str() = m_jid;
        data.ReqID.str() = m_id;
        data.Type.str() = QString::fromUtf8(el);
        Event e(EventAgentInfo, &data);
        e.process();
        free_data(jabberAgentInfo, &data);
        load_data(jabberAgentInfo, &data, NULL);
    }
}

void AgentInfoRequest::char_data(const char *el, int len)
{
    m_data.append(QString::fromUtf8(el, len));
}

QString JabberClient::get_agent_info(const QString &jid, const QString &node, const QString &type)
{
    AgentInfoRequest *req = new AgentInfoRequest(this, jid);
    req->start_element("query");
    QString xmlns = "jabber:iq:";
    xmlns += type;
    req->add_attribute("xmlns", xmlns);
    req->add_attribute("node", node);
    addLang(req);
    req->send();
    m_requests.push_back(req);
    return req->m_id.utf8();
}

typedef map<my_string, QString> VALUE_MAP;

class SearchRequest : public JabberClient::ServerRequest
{
public:
    SearchRequest(JabberClient *client, const QString &jid);
    ~SearchRequest();
protected:
    JabberSearchData data;
    QString m_data;
    QString m_attr;
    list<QString> m_fields;
    VALUE_MAP    m_values;
    bool m_bReported;
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
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRLIST, 1, 0 },
        { "", DATA_ULONG, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };


SearchRequest::SearchRequest(JabberClient *client, const QString &jid)
        : ServerRequest(client, _SET, NULL, jid)
{
    load_data(jabberSearchData, &data, NULL);
    m_bReported = false;
}

SearchRequest::~SearchRequest()
{
    Event e(EventSearchDone, (void*)m_id.utf8().data());
    e.process();
    free_data(jabberSearchData, &data);
}

void SearchRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "reported")){
        m_bReported = true;
    }else if (!strcmp(el, "item")){
        free_data(jabberSearchData, &data);
        load_data(jabberSearchData, &data, NULL);
        m_data = JabberClient::get_attr("jid", attr);
        data.JID.str() = m_data;
    }else if (!strcmp(el, "field")){
        QString var = JabberClient::get_attr("var", attr);
        if (m_bReported){
            if (!var.isEmpty() && (var != "jid")){
                QString label = JabberClient::get_attr("label", attr);
                if (label.isEmpty())
                    label = var;
                m_values.insert(VALUE_MAP::value_type(var, label));
                m_fields.push_back(var);
            }
        }else{
            m_attr = var;
        }
    }
    m_data = "";
}

void SearchRequest::element_end(const char *el)
{
    if (!strcmp(el, "reported")){
        m_bReported = false;
        free_data(jabberSearchData, &data);
        load_data(jabberSearchData, &data, NULL);
        for (list<QString>::iterator it = m_fields.begin(); it != m_fields.end(); ++it){
            QString value;
            VALUE_MAP::iterator itv = m_values.find((*it));
            if (itv != m_values.end())
                value = (*itv).second;
            set_str(&data.Fields, data.nFields.toULong() * 2, value);
            set_str(&data.Fields, data.nFields.toULong() * 2 + 1, value);
            data.nFields.asULong()++;
        }
        data.ID.str() = m_id;
        Event e(EventSearch, &data);
        e.process();
        m_values.clear();
    }else if (!strcmp(el, "item")){
        if (!data.JID.str().isEmpty()){
            for (list<QString>::iterator it = m_fields.begin(); it != m_fields.end(); ++it){
                VALUE_MAP::iterator itv = m_values.find((*it));
                if (itv != m_values.end()){
                    QString val = (*itv).second;
                    set_str(&data.Fields, data.nFields.toULong(), val);
                }
                data.nFields.asULong()++;
            }
            data.ID.str() = m_id;
            Event e(EventSearch, &data);
            e.process();
            m_values.clear();
        }
    }else if (!strcmp(el, "value") || !strcmp(el, "field")){
        if (!m_attr.isEmpty() && !m_data.isEmpty()){
            if (m_attr == "jid"){
                data.JID.str() = m_data;
            }else{
                m_values.insert(VALUE_MAP::value_type(m_attr, m_data));
            }
        }
        m_attr = "";
    }else if (!strcmp(el, "first")){
        data.First.str() = m_data;
    }else if (!strcmp(el, "last")){
        data.Last.str() = m_data;
    }else if (!strcmp(el, "nick")){
        data.Nick.str() = m_data;
    }else if (!strcmp(el, "email")){
        data.EMail.str() = m_data;
    }else if (!strcmp(el, "status")){
        data.Status.str() = m_data;
    }
}

void SearchRequest::char_data(const char *el, int len)
{
    m_data.append(QString::fromUtf8(el, len));
}

QString JabberClient::search(const QString &jid, const QString &node, const QString &condition)
{
    SearchRequest *req = new SearchRequest(this, jid);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:search");
    req->add_attribute("node", node);
    req->add_condition(condition, false);
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
    RegisterRequest(JabberClient *client, const QString &jid);
    ~RegisterRequest();
protected:
    QString   m_error;
    QString  *m_data;
    unsigned m_error_code;
    virtual void element_start(const char *el, const char **attr);
    virtual void element_end(const char *el);
    virtual void char_data(const char *el, int len);
};

RegisterRequest::RegisterRequest(JabberClient *client, const QString &jid)
        : ServerRequest(client, _SET, NULL, jid)
{
    m_data = NULL;
    m_error_code = (unsigned)(-1);
}

RegisterRequest::~RegisterRequest()
{
    agentRegisterInfo ai;
    ai.id = m_id;
    ai.err_code = m_error_code;
    ai.error = m_error;
    Event e(EventAgentRegister, &ai);
    e.process();
}

void RegisterRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "error")){
        m_error_code = JabberClient::get_attr("code", attr).toUInt();
        if (m_error_code == 0)
            m_error_code = (unsigned)(-1);
        m_data = &m_error;
        return;
    }
    if (!strcmp(el, "iq")){
        QString type = JabberClient::get_attr("type", attr);
        if (type == "result")
            m_error_code = 0;
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
    m_data->append(QString::fromUtf8(el, len));
}

QString JabberClient::process(const QString &jid, const QString &node, const QString &condition, const QString &type)
{
    RegisterRequest *req = new RegisterRequest(this, jid);
    req->start_element("query");
    QString xmlns = "jabber:iq:" + type;
    req->add_attribute("xmlns", xmlns);
    bool bData = (strcmp(type, "data") == 0);
    if (bData)
        req->add_attribute("type", "submit");
    req->add_attribute("node", node);
    req->add_condition(condition, bData);
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
        req->add_attribute("jid", r.jid);
        if ((*it).bDelete)
            req->add_attribute("subscription", "remove");
        if (!(*it).name.isEmpty())
            req->add_attribute("name", r.name);
        if (!(*it).bDelete)
            req->text_tag("group", r.grp);
        req->send();
        m_requests.push_back(req);
    }
    m_listRequests.clear();
}

#if 0

class SendFileRequest : public JabberClient::ServerRequest
{
public:
    SendFileRequest(JabberClient *client, const QString &jid, FileMessage *msg);
    ~SendFileRequest();
protected:
    virtual void	element_start(const char *el, const char **attr);
    virtual void	element_end(const char *el);
    virtual	void	char_data(const char *str, int len);
    QString *m_data;
    QString m_error;
    bool   m_bError;
    FileMessage *m_msg;
};

SendFileRequest::SendFileRequest(JabberClient *client, const QString &jid, FileMessage *msg)
        : JabberClient::ServerRequest(client, _SET, NULL, jid)
{
    m_msg    = msg;
    m_data   = NULL;
    m_bError = false;
}

SendFileRequest::~SendFileRequest()
{
    if (m_msg && m_bError){
        if (m_error.isEmpty())
            m_error = I18N_NOOP("File transfer declined");
        m_msg->setError(m_error);
        Event e(EventMessageSent, m_msg);
        e.process();
        delete m_msg;
    }
}

void SendFileRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "iq")){
        if (JabberClient::get_attr("type", attr) == "error")
            m_bError = true;
    }
    if (!strcmp(el, "error"))
        m_data = &m_error;
}

void SendFileRequest::element_end(const char*)
{
    m_data = NULL;
}

void SendFileRequest::char_data(const char *str, int len)
{
    if (m_data)
        m_data->append(QString::fromUtf8(str, len));
}

#endif

class SendFileRequest : public JabberClient::ServerRequest
{
public:
    SendFileRequest(JabberClient *client, const QString &jid, FileMessage *msg);
    ~SendFileRequest();
protected:
    virtual void	element_start(const char *el, const char **attr);
    virtual void	element_end(const char *el);
    virtual	void	char_data(const char *str, int len);
    FileMessage *m_msg;
};

SendFileRequest::SendFileRequest(JabberClient *client, const QString &jid, FileMessage *msg)
        : JabberClient::ServerRequest(client, _SET, NULL, jid)
{
    m_msg = msg;
}

SendFileRequest::~SendFileRequest()
{
}

void SendFileRequest::element_start(const char*, const char**)
{
}

void SendFileRequest::element_end(const char*)
{
}

void SendFileRequest::char_data(const char*, int)
{
}

void JabberClient::sendFileRequest(FileMessage *msg, unsigned short, JabberUserData *data, const QString &fname, unsigned size)
{
    QString jid = data->ID.str();
    if (msg->getResource().isEmpty()){
        if (!data->Resource.str().isEmpty()){
            jid += "/";
            jid += data->Resource.str();
        }
    }else{
        jid += "/";
        jid += msg->getResource();
    }
    SendFileRequest *req = new SendFileRequest(this, jid.utf8(), msg);
    req->start_element("si");
    req->add_attribute("xmlns", "http://jabber.org/protocol/si");
    req->add_attribute("profile", "http://jabber.org/protocol/si/profile/file-transfer");
    req->add_attribute("id", get_unique_id());
    req->start_element("file");
    req->add_attribute("xmns", "http://jabber.org/protocol/si/profile/file-transfer");
    req->add_attribute("size", QString::number(size));
    req->add_attribute("name", fname);
    req->start_element("range");
    req->end_element();
    req->end_element();
    req->start_element("feature");
    req->add_attribute("xmlns", "http://jabber.org/protocol/feature-neg");
    req->start_element("x");
    req->add_attribute("xmlns", "jabber:x:data");
    req->add_attribute("type", "form");
    req->start_element("field");
    req->add_attribute("type", "list-single");
    req->add_attribute("var", "stream-method");
    req->start_element("option");
    req->text_tag("value", "http://jabber.org/protocol/bytestreams");
#if 0
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:oob");
    QString url  = "http://";
    struct in_addr addr;
    addr.s_addr = m_socket->localHost();
    url += inet_ntoa(addr);
    url += ":";
	url += QString::number(port);
    url += "/";
    url += fname;
    QString desc;
    desc = msg->getText();
    req->text_tag("url", url);
    req->text_tag("desc", desc);
#endif
    req->send();
    m_requests.push_back(req);
}

void JabberClient::sendFileAccept(FileMessage *msg, JabberUserData *data)
{
    JabberFileMessage *m = static_cast<JabberFileMessage*>(msg);
    QString jid = data->ID.str();
    if (msg->getResource().isEmpty()){
        if (!data->Resource.str().isEmpty()){
            jid += "/";
            jid += data->Resource.str();
        }
    }else{
        jid += "/";
        jid += msg->getResource();
    }
    ServerRequest req(this, ServerRequest::_RESULT, NULL, jid, m->getID());
    req.start_element("si");
    req.add_attribute("xmlns", "http://jabber.org/protocol/si");
    req.start_element("feature");
    req.add_attribute("xmlns", "http://jabber.org/protocol/feature-neg");
    req.start_element("x");
    req.add_attribute("xmlns", "jabber:x:data");
    req.add_attribute("type", "submit");
    req.start_element("field");
    req.add_attribute("var", "stream-method");
    req.text_tag("value", "http://jabber.org/protocol/bytestreams");
    req.send();
}

class DiscoItemsRequest : public JabberClient::ServerRequest
{
public:
    DiscoItemsRequest(JabberClient *client, const QString &jid);
    ~DiscoItemsRequest();
protected:
    virtual void	element_start(const char *el, const char **attr);
    virtual void	element_end(const char *el);
    virtual	void	char_data(const char *str, int len);
    QString			*m_data;
    QString			m_error;
    unsigned		m_code;
};

DiscoItemsRequest::DiscoItemsRequest(JabberClient *client, const QString &jid)
        : JabberClient::ServerRequest(client, _GET, NULL, jid)
{
    m_data = NULL;
    m_code = 0;
}

DiscoItemsRequest::~DiscoItemsRequest()
{
    DiscoItem item;
    item.id		= m_id;
    if (m_code){
        item.name	= m_error;
        item.node	= QString::number(m_code);
    }
    Event e(EventDiscoItem, &item);
    e.process();
}

void DiscoItemsRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "item")){
        DiscoItem item;
        item.id		= m_id;
        item.jid	= JabberClient::get_attr("jid", attr);
        item.name	= JabberClient::get_attr("name", attr);
        item.node	= JabberClient::get_attr("node", attr);
        if (!item.jid.isEmpty()){
            Event e(EventDiscoItem, &item);
            e.process();
        }
    }
    if (!strcmp(el, "error")){
        m_code = JabberClient::get_attr("code", attr).toUInt();
        m_data = &m_error;
    }
}

void DiscoItemsRequest::element_end(const char *el)
{
    if (!strcmp(el, "error"))
        m_data = NULL;
}

void DiscoItemsRequest::char_data(const char *buf, int len)
{
    if (m_data)
        m_data->append(QString::fromUtf8(buf, len));
}

QString JabberClient::discoItems(const QString &jid, const QString &node)
{
    if (getState() != Connected)
        return "";
    DiscoItemsRequest *req = new DiscoItemsRequest(this, jid.utf8());
    req->start_element("query");
    req->add_attribute("xmlns", "http://jabber.org/protocol/disco#items");
    req->add_attribute("node", node);
    addLang(req);
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

class DiscoInfoRequest : public JabberClient::ServerRequest
{
public:
    DiscoInfoRequest(JabberClient *client, const QString &jid);
    ~DiscoInfoRequest();
protected:
    virtual void	element_start(const char *el, const char **attr);
    virtual void	element_end(const char *el);
    virtual	void	char_data(const char *str, int len);
    QString			*m_data;
    QString			m_error;
    QString			m_features;
    QString			m_name;
    QString			m_type;
    QString			m_category;
    unsigned		m_code;
};

DiscoInfoRequest::DiscoInfoRequest(JabberClient *client, const QString &jid)
        : JabberClient::ServerRequest(client, _GET, NULL, jid)
{
    m_data = NULL;
    m_code = 0;
}

DiscoInfoRequest::~DiscoInfoRequest()
{
    if (m_code == 0){
        DiscoItem item;
        item.id         = m_id;
        item.jid        = "info";
        item.name       = m_name;
        item.category	= m_category;
        item.type       = m_type;
        item.features	= m_features;
        Event e(EventDiscoItem, &item);
        e.process();
    }
    DiscoItem item;
    item.id		= m_id;
    if (m_code){
        item.name	= m_error;
        item.node	= QString::number(m_code);
    }
    Event e(EventDiscoItem, &item);
    e.process();
}

void DiscoInfoRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "identity")){
        m_category	= JabberClient::get_attr("category", attr);
        m_name		= JabberClient::get_attr("name", attr);
        m_type		= JabberClient::get_attr("type", attr);
    }
    if (!strcmp(el, "feature")){
        QString feature = JabberClient::get_attr("var", attr);
        if (!feature.isEmpty()){
            if (!m_features.isEmpty())
                m_features += "\n";
            m_features += feature;
        }
    }
    if (!strcmp(el, "error")){
        m_code = JabberClient::get_attr("code", attr).toUInt();
        m_data = &m_error;
    }
}

void DiscoInfoRequest::element_end(const char *el)
{
    if (!strcmp(el, "error"))
        m_data = NULL;
}

void DiscoInfoRequest::char_data(const char *buf, int len)
{
    if (m_data)
        m_data->append(QString::fromUtf8(buf, len));
}

QString JabberClient::discoInfo(const QString &jid, const QString &node)
{
    if (getState() != Connected)
        return "";
    DiscoInfoRequest *req = new DiscoInfoRequest(this, jid);
    req->start_element("query");
    req->add_attribute("xmlns", "http://jabber.org/protocol/disco#info");
    req->add_attribute("node", node);
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

class BrowseRequest : public JabberClient::ServerRequest
{
public:
    BrowseRequest(JabberClient *client, const QString &jid);
    ~BrowseRequest();
protected:
    virtual void	element_start(const char *el, const char **attr);
    virtual void	element_end(const char *el);
    virtual	void	char_data(const char *str, int len);
    QString			*m_data;
    QString			m_jid;
    QString			m_error;
    QString			m_name;
    QString			m_type;
    QString			m_category;
    QString			m_features;
    QString			m_ns;
    unsigned		m_code;
};

BrowseRequest::BrowseRequest(JabberClient *client, const QString &jid)
        : JabberClient::ServerRequest(client, _GET, NULL, jid)
{
    m_data	= NULL;
    m_code	= 0;
    m_jid	= jid;
}

BrowseRequest::~BrowseRequest()
{
    if (!m_jid.isEmpty() && !m_name.isEmpty() && (m_code == 0)){
        DiscoItem item;
        item.id     = m_id;
        item.jid        = m_jid;
        item.name       = m_name;
        item.type       = m_type;
        item.category   = m_category;
        item.features   = m_features.utf8();
        Event e(EventDiscoItem, &item);
        e.process();
    }
    DiscoItem item;
    item.id		= m_id;
    if (m_code){
        item.name	= m_error;
        item.node	= QString::number(m_code);
    }
    Event e(EventDiscoItem, &item);
    e.process();
}

void BrowseRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "error")){
        m_code = JabberClient::get_attr("code", attr).toUInt();
        m_data = &m_error;
    }
    if (!strcmp(el, "item") || !strcmp(el, "service") || !strcmp(el, "agent") || !strcmp(el, "headline")){
        if (!m_jid.isEmpty() && !m_name.isEmpty()){
            DiscoItem item;
            item.id         = m_id;
            item.jid        = m_jid;
            item.name       = m_name;
            item.type       = m_type;
            item.category   = m_category;
            item.features   = m_features;
            Event e(EventDiscoItem, &item);
            e.process();
        }
        m_jid		= JabberClient::get_attr("jid", attr);
        m_name		= JabberClient::get_attr("name", attr);
        m_type		= JabberClient::get_attr("type", attr);
        m_category	= JabberClient::get_attr("category", attr);
        if (!strcmp(el, "headline"))
            m_category = "headline";
        m_features	= "";
    }
    if (!strcmp(el, "query")){
        m_name		= JabberClient::get_attr("name", attr);
        m_type		= JabberClient::get_attr("type", attr);
        m_category	= JabberClient::get_attr("category", attr);
    }
    if (!strcmp(el, "ns"))
        m_data  = &m_ns;
}

void BrowseRequest::element_end(const char *el)
{
    if (!strcmp(el, "error"))
        m_data = NULL;
    if (!strcmp(el, "ns") && !m_ns.isEmpty()){
        if (!m_features.isEmpty())
            m_features += "\n";
        m_features += m_ns;
        m_ns = "";
        m_data = NULL;
    }
    if ((!strcmp(el, "item") || !strcmp(el, "service") ||
            !strcmp(el, "agent") || !strcmp(el, "headline"))
            && !m_jid.isEmpty()){
        DiscoItem item;
        item.id         = m_id;
        item.jid        = m_jid;
        item.name       = m_name;
        item.type       = m_type;
        item.category   = m_category;
        item.features   = m_features;
        Event e(EventDiscoItem, &item);
        e.process();
        m_jid = "";
    }
}

void BrowseRequest::char_data(const char *buf, int len)
{
    if (m_data)
        m_data->append(QString::fromUtf8(buf, len));
}

QString JabberClient::browse(const QString &jid)
{
    if (getState() != Connected)
        return "";
    BrowseRequest *req = new BrowseRequest(this, jid);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:browse");
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

class VersionInfoRequest : public JabberClient::ServerRequest
{
public:
    VersionInfoRequest(JabberClient *client, const QString &jid, const QString &node);
    ~VersionInfoRequest();
protected:
    virtual void	element_start(const char *el, const char **attr);
    virtual void	element_end(const char *el);
    virtual	void	char_data(const char *str, int len);
    QString			*m_data;
    QString			m_jid;
    QString			m_node;
    QString			m_name;
    QString			m_version;
    QString			m_os;
};

VersionInfoRequest::VersionInfoRequest(JabberClient *client, const QString &jid, const QString &node)
        : JabberClient::ServerRequest(client, _GET, NULL, jid)
{
    m_data = NULL;
    m_jid = jid;
    m_node = node;
}

VersionInfoRequest::~VersionInfoRequest()
{
    ClientVersionInfo info;
    info.jid = m_jid;
    info.node = m_node;
    info.name = m_name;
    info.version = m_version;
    info.os = m_os;
    Event e(EventClientVersion, &info);
    e.process();
}

void VersionInfoRequest::element_start(const char *el, const char**)
{
    if (!strcmp(el, "name"))
        m_data = &m_name;
    if (!strcmp(el, "version"))
        m_data = &m_version;
    if (!strcmp(el, "os"))
        m_data = &m_os;
}

void VersionInfoRequest::element_end(const char*)
{
    m_data = NULL;
}

void VersionInfoRequest::char_data(const char *buf, int len)
{
    if (m_data)
        m_data->append(QString::fromUtf8(buf, len));
}

QString JabberClient::versionInfo(const QString &jid, const QString &node)
{
    if (getState() != Connected)
        return "";
    VersionInfoRequest *req = new VersionInfoRequest(this, jid, node);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:version");
    req->add_attribute("node", node);
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

class TimeInfoRequest : public JabberClient::ServerRequest
{
public:
    TimeInfoRequest(JabberClient *client, const QString &jid);
    ~TimeInfoRequest();
protected:
    virtual void	element_start(const char *el, const char **attr);
    virtual void	element_end(const char *el);
    virtual	void	char_data(const char *str, int len);
    QString			*m_data;
    QString			m_time;
};

TimeInfoRequest::TimeInfoRequest(JabberClient *client, const QString &jid)
        : JabberClient::ServerRequest(client, _GET, NULL, jid)
{
    m_data = NULL;
}

TimeInfoRequest::~TimeInfoRequest()
{
    DiscoItem item;
    item.id     = m_id;
    item.jid    = m_time;
    Event e(EventDiscoItem, &item);
    e.process();
}

void TimeInfoRequest::element_start(const char *el, const char**)
{
    if (!strcmp(el, "utc"))
        m_data = &m_time;
}

void TimeInfoRequest::element_end(const char*)
{
    m_data = NULL;
}

void TimeInfoRequest::char_data(const char *buf, int len)
{
    if (m_data)
        m_data->append(QString::fromUtf8(buf, len));
}

QString JabberClient::timeInfo(const QString &jid, const QString &node)
{
    if (getState() != Connected)
        return "";
    TimeInfoRequest *req = new TimeInfoRequest(this, jid);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:time");
    req->add_attribute("node", node);
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

class LastInfoRequest : public JabberClient::ServerRequest
{
public:
    LastInfoRequest(JabberClient *client, const QString &jid);
protected:
    virtual void	element_start(const char *el, const char **attr);
};

LastInfoRequest::LastInfoRequest(JabberClient *client, const QString &jid)
        : JabberClient::ServerRequest(client, _GET, NULL, jid)
{
}

void LastInfoRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "query")){
        DiscoItem item;
        item.id		= m_id;
        item.jid	= JabberClient::get_attr("seconds", attr);
        Event e(EventDiscoItem, &item);
        e.process();
    }
}

QString JabberClient::lastInfo(const QString &jid, const QString &node)
{
    if (getState() != Connected)
        return "";
    LastInfoRequest *req = new LastInfoRequest(this, jid);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:last");
    req->add_attribute("node", node);
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

class StatRequest : public JabberClient::ServerRequest
{
public:
    StatRequest(JabberClient *client, const QString &jid, const QString &id);
    ~StatRequest();
protected:
    virtual void	element_start(const char *el, const char **attr);
    QString	m_id;
};

StatRequest::StatRequest(JabberClient *client, const QString &jid, const QString &id)
        : JabberClient::ServerRequest(client, _GET, NULL, jid)
{
    m_id = id;
}

StatRequest::~StatRequest()
{
    DiscoItem item;
    item.id     = m_id;
    item.jid    = "";
    Event e(EventDiscoItem, &item);
    e.process();
}

void StatRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "stat")){
        DiscoItem item;
        item.id		= m_id;
        item.jid	= JabberClient::get_attr("name", attr);
        item.name	= JabberClient::get_attr("units", attr);
        item.node	= JabberClient::get_attr("value", attr);
        Event e(EventDiscoItem, &item);
        e.process();
    }
}

class StatItemsRequest : public JabberClient::ServerRequest
{
public:
    StatItemsRequest(JabberClient *client, const QString &jid, const QString &node);
    ~StatItemsRequest();
protected:
    virtual void	element_start(const char *el, const char **attr);
    list<QString>    m_stats;
    QString			m_jid;
    QString			m_node;
};

StatItemsRequest::StatItemsRequest(JabberClient *client, const QString &jid, const QString &node)
        : JabberClient::ServerRequest(client, _GET, NULL, jid)
{
    m_jid  = jid;
    m_node = node;
}

StatItemsRequest::~StatItemsRequest()
{
    if (m_stats.empty()){
        DiscoItem item;
        item.id		= m_id;
        item.jid	= "";
        Event e(EventDiscoItem, &item);
        e.process();
        return;
    }
    StatRequest *req = new StatRequest(m_client, m_jid, m_id);
    req->start_element("query");
    req->add_attribute("xmlns", "http://jabber.org/protocol/stats");
    req->add_attribute("node", m_node);
    m_client->addLang(req);
    for (list<QString>::iterator it = m_stats.begin(); it != m_stats.end(); ++it){
        req->start_element("stat");
        req->add_attribute("name", (*it));
        req->end_element();
    }
    req->send();
    m_client->m_requests.push_back(req);
}

void StatItemsRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "stat")){
        QString name = JabberClient::get_attr("name", attr);
        if (!name.isEmpty())
            m_stats.push_back(name);
    }
}

QString JabberClient::statInfo(const QString &jid, const QString &node)
{
    if (getState() != Connected)
        return "";
    StatItemsRequest *req = new StatItemsRequest(this, jid, node);
    req->start_element("query");
    req->add_attribute("xmlns", "http://jabber.org/protocol/stats");
    req->add_attribute("node", node);
    addLang(req);
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

static char XmlLang[] = I18N_NOOP("Please translate this to short language name like 'ru' or 'de'. Do not translate this sentence!)");

void JabberClient::addLang(ServerRequest *req)
{
    QString s = i18n(XmlLang);
    if (s == XmlLang)
        return;
    req->add_attribute("xml:lang", s);
}

class ChangePasswordRequest : public JabberClient::ServerRequest
{
public:
    ChangePasswordRequest(JabberClient *client, const char *password);
    ~ChangePasswordRequest();
protected:
    QString	m_password;
};

ChangePasswordRequest::ChangePasswordRequest(JabberClient *client, const char *password)
        : JabberClient::ServerRequest(client, _SET, NULL, NULL)
{
    m_password = password;
}

ChangePasswordRequest::~ChangePasswordRequest()
{
    m_client->setPassword(m_password);
}

void JabberClient::changePassword(const QString &password)
{
    if (getState() != Connected)
        return;
    ChangePasswordRequest *req = new ChangePasswordRequest(this, password);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:register");
    req->text_tag("username", data.owner.ID.str());
    req->text_tag("password", password);
    m_requests.push_back(req);
    req->send();
}


