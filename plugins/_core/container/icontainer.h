#ifndef ICONTAINER_H
#define ICONTAINER_H

#include <QString>

class IUserWnd;

class IContainer
{
public:
    virtual QString name() = 0;
    virtual IUserWnd *wnd(unsigned wndId) = 0;
    virtual int id() const = 0;

    virtual void addUserWnd(IUserWnd* wnd) = 0;
    virtual void removeUserWnd(int wndId) = 0;
    virtual void raiseUserWnd(int wndId) = 0;
};

#endif // ICONTAINER_H
