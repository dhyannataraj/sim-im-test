#ifndef ICBMSNACHANDLER_H
#define ICBMSNACHANDLER_H

#include "snac.h"
#include "icq_defines.h"
#include "messaging/message.h"
#include "tlv.h"

class ICQ_EXPORT IcbmSnacHandler : public SnacHandler
{
    Q_OBJECT
public:
    struct IcbmParameters
    {
        unsigned int messageFlags;
        int maxSnacSize;
        int maxSenderWarnLevel;
        int maxReceiverWarnLevel;
        int minMessageInterval;
    };
    IcbmSnacHandler(ICQClient *client);
    virtual bool process(unsigned short  subtype, const QByteArray & data, int flags, unsigned int requestId);
    virtual void disconnect();
    void requestParametersInfo();
    bool sendNewParametersInfo();
    int minMessageInterval() const;
    void forceReady();
    bool isReady() const;
    bool sendMessage(const SIM::MessagePtr & message);

    static const int SnacId = 0x04;
    static const int SnacIcbmSetParameters = 0x02;
    static const int SnacIcbmParametersInfoRequest = 0x04;
    static const int SnacIcbmParametersInfo = 0x05;
    static const int SnacIcbmSendMessage = 0x06;
    static const int SnacIcbmIncomingMessage = 0x07;
    static const int TlvUserClass = 0x0001;
    static const int TlvMessage = 0x0002;
    static const int TlvServerAck = 0x0003;
    static const int TlvSendOffline = 0x0006;
    static const int TlvOnlineStatus = 0x0006;
    static const int MessageBlockInfo = 0x0101;

    static const int CharsetUtf16be = 0x0002;

signals:
    void ready();

private:

    bool processParametersInfo(const QByteArray & arr);
    QByteArray generateCookie();
    bool handleIncomingMessage(const QByteArray & data);
    QString parseMessageBlock(const QByteArray & block, const QString& contactEncoding);
    bool handleIncomingTextMessage(const Tlv& messageTlv, const QByteArray& name);

    int m_channel;
    IcbmParameters m_params;

    bool m_ready;
    quint64 m_currentCookie;
};

#endif // ICBMSNACHANDLER_H
