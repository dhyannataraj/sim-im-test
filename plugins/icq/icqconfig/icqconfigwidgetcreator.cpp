/*
 * icqconfigwidgetcreator.cpp
 *
 *  Created on: Aug 31, 2011
 */

#include "icqconfigwidgetcreator.h"
#include "icqinfo.h"
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
    h.nodeName = QString("FIXME");
    h.widget = new ICQInfo(0, icqcontact, m_client);
    hierarchy->children.append(h);
}
