#include <QByteArray>
#include <QDataStream>

#include "mrim_services.h"
#include "../../src/ev_types.h"

// place your code here
UpholdSvc::UpholdSvc()
{
	m_types.append(SUpholdConnect);
	m_types.append(SPingMsg);
	connect(&m_timer, SIGNAL(timeout()), this, SLOT(genUphold()));
}

void UpholdSvc::genUphold()
{
	SIntMsg msg;
	msg.parsed = true;
	msg.type = SUpholdConnect;
	emit toSend(msg);
}

void UpholdSvc::doIntMsg(SIntMsg& msg)
{
	quint32 period = readUL(msg.data);
	if(m_timer.timerId()==-1)
	{
		m_timer.start(period*1000);
		return;
	}
	m_timer.setInterval(period*1000);
}

quint32 UpholdSvc::readUL(QByteArray &block)
{
	QDataStream in(block);
	
	quint32 result;
	
	in >> result;
	
	return result;
}
