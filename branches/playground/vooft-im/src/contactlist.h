#ifndef __CONTACTLIST_H__
#define __CONTACTLIST_H__

#include <QList>
#include <QMap>
#include <QObject>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "contacts.h"
#include "protocol_base.h"
#include "ui_contact_list.h"

// place your code here

enum
{
	SNameCol,
	
END_OF_COLUMNS
};

class SContactList: public QWidget, public Ui::ContactList
{
	Q_OBJECT
	
	QList<SGroup *> m_groups;
	QList<SProtocol *> m_protocols;
	QMap<int, QString> m_states;
	
	QList<QTreeWidgetItem *> genItems();
	void fillStates();
	
public:
	SContactList();
	~SContactList();
	SGroup* findGroup(QString);
	
public slots:
	void addContact(SContact*, QString groupName="NOT_IN_LIST");
	void addGroup(SGroup *);
	void addEmptyGroup(QString);
	void removeGroup(QString);
	void groupChanged(QString);
	void addProto(SProtocol *);
	void repaintContactList();
	
signals:
	void changeStatus(int);
	void statusChanged(int);
	void groupAdded(QString);
	void debug(QString);
	
};

#endif // __CONTACTLIST_H__
