
#ifndef SIM_XEVENTHANDLER_H
#define SIM_XEVENTHANDLER_H

#include <QString>
#include <map>

namespace SIM
{
class XEvent;


class XEventHandler
{
public:
	typedef enum
	{
		fcContinue = 0, // Event didn't handled, call of the next handler in chain is allowed
		fcHandled = 1, // Event handled, call of the next handler in chain is allowed
		fcSufficient = 2, // Event handled, call of the next handler in chain is disallowed
		fcBreak = 3 // Event didn't handled, call of the next handler in chain is disallowed
	} tFlowControl;

	XEventHandler(int priority);
	virtual ~XEventHandler();
	virtual tFlowControl handle(XEvent* event);

	int priority();

private:
	int m_priority;
};

}

#endif
