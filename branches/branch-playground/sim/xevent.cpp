
#include "xevent.h"
#include "xeventhandlermanager.h"

namespace SIM
{

XEvent::XEvent(const QString& eventid) : m_id(eventid)
{
}

XEvent::~XEvent()
{
}

void XEvent::process()
{
	XEventHandlerManager::instance()->handle(this);
}

const QString& XEvent::id()
{
	return m_id;
}

}
