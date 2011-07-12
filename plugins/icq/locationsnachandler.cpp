#include "locationsnachandler.h"
#include "icqclient.h"
#include "log.h"
#include "requests/locationsnac/locationsnacrightsrequest.h"
#include "requests/locationsnac/locationsnacsetuserinforequest.h"

using SIM::log;
using SIM::L_WARN;

LocationSnacHandler::LocationSnacHandler(ICQClient* client) : SnacHandler(client, ICQ_SNACxFOOD_LOCATION),
    m_maxCapabilities(0), m_ready(false)
{
}

bool LocationSnacHandler::process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId)
{
    Q_UNUSED(flags);
    Q_UNUSED(requestId);
    switch(subtype)
    {
    case SnacLocationRightsInfo:
        {
            if(parseRightsInfo(data))
                return sendUserInfo();
            return false;
        }
    default:
        log(L_WARN, "Unhandled location snac, subtype: %04x", subtype);
        return false;
    }

    return true;
}

void LocationSnacHandler::requestRights()
{
    ICQRequestPtr rq = LocationSnacRightsRequest::create(client());

    ICQRequestManager* manager = client()->requestManager();
    Q_ASSERT(manager);
    manager->enqueue(rq);
}

int LocationSnacHandler::maxCapabilities() const
{
    return m_maxCapabilities;
}

void LocationSnacHandler::forceReady()
{
    m_ready = true;
    emit ready();
}

bool LocationSnacHandler::isReady() const
{
    return m_ready;
}

bool LocationSnacHandler::parseRightsInfo(const QByteArray& arr)
{
    TlvList list = TlvList::fromByteArray(arr);
    Tlv tlvMaxCaps = list.firstTlv(TlvMaxCapabilities);
    if(tlvMaxCaps.isValid())
        m_maxCapabilities = tlvMaxCaps.toUint16();
    return true;
}

bool LocationSnacHandler::sendUserInfo()
{
    ICQRequestManager* manager = client()->requestManager();
    Q_ASSERT(manager);

    ClientCapabilitiesRegistry* registry = m_client->clientCapabilitiesRegistry();
    Q_ASSERT(registry);

    QByteArray caps;
    caps.append(registry->capabilityByName("short_caps")->guid());
    caps.append(registry->capabilityByName("aim_support")->guid());
    caps.append(registry->capabilityByName("avatar")->guid());
    caps.append(registry->capabilityByName("send_file")->guid());
    caps.append(registry->capabilityByName("direct")->guid());
    caps.append(registry->capabilityByName("relay")->guid());
    caps.append(registry->capabilityByName("utf")->guid());

    manager->enqueue(LocationSnacSetUserInfoRequest::create(client(), caps));

    m_ready = true;
    emit ready();

    return true;
}
