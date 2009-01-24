
#include "xeventhandler.h"
#include "xeventhandlermanager.h"

namespace SIM
{

XEventHandler::XEventHandler(int priority) : m_priority(priority)
{
	XEventHandlerManager::instance()->addHandler(this);
}

XEventHandler::~XEventHandler()
{
	XEventHandlerManager::instance()->removeHandler(this);
}

XEventHandler::tFlowControl XEventHandler::handle(XEvent* event)
{
	return fcContinue;
}

int XEventHandler::priority()
{
	return m_priority;
}

}
