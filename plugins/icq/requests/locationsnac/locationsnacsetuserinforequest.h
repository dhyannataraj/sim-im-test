/*
 * locationsnacsetuserinforequest.h
 *
 *  Created on: Jul 12, 2011
 */

#ifndef LOCATIONSNACSETUSERINFOREQUEST_H_
#define LOCATIONSNACSETUSERINFOREQUEST_H_

#include "../icqrequest.h"
#include "../../icq_defines.h"

class ICQClient;
class ICQ_EXPORT LocationSnacSetUserInfoRequest : public ICQRequest
{
public:
    static ICQRequestPtr create(ICQClient* client, const QByteArray& caps);
    virtual ~LocationSnacSetUserInfoRequest();
    virtual void perform();

private:
    LocationSnacSetUserInfoRequest(ICQClient* client, const QByteArray& caps);

    ICQClient* m_client;
    QByteArray m_caps;
};

#endif /* LOCATIONSNACSETUSERINFOREQUEST_H_ */
