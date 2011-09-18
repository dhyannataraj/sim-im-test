
#include "simapp.h"
#include "cfg.h"
#include "log.h"
#include "misc.h"
#include "messaging/message.h"

SimApp::SimApp(int &argc, char **argv)
      : QApplication(argc, argv)
{
    registerMetaTypes();
}
SimApp::~SimApp()
{

}

void SimApp::commitData(QSessionManager&)
{
	//SIM::save_state();
}

void SimApp::saveState(QSessionManager &sm)
{
    QApplication::saveState(sm);
}

void SimApp::registerMetaTypes()
{
    qRegisterMetaType<SIM::MessagePtr>("SIM::MessagePtr");
}

// vim: set expandtab:

