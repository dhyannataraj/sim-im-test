/***************************************************************************
                          cfg.h  -  description
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

#ifndef _CFG_H
#define _CFG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
#if HAVE_INTTYPES_H
#include <inttypes.h>
#else
#if HAVE_STDINT_H
#include <stdint.h>
#endif
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4530)
#endif
#include <winsock.h>
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif

#include <iostream>
#include <list>
#include <string>

using namespace std;
#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4786)
#endif
#endif

class ConfigArray;

class ConfigValue
{
public:
    ConfigValue(ConfigArray *arr, const char *name);
    virtual ~ConfigValue() {}
    virtual bool save(string &s) = 0;
    virtual void load(string &s) = 0;
    virtual const ConfigValue &operator = (const ConfigValue&) = 0;
    virtual bool operator == (const ConfigValue&) const = 0;
protected:
    friend class ConfigArray;
    string m_name;
    bool isList;
private:
    ConfigValue(ConfigValue&);
};

class ConfigString : public ConfigValue, public string
{
public:
    ConfigString(ConfigArray *arr, const char *name, const char *def = NULL);
    operator const char*() { return c_str(); }
    char operator[] (int n) { return string::operator [] (n); }
    string &operator = (string s) { string::operator = (s); return *this; }
    string &operator () () { return *this; }
    const char *operator = (const char *s) { string::operator = (s ? s : ""); return s; }
    const ConfigString& operator = (const ConfigString &v) {  string::operator = (v.c_str()); return v; }
    const ConfigValue &operator = (const ConfigValue &v) { return (*this) = static_cast<const ConfigString&>(v); }
    virtual bool save(string &s);
    virtual void load(string &s);
    virtual bool operator == (const ConfigValue&) const;
    void setDefault(const char *defValue);
protected:
    string m_default;
};

class ConfigULong : public ConfigValue
{
public:
    ConfigULong(ConfigArray *arr, const char *name, unsigned long def = 0);
    unsigned long &operator()() { return m_nValue; }
    operator unsigned long&() { return m_nValue; }
    unsigned long operator = (unsigned long value) { m_nValue = value; return value; }
    const ConfigULong& operator = (const ConfigULong &value) { m_nValue = value.m_nValue; return value; }
    const ConfigValue &operator = (const ConfigValue &v) { return (*this) = static_cast<const ConfigULong&>(v); }
    virtual bool save(string &s);
    virtual void load(string &s);
    virtual bool operator == (const ConfigValue&) const;
protected:
    unsigned long m_nValue;
    unsigned long m_nDefault;
};

class ConfigUShort : public ConfigValue
{
public:
    ConfigUShort(ConfigArray *arr, const char *name, unsigned short def = 0);
    unsigned short &operator()() { return m_nValue; }
    operator unsigned short&() { return m_nValue; }
    unsigned short operator = (unsigned short value) { m_nValue = value; return value; }
    const ConfigUShort& operator = (const ConfigUShort &value) { m_nValue = value.m_nValue; return value; }
    const ConfigValue &operator = (const ConfigValue &v) { return (*this) = static_cast<const ConfigUShort&>(v); }
    virtual bool save(string &s);
    virtual void load(string &s);
    virtual bool operator == (const ConfigValue&) const;
protected:
    unsigned short m_nValue;
    unsigned short m_nDefault;
};

class ConfigShort : public ConfigValue
{
public:
    ConfigShort(ConfigArray *arr, const char *name, short def = 0);
    short &operator()() { return m_nValue; }
    operator short&() { return m_nValue; }
    short operator = (short value) { m_nValue = value; return value; }
    const ConfigShort& operator = (const ConfigShort &value) { m_nValue = value.m_nValue; return value; }
    const ConfigValue &operator = (const ConfigValue &v) { return (*this) = static_cast<const ConfigShort&>(v); }
    virtual bool save(string &s);
    virtual void load(string &s);
    virtual bool operator == (const ConfigValue&) const;
protected:
    short m_nValue;
    short m_nDefault;
};

class ConfigChar : public ConfigValue
{
public:
    ConfigChar(ConfigArray *arr, const char *name, char def = 0);
    char &operator()() { return m_nValue; }
    operator char&() { return m_nValue; }
    char operator = (char value) { m_nValue = value; return value; }
    const ConfigChar& operator = (const ConfigChar &value) { m_nValue = value.m_nValue; return value; }
    const ConfigValue &operator = (const ConfigValue &v) { return (*this) = static_cast<const ConfigChar&>(v); }
    virtual bool save(string &s);
    virtual void load(string &s);
    virtual bool operator == (const ConfigValue&) const;
protected:
    char m_nValue;
    char m_nDefault;
};

class ConfigBool : public ConfigValue
{
public:
    ConfigBool(ConfigArray *arr, const char *name, bool def = false);
    bool operator ()() { return m_bValue; }
    operator bool() { return m_bValue; }
    bool operator = (bool value) { m_bValue = value; return value; }
    const ConfigBool &operator = (const ConfigBool &value) { m_bValue = value.m_bValue; return value; }
    const ConfigValue &operator = (const ConfigValue &v) { return (*this) = static_cast<const ConfigBool&>(v); }
    virtual bool save(string &s);
    virtual void load(string &s);
    virtual bool operator == (const ConfigValue&) const;
protected:
    bool m_bValue;
    bool m_bDefault;
};

class ConfigULongs : public ConfigValue, public list<unsigned long>
{
public:
    ConfigULongs(ConfigArray *arr, const char *name);
    const ConfigULongs &operator = (const ConfigULongs &value) { list<unsigned long>::operator = (value); return value; }
    const ConfigValue &operator = (const ConfigValue &v) { return (*this) = static_cast<const ConfigULongs&>(v); }
    virtual bool save(string &s);
    virtual void load(string &s);
    virtual bool operator == (const ConfigValue&) const;
};

class ConfigPtrList : public list<ConfigArray*>
{
public:
    ConfigPtrList() {}
    virtual ~ConfigPtrList();
    virtual ConfigArray *create() = 0;
    const ConfigPtrList &operator = (const ConfigPtrList &value);
    bool operator == (const ConfigPtrList &v) const;
    bool operator != (const ConfigPtrList &v) const
        { return !operator == (v); }
};

class ConfigList : public ConfigValue, public ConfigPtrList
{
public:
    ConfigList(ConfigArray *arr, const char *name);
    virtual bool save(string&) { return false; }
    virtual void load(string&) {}
    const ConfigList &operator = (const ConfigList &value) { *((ConfigPtrList*)this) = value; return value; }
    const ConfigValue &operator = (const ConfigValue &v) { return (*this) = static_cast<const ConfigList&>(v); }
    void load(istream &s, string &nextPart);
    void save(ostream &s);
    virtual bool operator == (const ConfigValue&) const;
    bool operator == (const ConfigList &v) const
        { return ConfigPtrList::operator == (v); }
    bool operator != (const ConfigList &v) const
        { return ConfigPtrList::operator != (v); }
};

class ConfigArray
{
public:
    ConfigArray() {}
    virtual ~ConfigArray() {}
    virtual void save(ostream &s);
    virtual bool load(istream &s, string &nextPart);
    const ConfigArray &operator = (const ConfigArray &value);
    const ConfigValue *getValue(const string &name) const;
    bool operator == (const ConfigArray&) const;
protected:
    friend class ConfigValue;
    list<ConfigValue*> values;
private:
    ConfigArray(ConfigArray&);
};

#endif

