/*
 * standardicqrequestmanager.cpp
 *
 *  Created on: Jul 10, 2011
 */

#include "standardicqrequestmanager.h"
#include "../icqclient.h"
#include "oscarsocket.h"
#include "log.h"

using SIM::log;
using SIM::L_DEBUG;

StandardICQRequestManager::StandardICQRequestManager()
{
}

StandardICQRequestManager::~StandardICQRequestManager()
{
    m_requests.clear();
}

void StandardICQRequestManager::enqueue(const ICQRequestPtr& request)
{
    // TODO respect rate limit
    if(m_socket->isConnected())
    {
        request->perform(m_socket);
    }
    else
    {
        log(L_DEBUG, "StandardICQRequestManager::enqueue unconnected socket");
        m_requests.enqueue(request);
    }
}

void StandardICQRequestManager::setOscarSocket(OscarSocket* socket)
{
    m_socket = socket;
}

void StandardICQRequestManager::clearQueue()
{
    m_requests.clear();
}
