
#ifndef KDESIMAPP_H
#define KDESIMAPP_H

#include <kuniqueapplication.h>
#include <QSessionManager>

class SimApp : public KUniqueApplication
{
public:
    SimApp(int argc, char** argv);
    virtual ~SimApp();
    int newInstance();
    void commitData(QSessionManager&);
    void saveState(QSessionManager&);
protected:
    bool firstInstance;
};

#endif

// vim: set expandtab:

