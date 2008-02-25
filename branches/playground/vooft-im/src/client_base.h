#ifndef __CLIENT_BASE_H__
#define __CLIENT_BASE_H__

#include <QList>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "msg_base.h"

class SClient: public QObject
{
	Q_OBJECT
	
protected:
	QList<quint16> m_types;
	
public:
	virtual ~SClient() { }
	QList<quint16> types() { return m_types; }
	
public slots:
	virtual void getMsg(const SIntMsg&) = 0;
	virtual void getMsg(const STextMsg&) = 0;

signals:
	void processed(bool);
	void debug(QString);
};

Q_DECLARE_INTERFACE(SClient, "org.sim-im.SClient/1.0");

#endif // __CLIENT_BASE_H__
