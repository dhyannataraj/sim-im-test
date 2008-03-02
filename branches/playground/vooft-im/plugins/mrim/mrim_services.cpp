#include <QByteArray>
#include <QDataStream>
#include <QMap>
#include <QTextCodec>
#include <QTextStream>
#include <QString>

#include "mrim_services.h"
#include "../../src/ev_types.h"
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

	emit toSend(msg);
}

bool UpholdSvc::parse(SIntMsg& msg)
{
	if(!checkType(msg.type))
		return false;
	quint32 period = readUL(msg.data);
	emit debug("Ping period: " + QString::number(period));
	if(m_timer.timerId()==-1)
	{
		m_timer.start(period*1000);
		return true;
	}
	m_timer.setInterval(period*1000);
	
	return true;
}

quint32 UpholdSvc::readUL(QByteArray &block)
{
	QDataStream in(block);
	in.setByteOrder(QDataStream::LittleEndian);
	
	quint32 result;
	
	in >> result;
	
	return result;
}

ContactListParser::ContactListParser()
{
	m_types.append(SContacts);
//	m_types.append(SUserInfo);
}

ContactListParser::~ContactListParser()
{
	
}

bool ContactListParser::parse(SIntMsg& msg)
{
	if(!checkType(msg.type))
		return false;

// Mail.ru wants, that we make revers-engeneering. So, let it be :)
	
	QDataStream out(msg.data);
	out.setByteOrder(QDataStream::LittleEndian);
	
//	emit debug("Contact list size: " + QString::number(msg.data.size()));

/*	QByteArray userInfoMsg;
	
	QDataStream msg1(&userInfoMsg, QIODevice::ReadWrite);
	msg1.setByteOrder(QDataStream::LittleEndian);
	
	quint8 tmp;
	out >> tmp;

// These idiots puts into ONE package THREE mesages... omg...
	
	while(tmp!=0xEF) // workaround. fix it
	{
		msg1 << tmp;
		out >> tmp;
	}
	
	tmp = 0;
	
	while(tmp!=0xEF) // workaround. fix it
	{
		msg1 << tmp;
		out >> tmp;
	}
	
	msg1.device()->seek(0);
	
	for(int i=0; i<HEADER_SIZE-1; i++)
		out >> tmp;*/
	
	quint32 clStatus = readUL(out);
	quint32 groupNumber = readUL(out);
	
//	emit debug("Group number: " + QString::number(groupNumber));
	
	QString groupMask = readLPS(out);
	QString contactMask = readLPS(out);
	
	switch(clStatus)
	{
		case GET_CONTACTS_ERROR: emit debug("Contact list errir: Conctact list is incorrect"); return false;
		case GET_CONTACTS_INTERR: emit debug("Contact list error: Internal server error"); return false;
		default: emit debug("Start reading contact list...");
	}
	
//	emit debug("Masks readed. Group mask = " + groupMask + ". Contact mask = " + contactMask);
	
	for(int i=0; i<groupNumber; i++)
	{
		SIntMsg msg;
		msg.parsed = true;
		msg.type = SAddGroup;
		
		QDataStream in(&(msg.data), QIODevice::WriteOnly);
		
		quint32 flags = readUL(out);
		QString name = readLPS(out);
		
		name = convert(name);
		
		groups.insert(i, name);
		
		emit debug("Group readed: " + name + ", flags: " + QString::number(flags));
		
		in << name;
		
		emit parsed(msg);
	}
	
	while(!out.atEnd())
	{
	/*
		QString proto;
		QString id;
		quint16 status;
		quint16 Auth/Not auth;
		QString group;
	*/

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
		
		id = convert(id);
		nick = convert(nick);
		
		if(id=="")
			id = nick;
		
		putStr(in, "mrim");
		putStr(in, id);
		putStr(in, nick);
		
		in << (quint16) genStatus(status);
		
		if(srvflags&CONTACT_INTFLAG_NOT_AUTHORIZED)
			in << (quint16) SUnauth;
		else
			in << (quint16)  SAuth;
		
		//emit debug("GROUP_NUMBER = " + QString::number(group));
		
		QString groupName = getGroupName(group);
		
		putStr(in, groupName);
		
		emit parsed(msg);
	}
	
	return true;
	
}

QString ContactListParser::getGroupName(quint32 id)
{
	return groups.value(id, "NOT_IN_LIST");
}

bool ContactListParser::putStr(QDataStream &in, QString str)
{
	in << str;

	return true;
}

QString ContactListParser::convert(QString name)
{
	QTextCodec *cp1251 = QTextCodec::codecForName("CP1251");
	QTextCodec::setCodecForTr(cp1251);
	
	QByteArray block = name.toLatin1();
	
	return cp1251->toUnicode(block);
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
		default: result = SOffline;
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
