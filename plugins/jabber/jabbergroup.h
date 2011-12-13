#ifndef JABBERGROUP_H
#define JABBERGROUP_H

#include "contacts/imgroup.h"

class JabberClient;
class JabberGroup : public SIM::IMGroup
{
public:
    JabberGroup(JabberClient* cl);

    virtual QString name();
    virtual SIM::Client* client();
    virtual QList<SIM::IMContactPtr> contacts();

    virtual bool serialize(QDomElement& element);
    virtual bool deserialize(QDomElement& element);
    virtual bool deserialize(const QString& data);

    virtual SIM::PropertyHubPtr saveState();
    virtual bool loadState(SIM::PropertyHubPtr state);

private:
    JabberClient* m_client;
};

#endif // JABBERGROUP_H
