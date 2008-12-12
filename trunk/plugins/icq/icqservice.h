
#ifndef _ICQSERVICE_H
#define _ICQSERVICE_H

#include <list>

#include "snac.h"

class ServiceSocket;
class Tlv;
class SnacIcqService : public SnacHandler
{
public:
	SnacIcqService(ICQClient* client);
	virtual ~SnacIcqService();

	virtual bool process(unsigned short subtype, ICQBuffer* buf);
	void requestRateInfo();

	void addService(ServiceSocket* s);
	void deleteService(ServiceSocket* s);
	ServiceSocket* getService(unsigned short id);
	void clearServices();
	void setServiceSocket(Tlv *tlv_addr, Tlv *tlv_cookie, unsigned short service);
	void sendClientReady();
	void sendLogonStatus();
	void setInvisible();
	void sendStatus();
	void sendUpdate();
	void sendPluginInfoUpdate(unsigned plugin_id);
	void sendPluginStatusUpdate(unsigned plugin_id, unsigned long status);
	void sendIdleTime();
	void requestService(ServiceSocket *s);

protected:
	void fillDirectInfo(ICQBuffer &directInfo);

private:
	bool m_idleTime;
	
    std::list<ServiceSocket*> m_services;
    unsigned int m_nUpdates;
};

#endif
