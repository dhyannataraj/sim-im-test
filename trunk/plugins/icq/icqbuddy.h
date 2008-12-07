
#ifndef _ICQBUDDY_H
#define _ICQBUDDY_H

#include "snac.h"

class ICQClient;
class SnacIcqBuddy : public SnacHandler
{
public:
	SnacIcqBuddy(ICQClient* client);
	virtual ~SnacIcqBuddy();

	virtual bool process(unsigned short subtype, ICQBuffer* buf);
};

#endif
