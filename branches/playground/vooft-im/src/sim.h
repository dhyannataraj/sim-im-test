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
#include "gui_client.h"

class sim_im: public QObject
{
	Q_OBJECT
	
	QList<SProtocol *> protocols;
	SUiClient *gui;
public:
	sim_im();
	~sim_im();
	void RegisterProtocol(SProtocol *);
	int loadPlugins();
	QStringList getProtocols();

public slots:
	void debug_log(QString msg)
	{
	#ifdef USE_DEBUG
		msg = '[' + QTime::currentTime().toString("hh:mm:ss") + "] " + msg;
		qDebug() << msg;
	#endif
	}
	
};

#endif
