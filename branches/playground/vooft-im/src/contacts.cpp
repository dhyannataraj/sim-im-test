#include <QDataStream>
#include <QImage>
#include <QList>
#include <QString>
#include <QTextStream>

#include "contacts.h"

// place your code here
SContact::SContact(QString proto, QString id, quint16 status, QImage *img)
{
	m_proto = proto;
	m_name = id;
	m_showName = id;
	m_status = status;
	if(!img)
		m_img = new QImage();
	connect(this, SIGNAL(statusChanged(quint16)), this, SLOT(contactChanged()));
	connect(this, SIGNAL(imgChanged(const QImage&)), this, SLOT(contactChanged()));
	connect(this, SIGNAL(extraPropsChanged(QMap<QString, QString>)), this, SLOT(contactChanged()));
}

SContact* SContact::genContact(QByteArray &block)
{
	QString proto, id;
	quint16 status;
	
	QDataStream out(block);
	
	proto = readStr(out);
	id = readStr(out);
	
	out >> status;
	
	SContact *result = new SContact(proto, id, status);
	
	return result;
}

QString SContact::readStr(QDataStream& out)
{
	char c;
	QString str; 
	
	out.readRawData(&c, 1);
	
	while(c)
	{
		str += c;
		out.readRawData(&c, 1);
	}
	
	return str;
}

void SContact::contactChanged()
{
	emit changed(this);
	emit changed(m_proto, m_name);
}

SContact::~SContact()
{
	if(m_img)
		delete m_img;
}

void SContact::setImage(QImage *img)
{
	if(!img)
		return;
	if(m_img)
		delete m_img;
	m_img = new QImage(*img);
	
	emit imgChanged(*m_img);
}

void SContact::addExtraProps(QMap<QString, QString> map)
{
	QList<QString> keys = map.uniqueKeys();
	foreach(QString key, keys)
		m_extras.insert(key, map[key]);
	emit extraPropsChanged(m_extras);
}

void SContact::clearExtraProps()
{
	m_extras.clear();
	emit extraPropsChanged(m_extras);
}

void SContact::setExtraProps(QMap<QString, QString> map)
{
	m_extras.clear();
	addExtraProps(map);
}

void SContact::setStatus(quint16 status)
{
	m_status = status;
	emit statusChanged(status);
}

void SContact::clearImg()
{
	if(m_img)
		delete m_img;
	m_img = new QImage();
	emit imgChanged(*m_img);
}

SGroup::SGroup(QString name)
{
	m_name = name;
	connect(this, SIGNAL(contactAdded(SContact*)), this, SIGNAL(groupStaffChanged()));
}

SGroup::~SGroup()
{
	while(contacts.size())
		delete contacts.at(0);
}

SContact* SGroup::getContact(QString proto, QString id)
{
	SContact* result = 0;
	foreach(SContact *contact, contacts)
	{
		if(contact->getProto()==proto && contact->getName()==id)
		{
			result = contact;
			break;
		}
	}
	return result;
}

void SGroup::pushContact(SContact *cont)
{
	contacts.append(cont);
	connect(cont, SIGNAL(changed(QString, QString)), this, SLOT(contactChanged(QString, QString)));
	emit contactAdded(cont);
}

void SGroup::deleteContact(QString proto, QString id)
{
	int count = contacts.size();
	for(int i=0; i<count; i++)
		if(contacts.at(i)->getProto()==proto && contacts.at(i)->getName()==id)
			contacts.removeAt(i);
}

void SGroup::contactChanged(SContact* contact)
{
	emit contactChanged(contacts.lastIndexOf(contact));
	emit groupStaffChanged(m_name);
}

void SGroup::contactChanged(QString proto, QString id)
{
	SContact *cont = getContact(proto, id);
	if(!cont)
		return;
	emit contactChanged(contacts.lastIndexOf(cont));
	emit groupStaffChanged(m_name);
}

void SGroup::changeName(QString name)
{
	QString oldname = m_name;
	m_name = name;
	emit nameChanged(oldname, name);
}

	
/*signals:
	void nameChanged(QString);
	void contactAdded(SContact*);*/
