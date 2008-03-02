#ifndef __MRIM_SERVICES_H__
#define __MRIM_SERVICES_H__

#include <QTimer>
#include <QMap>

#include "../../src/msg_base.h"
#include "../../src/service_base.h"

// place your code here
class UpholdSvc: public SService
{
	Q_OBJECT
	
	bool started;
	
	QTimer m_timer;
	quint32 readUL(QByteArray&);
	
public:
	UpholdSvc();
	~UpholdSvc();
//	void doIntMsg(SIntMsg&);
	bool parse(SIntMsg&);
	
public slots:
	//void parse(SIntMsg &msg) { doIntMsg(msg); }
	void genUphold();
};

class ContactListParser: public SService
{
	Q_OBJECT
	
	QMap<quint32, QString> groups;
	
	quint32 readUL(QDataStream&);
	QString readLPS(QDataStream&);
	quint16 genStatus(quint32);
	QString convert(QString);
	bool putStr(QDataStream&, QString);
	QString getGroupName(quint32);

public:
	ContactListParser();
	~ContactListParser();
	bool parse(SIntMsg&);
	
public slots:
//	void parse(SIntMsg&);
//	void doIntMsg(SIntMsg&) {}
};

#endif // __MRIM_SERVICES_H__
