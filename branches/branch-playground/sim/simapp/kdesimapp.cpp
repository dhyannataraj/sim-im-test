
#include "kdesimapp.h"

SimApp::SimApp(int argc, char** argv) : KUniqueApplication()
{
    firstInstance = true;
}

SimApp::~SimApp()
{
}

int SimApp::newInstance()
{
    if (firstInstance)
	{
        firstInstance = false;
    }
	else
	{
        QWidgetList  *list = QApplication::topLevelWidgets();
        QWidgetListIt it( *list );
        QWidget *w;
        while((w = it.current()) != 0 )
		{
            ++it;
            if (w->inherits("MainWindow")){
                raiseWindow(w);
            }
        }
        delete list;
    }
    return 0;
}

void SimApp::commitData(QSessionManager&)
{
    save_state();
}

void SimApp::saveState(QSessionManager &sm)
{
    QApplication::saveState(sm);
}

void simMessageOutput( QtMsgType, const char *msg )
{
    if (logEnabled())
        log(L_DEBUG, "QT: %s", msg);
}

// vim: set expandtab:

