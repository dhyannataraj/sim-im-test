#ifndef HISTORYSTORAGE_H
#define HISTORYSTORAGE_H

#include "misc.h"
#include "messaging/message.h"

namespace SIM
{
    class EXPORT HistoryStorage
    {
    public:
        virtual ~HistoryStorage();
        virtual void addMessage(const MessagePtr& message) = 0;
        virtual QList<MessagePtr> getMessages(const QDateTime& start, const QDateTime& end) = 0;
    };


    void EXPORT createHistoryStorage();
    void EXPORT destroyHistoryStorage();
    EXPORT HistoryStorage* getHistoryStorage();
    void setHistoryStorage(HistoryStorage* storage);

}

#endif // HISTORYSTORAGE_H
