#include "privacysnachandler.h"
#include "icqclient.h"

using SIM::log;
using SIM::L_WARN;

PrivacySnacHandler::PrivacySnacHandler(ICQClient* client) : SnacHandler(client, ICQ_SNACxFOOD_BOS),
    m_ready(false)
{
}

bool PrivacySnacHandler::process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId)
{
    Q_UNUSED(flags);
    Q_UNUSED(requestId);
    switch(subtype)
    {
    case SnacPrivacyRightsInfo:
        return processRightsInfo(data);
    default:
        log(L_WARN, "Unhandled privacy snac, subtype: %04x", subtype);
        break;
    }

    return true;
}

void PrivacySnacHandler::requestRights()
{
    OscarSocket* socket = client()->oscarSocket();
    Q_ASSERT(socket);

    socket->snac(getType(), SnacPrivacyRightsRequest, 0, QByteArray());
}

void PrivacySnacHandler::forceReady()
{
    m_ready = true;
    emit ready();
}

bool PrivacySnacHandler::isReady() const
{
    return m_ready;
}

bool PrivacySnacHandler::processRightsInfo(const QByteArray& data)
{
    Q_UNUSED(data);
    if(!m_ready)
    {
        m_ready = true;
        emit ready();
    }
    return true;
}
