#include <QString>

#include "contactlist.h"
#include "contacts.h"
#include "protocol_base.h"
#include "ev_types.h"

// place your code here
SContactList::SContactList()
{
	setupUi(this);
//	connect(statusBox, SIGNAL(activated(int)), this, SIGNAL(statusChanged(int)));
	m_statuses.insert(SUnknow, tr("Unknow"));
	m_statuses.insert(SOffline, tr("Offline"));
	m_statuses.insert(SInvisible, tr("Invisible"));
	m_statuses.insert(SDND, tr("Do not disturb"));
	m_statuses.insert(SOccupied, tr("Occupied"));
	m_statuses.insert(SNA, tr("Not available"));
	m_statuses.insert(SAway, tr("Away"));
	m_statuses.insert(SOnline, tr("Online"));
	m_statuses.insert(SFFC, tr("Free for chat"));
	fillStatuses();
}

SContactList::~SContactList()
{
	while(m_groups.size())
	{
		SGroup *group = m_groups.first();
		m_groups.removeFirst();
		delete group;
	}
}

void SContactList::fillStatuses()
{
	for(int i=SUnknow; i<END_OF_STATUSES; i++)
		statusBox->addItem(m_statuses[i]);
}

void SContactList::addGroup(SGroup * group)
{
	if(!group)
		return;
	m_groups.append(group);
}

void SContactList::addEmptyGroup(QString name)
{
	SGroup *group = new SGroup(name);
	addGroup(group);
}

void SContactList::removeGroup(QString name)
{
	SGroup *result = 0;
	foreach(SGroup *group, m_groups)
	{
		if(group->getName()==name)
			result = group;
	}
	if(!result)
		return;
	if(result->getCount())
		return;
	int num = m_groups.lastIndexOf(result);
	m_groups.removeAt(num);
	if(result)
		delete result;
}

void SContactList::groupChanged(QString name)
{
	
}

void SContactList::addProto(SProtocol *proto)
{
	if(proto)
		m_protocols.append(proto);
}

void SContactList::repaintContactList()
{
	contactTree->clear();
	QList<QTreeWidgetItem *> items = genItems();
	
	contactTree->addTopLevelItems(items);
}

QList<QTreeWidgetItem *> SContactList::genItems()
{
	QList<QTreeWidgetItem *> result;
	
	foreach(SGroup *group, m_groups)
	{
		const QList<SContact *> contacts = group->getContacts();
		QTreeWidgetItem *groupTree = new QTreeWidgetItem(0); 
		foreach(SContact *contact, contacts)
		{
			QTreeWidgetItem *item = new QTreeWidgetItem(groupTree, contact->getID());
			item->setText(SNameCol, contact->getName());
		}
		result.append(groupTree);
	}
	
	return result;
}
