/*
 * bartsnachandler.h
 *
 *  Created on: Jul 3, 2011
 *      Author: todin
 */

#ifndef BARTSNACHANDLER_H_
#define BARTSNACHANDLER_H_

#include "snac.h"
#include "icq_defines.h"
#include "oscarsocket.h"
#include "requests/icqrequestmanager.h"

class ICQ_EXPORT BartSnacHandler : public SnacHandler
{
    Q_OBJECT
public:
    BartSnacHandler(ICQClient* client);
    virtual ~BartSnacHandler();

    virtual bool process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId);
    bool processService(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId);

    void requestAvatar(const QString& screen, const QByteArray& hash);

    void requestBartService();

    void bartServiceAvailable(const QString& address, const QByteArray& cookie);

    void setOscarSocket(OscarSocket* socket);
    OscarSocket* oscarSocket() const;

    ICQRequestManager* requestManager() const;

    static const int SnacId = 0x10;

    static const int SnacRequestAvatar = 0x04;
    static const int SnacResponseAvatar = 0x05;

protected slots:
    void oscarSocketPacket(int channel, const QByteArray& data);
    void socketConnected();

private:
    bool parseAvatarPacket(const QByteArray& arr);

    void sendLoginCookie(const QByteArray& cookie);
    void handleLoginAndCloseChannels(int channel, const QByteArray& data);

    bool parseServerReady(const QByteArray& data);
    void sendCapabilities();
    void requestRateInfo();
    void ackRateInfo();
    void sendReady();

    QByteArray m_loginCookie;
    OscarSocket* m_socket;
    ICQRequestManager* m_requestManager;
};

#endif /* BARTSNACHANDLER_H_ */
