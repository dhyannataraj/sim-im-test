
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
	//log(L_DEBUG, "PropertyHub::save()");
	QList<QByteArray> props = this->dynamicPropertyNames();
	if(!ProfileManager::instance()->currentProfile())
		return false;
	ConfigPtr profile = ProfileManager::instance()->currentProfile()->config();
	if(!profile)
		return false;
	profile->beginGroup(m_namespace);
	foreach(const QByteArray &prop, props)
	{
		//log(L_DEBUG, "Saving property: %s", prop.data());
		profile->setValue(prop, this->property(prop));
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
		if(key == "enabled")
			continue;
		//log(L_DEBUG, "Loading property: %s : %s", key.toUtf8().data(), profile->value(key).toString().toUtf8().data());
		this->setProperty(qPrintable(key), profile->value(key));
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
        this->setProperty(qPrintable(line[0].trimmed()), line[1]);
    }
}


}
