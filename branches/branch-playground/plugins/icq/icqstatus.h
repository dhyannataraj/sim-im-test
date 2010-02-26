
#ifndef ICQSTATUS_H
#define ICQSTATUS_H

#include "contacts/imstatus.h"

class ICQStatus : public SIM::IMStatus
{
public:
    ICQStatus(const QString& id, const QString& name, const QString& defaultText, const QIcon& icon);
    virtual ~ICQStatus();

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

typedef QSharedPointer<ICQStatus> ICQStatusPtr;

#endif

// vim: set expandtab:

