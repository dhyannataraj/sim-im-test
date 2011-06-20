#ifndef IMCONTACTID_H
#define IMCONTACTID_H

#include <QString>
#include "misc.h"

namespace SIM
{
    class EXPORT IMContactId
    {
    public:
        IMContactId();
        IMContactId(const QString& str, int parentContactId);
        QString toString() const;

        QString clientId() const;
        int parentContactId() const;
        QString clientSpecificId() const;
        bool isValid() const;

    private:
        bool m_valid;
        int m_parentContactId;
        QString m_clientId;
        QString m_clientSpecificId;

    };
}

#endif // IMCONTACTID_H
