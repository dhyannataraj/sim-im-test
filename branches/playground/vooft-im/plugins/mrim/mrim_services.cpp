#include <QByteArray>
#include <QDataStream>

#include "mrim_services.h"
#include "../../src/ev_types.h"
#include "../../src/contacts.h"
#include "proto.h"

// place your code here
UpholdSvc::UpholdSvc()
{
	m_types.append(SUpholdConnect);
	m_types.append(SPingMsg);
	started = false;
	connect(&m_timer, SIGNAL(timeout()), this, SLOT(genUphold()));
}

UpholdSvc::~UpholdSvc()
{
	
}

void UpholdSvc::genUphold()
{
	SIntMsg msg;
	msg.parsed = true;
	msg.type = SUpholdConnect;
	
	QDataStream in(&(msg.data), QIODevice::WriteOnly);
	
	in << 0;
	
	emit debug("UpholdSvc::genUphold()");
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

ContactListParser::ContactListParser()
{
	m_types.append(SContacts);
}

ContactListParser::~ContactListParser()
{
	
}

void ContactListParser::parse(SIntMsg& msg)
{
	QDataStream out(msg.data);
	
	quint32 clStatus = readUL(out);
	quint32 groupNumber = readUL(out);
	
	QString groupMask = readLPS(out);
	QString contactMask = readLPS(out);
	
	switch(clStatus)
	{
		case GET_CONTACTS_ERROR: emit debug("Contact list errir: Conctact list is incorrect"); return;
		case GET_CONTACTS_INTERR: emit debug("Contact list error: Internal server error"); return;
	}
	
	for(int i=0; i<groupNumber; i++)
	{
		SIntMsg msg;
		msg.parsed = true;
		msg.type = SAddGroup;
		
		QDataStream in(&(msg.data), QIODevice::WriteOnly);
		
		quint32 flags = readUL(out);
		QString name = readLPS(out);
		
		in << name;
		
		emit parsed(msg);
	}
	
	while(!out.atEnd())
	{
		SIntMsg msg;
		msg.parsed = true;
		msg.type = SAddContact;
		
		QDataStream in(&(msg.data), QIODevice::WriteOnly);
		
		quint32 flags = readUL(out);
		quint32 group = readUL(out);
		QString id = readLPS(out);
		QString nick = readLPS(out);
		quint32 srvflags = readUL(out);
		quint32 status = readUL(out);
		
		in << "mrim";
		in << id;
		in << genStatus(status);
		
		if(srvflags&CONTACT_INTFLAG_NOT_AUTHORIZED)
			in << (quint16) SUnauth;
		else
			in << (quint16) SAuth;
			
		in << QString();
		
		emit parsed(msg);
		
	}
	
}

quint32 ContactListParser::readUL(QDataStream &in)
{
	quint32 result;
			
	in >> result;
	
	return result;
}

QString ContactListParser::readLPS(QDataStream &in)
{
	quint8 c;
	QString result;
	
	quint32 len = readUL(in);
			
	for(quint32 i=0; i<len; i++)
	{
		in >> c;
		result += c;
	}
	
	return result;
}

quint16 ContactListParser::genStatus(quint32 status)
{
	quint16 result = SUnknow;
	
	switch(status)
	{
		case STATUS_OFFLINE: result = SOffline; break;
		case STATUS_ONLINE: result = SOnline; break;
		case STATUS_AWAY: result = SAway; break;
		case STATUS_FLAG_INVISIBLE: result = SInvisible; break;
	}
	
	return result;
}

/*QList<SContact> ContactListParser::readContacts(QString mask, QDataStream& out)
{
	QList<SContact> list;
	while(true)
	{
		QString name,
		foreach(QChar c, mask)
		{
			if(c=='s')
		}
	}
}*/
