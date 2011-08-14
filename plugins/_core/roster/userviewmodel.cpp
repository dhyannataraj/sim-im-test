#include "userviewmodel.h"
#include "contacts/contact.h"
#include "imagestorage/imagestorage.h"
#include "log.h"
#include "events/eventhub.h"
#include "events/contactevent.h"

using SIM::log;
using SIM::L_DEBUG;

const int UserViewModel::OnlineRow = 0;
const int UserViewModel::OfflineRow = 1;


UserViewModel::UserViewModel(SIM::ContactList* contactList, QObject *parent) :
        QAbstractItemModel(parent),
        m_contactList(contactList),
        m_contactsCacheValid(false),
        m_showOffline(false)
{
    m_onlineItemsParent = createIndex(OnlineRow, 0, 0);
    m_offlineItemsParent = createIndex(OfflineRow, 0, 0);

    SIM::getEventHub()->getEvent("contact_change_status")->connectTo(this, SLOT(contactStatusChanged(int)));
}

UserViewModel::~UserViewModel()
{

}

int UserViewModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant UserViewModel::data(const QModelIndex& index, int role) const
{
    if(index.column() != 0)
        return QVariant();
    if(!index.parent().isValid())
    {
        if(role == Qt::DisplayRole)
        {
            if(index.row() == OnlineRow)
                return I18N_NOOP("Online");
            else
                return I18N_NOOP("Offline");
        }
        else if(role == ItemType)
            return itStatusGroup;
    }
    else
    {
        return contactData(index, role);
    }
    return QVariant();
}

QVariant UserViewModel::contactData(const QModelIndex& index, int role) const
{
    if(!m_contactsCacheValid)
        fillCaches();

    int row = index.row();
    QList<int>* ids = 0;
    if(index.parent().row() == OnlineRow)
        ids = &m_onlineContacts;
    else if(index.parent().row() == OfflineRow)
        ids = &m_offlineContacts;
    if(!ids)
        return QVariant();

    if((row < 0) || (row >= ids->size()))
        return QVariant();

    SIM::ContactPtr contact = m_contactList->contact(ids->at(row));
    if(!contact)
        return QVariant();

    switch(role)
    {
    case ItemType:
        return itContact;
    case ContactName:
    case Qt::DisplayRole:
        {
            return contact->name();
        }
    case ContactId:
        {
            return contact->id();
        }
    case CurrentStatusIcon:
    case Qt::DecorationRole:
        {
            if(contact->clientContactCount() == 0)
                return SIM::getImageStorage()->pixmap("nonim");
            SIM::IMContactPtr imcontact = contact->clientContact(0);
            if(!imcontact)
                return SIM::getImageStorage()->pixmap("nonim");
            return imcontact->status()->icon();
        }
    }
    return QVariant();
}

QModelIndex UserViewModel::index(int row, int column, const QModelIndex& parent) const
{
    if(column != 0)
        return QModelIndex();
    if(!parent.isValid())
    {
        if(row == OnlineRow)
            return m_onlineItemsParent;
        else if(row == OfflineRow)
            return m_offlineItemsParent;
        else return QModelIndex();
    }
    else
    {
        if(parent.parent().isValid())
            return QModelIndex();

        int parentId = -1;
        if(parent.row() == OnlineRow)
            parentId = IdOnlineParent;
        else if(parent.row() == OfflineRow)
            parentId = IdOfflineParent;
        if(parentId < 0)
            return QModelIndex();

        return createIndex(row, column, (quint32)parentId);
    }
    return QModelIndex(); //Fixme Unreachable
}

QModelIndex UserViewModel::parent(const QModelIndex& index) const
{
    if(index.internalId() == IdOnlineParent)
        return m_onlineItemsParent;
    else if(index.internalId() == IdOfflineParent)
        return m_offlineItemsParent;
    return QModelIndex();
}

int UserViewModel::rowCount(const QModelIndex& parent) const
{
    if(!parent.isValid())
    {
        if(m_showOffline)
            return 2; // Online and offline
        else
            return 1; // Only online
    }
    else
    {
        fillCaches();
        if(parent.parent().isValid())
            return 0;
        if(parent.row() == OnlineRow)
            return m_onlineContacts.size();
        else if(parent.row() == OfflineRow)
            return m_offlineContacts.size();
    }
    return 0;
}

void UserViewModel::contactListUpdated()
{
    emit beginResetModel();
    emit endResetModel();
}

void UserViewModel::invalidateCache()
{
    m_contactsCacheValid = false;
}

void UserViewModel::fillCaches() const
{
    m_onlineContacts.clear();
    m_offlineContacts.clear();
    QList<int> allContacts = m_contactList->contactIds();
    foreach(int id, allContacts)
    {
        SIM::ContactPtr contact = m_contactList->contact(id);
        if(contact->isOnline())
            m_onlineContacts.append(id);
        else
            m_offlineContacts.append(id);
    }
    m_contactsCacheValid = true;
}

void UserViewModel::contactStatusChanged(int contactId)
{
    Q_UNUSED(contactId);
    // TODO proper QModelIndex removal and addition
    beginResetModel();
    fillCaches();
    emit dataChanged(index(OnlineRow, 0), index(OfflineRow, 0));
    endResetModel();
}

int UserViewModel::positionIn(const QList<int> list, int value)
{
    int i = 0;
    for(QList<int>::const_iterator it = list.begin(); it != list.end(); ++it, i++)
    {
        if(*it == value)
        {
            return i;
        }
    }
    return -1;
}

void UserViewModel::setShowOffline(bool s)
{
    m_showOffline = s;
    reset();
}

bool UserViewModel::isShowOffline() const
{
    return m_showOffline;
}
