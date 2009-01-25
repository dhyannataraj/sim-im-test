#ifndef SIM_XEVENT
#define SIM_XEVENT

#include <QString>

#include "propertyhub.h"

namespace SIM
{

class EXPORT XEvent : public PropertyHub
{
public:
	XEvent(unsigned long id);
	virtual ~XEvent();
	void process();
	unsigned long id();

private:
	unsigned long m_id;
};

}
#endif
