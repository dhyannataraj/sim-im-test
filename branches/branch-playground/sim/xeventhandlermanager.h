
#ifndef SIM_XEVENTHANDLERMANAGER_H
#define SIM_XEVENTHANDLERMANAGER_H

#include <list>

#include "xeventhandler.h"
#include "xevent.h"

namespace SIM
{

class XEventHandlerManager
{
public:
	XEventHandlerManager();
	virtual ~XEventHandlerManager();
	static XEventHandlerManager* instance();

	void handle(XEvent* event);
	void addHandler(XEventHandler* handler);
	void removeHandler(XEventHandler* handler);

	typedef std::list<XEventHandler*> listHandlers;
private:
	static XEventHandlerManager* m_instance;
	listHandlers m_handlers;
};

}

#endif
