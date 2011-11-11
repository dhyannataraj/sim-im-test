#ifndef SERVICESNACHANDLER_H
#define SERVICESNACHANDLER_H

#include <QDateTime>

#include "snac.h"
#include "icq_defines.h"
#include "rateinfo.h"
#include "bytearrayparser.h"

class ICQClient;
class ICQ_EXPORT ServiceSnacHandler : public SnacHandler
{
    Q_OBJECT
public:
    ServiceSnacHandler(ICQClient* client);

    virtual bool process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId);

    virtual void disconnect();

    bool requestSelfInfo();
    void sendLoginStatus();
    void sendClientReady();

    RateInfoPtr rateInfo(int group) const;

    static const int SnacId = 0x01;

    static const int SnacServiceReady = 0x0002;
    static const int SnacServiceServerReady = 0x0003;
    static const int SnacServiceRequestService = 0x0004;
    static const int SnacServiceAvailable = 0x0005;
    static const int SnacServiceRateInfoRequest = 0x0006;
    static const int SnacServiceRateInfo = 0x0007;
    static const int SnacServiceRateInfoAck = 0x0008;
    static const int SnacServiceSelfInfoRequest = 0x000e;
    static const int SnacServiceSelfInfoReply = 0x000f;
    static const int SnacServiceCapabilities = 0x0017;
    static const int SnacServiceCapabilitiesAck = 0x0018;
    static const int SnacServiceSetStatus = 0x001e;
    static const int SnacServiceStatus = 0x0021;

    static const int TlvOnlineStatus = 0x0006;

    static const int TlvServiceId = 0x0d;
    static const int TlvServiceAddress = 0x05;
    static const int TlvAuthCookie = 0x06;

signals:
    void initiateLoginStep2();
    void serviceAvailable(int serviceId, const QString& address, const QByteArray& cookie);
    void statusTransitionComplete();

private:
    bool sendServices(const QByteArray& data);
    bool requestRateInfo();
    bool parseRateInfo(const QByteArray& data);
    bool handleServiceResponse(const QByteArray& data);
    bool handleSelfInfoReply(const QByteArray& data);
    RateInfoPtr readNextRateInfoClass(ByteArrayParser& parser);
    int readNextRateInfoGroup(ByteArrayParser& parser);

    bool parseServiceStatus(const QByteArray& data);

    QList<RateInfoPtr> m_rateInfoList;
    QDateTime m_loginSince;
};

#endif // SERVICESNACHANDLER_H
