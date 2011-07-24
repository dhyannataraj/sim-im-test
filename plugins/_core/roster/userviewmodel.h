#ifndef USERVIEWMODEL_H
#define USERVIEWMODEL_H

#include <QAbstractItemModel>

#include "contacts/contactlist.h"
#include "core_api.h"

class CORE_EXPORT UserViewModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit UserViewModel(SIM::ContactList* contactList, QObject *parent = 0);
    virtual ~UserViewModel();

    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& index) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    static const int UserViewModel::OnlineRow = 0;
    static const int UserViewModel::OfflineRow = 1;
    
    enum ItemRole
    {
        ItemType = Qt::UserRole + 1,
        ContactId,
        ContactName,
        CurrentStatusIcon
    };

    enum ItemType
    {
        itContact,
        itGroup,
        itStatusGroup
    };

    void setShowOffline(bool s);
    bool isShowOffline() const;

signals:

public slots:
    void contactListUpdated();
    void invalidateCache();
    void contactStatusChanged(int contactId);

private:
    QVariant contactData(const QModelIndex& index, int role = Qt::DisplayRole) const;
    void fillCaches() const;
    int positionIn(const QList<int> list, int value);

    SIM::ContactList* m_contactList;
    QModelIndex m_onlineItemsParent;
    QModelIndex m_offlineItemsParent;

    static const int IdOnlineParent = 1000000;
    static const int IdOnlineBase = 1000001;
    static const int IdOfflineParent = 2000000;
    static const int IdOfflineBase = 2000001;

    mutable bool m_contactsCacheValid;
    mutable QList<int> m_onlineContacts;
    mutable QList<int> m_offlineContacts;
    bool m_showOffline;

};

#endif // USERVIEWMODEL_H
