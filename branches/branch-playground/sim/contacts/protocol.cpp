
#include "protocol.h"
#include "contactlistprivate.h"
#include <list>

namespace SIM
{
	Protocol::Protocol(Plugin *plugin)
	{
		m_plugin = plugin;
		ContactListPrivate *p = getContacts()->p;
		p->protocols.push_back(this);
	}

	Protocol::~Protocol()
	{
		ContactListPrivate *p = getContacts()->p;
		for (std::list<Protocol*>::iterator it = p->protocols.begin(); it != p->protocols.end(); ++it){
			if ((*it) != this)
				continue;
			p->protocols.erase(it);
			break;
		}
	}

}

// vim: set expandtab:

