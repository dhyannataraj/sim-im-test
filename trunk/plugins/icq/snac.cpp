
#include "snac.h"

SnacHandler::SnacHandler(ICQClient* client, unsigned short snac) : m_snac(snac)
{
	m_client = client;
}

SnacHandler::~SnacHandler()
{
}

#ifndef NO_MOC_INCLUDES
#include "icqbuddy.moc"
#endif

