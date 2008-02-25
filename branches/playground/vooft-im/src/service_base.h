#ifndef __SERVICE_BASE_H__
#define __SERVICE_BASE_H__

#include <QObject>

#include "msg_base.h"

class SService: public QObject
{
	Q_OBJECT

protected:
	QList<quint16> m_types;
	
public:
	virtual ~SService() { }
	//virtual bool isBin() = 0;
	QList<quint16> types() { return m_types; }
	virtual void doIntMsg(SIntMsg&) = 0;
	//virtual STextMsg doTextMsg(const SIntMsg&) = 0;
	
public slots:
	virtual void parse(SIntMsg&) = 0;
	
signals: 
	void parsed(SIntMsg&);
	void parsed(bool);
	void debug(QString);
	void toSend(const SIntMsg&);
};

Q_DECLARE_INTERFACE(SService, "org.sim-im.SService/1.0");
#endif // __SERVICE_BASE_H__
