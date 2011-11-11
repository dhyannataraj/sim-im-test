#include "buddysnachandler.h"
#include "icqclient.h"
#include "log.h"
#include "events/eventhub.h"
#include "events/contactevent.h"
#include "requests/icqrequestmanager.h"
#include "requests/buddysnac/buddysnacrightsrequest.h"
#include "requests/bartsnac/bartsnacavatarrequest.h"

using SIM::log;
using SIM::L_DEBUG;
using SIM::L_WARN;

BuddySnacHandler::BuddySnacHandler(ICQClient* client) : SnacHandler(client, SnacId),
    m_ready(false), m_cnt(0)
{
}

bool BuddySnacHandler::process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId)
{
    Q_UNUSED(flags);
    Q_UNUSED(requestId);
    switch(subtype)
    {
    case SnacBuddyUserOnline:
        return processUserOnline(data);
        break;
    case SnacBuddyRights:
        {
            log(L_DEBUG, "Buddy snac ready");
            m_ready = true;
            emit ready();
        }
        break;
    default:
        log(L_WARN, "Unknown buddy snac subtype: %04x", subtype);
        break;
    }

    return true;
}

void BuddySnacHandler::requestRights()
{
    ICQRequestManager* manager = client()->requestManager();
    Q_ASSERT(manager);

    manager->enqueue(BuddySnacRightsRequest::create(client()));
}

void BuddySnacHandler::forceReady()
{
    m_ready = true;
    emit ready();
}

bool BuddySnacHandler::isReady()
{
    return m_ready;
}

void BuddySnacHandler::disconnect()
{
    ICQContactList* contactList = m_client->contactList();
    QList<ICQContactPtr> contacts = contactList->allContacts();
    ICQStatusPtr offline = client()->getDefaultStatus("offline");
    foreach(const ICQContactPtr& contact, contacts)
    {
        contact->setIcqStatus(offline);
        SIM::getEventHub()->triggerEvent("contact_change_status", SIM::ContactEventData::create(contact->metaContactId()));
    }

    m_ready = false;
}

void BuddySnacHandler::parseBuddyTlvs(const TlvList& list, const ICQContactPtr& contact)
{
    QString screen = contact->getScreen();
    ICQStatusConverter* converter = m_client->statusConverter();
    Tlv statusTlv = list.firstTlv(TlvOnlineStatus);
    if(statusTlv.isValid())
    {
        log(L_DEBUG, "Buddy online[%s] status: %08x", qPrintable(screen), statusTlv.toUint32());
        ICQStatusPtr newStatus = converter->makeStatus(statusTlv.toUint32());
        if(contact)
        {
            if(newStatus)
                contact->setIcqStatus(newStatus);
            else
                contact->setIcqStatus(m_client->getDefaultStatus("online")->clone().dynamicCast<ICQStatus>());
            log(L_DEBUG, "metaContactId: %08x", contact->metaContactId());
            SIM::getEventHub()->triggerEvent("contact_change_status", SIM::ContactEventData::create(contact->metaContactId()));
        }
    }
    else
    {
        contact->setIcqStatus(client()->getDefaultStatus("online"));
    }

    Tlv avatarTlv = list.firstTlv(TlvAvatar);
    if(avatarTlv.isValid())
    {
        parseAvatarTlv(avatarTlv, contact);
    }
}

void BuddySnacHandler::parseAvatarTlv(const Tlv& avatarTlv, const ICQContactPtr& contact)
{
    ByteArrayParser parser(avatarTlv.data());
    parser.readWord(); // Unknown bytes
    parser.readByte(); // Unknown bytes
    int hashLength = parser.readByte();
    QByteArray hash = parser.readBytes(hashLength);

    if(hash == contact->getAvatarHash())
        return;

    client()->bartSnacHandler()->requestAvatar(contact->getScreen(), hash);
}

bool BuddySnacHandler::processUserOnline(const QByteArray& data)
{
    ICQContactList* contactList = m_client->contactList();

    ByteArrayParser parser(data);
    int screenLength = parser.readByte();
    QByteArray screenRaw = parser.readBytes(screenLength);
    QString screen(screenRaw);

    ICQContactPtr contact = contactList->contactByScreen(screen);
    if(!contact)
    {
        // TODO check if it's in ignore list
        log(L_WARN, "Contact not in contact list: %s", qPrintable(screen));
        return false;
    }

    parser.readWord(); // Read Warning level and discard it
    int tlvCount = parser.readWord();
    Q_UNUSED(tlvCount);

    TlvList list = TlvList::fromByteArray(parser.readAll());
    parseBuddyTlvs(list, contact);

    return true;
}
