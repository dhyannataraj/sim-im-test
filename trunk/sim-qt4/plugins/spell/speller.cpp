/***************************************************************************
                          speller.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "speller.h"

#include <qfile.h>
#include <qstringlist.h>

#ifdef WIN32

SpellerBase::SpellerBase(const char *path)
{
    init();
    QString p = QFile::decodeName(path);
    int n = p.findRev("\\");
    if (n >= 0)
        p = p.left(n);
    p += "\\aspell-15.dll";
    hLib = LoadLibraryA(p.local8Bit());
    if (hLib){
        (DWORD&)_new_aspell_config = (DWORD)GetProcAddress(hLib, "new_aspell_config");
        (DWORD&)_delete_aspell_config = (DWORD)GetProcAddress(hLib, "new_aspell_config");
        (DWORD&)_get_aspell_dict_info_list = (DWORD)GetProcAddress(hLib, "get_aspell_dict_info_list");
        (DWORD&)_aspell_dict_info_list_elements = (DWORD)GetProcAddress(hLib, "aspell_dict_info_list_elements");
        (DWORD&)_delete_aspell_dict_info_enumeration = (DWORD)GetProcAddress(hLib, "delete_aspell_dict_info_enumeration");
        (DWORD&)_aspell_dict_info_enumeration_next = (DWORD)GetProcAddress(hLib, "aspell_dict_info_enumeration_next");
        (DWORD&)_aspell_config_replace = (DWORD)GetProcAddress(hLib, "aspell_config_replace");
        (DWORD&)_new_aspell_speller = (DWORD)GetProcAddress(hLib, "new_aspell_speller");
        (DWORD&)_to_aspell_speller = (DWORD)GetProcAddress(hLib, "to_aspell_speller");
        (DWORD&)_delete_aspell_speller = (DWORD)GetProcAddress(hLib, "delete_aspell_speller");
        (DWORD&)_aspell_error_message = (DWORD)GetProcAddress(hLib, "aspell_error_message");
        (DWORD&)_aspell_error = (DWORD)GetProcAddress(hLib, "aspell_error");
        (DWORD&)_delete_aspell_can_have_error = (DWORD)GetProcAddress(hLib, "delete_aspell_can_have_error");
        (DWORD&)_aspell_speller_check = (DWORD)GetProcAddress(hLib, "aspell_speller_check");
        (DWORD&)_aspell_speller_suggest = (DWORD)GetProcAddress(hLib, "aspell_speller_suggest");
        (DWORD&)_aspell_word_list_elements = (DWORD)GetProcAddress(hLib, "aspell_word_list_elements");
        (DWORD&)_aspell_string_enumeration_next = (DWORD)GetProcAddress(hLib, "aspell_string_enumeration_next");
        (DWORD&)_aspell_speller_add_to_personal = (DWORD)GetProcAddress(hLib, "aspell_speller_add_to_personal");
        if ((_new_aspell_config == NULL) ||
                (_delete_aspell_config == NULL) ||
                (_get_aspell_dict_info_list == NULL) ||
                (_aspell_dict_info_list_elements == NULL) ||
                (_delete_aspell_dict_info_enumeration == NULL) ||
                (_aspell_dict_info_enumeration_next == NULL) ||
                (_aspell_config_replace == NULL) ||
                (_new_aspell_speller == NULL) ||
                (_to_aspell_speller == NULL) ||
                (_delete_aspell_speller == NULL) ||
                (_aspell_error_message == NULL) ||
                (_aspell_error == NULL) ||
                (_delete_aspell_can_have_error == NULL) ||
                (_aspell_speller_check == NULL) ||
                (_aspell_speller_suggest == NULL) ||
                (_aspell_word_list_elements == NULL) ||
                (_aspell_string_enumeration_next == NULL) ||
                (_aspell_speller_add_to_personal == NULL)){
            FreeLibrary(hLib);
            init();
        }
    }
}

SpellerBase::~SpellerBase()
{
    if (hLib)
        FreeLibrary(hLib);
}

void SpellerBase::init()
{
    _new_aspell_config = NULL;
    _delete_aspell_config = NULL;
    _get_aspell_dict_info_list = NULL;
    _aspell_dict_info_list_elements = NULL;
    _delete_aspell_dict_info_enumeration = NULL;
    _aspell_dict_info_enumeration_next = NULL;
    _aspell_config_replace = NULL;
    _new_aspell_speller = NULL;
    _to_aspell_speller = NULL;
    _delete_aspell_speller = NULL;
    _aspell_error_message = NULL;
    _aspell_error = NULL;
    _delete_aspell_can_have_error = NULL;
    _aspell_speller_check = NULL;
    _aspell_speller_suggest = NULL;
    _aspell_word_list_elements = NULL;
    _aspell_string_enumeration_next = NULL;
    _aspell_speller_add_to_personal = NULL;
}

#define new_aspell_config()							m_base._new_aspell_config()
#define delete_aspell_config(cfg)					m_base._delete_aspell_config(cfg)
#define get_aspell_dict_info_list(cfg)				m_base._get_aspell_dict_info_list(cfg)
#define aspell_dict_info_list_elements(dlist)		m_base._aspell_dict_info_list_elements(dlist)
#define delete_aspell_dict_info_enumeration(dlist)	m_base._delete_aspell_dict_info_enumeration(dlist)
#define aspell_dict_info_enumeration_next(dlist)	m_base._aspell_dict_info_enumeration_next(dlist)
#define aspell_config_replace(c, k, v)				m_base._aspell_config_replace(c, k, v)
#define new_aspell_speller(dlist)					m_base._new_aspell_speller(dlist)
#define to_aspell_speller(dlist)					m_base._to_aspell_speller(dlist)
#define delete_aspell_speller(dlist)				m_base._delete_aspell_speller(dlist)
#define aspell_error_message(dlist)					m_base._aspell_error_message(dlist)
#define aspell_error(dlist)							m_base._aspell_error(dlist)
#define delete_aspell_can_have_error(dlist)			m_base._delete_aspell_can_have_error(dlist)
#define aspell_speller_check(c, v, s)				m_base._aspell_speller_check(c, v, s)
#define aspell_speller_suggest(c, v, s)				m_base._aspell_speller_suggest(c, v, s)
#define aspell_word_list_elements(c)				m_base._aspell_word_list_elements(c)
#define aspell_string_enumeration_next(c)			m_base._aspell_string_enumeration_next(c)
#define aspell_speller_add_to_personal(c, v, s)		m_base._aspell_speller_suggest(c, v, s)

#else

SpellerBase::SpellerBase()
{
}

SpellerBase::~SpellerBase()
{
}

#endif

SpellerConfig::SpellerConfig(SpellerBase &base)
        : m_base(base)
{
#ifdef WIN32
    cfg = NULL;
    if (m_base._new_aspell_config)
        cfg = new_aspell_config();
#else
    cfg = new_aspell_config();
#endif
}

SpellerConfig::~SpellerConfig()
{
    if (cfg)
        delete_aspell_config(cfg);
}

string SpellerConfig::getLangs()
{
    string res;
    if (cfg == NULL)
        return res;
    AspellDictInfoList *dlist = get_aspell_dict_info_list(cfg);
    AspellDictInfoEnumeration *dels = aspell_dict_info_list_elements(dlist);
    const AspellDictInfo *entry;
    while ((entry = aspell_dict_info_enumeration_next(dels)) != NULL){
        if (!res.empty())
            res += ";";
        res += entry->name;
    }
    delete_aspell_dict_info_enumeration(dels);
    return res;
}

int SpellerConfig::setKey(const char *key, const char *val)
{
    if (cfg == NULL)
        return -1;
    return aspell_config_replace(cfg, key, val);
}

Speller::Speller(SpellerConfig *cfg)
        : m_base(cfg->m_base)
{
    speller = NULL;
    if (cfg->cfg){
        AspellCanHaveError *ret = new_aspell_speller(cfg->cfg);
        if (aspell_error(ret) != 0){
            log(L_WARN, "Spell: %s", aspell_error_message(ret));
            delete_aspell_can_have_error(ret);
            return;
        }
        speller = to_aspell_speller(ret);
    }
}

Speller::~Speller()
{
    if (speller)
        delete_aspell_speller(speller);
}

int Speller::check(const char *word)
{
    if (speller == NULL)
        return -1;
    return aspell_speller_check(speller, word, strlen(word));
}

bool Speller::add(const char *word)
{
    if (speller == NULL)
        return false;
    return aspell_speller_check(speller, word, strlen(word)) != 0;
}

QStringList Speller::suggestions(const char *word)
{
    QStringList res;
    const AspellWordList *wl = aspell_speller_suggest(speller, word, -1);
    if (wl){
        AspellStringEnumeration *els = aspell_word_list_elements(wl);
        const char *word;
        while ((word = aspell_string_enumeration_next(els)) != NULL) {
            res.append(QString::fromUtf8(word));
        }
    }
    return res;
}



