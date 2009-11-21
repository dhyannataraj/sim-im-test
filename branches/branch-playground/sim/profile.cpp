
#include "profile.h"
#include "log.h"

namespace SIM
{
    Profile::Profile(const QString& name) : m_name(name)
    {
    }

	Profile::Profile(const ConfigPtr& conf, const QString& name) : m_name(name),
        m_config(conf)
    {
    }

	Profile::~Profile()
    {
    }

	ConfigPtr Profile::config()
    {
        return m_config;
    }

	void Profile::setConfig(const ConfigPtr& conf)
    {
        m_config = conf;
    }

    QString Profile::name()
    {
        return m_name;
    }

    QStringList Profile::enabledPlugins()
    {
        if(m_config.isNull())
            return QStringList();
        QStringList keys = m_config->allKeys();
        QStringList plugins;
        foreach(QString s, keys)
        {
            if(s.endsWith("/enabled"))
            {
                bool val = m_config->value(s).toBool();
                if(val)
                {
                    QString pluginName = s.left(s.indexOf('/'));
                    if(!pluginName.isEmpty())
                        plugins.append(pluginName);
                }
            }
        }
		return plugins;
    }

    void Profile::enablePlugin(const QString& name)
    {
        if(m_config.isNull())
            return;
        if(!enabledPlugins().contains(name))
        {
            log(L_DEBUG, "enablePlugin(%s)", qPrintable(name));
            m_config->setValue(name + "/enabled", true);
            addPlugin(name);
        }
    }

    void Profile::disablePlugin(const QString& name)
    {
        if(m_config.isNull())
            return;
        if(enabledPlugins().contains(name))
        {
            log(L_DEBUG, "disablePlugin(%s)", qPrintable(name));
            m_config->setValue(name + "/enabled", false);
            removePlugin(name);
        }
    }

    void Profile::addPlugin(const QString& name)
    {
        PluginPtr plugin = getPluginManager()->plugin(name);
        if(!plugin.isNull())
            m_plugins.append(plugin);
    }

    void Profile::removePlugin(const QString& name)
    {
        int i = 0;
        foreach(PluginPtr p, m_plugins) // FIXME
        {
            if(p->name() == name)
            {
                m_plugins.removeAt(i);
                break;
            }
            i++;
        }
    }

    void Profile::loadPlugins()
    {
        m_plugins.clear();
        QStringList plugins = enabledPlugins();
        foreach(QString name, plugins)
        {
            addPlugin(name);
        }
    }
}

// vim: set expandtab:

