#ifndef __CLIENT_BASE_H__
#define __CLIENT_BASE_H__

#include <QList>
#include <QObject>

#include "msg_base.h"

class SClient: public QObject
{
	QList<quint16> m_types;
	
public:
	virtual ~SClient() { }
	QList<quint16> types() { return m_types; }
	
public slots:
	virtual void getMsg(SIntMsg) = 0;
	virtual void getMsg(STextMsg) = 0;

signals:
	void processed(bool);
};

Q_DECLARE_INTERFACE(SClient, "org.sim-im.SClient/1.0");

#endif // __CLIENT_BASE_H__
