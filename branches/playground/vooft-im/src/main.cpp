#include <QApplication>

#include "sim.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	
	sim_im sim;
//	sim.show();
	sim.loadPlugins();
	
	return app.exec();
	
}
