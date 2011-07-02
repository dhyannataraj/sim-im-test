#ifndef ICONTAINER_H
#define ICONTAINER_H

#include <QString>
#include <QSharedPointer>

class IUserWnd;
class ContainerController;
class IContainer
{
public:
    virtual ~IContainer() {}
    virtual QString name() = 0;
    //virtual IUserWnd *wnd(int wndId) = 0;
    virtual int id() const = 0;

    virtual void addUserWnd(IUserWnd* wnd) = 0;
    virtual void removeUserWnd(int wndId) = 0;
    virtual void raiseUserWnd(int wndId) = 0;

    virtual void setController(ContainerController* controller) = 0;
};

typedef QSharedPointer<IContainer> IContainerPtr;

#endif // ICONTAINER_H
