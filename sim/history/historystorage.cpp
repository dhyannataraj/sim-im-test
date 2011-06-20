#include "historystorage.h"
#include "sqlitehistorystorage.h"

namespace SIM
{
    HistoryStorage::~HistoryStorage()
    {
    }

    static HistoryStorage* gs_historyStorage = 0;

    void EXPORT createHistoryStorage()
    {

    }

    void EXPORT destroyHistoryStorage()
    {

    }

    EXPORT HistoryStorage* getHistoryStorage()
    {
        return gs_historyStorage;
    }

    void setHistoryStorage(HistoryStorage* storage)
    {
        gs_historyStorage = storage;
    }
}
