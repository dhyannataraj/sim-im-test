
#include "propertyhub.h"
#include "log.h"

namespace SIM
{

PropertyHub::PropertyHub()
{
}

PropertyHub::~PropertyHub()
{
}

void PropertyHub::setString(const QString& key, const QString& str)
{
	QVariant* variant = new QVariant(str);
	m_map[key] = variant;
}

QString PropertyHub::getString(const QString& key)
{
	mapVariant::iterator it = m_map.find(key);
	if(it == m_map.end())
	{
		log(L_DEBUG, "PropertyHub: requested empty key: %s", key.toUtf8().data());
		return QString::null;
	}
	return it.value()->toString();
}

void PropertyHub::setInt(const QString& key, const int val)
{
	QVariant* variant = new QVariant(val);
	m_map[key] = variant;
}

int PropertyHub::getInt(const QString& key)
{
	mapVariant::iterator it = m_map.find(key);
	if(it == m_map.end())
	{
		log(L_DEBUG, "PropertyHub: requested empty key: %s", key.toUtf8().data());
		return 0;
	}
	return it.value()->toInt();
}

void PropertyHub::setBool(const QString& key, const bool val)
{
	QVariant* variant = new QVariant(val);
	m_map[key] = variant;
}

bool PropertyHub::getBool(const QString& key)
{
	mapVariant::iterator it = m_map.find(key);
	if(it == m_map.end())
	{
		log(L_DEBUG, "PropertyHub: requested empty key: %s", key.toUtf8().data());
		return false;
	}
	return it.value()->toBool();
}

}
