
#include <QStringList>
#include "propertyhub.h"
#include "log.h"
#include "profilemanager.h"


namespace SIM
{

PropertyHub::PropertyHub(const QString& ns) : QObject(NULL), m_namespace(ns)
{
}

PropertyHub::~PropertyHub()
{
}

bool PropertyHub::save()
{
	log(L_DEBUG, "PropertyHub::save()");
	QList<QByteArray> props = this->dynamicPropertyNames();
	Config* profile = ProfileManager::instance()->currentProfile();
	if(!profile)
		return false;
	profile->beginGroup(m_namespace);
	foreach(QByteArray prop, props)
	{
		//log(L_DEBUG, "Saving property: %s", prop.data());
		profile->setValue(prop, this->property(prop));
	}
	profile->endGroup();
	return true;
}

bool PropertyHub::load()
{
	log(L_DEBUG, "PropertyHub::load()");
	Config* profile = ProfileManager::instance()->currentProfile();
	if(!profile)
		return false;
	profile->beginGroup(m_namespace);
	QStringList keys = profile->allKeys();
	foreach(QString key, keys)
	{
		//log(L_DEBUG, "Loading property: %s : %s", key.toUtf8().data(), profile->value(key).toString().toUtf8().data());
		this->setProperty(key, profile->value(key));
	}
	profile->endGroup();
	return true;
}

}
