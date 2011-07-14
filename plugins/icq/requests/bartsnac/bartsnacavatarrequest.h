/*
 * bartsnacavatarrequest.h
 *
 *  Created on: Jul 12, 2011
 */

#ifndef BARTSNACAVATARREQUEST_H_
#define BARTSNACAVATARREQUEST_H_

#include "../icqrequest.h"
#include "../icqrequestmanager.h"

class ICQClient;
class BartSnacAvatarRequest : public ICQRequest
{
public:
    static ICQRequestPtr create(ICQClient* client, const QString& screen, const QByteArray& avatarHash);
    virtual ~BartSnacAvatarRequest();

    virtual void perform(OscarSocket* socket);

private:
    BartSnacAvatarRequest(ICQClient* client, const QString& screen, const QByteArray& avatarHash);
    QByteArray makeRequestAvatarPacket(const QString& screen, const QByteArray& hash);

    ICQClient* m_client;
    QString m_screen;
    QByteArray m_hash;
};

#endif /* BARTSNACAVATARREQUEST_H_ */
