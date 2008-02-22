#include <QDir>
#include <QApplication>
#include <QByteArray>
#include <QPluginLoader>
#include <QString>
#include <QWidget>

#include "sim.h"
#include "gui_client.h"
//
sim_im::sim_im() 
{
	gui = new SUiClient();
	connect(gui, SIGNAL(debug(QString)), this, SLOT(debug_log(QString)));
//	gui->Init();
	//gui->showAllWindows();
}

sim_im::~sim_im()
{
	if(gui)
		delete gui;
}

void sim_im::RegisterProtocol(SProtocol *proto)
{
	debug_log("Loaded: " + proto->protoName());
	protocols.append(proto);
	connect(gui, SIGNAL(createWidget(quint16, QWidget *, QString)), proto, SLOT(fillUi(quint16, QWidget*)));
	proto->RegisterClient(gui);
	gui->createMsgWindow("me", proto->protoName());
}

int sim_im::loadPlugins()
{
	int count = 0;
	QString str;
	
	QDir pluginDir(QApplication::applicationDirPath());
	

	pluginDir.cd("plugins");
	debug_log(QApplication::applicationDirPath());
	debug_log(pluginDir.path());

	
	foreach(QString fileName, pluginDir.entryList(QDir::Files))
	{
		QPluginLoader loader(pluginDir.absoluteFilePath(fileName));
		
		debug_log(pluginDir.absoluteFilePath(fileName));
		
		if (SProtocol *proto = qobject_cast<SProtocol *>(loader.instance()))
		{
			RegisterProtocol(proto);
		}
	}
	
//	if(protocols.size())
//		protocols.at(0)->Login("vooft@bk.ru", "asarhaddon");
	
	return count;
}
