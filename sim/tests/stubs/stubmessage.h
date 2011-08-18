#ifndef STUBMESSAGE_H
#define STUBMESSAGE_H

#include "messaging/message.h"

namespace StubObjects
{
    class StubMessage : public SIM::Message
    {
    public:
        virtual SIM::Client* client()
        {
            return 0;
        }

        virtual QString type() const
        {
            return QString();
        }

        virtual SIM::IMContactWeakPtr targetContact() const
        {
            return SIM::IMContactWeakPtr();
        }

        virtual SIM::IMContactWeakPtr sourceContact() const
        {
            return SIM::IMContactWeakPtr();
        }

        virtual QString targetContactName() const
        {
            return QString();
        }

        virtual QString sourceContactName() const
        {
            return QString();
        }

        virtual QIcon icon()
        {
            return QIcon();
        }

        virtual QDateTime timestamp()
        {
            return QDateTime();
        }

        virtual QString toXml()
        {
            return QString::null;
        }

        virtual QString toPlainText()
        {
            return QString::null;
        }

        virtual QString originatingClientId() const
        {
            return QString::null;
        }

        virtual QStringList choices()
        {
            return QStringList();
        }
    };
}

#endif // STUBMESSAGE_H
