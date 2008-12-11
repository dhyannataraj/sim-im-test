
#ifndef _SNAC_H
#define _SNAC_H

class ICQClient;
class ICQBuffer;
class SnacHandler
{
public:
	SnacHandler(ICQClient* client, unsigned short snac);
	virtual ~SnacHandler();

	unsigned short getType() { return m_snac; };
	virtual bool process(unsigned short subtype, ICQBuffer* buf) = 0;

protected:
	unsigned short m_snac;
	ICQClient* m_client;

};

#endif
