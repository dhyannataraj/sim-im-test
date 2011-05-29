#ifndef CONTAINERCONTROLLER_H
#define CONTAINERCONTROLLER_H

#include <QObject>
#include "messaging/message.h"
#include "core_api.h"

class Container;
class CORE_EXPORT ContainerController : public QObject
{
    Q_OBJECT
public:
    explicit ContainerController(Container* view, int id);

    int id() const;

    void sendMessage(const SIM::MessagePtr& msg);

signals:

public slots:

private:
    Container* m_view;
    int m_id;
};

#endif // CONTAINERCONTROLLER_H
