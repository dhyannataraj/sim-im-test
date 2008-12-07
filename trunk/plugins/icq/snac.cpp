
#include "snac.h"

SnacHandler::SnacHandler(ICQClient* client, unsigned short snac) : m_snac(snac)
{
	m_client = client;
}

SnacHandler::~SnacHandler()
{
}
