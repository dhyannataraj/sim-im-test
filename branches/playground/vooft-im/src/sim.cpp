#include <QDir>
#include <QApplication>
#include <QByteArray>
#include <QPluginLoader>
#include <QString>

#include "sim.h"
//
sim_im::sim_im(  ) 
{
	// TODO
}

void sim_im::RegisterProtocol(SProtocol *proto)
{
	debug_log("Loaded: " + proto->protoName());
	protocols.append(proto);
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
		
		if (SProtocol *proto = qobject_cast<SProtocol *>(loader.instance()))
		{
			RegisterProtocol(proto);
		}
	}
	
	if(protocols.size())
		protocols.at(0)->Login("vooft@bk.ru", "asarhaddon");
	
	return count;
}
