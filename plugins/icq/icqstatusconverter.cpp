#include "icqstatusconverter.h"
#include "icqclient.h"
#include "log.h"

using SIM::log;
using SIM::L_ERROR;

ICQStatusConverter::ICQStatusConverter(ICQClient* client) : m_client(client)
{
}

ICQStatusPtr ICQStatusConverter::makeStatus(unsigned int icqStatusId)
{
    ICQStatusPtr status;
    int statusId = icqStatusId & (StatusMask);
    if(statusId == ICQ_STATUS_ONLINE)
    {
        status = m_client->getDefaultStatus("online");
    }
    else if(statusId == ICQ_STATUS_AWAY)
    {
        status = m_client->getDefaultStatus("away");
    }
    else if(statusId & ICQ_STATUS_NA)
    {
        status = m_client->getDefaultStatus("n/a");
    }
    else if(statusId & ICQ_STATUS_OCCUPIED)
    {
        status = m_client->getDefaultStatus("occupied");
    }
    else if(statusId & ICQ_STATUS_DND)
    {
        status = m_client->getDefaultStatus("dnd");
    }
    if(status)
    {
        if(icqStatusId & StatusInvisible)
            status->setFlag(SIM::IMStatus::flInvisible, true);
        return status->clone().dynamicCast<ICQStatus>();
    }
    log(L_ERROR, "Unable to convert status: %08x", icqStatusId);
    return ICQStatusPtr();
}
