
#include <vector>
#include <map>

#include "contactlistprivate.h"
#include "contacts.h"
#include "userdata.h"


namespace SIM
{
    ContactListPrivate::ContactListPrivate()
        :m_owner(new Contact(0))
        , m_bNoRemove(false)
		, m_notInList(new Group(0))
    {
        groups.push_back(m_notInList);
	}

    ContactListPrivate::~ContactListPrivate()
    {
        clear(true);
        delete m_owner;
    }

    void ContactListPrivate::clear(bool bClearAll)
    {
        m_bNoRemove = true;
        while (contacts.size() != 0)
            delete contacts.begin()->second;

        for (std::vector<Group*>::iterator it_g = groups.begin(); it_g != groups.end();){
            Group *group = *it_g;
            if (!bClearAll && (group->id() == 0)){
                ++it_g;
                continue;
            }
            delete group;
            it_g = groups.begin();
        }
        m_bNoRemove = false;
    }

    unsigned ContactListPrivate::registerUserData(const QString &name, const DataDef *def)
    {
        unsigned id = 0x1000;   // must be unique...
        for (std::map<unsigned long, UserDataDef>::iterator it = userDataDef.begin(); it != userDataDef.end(); ++it){
            if (id <= it->first)
                id = it->first + 1;
        }
        UserDataDef d;
        d.id = id;
        d.name = name;
        d.def = def;
        userDataDef.insert(std::pair<unsigned long, UserDataDef>(id, d));
        return id;
    }

    void ContactListPrivate::unregisterUserData(unsigned /*id*/)
    {
//        for (std::map<unsigned long, Contact*>::iterator it_c = contacts.begin(); it_c != contacts.end(); ++it_c){
//            it_c->second->userData.freeUserData(id);
//        }
//        for (std::vector<Group*>::iterator it_g = groups.begin(); it_g != groups.end(); ++it_g)
//            (*it_g)->userData.freeUserData(id);
//        userData.freeUserData(id);
//        userDataDef.erase(id);
    }

}

// vim: set expandtab:

