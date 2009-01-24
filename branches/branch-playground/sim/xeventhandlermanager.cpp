
#include <QtGlobal>
#include "xeventhandlermanager.h"
#include <climits>

namespace SIM
{

XEventHandlerManager* XEventHandlerManager::m_instance = NULL;

XEventHandlerManager::XEventHandlerManager()
{
	Q_ASSERT_X(m_instance == NULL, "XEventHandlerManager::XEventHandlerManager", "Singleton already initialized");
	m_instance = this;
}

XEventHandlerManager::~XEventHandlerManager()
{
	m_instance = NULL;
}

XEventHandlerManager* XEventHandlerManager::instance()
{
	Q_ASSERT_X(m_instance != NULL, "XEventHandlerManager::instance", "Accessing uninitialized singleton");
	return m_instance;
}

void XEventHandlerManager::handle(XEvent* event)
{
	for(listHandlers::iterator it = m_handlers.begin(); it != m_handlers.end(); ++it)
	{
		XEventHandler::tFlowControl fc = (*it)->handle(event);
		if((fc == XEventHandler::fcSufficient) || (fc == XEventHandler::fcBreak))
			break;
	}
}

void XEventHandlerManager::addHandler(XEventHandler* handler)
{
	Q_ASSERT(handler);
	for(listHandlers::iterator it = m_handlers.begin(); it != m_handlers.end(); ++it)
	{
		if(handler->priority() > (*it)->priority())
		{
			++it; // To insert AFTER
			m_handlers.insert(it, handler);
			return;
		}
	}
	// No insertion occured, just push it back
	m_handlers.push_back(handler);
}

void XEventHandlerManager::removeHandler(XEventHandler* handler)
{
	if(!handler)
		return;
	for(listHandlers::iterator it = m_handlers.begin(); it != m_handlers.end(); ++it)
	{
		if((*it) == handler)
		{
			m_handlers.erase(it);
			return;
		}
	}
}

}
