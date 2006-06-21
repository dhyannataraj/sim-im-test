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
        JabberListRequest *lr = m_client->findRequest(QString::fromUtf8(m_jid.c_str()), false);
        Contact *contact;
        string resource;
        JabberUserData *data = m_client->findContact(m_jid.c_str(), m_name.c_str(), false, contact, resource);
        if (data == NULL){
            if (lr && lr->bDelete){
                m_client->findRequest(QString::fromUtf8(m_jid.c_str()), true);
            }else{
                bChanged = true;
                string resource;
                data = m_client->findContact(m_jid.c_str(), m_name.c_str(), true, contact, resource);
                if (m_bSubscription){
                    contact->setFlags(CONTACT_TEMP);
                    Event eContact(EventContactChanged, contact);
                    eContact.process();
                    m_client->auth_request(m_jid.c_str(), MessageAuthRequest, m_subscription.c_str(), true);
                    data = m_client->findContact(m_jid.c_str(), m_name.c_str(), false, contact, resource);
                }
            }
        }
        if (data == NULL)
            return;
        if (data->Subscribe.toULong() != m_subscribe){
            bChanged = true;
            data->Subscribe.asULong() = m_subscribe;
        }
        data->Group.str() = QString::fromUtf8(m_grp.c_str());
        data->bChecked.asBool() = true;
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
    InfoRequest(JabberClient *client, JabberUserData *data, bool bVCard);
    ~InfoRequest();
protected:
    virtual void element_start(const char *el, const char **attr);
    virtual void element_end(const char *el);
    virtual void char_data(const char *str, int len);
    string	m_jid;
    string  m_node;
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
    m_jid	= data->ID.str().utf8();
    if (!data->Node.str().isEmpty())
        m_node  = data->Node.str().utf8();
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
            load_data(jabberUserData, &u_data);
            data = &u_data;
            data->ID.str() = QString::fromUtf8(m_jid.c_str());
            data->Node.str() = QString::fromUtf8(m_node.c_str());
        }else{
            if (QString::fromUtf8(m_jid.c_str()) == m_client->data.owner.ID.str()){
                data = &m_client->data.owner;
            }else{
                string jid = m_jid;
                if (strchr(jid.c_str(), '@') == NULL){
                    jid += "@";
                    jid += m_host;
                }
                string resource;
                data = m_client->findContact(m_jid.c_str(), NULL, false, contact, resource);
                if (data == NULL)
                    return;
            }
        }
        bool bChanged = false;
        bChanged |= data->FirstName.setStr(QString::fromUtf8(m_firstName.c_str()));
        bChanged |= data->Nick.setStr(QString::fromUtf8(m_nick.c_str()));
        bChanged |= data->Desc.setStr(QString::fromUtf8(m_desc.c_str()));
        bChanged |= data->Bday.setStr(QString::fromUtf8(m_bday.c_str()));
        bChanged |= data->Url.setStr(QString::fromUtf8(m_url.c_str()));
        bChanged |= data->OrgName.setStr(QString::fromUtf8(m_orgName.c_str()));
        bChanged |= data->OrgUnit.setStr(QString::fromUtf8(m_orgUnit.c_str()));
        bChanged |= data->Title.setStr(QString::fromUtf8(m_title.c_str()));
        bChanged |= data->Role.setStr(QString::fromUtf8(m_role.c_str()));
        bChanged |= data->Street.setStr(QString::fromUtf8(m_street.c_str()));
        bChanged |= data->ExtAddr.setStr(QString::fromUtf8(m_ext.c_str()));
        bChanged |= data->City.setStr(QString::fromUtf8(m_city.c_str()));
        bChanged |= data->Region.setStr(QString::fromUtf8(m_region.c_str()));
        bChanged |= data->PCode.setStr(QString::fromUtf8(m_pcode.c_str()));
        bChanged |= data->Country.setStr(QString::fromUtf8(m_country.c_str()));
        bChanged |= data->EMail.setStr(QString::fromUtf8(m_email.c_str()));
        bChanged |= data->Phone.setStr(QString::fromUtf8(m_phone.c_str()));

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
            if ((photo.width() != (int)(data->PhotoWidth.toULong())) ||
                    (photo.height() != (int)(data->PhotoHeight.toULong())))
                bChanged = true;
            data->PhotoWidth.asULong()  = photo.width();
            data->PhotoHeight.asULong() = photo.height();
            if (QString::fromUtf8(m_jid.c_str()) == m_client->data.owner.ID.str())
                m_client->setPhoto(m_client->photoFile(data));
        }else{
            if (data->PhotoWidth.toULong() || data->PhotoHeight.toULong())
                bChanged = true;
            data->PhotoWidth.asULong()  = 0;
            data->PhotoHeight.asULong() = 0;
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
            if ((logo.width() != (int)(data->LogoWidth.toULong())) ||
                    (logo.height() != (int)(data->LogoHeight.toULong())))
                bChanged = true;
            data->LogoWidth.asULong()  = logo.width();
            data->LogoHeight.asULong() = logo.height();
            if (QString::fromUtf8(m_jid.c_str()) == m_client->data.owner.ID.str())
                m_client->setLogo(m_client->logoFile(data));
        }else{
            if (data->LogoWidth.toULong() || data->LogoHeight.toULong())
                bChanged = true;
            data->LogoWidth.asULong()  = 0;
            data->LogoHeight.asULong() = 0;
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

void JabberClient::setupContact(Contact *contact, void *_data)
{
    JabberUserData *data = (JabberUserData*)_data;
    QString mail = data->EMail.str();
    contact->setEMails(mail, name());
    QString phones;
    if (!data->Phone.str().isEmpty()){
        phones = data->Phone.str();
        phones += ",Home Phone,";
		phones += QString::number(PHONE);
    }
    contact->setPhones(phones, name());

    if (contact->getFirstName().isEmpty() && !data->FirstName.str().isEmpty())
        contact->setFirstName(data->FirstName.str(), name());

    if (contact->getName().isEmpty())
        contact->setName(data->ID.str());
}

void InfoRequest::element_start(const char *el, const char**)
{
    m_data = NULL;
    if (!strcmp(el, "vcard")){
        m_bStarted = true;
        return;
    }
    if (!strcmp(el, "nickname")){
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
    if (!strcmp(el, "photo")){
        m_bPhoto = true;
        return;
    }
    if (!strcmp(el, "logo")){
        m_bLogo = true;
        return;
    }
    if (!strcmp(el, "binval")){
        if (m_bPhoto)
            m_cdata = &m_photo;
        if (m_bLogo)
            m_cdata = &m_logo;
    }
}

void InfoRequest::element_end(const char *el)
{
    m_data  = NULL;
    m_cdata = NULL;
    if (!strcmp(el, "photo")){
        m_bPhoto = false;
        return;
    }
    if (!strcmp(el, "logo")){
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
        m_data->append(str, len);
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
        string type = JabberClient::get_attr("type", attr);
        if (type == "result")
            m_client->setInfoUpdated(false);
    }
}

void JabberClient::setClientInfo(void *_data)
{
    JabberUserData *data = (JabberUserData*)_data;
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
    if (!data->Node.str().isEmpty())
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
    AddRequest(JabberClient *client, const char *jid, unsigned grp);
protected:
    virtual void element_start(const char *el, const char **attr);
    string m_jid;
    unsigned m_grp;
};

AddRequest::AddRequest(JabberClient *client, const char *jid, unsigned grp)
        : JabberClient::ServerRequest(client, _SET, NULL, NULL)
{
    m_jid = jid;
    m_grp = grp;
}

void AddRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "iq")){
        string type = JabberClient::get_attr("type", attr);
        if (type == "result"){
            Contact *contact;
            string resource;
            JabberUserData *data = m_client->findContact(m_jid.c_str(), NULL, true, contact, resource);
            if (data && (contact->getGroup() != m_grp)){
                contact->setGroup(m_grp);
                Event e(EventContactChanged, contact);
                e.process();
            }
        }
    }
}

bool JabberClient::add_contact(const char *jid, unsigned grp)
{
    Contact *contact;
    string resource;
    if (findContact(jid, NULL, false, contact, resource)){
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

static unsigned get_number(string &s, unsigned digits)
{
    if (s.length() < digits){
        s = "";
        return 0;
    }
    string p = s.substr(0, digits);
    s = s.substr(digits);
    return atol(p.c_str());
}

static time_t fromDelay(const char *t)
{
    string s = t;
    time_t now;
    time(&now);
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
            }else if (m_status == "Invisible"){
                status = STATUS_ONLINE;
                bInvisible = true;
            }else if (!m_status.empty()){
                status = STATUS_ONLINE;
            }
        }else{
            log(L_DEBUG, "Unsupported available status %s", m_show.c_str());
        }
    }else{
        log(L_DEBUG, "Unsupported presence type %s", m_type.c_str());
    }
    time_t now;
    time(&now);
    time_t time1 = now;
    time_t time2 = 0;
    if (!m_stamp1.empty())
        time1 = fromDelay(m_stamp1.c_str());
    if (!m_stamp2.empty()){
        time2 = fromDelay(m_stamp2.c_str());
        if (time2 > time1){
            time_t t = time1;
            time1 = time2;
            time2 = t;
        }
    }

    if (status != STATUS_UNKNOWN){
        Contact *contact;
        string resource;
        JabberUserData *data = m_client->findContact(m_from.c_str(), NULL, false, contact, resource);
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
                    vector<string> resources;
                    vector<string> resourceReply;
                    vector<string> resourceStatus;
                    vector<string> resourceStatusTime;
                    vector<string> resourceOnlineTime;
                    for (unsigned n = 1; n <= data->nResources.toULong(); n++){
                        if (i == n)
                            continue;
                        resources.push_back(get_str(data->Resources, n));
                        resourceReply.push_back(get_str(data->ResourceReply, n));
                        resourceStatus.push_back(get_str(data->ResourceStatus, n));
                        resourceStatusTime.push_back(get_str(data->ResourceStatusTime, n));
                        resourceOnlineTime.push_back(get_str(data->ResourceOnlineTime, n));
                    }
                    data->Resources.clear();
                    data->ResourceReply.clear();
                    data->ResourceStatus.clear();
                    data->ResourceStatusTime.clear();
                    data->ResourceOnlineTime.clear();
                    for (i = 0; i < resources.size(); i++){
                        set_str(&data->Resources, i + 1, resources[i].c_str());
                        set_str(&data->ResourceReply, i + 1, resourceReply[i].c_str());
                        set_str(&data->ResourceStatus, i + 1, resourceStatus[i].c_str());
                        set_str(&data->ResourceStatusTime, i + 1, resourceStatusTime[i].c_str());
                        set_str(&data->ResourceOnlineTime, i + 1, resourceOnlineTime[i].c_str());
                    }
                    data->nResources.asULong() = resources.size();
                }
                if (data->nResources.toULong() == 0)
                    data->AutoReply.str() = QString::fromUtf8(m_status.c_str());
            }else{
                if (i > data->nResources.toULong()){
                    bChanged = true;
                    data->nResources.asULong() = i;
                    set_str(&data->Resources, i, resource.c_str());
                    set_str(&data->ResourceOnlineTime, i, number(time2 ? time2 : time1).c_str());
                }
                if (number(status) != get_str(data->ResourceStatus, i)){
                    bChanged = true;
                    set_str(&data->ResourceStatus, i, number(status).c_str());
                    set_str(&data->ResourceStatusTime, i, number(time1).c_str());
                }
                if (m_status != get_str(data->ResourceReply, i)){
                    bChanged = true;
                    set_str(&data->ResourceReply, i, m_status.c_str());
                }
            }
            bool bOnLine = false;
            status = STATUS_OFFLINE;
            for (i = 1; i <= data->nResources.toULong(); i++){
                unsigned rStatus = atol(get_str(data->ResourceStatus, i));
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
                data->Status.asULong() = status;
                data->StatusTime.asULong() = time1;
            }
            if (data->invisible.toBool() != bInvisible){
                data->invisible.asBool() = bInvisible;
                bChanged = true;
            }
            if (bChanged){
                StatusMessage m;
                m.setContact(contact->id());
                m.setClient(m_client->dataName(data));
                m.setFlags(MESSAGE_RECEIVED);
                m.setStatus(status);
                Event e(EventMessageReceived, &m);
                e.process();
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
            string stamp = JabberClient::get_attr("stamp", attr);
            if (!stamp.empty()){
                if (m_stamp1.empty()){
                    m_stamp1 = stamp;
                }else if (m_stamp2.empty()){
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
    m_data.append(str, len);
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
        string proto = m_url.substr(0, 7);
        if (proto != "http://"){
            log(L_WARN, "Unknown protocol");
            return;
        }
        m_url = m_url.substr(7);
        int n = m_url.find(':');
        if (n < 0){
            log(L_WARN, "Port not found");
            return;
        }
        string host = m_url.substr(0, n);
        unsigned short port = (unsigned short)atol(m_url.c_str() + n + 1);
        n = m_url.find('/');
        if (n < 0){
            log(L_WARN, "File not found");
            return;
        }
        string file = m_url.substr(n + 1);
        msg = new JabberFileMessage;
        msg->setDescription(QString::fromUtf8(file.c_str()));
        msg->setText(QString::fromUtf8(m_descr.c_str()));
        msg->setHost(host.c_str());
        msg->setPort(port);
    }else if (!m_file_name.empty()){
        msg = new JabberFileMessage;
        msg->setDescription(QString::fromUtf8(m_file_name.c_str()));
        msg->setSize(m_file_size);
    }
    if (msg){
        Contact *contact;
        string resource;
        JabberUserData *data = m_client->findContact(m_from.c_str(), NULL, false, contact, resource);
        if (data == NULL){
            string resource;
            data = m_client->findContact(m_from.c_str(), NULL, true, contact, resource);
            if (data == NULL)
                return;
            contact->setFlags(CONTACT_TEMP);
        }
        msg->setFrom(m_from.c_str());
        msg->setID(m_id.c_str());
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
        return;
    }
    if (!strcmp(el, "query")){
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
                    string resource;
                    JabberUserData *data = m_client->findContact(jid.c_str(), name.c_str(), false, contact, resource);
                    if ((data == NULL) && (subscribe != SUBSCRIBE_NONE)){
                        string resource;
                        data = m_client->findContact(jid.c_str(), name.c_str(), true, contact, resource);
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
        }
    }
    if (!strcmp(el, "url"))
        m_data = &m_url;
    if (!strcmp(el, "desc"))
        m_data = &m_descr;
    if (!strcmp(el, "file")){
        m_file_name = JabberClient::get_attr("name", attr);
        m_file_size = atol(JabberClient::get_attr("size", attr).c_str());
    }
}

void JabberClient::IqRequest::element_end(const char*)
{
    m_data = NULL;
}

void JabberClient::IqRequest::char_data(const char *data, int len)
{
    if (m_data)
        m_data->append(data, len);
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
    if (m_from.empty())
        return;
    Contact *contact;
    string resource;
    JabberUserData *data = m_client->findContact(m_from.c_str(), NULL, false, contact, resource);
    if (data == NULL){
        data = m_client->findContact(m_from.c_str(), NULL, true, contact, resource);
        if (data == NULL)
            return;
        contact->setFlags(CONTACT_TEMP);
    }
    Message *msg = NULL;
    if (!m_id.empty()){
        if (m_bError)
            return;
        QString typing_id = data->TypingId.str();
        QString new_typing_id;
        bool bProcess = false;
        while (!typing_id.isEmpty()){
            QString id = getToken(typing_id, ';');
            if (id == m_id){
                if (!m_bCompose)
                    continue;
                bProcess = true;
            }
            if (!new_typing_id.isEmpty())
                new_typing_id += ";";
            new_typing_id += id;
        }
        if (!bProcess && m_bCompose){
            if (!new_typing_id.isEmpty())
                new_typing_id += ";";
            new_typing_id += m_id;
        }
        if (data->TypingId.setStr(new_typing_id)){
            Event e(EventContactStatus, contact);
            e.process();
        }
    }
    if (m_errorCode || !m_error.empty()){
        if (!m_bEvent){
            JabberMessageError *m = new JabberMessageError;
            m->setError(QString::fromUtf8(m_error.c_str()));
            m->setCode(m_errorCode);
            msg = m;
        }
    }else if (m_bBody){
        if (!m_contacts.empty()){
            msg = new ContactsMessage;
            static_cast<ContactsMessage*>(msg)->setContacts(QString::fromUtf8(m_contacts.c_str()));
        }else if (m_subj.empty()){
            msg = new Message(MessageGeneric);
        }else{
            JabberMessage *m = new JabberMessage;
            m->setSubject(QString::fromUtf8(m_subj.c_str()));
            msg = m;
        }
    }
    if (msg == NULL)
        return;
    if (m_bBody && m_contacts.empty()){
        if (m_richText.empty()){
            data->richText.asBool() = false;
            msg->setText(QString::fromUtf8(m_body.c_str()));
        }else{
            JabberBgParser p;
            msg->setText(p.parse(QString::fromUtf8(m_richText.c_str())));
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
                text += quoteString(QString::fromUtf8(m_targets[i].c_str()));
                text += "\">";
                text += quoteString(QString::fromUtf8(m_descs[i].c_str()));
                text += "</a>";
            }
        }
    }else{
        msg->setText(QString::fromUtf8(m_body.c_str()));
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
        m_errorCode = atol(JabberClient::get_attr("code", attr).c_str());
        m_data = &m_error;
        return;
    }
    if (!strcmp(el, "composing")){
        m_bCompose = true;
        return;
    }
    if (!strcmp(el, "id")){
        m_data = &m_id;
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
        string jid  = JabberClient::get_attr("jid", attr);
        string name = JabberClient::get_attr("name", attr);
        if (!jid.empty()){
            if (!m_contacts.empty())
                m_contacts += ";";
            m_contacts += "jabber:";
            m_contacts += jid;
            if (name.empty()){
                int n = jid.find('@');
                if (n >= 0){
                    name = jid.substr(0, n);
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
        string xmlns = JabberClient::get_attr("xmlns", attr);
        if (xmlns == "http://jabber.org/protocol/xhtml-im"){
            m_bRichText = true;
            m_data = &m_richText;
        }
    }
}

void JabberClient::MessageRequest::element_end(const char *el)
{
    if (m_bRichText){
        if (!strcmp(el, "html")){
            m_bRichText = false;
            m_data = NULL;
            return;
        }
        *m_data += "</";
        *m_data += el;
        *m_data += ">";
        return;
    }
    if (!strcmp(el, "x"))
        m_bRosters = false;
    if (!strcmp(el, "url-data")){
        if (!m_target.empty()){
            if (m_desc.empty())
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
        m_data->append(str, len);
}

class AgentRequest : public JabberClient::ServerRequest
{
public:
    AgentRequest(JabberClient *client, const char *jid);
    ~AgentRequest();
protected:
    JabberAgentsInfo	data;
    string m_data;
    string m_jid;
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
    AgentDiscoRequest(JabberClient *client, const char *jid);
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

AgentDiscoRequest::AgentDiscoRequest(JabberClient *client, const char *jid)
        : ServerRequest(client, _GET, NULL, jid)
{
    load_data(jabberAgentsInfo, &data);
    data.ID.str() = QString::fromUtf8(jid);
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
        data.Name.str() = QString::fromUtf8(JabberClient::get_attr("name", attr).c_str());
        return;
    }
    if (!strcmp(el, "feature")){
        string s = JabberClient::get_attr("var", attr);
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
    string jid = JabberClient::get_attr("jid", attr);
    if (!jid.empty()){
        AgentDiscoRequest *req = new AgentDiscoRequest(m_client, jid.c_str());
        req->start_element("query");
        req->add_attribute("xmlns", "http://jabber.org/protocol/disco#info");
        req->send();
        m_client->m_requests.push_back(req);
    }
}

AgentRequest::AgentRequest(JabberClient *client, const char *jid)
        : ServerRequest(client, _GET, NULL, jid)
{
    load_data(jabberAgentsInfo, &data);
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
        load_data(jabberAgentsInfo, &data);
        m_data = JabberClient::get_attr("jid", attr);
        data.ID.str() = QString::fromUtf8(m_data.c_str());
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
        data.Name.str() = QString::fromUtf8(m_data.c_str());
    }
}

void AgentRequest::char_data(const char *el, int len)
{
    m_data.append(el, len);
}

string JabberClient::get_agents(const QString &jid)
{
    AgentRequest *req = new AgentRequest(this, jid.utf8());
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
    AgentInfoRequest(JabberClient *client, const char *jid);
    ~AgentInfoRequest();
protected:
    JabberAgentInfo		data;
    bool   m_bOption;
    string m_data;
    string m_jid;
    string m_error;
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


AgentInfoRequest::AgentInfoRequest(JabberClient *client, const char *jid)
        : ServerRequest(client, _GET, NULL, jid)
{
    m_jid = jid;
    m_bOption = false;
    m_error_code = 0;
    m_bError = false;
    load_data(jabberAgentInfo, &data);
}

AgentInfoRequest::~AgentInfoRequest()
{
    free_data(jabberAgentInfo, &data);
    load_data(jabberAgentInfo, &data);
    data.ID.str() = QString::fromUtf8(m_jid.c_str());
    data.ReqID.str() = m_id;
    data.nOptions.asULong() = m_error_code;
    data.Label.str() = QString::fromUtf8(m_error.c_str());
    Event e(EventAgentInfo, &data);
    e.process();
    free_data(jabberAgentInfo, &data);
}

void AgentInfoRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "error")){
        m_bError = true;
        m_error_code = atol(JabberClient::get_attr("code", attr).c_str());
    }
    if (m_bError)
        return;
    if (!strcmp(el, "field")){
        free_data(jabberAgentInfo, &data);
        load_data(jabberAgentInfo, &data);
        data.ID.str() = QString::fromUtf8(m_jid.c_str());
        m_data = JabberClient::get_attr("var", attr);
        data.Field.str() = QString::fromUtf8(m_data.c_str());
        m_data = JabberClient::get_attr("type", attr);
        data.Type.str() = QString::fromUtf8(m_data.c_str());
        m_data = JabberClient::get_attr("label", attr);
        data.Label.str() = QString::fromUtf8(m_data.c_str());
    }
    if (!strcmp(el, "option")){
        m_bOption = true;
        m_data = JabberClient::get_attr("label", attr);
        set_str(&data.OptionLabels, data.nOptions.toULong(), m_data.c_str());
    }
    if (!strcmp(el, "x")){
        data.VHost.str() = m_client->VHost();
        data.Type.str() = "x";
        data.ReqID.str() = m_id;
        data.ID.str() = QString::fromUtf8(m_jid.c_str());
        Event e(EventAgentInfo, &data);
        e.process();
        free_data(jabberAgentInfo, &data);
        load_data(jabberAgentInfo, &data);
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
        data.Desc.str() = QString::fromUtf8(m_data.c_str());
        return;
    }
    if (!strcmp(el, "field")){
        if (!data.Field.str().isEmpty()){
            data.VHost.str() = m_client->VHost();
            data.ReqID.str() = m_id;
            data.ID.str() = QString::fromUtf8(m_jid.c_str());
            Event e(EventAgentInfo, &data);
            e.process();
            free_data(jabberAgentInfo, &data);
            load_data(jabberAgentInfo, &data);
        }
    }else if (!strcmp(el, "option")){
        m_bOption = false;
        const char *str = get_str(data.Options, data.nOptions.toULong());
        if (str && *str)
            data.nOptions.asULong()++;
    }else if (!strcmp(el, "value")){
        if (m_bOption){
            set_str(&data.Options, data.nOptions.toULong(), m_data.c_str());
        }else{
            data.Value.str() = QString::fromUtf8(m_data.c_str());
        }
    }else if (!strcmp(el, "required")){
        data.bRequired.asBool() = true;
    }else if (!strcmp(el, "key") || !strcmp(el, "instructions")){
        data.Value.str() = QString::fromUtf8(m_data.c_str());
        data.ID.str() = QString::fromUtf8(m_jid.c_str());
        data.ReqID.str() = m_id;
        data.Type.str() = QString::fromUtf8(el);
        Event e(EventAgentInfo, &data);
        e.process();
        free_data(jabberAgentInfo, &data);
        load_data(jabberAgentInfo, &data);
    }else if (strcmp(el, "error") && strcmp(el, "iq") && strcmp(el, "query") && strcmp(el, "x")){
        data.Value.str() = QString::fromUtf8(m_data.c_str());
        data.ID.str() = QString::fromUtf8(m_jid.c_str());
        data.ReqID.str() = m_id;
        data.Type.str() = QString::fromUtf8(el);
        Event e(EventAgentInfo, &data);
        e.process();
        free_data(jabberAgentInfo, &data);
        load_data(jabberAgentInfo, &data);
    }
}

void AgentInfoRequest::char_data(const char *el, int len)
{
    m_data.append(el, len);
}

string JabberClient::get_agent_info(const char *jid, const char *node, const char *type)
{
    AgentInfoRequest *req = new AgentInfoRequest(this, jid);
    req->start_element("query");
    string xmlns = "jabber:iq:";
    xmlns += type;
    req->add_attribute("xmlns", xmlns.c_str());
    if (node && *node)
        req->add_attribute("node", node);
    addLang(req);
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

typedef map<my_string, string> VALUE_MAP;

class SearchRequest : public JabberClient::ServerRequest
{
public:
    SearchRequest(JabberClient *client, const char *jid);
    ~SearchRequest();
protected:
    JabberSearchData data;
    string m_data;
    string m_attr;
    list<string> m_fields;
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


SearchRequest::SearchRequest(JabberClient *client, const char *jid)
        : ServerRequest(client, _SET, NULL, jid)
{
    load_data(jabberSearchData, &data);
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
        load_data(jabberSearchData, &data);
        m_data = JabberClient::get_attr("jid", attr);
        data.JID.str() = QString::fromUtf8(m_data.c_str());
    }else if (!strcmp(el, "field")){
        string var = JabberClient::get_attr("var", attr);
        if (m_bReported){
            if (!var.empty() && (var != "jid")){
                string label = JabberClient::get_attr("label", attr);
                if (label.empty())
                    label = var;
                m_values.insert(VALUE_MAP::value_type(var.c_str(), label));
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
        load_data(jabberSearchData, &data);
        for (list<string>::iterator it = m_fields.begin(); it != m_fields.end(); ++it){
            string value;
            VALUE_MAP::iterator itv = m_values.find((*it).c_str());
            if (itv != m_values.end())
                value = (*itv).second;
            set_str(&data.Fields, data.nFields.toULong() * 2, value.c_str());
            set_str(&data.Fields, data.nFields.toULong() * 2 + 1, value.c_str());
            data.nFields.asULong()++;
        }
        data.ID.str() = m_id;
        Event e(EventSearch, &data);
        e.process();
        m_values.clear();
    }else if (!strcmp(el, "item")){
        if (!data.JID.str().isEmpty()){
            for (list<string>::iterator it = m_fields.begin(); it != m_fields.end(); ++it){
                VALUE_MAP::iterator itv = m_values.find((*it).c_str());
                if (itv != m_values.end()){
                    string val = (*itv).second.c_str();
                    set_str(&data.Fields, data.nFields.toULong(), val.c_str());
                }
                data.nFields.asULong()++;
            }
            data.ID.str() = m_id;
            Event e(EventSearch, &data);
            e.process();
            m_values.clear();
        }
    }else if (!strcmp(el, "value") || !strcmp(el, "field")){
        if (!m_attr.empty() && !m_data.empty()){
            if (m_attr == "jid"){
                data.JID.str() = QString::fromUtf8(m_data.c_str());
            }else{
                m_values.insert(VALUE_MAP::value_type(m_attr.c_str(), m_data));
            }
        }
        m_attr = "";
    }else if (!strcmp(el, "first")){
        data.First.str() = QString::fromUtf8(m_data.c_str());
    }else if (!strcmp(el, "last")){
        data.Last.str() = QString::fromUtf8(m_data.c_str());
    }else if (!strcmp(el, "nick")){
        data.Nick.str() = QString::fromUtf8(m_data.c_str());
    }else if (!strcmp(el, "email")){
        data.EMail.str() = QString::fromUtf8(m_data.c_str());
    }else if (!strcmp(el, "status")){
        data.Status.str() = QString::fromUtf8(m_data.c_str());
    }
}

void SearchRequest::char_data(const char *el, int len)
{
    m_data.append(el, len);
}

string JabberClient::search(const char *jid, const char *node, const char *condition)
{
    SearchRequest *req = new SearchRequest(this, jid);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:search");
    if (node && *node)
        req->add_attribute("node", node);
    req->add_condition(QString::fromUtf8(condition), false);
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
    string   m_error;
    string  *m_data;
    unsigned m_error_code;
    virtual void element_start(const char *el, const char **attr);
    virtual void element_end(const char *el);
    virtual void char_data(const char *el, int len);
};

RegisterRequest::RegisterRequest(JabberClient *client, const char *jid)
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
    ai.error = m_error.c_str();
    Event e(EventAgentRegister, &ai);
    e.process();
}

void RegisterRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "error")){
        m_error_code = atol(JabberClient::get_attr("code", attr).c_str());
        if (m_error_code == 0)
            m_error_code = (unsigned)(-1);
        m_data = &m_error;
        return;
    }
    if (!strcmp(el, "iq")){
        string type = JabberClient::get_attr("type", attr);
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
    m_data->append(el, len);
}

string JabberClient::process(const char *jid, const char *node, const char *condition, const char *type)
{
    RegisterRequest *req = new RegisterRequest(this, jid);
    req->start_element("query");
    string xmlns = "jabber:iq:";
    xmlns += type;
    req->add_attribute("xmlns", xmlns.c_str());
    bool bData = (strcmp(type, "data") == 0);
    if (bData)
        req->add_attribute("type", "submit");
    if (node && *node)
        req->add_attribute("node", node);
    req->add_condition(QString::fromUtf8(condition), bData);
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
    SendFileRequest(JabberClient *client, const char *jid, FileMessage *msg);
    ~SendFileRequest();
protected:
    virtual void	element_start(const char *el, const char **attr);
    virtual void	element_end(const char *el);
    virtual	void	char_data(const char *str, int len);
    string *m_data;
    string m_error;
    bool   m_bError;
    FileMessage *m_msg;
};

SendFileRequest::SendFileRequest(JabberClient *client, const char *jid, FileMessage *msg)
        : JabberClient::ServerRequest(client, _SET, NULL, jid)
{
    m_msg    = msg;
    m_data   = NULL;
    m_bError = false;
}

SendFileRequest::~SendFileRequest()
{
    if (m_msg && m_bError){
        if (m_error.empty())
            m_error = I18N_NOOP("File transfer declined");
        m_msg->setError(m_error.c_str());
        Event e(EventMessageSent, m_msg);
        e.process();
        delete m_msg;
    }
}

void SendFileRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "iq")){
        string type = JabberClient::get_attr("type", attr);
        if (type == "error")
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
        m_data->append(str, len);
}

#endif

class SendFileRequest : public JabberClient::ServerRequest
{
public:
    SendFileRequest(JabberClient *client, const char *jid, FileMessage *msg);
    ~SendFileRequest();
protected:
    virtual void	element_start(const char *el, const char **attr);
    virtual void	element_end(const char *el);
    virtual	void	char_data(const char *str, int len);
    FileMessage *m_msg;
};

SendFileRequest::SendFileRequest(JabberClient *client, const char *jid, FileMessage *msg)
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

void JabberClient::sendFileRequest(FileMessage *msg, unsigned short, JabberUserData *data, const char *fname, unsigned size)
{
    QString jid = data->ID.str();
    string resource;
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
    string resource;
    if (msg->getResource().isEmpty()){
        if (!data->Resource.str().isEmpty()){
            jid += "/";
            jid += data->Resource.str();
        }
    }else{
        jid += "/";
        jid += msg->getResource();
    }
    ServerRequest req(this, ServerRequest::_RESULT, NULL, jid.utf8(), m->getID());
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
    DiscoItemsRequest(JabberClient *client, const char *jid);
    ~DiscoItemsRequest();
protected:
    virtual void	element_start(const char *el, const char **attr);
    virtual void	element_end(const char *el);
    virtual	void	char_data(const char *str, int len);
    string			*m_data;
    string			m_error;
    unsigned		m_code;
};

DiscoItemsRequest::DiscoItemsRequest(JabberClient *client, const char *jid)
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
        if (!item.jid.empty()){
            Event e(EventDiscoItem, &item);
            e.process();
        }
    }
    if (!strcmp(el, "error")){
        m_code = atol(JabberClient::get_attr("code", attr).c_str());
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
        m_data->append(buf, len);
}

string JabberClient::discoItems(const QString &jid, const QString &node)
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
    DiscoInfoRequest(JabberClient *client, const char *jid);
    ~DiscoInfoRequest();
protected:
    virtual void	element_start(const char *el, const char **attr);
    virtual void	element_end(const char *el);
    virtual	void	char_data(const char *str, int len);
    string			*m_data;
    string			m_error;
    string			m_features;
    string			m_name;
    string			m_type;
    string			m_category;
    unsigned		m_code;
};

DiscoInfoRequest::DiscoInfoRequest(JabberClient *client, const char *jid)
        : JabberClient::ServerRequest(client, _GET, NULL, jid)
{
    m_data = NULL;
    m_code = 0;
}

DiscoInfoRequest::~DiscoInfoRequest()
{
    if (m_code == 0){
        DiscoItem item;
        item.id			= m_id;
        item.jid		= "info";
        item.name		= m_name;
        item.category	= m_category;
        item.type		= m_type;
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
        string feature = JabberClient::get_attr("var", attr);
        if (!feature.empty()){
            if (!m_features.empty())
                m_features += "\n";
            m_features += feature;
        }
    }
    if (!strcmp(el, "error")){
        m_code = atol(JabberClient::get_attr("code", attr).c_str());
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
        m_data->append(buf, len);
}

string JabberClient::discoInfo(const QString &jid, const QString &node)
{
    if (getState() != Connected)
        return "";
    DiscoInfoRequest *req = new DiscoInfoRequest(this, jid.utf8());
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
    BrowseRequest(JabberClient *client, const char *jid);
    ~BrowseRequest();
protected:
    virtual void	element_start(const char *el, const char **attr);
    virtual void	element_end(const char *el);
    virtual	void	char_data(const char *str, int len);
    string			*m_data;
    string			m_jid;
    string			m_error;
    string			m_name;
    string			m_type;
    string			m_category;
    string			m_features;
    string			m_ns;
    unsigned		m_code;
};

BrowseRequest::BrowseRequest(JabberClient *client, const char *jid)
        : JabberClient::ServerRequest(client, _GET, NULL, jid)
{
    m_data	= NULL;
    m_code	= 0;
    m_jid	= jid;
}

BrowseRequest::~BrowseRequest()
{
    if (!m_jid.empty() && !m_name.empty() && (m_code == 0)){
        DiscoItem item;
        item.id			= m_id;
        item.jid		= m_jid;
        item.name		= m_name;
        item.type		= m_type;
        item.category	= m_category;
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

void BrowseRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "error")){
        m_code = atol(JabberClient::get_attr("code", attr).c_str());
        m_data = &m_error;
    }
    if (!strcmp(el, "item") || !strcmp(el, "service") || !strcmp(el, "agent") || !strcmp(el, "headline")){
        if (!m_jid.empty() && !m_name.empty()){
            DiscoItem item;
            item.id			= m_id;
            item.jid		= m_jid;
            item.name		= m_name;
            item.type		= m_type;
            item.category	= m_category;
            item.features	= m_features;
            Event e(EventDiscoItem, &item);
            e.process();
        }
        m_jid		= JabberClient::get_attr("jid", attr).c_str();
        m_name		= JabberClient::get_attr("name", attr).c_str();
        m_type		= JabberClient::get_attr("type", attr).c_str();
        m_category	= JabberClient::get_attr("category", attr).c_str();
        if (!strcmp(el, "headline"))
            m_category = "headline";
        m_features	= "";
    }
    if (!strcmp(el, "query")){
        m_name		= JabberClient::get_attr("name", attr).c_str();
        m_type		= JabberClient::get_attr("type", attr).c_str();
        m_category	= JabberClient::get_attr("category", attr).c_str();
    }
    if (!strcmp(el, "ns"))
        m_data  = &m_ns;
}

void BrowseRequest::element_end(const char *el)
{
    if (!strcmp(el, "error"))
        m_data = NULL;
    if (!strcmp(el, "ns") && !m_ns.empty()){
        if (!m_features.empty())
            m_features += "\n";
        m_features += m_ns;
        m_ns = "";
        m_data = NULL;
    }
    if ((!strcmp(el, "item") || !strcmp(el, "service") ||
            !strcmp(el, "agent") || !strcmp(el, "headline"))
            && !m_jid.empty()){
        DiscoItem item;
        item.id			= m_id;
        item.jid		= m_jid;
        item.name		= m_name;
        item.type		= m_type;
        item.category	= m_category;
        item.features	= m_features;
        Event e(EventDiscoItem, &item);
        e.process();
        m_jid = "";
    }
}

void BrowseRequest::char_data(const char *buf, int len)
{
    if (m_data)
        m_data->append(buf, len);
}

string JabberClient::browse(const QString &jid)
{
    if (getState() != Connected)
        return "";
    BrowseRequest *req = new BrowseRequest(this, jid.utf8());
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:browse");
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

class VersionInfoRequest : public JabberClient::ServerRequest
{
public:
    VersionInfoRequest(JabberClient *client, const char *jid);
    ~VersionInfoRequest();
protected:
    virtual void	element_start(const char *el, const char **attr);
    virtual void	element_end(const char *el);
    virtual	void	char_data(const char *str, int len);
    string			*m_data;
    string			m_name;
    string			m_version;
    string			m_os;
};

VersionInfoRequest::VersionInfoRequest(JabberClient *client, const char *jid)
        : JabberClient::ServerRequest(client, _GET, NULL, jid)
{
    m_data = NULL;
}

VersionInfoRequest::~VersionInfoRequest()
{
    DiscoItem item;
    item.id		= m_id;
    item.jid	= m_version;
    item.name	= m_name;
    item.node	= m_os;
    Event e(EventDiscoItem, &item);
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
        m_data->append(buf, len);
}

string JabberClient::versionInfo(const QString &jid, const QString &node)
{
    if (getState() != Connected)
        return "";
    VersionInfoRequest *req = new VersionInfoRequest(this, jid.utf8());
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
    TimeInfoRequest(JabberClient *client, const char *jid);
    ~TimeInfoRequest();
protected:
    virtual void	element_start(const char *el, const char **attr);
    virtual void	element_end(const char *el);
    virtual	void	char_data(const char *str, int len);
    string			*m_data;
    string			m_time;
};

TimeInfoRequest::TimeInfoRequest(JabberClient *client, const char *jid)
        : JabberClient::ServerRequest(client, _GET, NULL, jid)
{
    m_data = NULL;
}

TimeInfoRequest::~TimeInfoRequest()
{
    DiscoItem item;
    item.id		= m_id;
    item.jid	= m_time;
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
        m_data->append(buf, len);
}

string JabberClient::timeInfo(const QString &jid, const QString &node)
{
    if (getState() != Connected)
        return "";
    TimeInfoRequest *req = new TimeInfoRequest(this, jid.utf8());
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
    LastInfoRequest(JabberClient *client, const char *jid);
protected:
    virtual void	element_start(const char *el, const char **attr);
};

LastInfoRequest::LastInfoRequest(JabberClient *client, const char *jid)
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

string JabberClient::lastInfo(const QString &jid, const QString &node)
{
    if (getState() != Connected)
        return "";
    LastInfoRequest *req = new LastInfoRequest(this, jid.utf8());
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
    StatRequest(JabberClient *client, const char *jid, const char *id);
    ~StatRequest();
protected:
    virtual void	element_start(const char *el, const char **attr);
    string	m_id;
};

StatRequest::StatRequest(JabberClient *client, const char *jid, const char *id)
        : JabberClient::ServerRequest(client, _GET, NULL, jid)
{
    m_id = id;
}

StatRequest::~StatRequest()
{
    DiscoItem item;
    item.id		= m_id;
    item.jid	= "";
    Event e(EventDiscoItem, &item);
    e.process();
}

void StatRequest::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "stat")){
        DiscoItem item;
        item.id		= m_id;
        item.jid	= JabberClient::get_attr("name", attr);;
        item.name	= JabberClient::get_attr("units", attr);;
        item.node	= JabberClient::get_attr("value", attr);;
        Event e(EventDiscoItem, &item);
        e.process();
    }
}

class StatItemsRequest : public JabberClient::ServerRequest
{
public:
    StatItemsRequest(JabberClient *client, const char *jid, const char *node);
    ~StatItemsRequest();
protected:
    virtual void	element_start(const char *el, const char **attr);
    list<string>    m_stats;
    string			m_jid;
    string			m_node;
};

StatItemsRequest::StatItemsRequest(JabberClient *client, const char *jid, const char *node)
        : JabberClient::ServerRequest(client, _GET, NULL, jid)
{
    m_jid  = jid;
    if (node)
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
    StatRequest *req = new StatRequest(m_client, m_jid.c_str(), m_id.utf8());
    req->start_element("query");
    req->add_attribute("xmlns", "http://jabber.org/protocol/stats");
    if (!m_node.empty())
        req->add_attribute("node", m_node);
    m_client->addLang(req);
    for (list<string>::iterator it = m_stats.begin(); it != m_stats.end(); ++it){
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
        string name = JabberClient::get_attr("name", attr);
        if (!name.empty())
            m_stats.push_back(name);
    }
}

string JabberClient::statInfo(const QString &jid, const QString &node)
{
    if (getState() != Connected)
        return "";
    StatItemsRequest *req = new StatItemsRequest(this, jid.utf8(), node.utf8());
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
    string	m_password;
};

ChangePasswordRequest::ChangePasswordRequest(JabberClient *client, const char *password)
        : JabberClient::ServerRequest(client, _SET, NULL, NULL)
{
    m_password = password;
}

ChangePasswordRequest::~ChangePasswordRequest()
{
    m_client->setPassword(m_password.c_str());
}

void JabberClient::changePassword(const char *password)
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


