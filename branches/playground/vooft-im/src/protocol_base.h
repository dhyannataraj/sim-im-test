#ifndef __PROTOCOL_BASE_H__
#define __PROTOCOL_BASE_H__

#define USE_DEBUG

#include <QtDebug>
#include <QList>
#include <QQueue>
#include <QString>
#include <QTime>
#include <QTcpSocket>

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
	QQueue<SIntMsg> msgs;
	QString m_ProtoName;
	bool m_Logined;

	virtual void Login() = 0;
	
protected slots:
	void debug_log(QString msg) 
	{
	#ifdef USE_DEBUG
		msg = '[' + QTime::currentTime().toString("hh:mm:ss") + "] " + protoName() + ": " + msg;
		qDebug() << msg;
	#endif
	}
	
public:
	virtual ~SProtocol() { }
	void RegisterService(SService *service) { services.append(service); }
	void RegisterClient(SClient *client) { clients.append(client); }
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
	
signals:
	void recieved(SIntMsg);
	void processed(SIntMsg);
	void connected(bool);
	void disconnected(bool);
	
};

Q_DECLARE_INTERFACE(SProtocol, "org.sim-im.SProtocol/1.0");
#endif // __PROTOCOL_BASE_H__
