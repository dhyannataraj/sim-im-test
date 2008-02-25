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
	virtual ~SProtocol() { }
	void RegisterService(SService *service)
	{ 
		services.append(service);
		connect(service, SIGNAL(debug(QString)), this, SIGNAL(debug(QString))); 
		connect(service, SIGNAL(toSend(const SIntMsg&)), this, SLOT(toSend(const SIntMsg&)));
		connect(service, SIGNAL(parsed(const SIntMsg&)), this, SLOT(parsed(const SIntMsg&)));
		connect(this, SIGNAL(recieved(SIntMsg&)), service, SLOT(parse(SIntMsg&)));
	}
	void RegisterClient(SClient *client)
	{
		clients.append(client);
		connect(client, SIGNAL(debug(QString)), this, SIGNAL(debug(QString)));
		connect(this, SIGNAL(processed(const SIntMsg&)), client, SLOT(getMsg(const SIntMsg&)));
	}
	QString protoName() { return m_ProtoName; }
	bool isConnected()
	{
		return state()==QAbstractSocket::ConnectedState;
	}
	bool isLogined() { return m_Logined; }
	bool isInNetwork() { return isConnected() && isLogined(); }
	
public slots:
	virtual void Login(QString, QString) = 0;
	virtual void Logout() = 0;
	virtual void listen() = 0;
	virtual void toSend(const SIntMsg&) = 0;
	virtual void parsed(const SIntMsg&) = 0;
	virtual void fillUi(quint16, QWidget *) = 0;
	
signals:
	void recieved(SIntMsg&);
	void processed(const SIntMsg&);
	void connected(bool);
	void disconnected(bool);
	void debug(QString);
	
};

Q_DECLARE_INTERFACE(SProtocol, "org.sim-im.SProtocol/1.0");
#endif // __PROTOCOL_BASE_H__
