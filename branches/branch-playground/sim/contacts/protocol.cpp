
#include "protocol.h"
#include "contactlistprivate.h"
#include <list>

namespace SIM
{
	Protocol::Protocol(Plugin *plugin)
	{
		m_plugin = plugin;
	}

	Protocol::~Protocol()
	{
	}

}

// vim: set expandtab:

