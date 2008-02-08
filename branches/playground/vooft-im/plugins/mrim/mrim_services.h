#ifndef __MRIM_SERVICES_H__
#define __MRIM_SERVICES_H__

#include <QTimer>

#include "../../src/msg_base.h"
#include "../../src/service_base.h"

// place your code here
class UpholdSvc: public SService
{
	QTimer m_timer;
	quint32 readUL(QByteArray&);
public:
	UpholdSvc();
	~UpholdSvc();
	void doIntMsg(SIntMsg&);
	
public slots:
	void parse(SIntMsg) { }
	void genUphold();
	
};

#endif // __MRIM_SERVICES_H__
