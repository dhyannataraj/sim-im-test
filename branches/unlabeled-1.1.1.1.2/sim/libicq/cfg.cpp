/***************************************************************************
                          cfg.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4786)
#endif
#endif

#include "cfg.h"
#include "log.h"

#include <stdio.h>

ConfigValue::ConfigValue(ConfigArray *arr, const char *name)
{
    isList = false;
    m_name = name;
    if (arr) arr->values.push_back(this);
}

ConfigString::ConfigString(ConfigArray *arr, const char *name, const char *def)
        : ConfigValue(arr, name)
{
    if (def){
        m_default = def;
        *this = def;
    }
}

void ConfigString::setDefault(const char *def)
{
    m_default = def;
    *this = def;
}

bool ConfigString::save(string &s)
{
    if (!strcmp(c_str(), m_default.c_str()))
        return false;
    s = (*this);
    return true;
}

void ConfigString::load(string &s)
{
    *this = s;
}

bool ConfigString::operator == (const ConfigValue &a) const
{
    return (string&)(*this) == (*static_cast<const ConfigString*>(&a));
}

ConfigULong::ConfigULong(ConfigArray *arr, const char *name, unsigned long def)
        : ConfigValue(arr, name), m_nValue(def), m_nDefault(def)
{
}

bool ConfigULong::save(string &s)
{
    if (m_nValue == m_nDefault) return false;
    char buf[32];
    snprintf(buf, sizeof(buf), "%lu", m_nValue);
    s = buf;
    return true;
}

void ConfigULong::load(string &s)
{
    m_nValue = strtoul(s.c_str(), NULL, 10);
}

bool ConfigULong::operator == (const ConfigValue &a) const
{
    return m_nValue == static_cast<const ConfigULong*>(&a)->m_nValue;
}

ConfigUShort::ConfigUShort(ConfigArray *arr, const char *name, unsigned short def)
        : ConfigValue(arr, name), m_nValue(def), m_nDefault(def)
{
}

bool ConfigUShort::save(string &s)
{
    if (m_nValue == m_nDefault) return false;
    char buf[32];
    snprintf(buf, sizeof(buf), "%u", m_nValue);
    s = buf;
    return true;
}

void ConfigUShort::load(string &s)
{
    m_nValue = atoi(s.c_str());
}

bool ConfigUShort::operator == (const ConfigValue &a) const
{
    return m_nValue == static_cast<const ConfigUShort*>(&a)->m_nValue;
}

ConfigShort::ConfigShort(ConfigArray *arr, const char *name, short def)
        : ConfigValue(arr, name), m_nValue(def), m_nDefault(def)
{
}

bool ConfigShort::save(string &s)
{
    if (m_nValue == m_nDefault) return false;
    char buf[32];
    snprintf(buf, sizeof(buf), "%i", m_nValue);
    s = buf;
    return true;
}

void ConfigShort::load(string &s)
{
    m_nValue = atoi(s.c_str());
}

bool ConfigShort::operator == (const ConfigValue &a) const
{
    return m_nValue == static_cast<const ConfigShort*>(&a)->m_nValue;
}

ConfigChar::ConfigChar(ConfigArray *arr, const char *name, char def)
        : ConfigValue(arr, name), m_nValue(def), m_nDefault(def)
{
}

bool ConfigChar::save(string &s)
{
    if (m_nValue == m_nDefault) return false;
    char buf[32];
    snprintf(buf, sizeof(buf), "%i", m_nValue);
    s = buf;
    return true;
}

void ConfigChar::load(string &s)
{
    m_nValue = atoi(s.c_str());
}

bool ConfigChar::operator == (const ConfigValue &a) const
{
    return m_nValue == static_cast<const ConfigChar*>(&a)->m_nValue;
}

ConfigBool::ConfigBool(ConfigArray *arr, const char *name, bool def)
        : ConfigValue(arr, name), m_bValue(def), m_bDefault(def)
{
}

bool ConfigBool::save(string &s)
{
    if (m_bValue == m_bDefault) return false;
    s = m_bValue ? "true" : "false";
    return true;
}

void ConfigBool::load(string &s)
{
    m_bValue = true;
    if (!strcmp(s.c_str(), "false") || !strcmp(s.c_str(), "0")) m_bValue = false;
}

bool ConfigBool::operator == (const ConfigValue &a) const
{
    return m_bValue == static_cast<const ConfigBool*>(&a)->m_bValue;
}

ConfigULongs::ConfigULongs(ConfigArray *arr, const char *name)
        : ConfigValue(arr, name)
{
}

bool ConfigULongs::save(string &s)
{
    if (size() == 0) return false;
    for (list<unsigned long>::iterator it = begin(); it != end(); it++){
        char b[15];
        snprintf(b, sizeof(b), "%lu", *it);
        if (s.length()) s += ",";
        s += b;
    }
    return true;
}

void ConfigULongs::load(string &s)
{
    for (const char *p = s.c_str(); *p; ){
        push_back(atol(p));
        p = strchr(p, ',');
        if (p == NULL) break;
        p++;
    }
}

bool ConfigULongs::operator == (const ConfigValue &a) const
{
    return (list<unsigned long>&)(*this) == (*static_cast<const ConfigULongs*>(&a));
}

ConfigList::ConfigList(ConfigArray *arr, const char *name)
        : ConfigValue(arr, name)
{
    isList = true;
}

ConfigPtrList::~ConfigPtrList()
{
    for (list<ConfigArray*>::iterator it = begin(); it != end(); it++){
        delete (*it);
    }
}

void ConfigList::save(std::ostream &sout)
{
    if (size() == 0) return;
    for (list<ConfigArray*>::iterator it = begin(); it != end(); it++){
        sout << "[" << m_name << "]\n";
        (*it)->save(sout);
    }
    sout << "[]\n";
}

void ConfigList::load(std::istream &sin, string &line)
{
    for (;;){
        ConfigArray *el = create();
        push_back(el);
        el->load(sin, line);
        if ((line == "[]") || (line == "")) return;
    }
}

bool ConfigList::operator == (const ConfigValue &a) const
{
    return operator == (*static_cast<const ConfigList*>(&a));
}

const ConfigPtrList &ConfigPtrList::operator = (const ConfigPtrList &value)
{
    list<ConfigArray*>::iterator it;
    for (it = begin(); it != end(); it++){
        delete *it;
    }
    clear();
    list<ConfigArray*>::const_iterator v_it;
    for (v_it = value.begin(); v_it != value.end(); v_it++){
        ConfigArray *arr = create();
        *arr = *(*v_it);
        push_back(arr);
    }
    return value;
}

bool ConfigPtrList::operator == (const ConfigPtrList &l) const
{
    if (size() != l.size()) return false;
    list<ConfigArray*>::const_iterator it = begin();
    list<ConfigArray*>::const_iterator it1 = l.begin();
    for (; it != end(); ++it, ++it1)
        if (!((*it) == (*it1))) return false;
    return true;
}

const ConfigArray& ConfigArray::operator = (const ConfigArray &value)
{
    list<ConfigValue*>::iterator it;
    for (it = values.begin(); it != values.end(); it++){
        const ConfigValue *v = value.getValue((*it)->m_name);
        if (v == NULL) continue;
        *(*it) = *v;
    }
    return value;
}

bool ConfigArray::operator == (const ConfigArray &a) const
{
    list<ConfigValue*>::const_iterator it;
    for (it = values.begin(); it != values.end(); it++){
        const ConfigValue *v = a.getValue((*it)->m_name);
        if (v == NULL) return false;
        if (!(*v == *(*it))) return false;
    }
    return true;
}

static char toHex(char c)
{
    c &= 0x0F;
    if (c < 10) return c + '0';
    return c - 10 + 'a';
}

void ConfigArray::save(std::ostream &sout)
{
    list<ConfigValue*>::iterator it;
    for (it = values.begin(); it != values.end(); it++){
        string s;
        string quoted;
        if ((*it)->isList){
            static_cast<ConfigList*>(*it)->save(sout);
            continue;
        }
        if (!(*it)->save(s)) continue;
        for (unsigned i = 0; i < s.size(); i++){
            switch (s[i]){
            case '\\':
                quoted += "\\\\";
                break;
            case '\n':
                quoted += "\\n";
                break;
            default:
                if ((unsigned char)s[i] >= ' '){
                    quoted += s[i];
                }else if (s[i]){
                    quoted += "\\x";
                    quoted += toHex(s[i] >> 4);
                    quoted += toHex(s[i]);
                }
            }
        }
        sout << (*it)->m_name << "=" << quoted << "\n";
    }
}

static char fromHex(char c)
{
    if ((c >= '0') && (c <= '9')) return c - '0';
    if ((c >= 'A') && (c <= 'F')) return c + 10 - 'A';
    if ((c >= 'a') && (c <= 'f')) return c + 10 - 'a';
    return 0;
}

bool ConfigArray::load(std::istream &sin, string &s)
{
    s.erase();
    for (;;){
        if (sin.eof() || sin.fail()){
            s = "";
            return true;
        }
        char buf[1024];
        sin.getline(buf, sizeof(buf));
        string line(buf);
        for (;line[0] == '[';){
            string name = line.c_str() + 1;
            char *p = strchr((char*)(name.c_str()), ']');
            if (p){
                *p = 0;
                list<ConfigValue*>::iterator it;
                for (it = values.begin(); it != values.end(); it++){
                    if (!(*it)->isList) continue;
                    if (strcmp(name.c_str(), (*it)->m_name.c_str())) continue;
                    ConfigList *list = static_cast<ConfigList*>(*it);
                    list->load(sin, line);
                    break;
                }
                if (it != values.end()){
                    if (line == "[]"){
                        if (sin.eof() || sin.fail()){
                            s = "";
                            return true;
                        }
                        sin.getline(buf, sizeof(buf));
                        line = buf;
                    }
                    continue;
                }
            }
            s = line;
            return true;
        }
        if (line.size() == 0) continue;
        char *p = strchr(line.c_str(), '=');
        if (p == NULL){
            log(L_WARN, "Bad string in config %s", line.c_str());
            continue;
        }
        *p = 0;
        p++;
        list<ConfigValue*>::iterator it;
        for (it = values.begin(); it != values.end(); it++){
            if (strcmp(line.c_str(), (*it)->m_name.c_str())) continue;
            string unquoted;
            for (; *p; p++){
                if (*p != '\\'){
                    unquoted += *p;
                    continue;
                }
                p++;
                if (*p == 0) break;
                switch (*p){
                case '\\':
                    unquoted += '\\';
                    break;
                case 'n':
                    unquoted += '\n';
                    break;
                case 't':
                    unquoted += '\t';
                    break;
                case 'x':
                    if (p[1] && p[2]){
                        char c = 0;
                        c = (fromHex(p[1]) << 4) + fromHex(p[2]);
                        unquoted += c;
                        p += 2;
                    }
                    break;
                default:
                    p--;
                }
            }
            (*it)->load(unquoted);
            break;
        }
        if (it == values.end())
            log(L_WARN, "Bad key %s in config", line.c_str());
    }
    return true;
}

const ConfigValue *ConfigArray::getValue(const string &name) const
{
    for (list<ConfigValue*>::const_iterator it = values.begin(); it != values.end(); it++){
        if ((*it)->m_name == name) return (*it);
    }
    return NULL;
}
