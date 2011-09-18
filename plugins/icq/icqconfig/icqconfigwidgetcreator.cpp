/*
 * icqconfigwidgetcreator.cpp
 *
 *  Created on: Aug 31, 2011
 */

#include "icqconfigwidgetcreator.h"
#include "icqinfo.h"
#include "aboutinfo.h"
#include "homeinfo.h"
#include "interestsinfo.h"
#include "workinfo.h"
#include "moreinfo.h"
#include "pastinfo.h"
#include "icqpicture.h"
#include "contacts/contactlist.h"
#include "icqclient.h"
#include "icqcontact.h"

IcqConfigWidgetCreator::IcqConfigWidgetCreator(ICQClient* client) : m_client(client)
{
}

IcqConfigWidgetCreator::~IcqConfigWidgetCreator()
{
}

void IcqConfigWidgetCreator::contactConfigRequested(SIM::WidgetHierarchy* hierarchy, const QString& context)
{
    int contactId = context.toInt();
    SIM::ContactPtr contact = SIM::getContactList()->contact(contactId);
    if(!contact)
        return;

    ICQContactPtr icqcontact = contact->clientContact(m_client->name()).dynamicCast<ICQContact>();
    if(!icqcontact)
        return;

    SIM::WidgetHierarchy h;
    h.nodeName = QString("ICQ ") + m_client->ownerIcqContact()->getScreen();
    h.widget = new ICQInfo(0, icqcontact, m_client);
    h.iconId = "ICQ_online";

    addIcqConfigWidgets(&h, icqcontact);

    hierarchy->children.append(h);
    emit fullInfoRequest(icqcontact->getScreen());
}

void IcqConfigWidgetCreator::globalConfigRequested(SIM::WidgetHierarchy* hierarchy, const QString& context)
{
    ICQContactPtr icqcontact = m_client->ownerIcqContact();
    if(!icqcontact)
        return;

    SIM::WidgetHierarchy h;
    h.nodeName = QString("ICQ ") + m_client->ownerIcqContact()->getScreen();
    h.widget = new ICQInfo(0, icqcontact, m_client);
    h.iconId = "ICQ_online";

    addIcqConfigWidgets(&h, icqcontact);

    hierarchy->children.append(h);
    emit fullInfoRequest(icqcontact->getScreen());
}

void IcqConfigWidgetCreator::addIcqConfigWidgets(SIM::WidgetHierarchy* root, const ICQContactPtr& contact)
{
    SIM::WidgetHierarchy homeinfo;
    homeinfo.nodeName = I18N_NOOP("Home info");
    homeinfo.widget = new HomeInfo(0, contact, m_client);
    homeinfo.iconId = "home";
    root->children.append(homeinfo);

    SIM::WidgetHierarchy workinfo;
    workinfo.nodeName = I18N_NOOP("Work info");
    workinfo.widget = new WorkInfo(0, contact, m_client);
    workinfo.iconId = "work";
    root->children.append(workinfo);

    SIM::WidgetHierarchy moreinfo;
    moreinfo.nodeName = I18N_NOOP("More info");
    moreinfo.widget = new MoreInfo(0, contact, m_client);
    moreinfo.iconId = "more";
    root->children.append(moreinfo);

    SIM::WidgetHierarchy aboutinfo;
    aboutinfo.nodeName = I18N_NOOP("About");
    aboutinfo.widget = new AboutInfo(0, contact, m_client);
    aboutinfo.iconId = "info";
    root->children.append(aboutinfo);

    SIM::WidgetHierarchy interests;
    interests.nodeName = I18N_NOOP("Interest");
    interests.widget = new InterestsInfo(0, contact, m_client);
    interests.iconId = "interest";
    root->children.append(interests);

    SIM::WidgetHierarchy past;
    past.nodeName = I18N_NOOP("Group Past");
    past.widget = new PastInfo(0, contact, m_client);
    past.iconId = "past";
    root->children.append(past);

    SIM::WidgetHierarchy picture;
    picture.nodeName = I18N_NOOP("Picture");
    picture.widget = new ICQPicture(0, contact, m_client);
    picture.iconId = "pict";
    root->children.append(picture);
}
