
#include <map>
#include <QTextStream>
#include "contacts.h"
#include "userdata.h"

namespace SIM
{
    UserData::UserData()
    {
        m_root = PropertyHub::create("userdata");
    }

    UserData::~UserData()
    {
    }

    PropertyHubPtr UserData::getUserData(const QString& id)
    {
        if(id.isEmpty())
            return m_root;
        return m_root->propertyHub(id);
    }

    PropertyHubPtr UserData::createUserData(const QString& id)
    {
        if(id.isEmpty())
            return m_root;
        PropertyHubPtr hub = getUserData(id);
        if(hub.isNull())
        {
            hub = PropertyHub::create(id);
            m_root->addPropertyHub(hub);
        }
        return hub;
    }

    void UserData::destroyUserData(const QString& id)
    {
        m_root->deletePropertyHub(id);
    }

    PropertyHubPtr UserData::root()
    {
        return m_root;
    }

    bool UserData::serialize(QDomElement el)
    {
        QDomElement root = el.ownerDocument().createElement("userdata");
        el.appendChild(root);
        QDomElement roothub = el.ownerDocument().createElement("propertyhub");
        m_root->serialize(roothub);
        root.appendChild(roothub);
        QStringList hubNames = m_root->propertyHubNames();
        foreach(const QString& hubName, hubNames)
        {
            PropertyHubPtr hub = m_root->propertyHub(hubName);
            QDomElement hubelement = el.ownerDocument().createElement("propertyhub");
            hubelement.setAttribute("name", hub->getNamespace());
            hub->serialize(hubelement);
            root.appendChild(hubelement);
        }
        return true;
    }

    bool UserData::deserialize(QDomElement el)
    {
        QDomElement root = el.elementsByTagName("userdata").at(0).toElement();
        if(root.isNull())
            return false;
        QDomNodeList list = root.elementsByTagName("propertyhub");
        for(int i = 0; i < list.size(); i++)
        {
            QByteArray tmp_array;
            QTextStream stream(&tmp_array);
            QDomNode child = list.at(i);
            QDomElement elr = child.firstChildElement("root");
            elr.save(stream, 1);
            QString hubname = child.toElement().attribute("name");
            PropertyHubPtr hub;
            hub = createUserData(hubname);
            if(!hub->deserialize(elr))
                return false;
        }
        return true;
    }

    UserDataPtr UserData::create()
    {
        return UserDataPtr(new UserData());
    }

    QStringList UserData::userDataIds() const
    {
        return m_root->propertyHubNames();
    }

    bool UserData::loadState(PropertyHubPtr state)
    {
        if (state.isNull())
            return false;
        m_root = state;
        return true;
    }

    PropertyHubPtr UserData::saveState()
    {
        return m_root;
    }

}

// vim: set expandtab:

