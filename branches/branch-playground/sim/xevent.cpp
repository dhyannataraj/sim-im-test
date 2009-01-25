
#include "xevent.h"
#include "xeventhandlermanager.h"

namespace SIM
{

XEvent::XEvent(unsigned long id) : m_id(id)
{
}

XEvent::~XEvent()
{
}

void XEvent::process()
{
	XEventHandlerManager::instance()->handle(this);
}

unsigned long XEvent::id()
{
	return m_id;
}

}
