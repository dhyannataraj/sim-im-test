
#include <QStringList>
#include "propertyhub.h"
#include "log.h"
#include "profilemanager.h"


namespace SIM
{

PropertyHub::PropertyHub()
{
}

PropertyHub::PropertyHub(const QString& ns) : m_namespace(ns)
{
}

PropertyHub::~PropertyHub()
{
}

bool PropertyHub::save()
{
	//log(L_DEBUG, "PropertyHub::save()");
	QList<QString> props = this->allKeys();
	if(!ProfileManager::instance()->currentProfile())
		return false;
	ConfigPtr profile = ProfileManager::instance()->currentProfile()->config();
	if(!profile)
		return false;
	profile->beginGroup(m_namespace);
	foreach(const QString &prop, props)
	{
		//log(L_DEBUG, "Saving property: %s", prop.data());
		profile->setValue(prop, this->value(prop));
	}
	profile->endGroup();
	return true;
}

bool PropertyHub::load()
{
	//log(L_DEBUG, "PropertyHub::load()");
	ConfigPtr profile = ProfileManager::instance()->currentProfile()->config();
	if(!profile)
		return false;
	profile->beginGroup(m_namespace);
	QStringList keys = profile->allKeys();
	foreach(const QString &key, keys)
	{
		if(key == "enabled") // FIXME hack
			continue;
		//log(L_DEBUG, "Loading property: %s : %s", key.toUtf8().data(), profile->value(key).toString().toUtf8().data());
		this->setValue(key, profile->value(key));
	}
	profile->endGroup();
	return true;
}

void PropertyHub::parseSection(const QString& string)
{
    // Probably, we should remove '\r' from the string first
    QStringList lines = string.split('\n');
    for(QStringList::iterator it = lines.begin(); it != lines.end(); ++it)
    {
        QStringList line = it->split('=');
        if(line.size() != 2)
            continue;
        this->setValue(line[0].trimmed(), line[1]);
    }
}

void PropertyHub::setStringMapValue(const QString& mapname, int key, const QString& value)
{
	setValue(QString("%1/%2").arg(mapname).arg(key), value);
}

QString PropertyHub::stringMapValue(const QString& mapname, int key)
{
	return value(QString("%1/%2").arg(mapname).arg(key)).toString();
}

QList<QString> PropertyHub::allKeys()
{
	return m_data.keys();
}

void PropertyHub::setValue(const QString& key, const QVariant& value)
{
	m_data.insert(key, value);
}

QVariant PropertyHub::value(const QString& key)
{
	QVariantMap::const_iterator it = m_data.find(key);
	if(it != m_data.end())
		return it.value();
	return QVariant();
}

}

// vim: set expandtab:

