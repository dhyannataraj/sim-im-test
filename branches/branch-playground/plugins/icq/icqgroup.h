#ifndef ICQGROUP_H
#define ICQGROUP_H

#include "contacts/imgroup.h"
#include "contacts/client.h"

class ICQClient;
class ICQGroup : public SIM::IMGroup
{
public:
    ICQGroup(ICQClient* cl);
    virtual SIM::Client* client();
    virtual QList<SIM::IMContactPtr> contacts();

    virtual bool serialize(QDomElement& element);
    virtual bool deserialize(QDomElement& element);
    virtual bool deserialize(const QString& data);

private:
    ICQClient* m_client;
};

#endif // ICQGROUP_H
