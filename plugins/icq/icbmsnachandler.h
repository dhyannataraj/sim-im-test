#ifndef ICBMSNACHANDLER_H
#define ICBMSNACHANDLER_H

#include "snac.h"
#include "icq_defines.h"
#include "messaging/message.h"

class ICQ_EXPORT IcbmSnacHandler : public SnacHandler
{
    Q_OBJECT
public:
    IcbmSnacHandler(ICQClient *client);
    virtual bool process(unsigned short  subtype, const QByteArray & data, int flags, unsigned int requestId);
    void requestParametersInfo();
    bool sendNewParametersInfo();
    int minMessageInterval() const;
    void forceReady();
    bool isReady() const;
    bool sendMessage(const SIM::MessagePtr & message);

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

signals:
    void ready();

private:
    bool processParametersInfo(const QByteArray & arr);
    QByteArray makeSendPlainTextPacket(const SIM::MessagePtr & message);
    QByteArray generateCookie();
    QByteArray makeMessageTlv(const SIM::MessagePtr & message);
    bool handleIncomingMessage(const QByteArray & data);
    QString parseMessageBlock(const QByteArray & block);

    int m_channel;
    int m_messageFlags;
    int m_maxSnacSize;
    int m_maxSenderWarnLevel;
    int m_maxReceiverWarnLevel;
    int m_minMessageInterval;
    quint64 m_currentCookie;
    bool m_ready;
};

#endif // ICBMSNACHANDLER_H
