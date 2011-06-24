#ifndef IUSERWND_H
#define IUSERWND_H

#include <QWidget>
#include "messaging/message.h"
#include "core_api.h"

class CORE_EXPORT IUserWnd : public QWidget
{
    Q_OBJECT
public:
    virtual ~IUserWnd() {}
    virtual void addMessageToView(const SIM::MessagePtr& message) = 0;
    virtual int messagesInViewArea() const = 0;

    virtual unsigned long id() const = 0;
    virtual QString getName() = 0;
};

#endif // IUSERWND_H
