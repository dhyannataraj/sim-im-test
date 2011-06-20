#include "sqlitehistorystorage.h"

namespace SIM {

SQLiteHistoryStorage::SQLiteHistoryStorage(QObject *parent) : QObject(parent)
{
}

SQLiteHistoryStorage::~SQLiteHistoryStorage()
{

}

void SQLiteHistoryStorage::addMessage(const MessagePtr& message)
{

}

QList<MessagePtr> SQLiteHistoryStorage::getMessages(const QDateTime& start, const QDateTime& end)
{
    return QList<MessagePtr>();
}

void SQLiteHistoryStorage::profileChanged(const QString& newProfile)
{
    // Save old history, if any
    // Load history
}

void SQLiteHistoryStorage::eventShutdown()
{
    // Save history
}


} // namespace SIM
