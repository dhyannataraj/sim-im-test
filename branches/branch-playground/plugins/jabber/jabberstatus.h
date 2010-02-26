
#ifndef JABBERSTATUS_H
#define JABBERSTATUS_H

#include "contacts/imstatus.h"

class JabberStatus : public SIM::IMStatus
{
public:
    JabberStatus(const QString& id, const QString& name, const QString& defaultText, const QIcon& icon);
    virtual ~JabberStatus();

    virtual QString id() const;
    virtual QString name() const;
    virtual QString text() const;
    virtual QIcon icon() const;

    virtual QStringList substatuses();
    virtual SIM::IMStatus const* substatus(const QString& id);

private:
    QString m_id;
    QString m_name;
    QString m_text;
    QIcon m_icon;

};

typedef QSharedPointer<JabberStatus> JabberStatusPtr;

#endif

// vim: set expandtab:

