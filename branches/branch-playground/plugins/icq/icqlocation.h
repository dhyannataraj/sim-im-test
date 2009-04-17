
#ifndef ICQLOCATION_H
#define ICQLOCATION_H

#include "snac.h"

class SnacIcqLocation : public QObject, public SnacHandler
{
	Q_OBJECT
public:
	SnacIcqLocation(ICQClient* client);
	virtual ~SnacIcqLocation();
	
};


#endif
