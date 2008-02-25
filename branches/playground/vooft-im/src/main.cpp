#include <QApplication>

#include "sim.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	
	sim_im sim;
//	sim.show();
	sim.loadPlugins();
	QStringList protos = sim.getProtocols();
	foreach(QString proto, protos)
	{
		if(proto=="mrim")
			sim.login(proto, "sim-im-test@inbox.ru", "sim-tesT");
	}
	
	return app.exec();
	
}
