#ifndef SIM_H
#define SIM_H

#define USE_DEBUG
//
#include <QWidget>
#include <QtDebug>
#include <QList>
#include <QStringList>
#include <QTime>

#include "protocol_base.h"

class sim_im: public QWidget
{
	QList<SProtocol *> protocols;
public:
	sim_im();
	void debug_log(QString msg)
	{
	#ifdef USE_DEBUG
		msg = '[' + QTime::currentTime().toString("hh:mm:ss") + "] " + msg;
		qDebug() << msg;
	#endif
	}
	void RegisterProtocol(SProtocol *);
	int loadPlugins();
	QStringList getProtocols();

//public slots
	
};

#endif
