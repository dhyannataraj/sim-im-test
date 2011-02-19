#ifndef PRIVACYSNACHANDLER_H
#define PRIVACYSNACHANDLER_H

#include "snac.h"
#include "icq_defines.h"

class ICQ_EXPORT PrivacySnacHandler : public SnacHandler
{
    Q_OBJECT
public:
    PrivacySnacHandler(ICQClient* client);

    bool process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId);

    void requestRights();

    void forceReady();
    bool isReady() const;

    static const int SnacPrivacyRightsRequest = 0x02;
    static const int SnacPrivacyRightsInfo = 0x03;

    static const int TlvMaxVisible = 0x0001;
    static const int TlvMaxInvisible = 0x0002;

signals:
    void ready();

private:
    bool processRightsInfo(const QByteArray& data);

    bool m_ready;
};

#endif // PRIVACYSNACHANDLER_H
