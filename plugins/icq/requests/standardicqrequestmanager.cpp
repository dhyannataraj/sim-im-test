/*
 * standardicqrequestmanager.cpp
 *
 *  Created on: Jul 10, 2011
 */

#include "standardicqrequestmanager.h"
#include "icqclient.h"

StandardICQRequestManager::StandardICQRequestManager(ICQClient* client) : m_client(client)
{
}

StandardICQRequestManager::~StandardICQRequestManager()
{
}

void StandardICQRequestManager::enqueue(const ICQRequestPtr& request)
{
    // TODO respect rate limit
    request->perform();
}

