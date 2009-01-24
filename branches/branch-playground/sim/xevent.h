#ifndef SIM_XEVENT
#define SIM_XEVENT

#include <QString>

#include "propertyhub.h"

namespace SIM
{

class EXPORT XEventID
{
public:
	XEventID(unsigned long numid);
	XEventID(const QString& strid);

private:
	unsigned long m_numID;
	QString* m_stringID;

};

class EXPORT XEvent : public PropertyHub
{
public:
	XEvent(const QString& eventid);
	virtual ~XEvent();
	void process();
	const QString& id();

private:
	QString m_id;
};

}
#endif
