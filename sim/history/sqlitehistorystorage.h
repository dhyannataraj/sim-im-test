#ifndef SQLITEHISTORYSTORAGE_H
#define SQLITEHISTORYSTORAGE_H

#include "historystorage.h"

namespace SIM {

class SQLiteHistoryStorage : public QObject, public SIM::HistoryStorage
{
    Q_OBJECT
public:
    explicit SQLiteHistoryStorage(QObject *parent = 0);
    virtual ~SQLiteHistoryStorage();

    virtual void addMessage(const MessagePtr& message);
    virtual QList<MessagePtr> getMessages(const QDateTime& start, const QDateTime& end);

signals:

public slots:
    void profileChanged(const QString& newProfile);
    void eventShutdown();

};

} // namespace SIM

#endif // SQLITEHISTORYSTORAGE_H
