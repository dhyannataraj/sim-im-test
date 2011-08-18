#ifndef IUSERWND_H
#define IUSERWND_H

#include <QWidget>
#include "messaging/message.h"
#include "simgui/messageeditor.h"
#include "core_api.h"

class CORE_EXPORT IUserWnd : public QWidget
{
    Q_OBJECT
public:
    virtual ~IUserWnd() {}
    virtual void addMessageToView(const SIM::MessagePtr& message) = 0;
    virtual int messagesInViewArea() const = 0;

    virtual int id() const = 0;
    virtual QString getName() = 0;
    virtual void setMessageEditor(SIM::MessageEditor* editor) = 0;
    virtual QString selectedClientId() const = 0;
};

#endif // IUSERWND_H
