#ifndef __PROTOCOL_BASE_H__
#define __PROTOCOL_BASE_H__


#include <QList>
#include <QString>
#include <QTime>
#include <QTcpSocket>
#include <QWidget>

#include "msg_base.h"
#include "service_base.h"
#include "client_base.h"

// place your code here
class SProtocol: public QTcpSocket
{
	Q_OBJECT

protected:
	QString m_ID;
	QString m_Pass;
	QList<SService *> services;
	QList<SClient *> clients;
//	QQueue<SIntMsg> msgs;
	QString m_ProtoName;
	bool m_Logined;

protected slots:
	void debug_log(QString msg) 
	{
		emit debug(protoName() + ": " + msg);
	}
	
public:
	virtual void init() = 0;
	virtual ~SProtocol() { }
	void RegisterService(SService *service)
	{ 
		services.append(service);
//		connect(this, SIGNAL(recieved(SIntMsg&)), service, SLOT(parse(SIntMsg&))); // Do not connect?
		connect(service, SIGNAL(debug(QString)), this, SLOT(debug_log(QString))); 
		connect(service, SIGNAL(toSend(SIntMsg&)), this, SLOT(toSend(SIntMsg&)));
		connect(service, SIGNAL(parsed(SIntMsg&)), this, SLOT(parsed(SIntMsg&)));
	}
	void RegisterClient(SClient *client)
	{
		clients.append(client);
		connect(client, SIGNAL(debug(QString)), this, SLOT(debug_log(QString)));
//		connect(this, SIGNAL(processed(SIntMsg&)), client, SLOT(getMsg(SIntMsg&)));
	}
	QString protoName() { return m_ProtoName; }
	bool isConnected()
	{
		return state()==QAbstractSocket::ConnectedState;
	}
	bool isLogined() { return m_Logined; }
	bool isInNetwork() { return isConnected() && isLogined(); }
	
	bool askServices(SIntMsg &msg)
	{
		bool result = false;
		foreach(SService *srvc, services)
			if(result = srvc->parse(msg))
				break;
		return result;
	}
	
	int askClients(SIntMsg &msg)
	{
		int result = 0;
		foreach(SClient *client, clients)
			if(client->process(msg))
				result++;
		return result;
	}
	
public slots:
	virtual void Login(QString, QString) = 0;
	virtual void Logout() = 0;
	virtual void listen() = 0;
	virtual void toSend(SIntMsg&) = 0;
	virtual void parsed(SIntMsg&) = 0;
	virtual void fillUi(quint16, QWidget *) = 0;
	
signals:
//	void recieved(SIntMsg&);
	void processed(SIntMsg&);
	void connected(bool);
	void disconnected(bool);
	void debug(QString);
	
};

Q_DECLARE_INTERFACE(SProtocol, "org.sim-im.SProtocol/1.0");
#endif // __PROTOCOL_BASE_H__
