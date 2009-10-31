
#ifndef SIMAPP_H
#define SIMAPP_H
#include <QApplication>
#include <QSessionManager>

class SimApp : public QApplication
{
public:
    SimApp(int &argc, char **argv)
      : QApplication(argc, argv)
    {}
    ~SimApp()
    {}
protected:
    void commitData(QSessionManager&);
    void saveState(QSessionManager&);
};

#endif

// vim: set expandtab:

