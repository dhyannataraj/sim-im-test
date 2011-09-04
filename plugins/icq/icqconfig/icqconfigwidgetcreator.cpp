/*
 * icqconfigwidgetcreator.cpp
 *
 *  Created on: Aug 31, 2011
 */

#include "icqconfigwidgetcreator.h"
#include "icqinfo.h"
#include "homeinfo.h"
#include "workinfo.h"
#include "moreinfo.h"
#include "contacts/contactlist.h"
#include "../icqclient.h"
#include "../icqcontact.h"

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
    SIM::WidgetHierarchy h;
    h.nodeName = QString("ICQ ") + m_client->ownerIcqContact()->getScreen();
    h.widget = new ICQInfo(0, icqcontact, m_client);
    h.iconId = "ICQ_online";

    SIM::WidgetHierarchy homeinfo;
    homeinfo.nodeName = I18N_NOOP("Home info");
    homeinfo.widget = new HomeInfo(0, icqcontact, m_client);
    homeinfo.iconId = "home";
    h.children.append(homeinfo);

    SIM::WidgetHierarchy workinfo;
    workinfo.nodeName = I18N_NOOP("Work info");
    workinfo.widget = new WorkInfo(0, icqcontact, m_client);
    workinfo.iconId = "work";
    h.children.append(workinfo);

    SIM::WidgetHierarchy moreinfo;
    moreinfo.nodeName = I18N_NOOP("More info");
    moreinfo.widget = new MoreInfo(0, icqcontact, m_client);
    moreinfo.iconId = "more";
    h.children.append(moreinfo);

    hierarchy->children.append(h);
}
