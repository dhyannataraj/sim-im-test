#ifndef BUDDYSNACHANDLER_H
#define BUDDYSNACHANDLER_H

#include "snac.h"
#include "icq_defines.h"
#include "icqcontact.h"
#include "tlvlist.h"

class ICQ_EXPORT BuddySnacHandler : public SnacHandler
{
    Q_OBJECT
public:
    BuddySnacHandler(ICQClient *client);
    virtual bool process(unsigned short  subtype, const QByteArray & data, int flags, unsigned int requestId);
    void requestRights();
    void forceReady();
    bool isReady();

    static const int SnacId = 0x03;
    static const int SnacBuddyRightsRequest = 0x02;
    static const int SnacBuddyRights = 0x03;
    static const int SnacBuddyUserOnline = 0x0b;
    static const int SnacBuddyUserOffline = 0x0c;
    static const int TlvUserClass = 0x0001;
    static const int TlvOnlineSince = 0x0003;
    static const int TlvOnlineStatus = 0x0006;
    static const int TlvUserIp = 0x000a;
    static const int TlvMaxContacts = 0x0001;
    static const int TlvMaxWatchers = 0x0002;
    static const int TlvMaxOnlineNotifications = 0x0003;
    static const int TlvAvatar = 0x001d;

signals:
    void ready();

private:
    bool processUserOnline(const QByteArray & data);
    void parseBuddyTlvs(const TlvList& list, const ICQContactPtr& contact);
    void parseAvatarTlv(const Tlv& avatarTlv, const ICQContactPtr& contact);

    bool m_ready;
};

#endif // BUDDYSNACHANDLER_H
