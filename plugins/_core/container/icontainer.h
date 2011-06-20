#ifndef ICONTAINER_H
#define ICONTAINER_H

#include <QString>

class UserWnd;

class IContainer
{
public:
    virtual QString name() = 0;
    virtual UserWnd *wnd(unsigned wndId) = 0;
    virtual int id() const = 0;

	virtual void addUserWnd(UserWnd* wnd) = 0;
    virtual void removeUserWnd(int wndId) = 0;
    virtual void raiseUserWnd(int wndId) = 0;
};

#endif // ICONTAINER_H
