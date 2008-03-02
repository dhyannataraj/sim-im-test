#include <QHeaderView>
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
	contactTree->header()->hide();
	m_states.insert(SUnknow, tr("Unknow"));
	m_states.insert(SOffline, tr("Offline"));
	m_states.insert(SInvisible, tr("Invisible"));
	m_states.insert(SDND, tr("Do not disturb"));
	m_states.insert(SOccupied, tr("Occupied"));
	m_states.insert(SNA, tr("Not available"));
	m_states.insert(SAway, tr("Away"));
	m_states.insert(SOnline, tr("Online"));
	m_states.insert(SFFC, tr("Free for chat"));
	fillStates();
	addEmptyGroup("NOT_IN_LIST");
	
//	connect(this, SIGNAL(groupAdded(QString)), this, SLOT(groupChanged(QString))); // FIX_LATER
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

void SContactList::fillStates()
{
	statusBox->clear();
	for(int i=SUnknow; i<END_OF_STATES; i++)
		statusBox->addItem(m_states[i]);
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
	repaintContactList();
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
			item->setText(SNameCol, contact->getShowName());
		}
		result.append(groupTree);
	}
	
	return result;
}

SGroup* SContactList::findGroup(QString name)
{
	foreach(SGroup* group, m_groups)
	{
		if(group->getName()==name)
			return group;
	}
	return 0;
}

void SContactList::addContact(SContact* contact, QString groupName)
{
	if(contact->getGroupName()=="")
		contact->setGroupName(groupName);
	else groupName = contact->getGroupName();
	
	emit debug("Contact list, add contact. Proto: " + contact->getProto() + ". Group: " + contact->getGroupName());
	
	if(!findGroup(groupName))
	{
		addEmptyGroup(groupName);
		emit groupAdded(groupName);
	}
	
	SGroup* group = findGroup(groupName);
	group->pushContact(contact);
	groupChanged(groupName);
}
