/***************************************************************************
                          cfg.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#include "defs.h"
#include "cfg.h"
#include "log.h"

#include <stdio.h>

#include <set>
#include <string>

#include <qfile.h>

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4786)
#endif
#endif

using namespace std;

const char *empty_str = "";

typedef void *paramProc(void*);

void set_str(char **p, const char *str)
{
    if (*p){
        free(*p);
        *p = NULL;
    }
    if (str && *str)
        *p = strdup(str);
}

void init(void *_obj, const cfgParam *params)
{
    const cfgParam *p;
    paramProc *proc;
    char *obj = (char*)_obj;
    const cfgParam *pLast;
    for (pLast = params; pLast->type; pLast++);
    pLast--;
    for (p = pLast;; p--){
        switch (p->type){
        case PARAM_PROC:
            proc = (paramProc*)(p->offs);
            init(proc(obj), (cfgParam*)(p->defValue));
            break;
        case PARAM_OFFS:
            init(obj + p->offs, (cfgParam*)(p->defValue));
            break;
        }
        if (p == params) break;
    }

    for (p = pLast;; p--){
        switch (p->type){
        case PARAM_ULONG:
            *((unsigned long*)(obj + p->offs)) = p->defValue;
            break;
        case PARAM_USHORT:
            *((unsigned short*)(obj + p->offs)) = p->defValue;
            break;
        case PARAM_SHORT:
            *((short*)(obj + p->offs)) = p->defValue;
            break;
        case PARAM_STRING:
            if (p->defValue){
                string *s = (string*)(obj + p->offs);
                *s = (const char*)(p->defValue);
            }
            break;
        case PARAM_CHARS:
            *((char**)(obj + p->offs)) = NULL;
            set_str((char**)(obj + p->offs), (const char*)(p->defValue));
            break;
        case PARAM_LIST:
            *((unsigned long**)(obj + p->offs)) = NULL;
            break;
        case PARAM_BOOL:
            *((bool*)(obj + p->offs)) = (bool)p->defValue;
            break;
        case PARAM_I18N:
            *((char**)(obj + p->offs)) = NULL;
            if (p->defValue)
                set_str((char**)(obj + p->offs), i18n((const char*)(p->defValue)).local8Bit());
            break;
        case PARAM_CHAR:
            obj[p->offs] = p->defValue;
            break;
        }
        if (p == params) break;
    }
}

void free(void *_obj, const cfgParam *params)
{
    const cfgParam *p;
    unsigned long **pp;
    paramProc *proc;
    char *obj = (char*)_obj;
    for (p = params; p->type; p++){
        switch (p->type){
        case PARAM_PROC:
            proc = (paramProc*)(p->offs);
            free(proc(obj), (cfgParam*)(p->defValue));
            break;
        case PARAM_OFFS:
            free(obj + p->offs, (cfgParam*)(p->defValue));
            break;
        case PARAM_CHARS:
        case PARAM_I18N:
            set_str((char**)(obj + p->offs), NULL);
            break;
        case PARAM_LIST:
            pp = (unsigned long**)(obj + p->offs);
            if (*pp){
                free(*pp);
                *pp = NULL;
            }
            break;
        }
    }
}


static char toHex(char c)
{
    c &= 0x0F;
    if (c < 10) return c + '0';
    return c - 10 + 'a';
}

struct ltstr
{
    bool operator()(const char *s1, const char *s2) const
    {
        return strcmp(s1, s2) < 0;
    }
};

typedef set<const char*, ltstr>	DICT;

void writeStr(QFile &f, const char *str)
{
    f.writeBlock(str, strlen(str));
}

void save(void *_obj, const cfgParam *params, QFile &out, DICT &dict)
{
    char *obj = (char*)(_obj);
    char **p_str;
    const cfgParam *p;
    unsigned long *pp;
    paramProc *proc;
    for (p = params; p->type; p++){
        switch (p->type){
        case PARAM_PROC:
            proc = (paramProc*)(p->offs);
            save(proc(obj), (cfgParam*)(p->defValue), out, dict);
            break;
        case PARAM_OFFS:
            save(obj + p->offs, (cfgParam*)(p->defValue), out, dict);
            break;
        default:
            if (dict.find(p->name) != dict.end()) break;
            dict.insert(p->name);

            bool writeEmpty = false;
            string value;
            string *s;
            string v;
            list<unsigned long> *l;
            list<unsigned long>::iterator it;

            switch (p->type){
            case PARAM_ULONG:
                if (*((unsigned long*)(obj + p->offs)) != p->defValue){
                    char buf[32];
                    snprintf(buf, sizeof(buf), "%lu", *((unsigned long*)(obj + p->offs)));
                    value = buf;
                }
                break;
            case PARAM_USHORT:
                if (*((unsigned short*)(obj + p->offs)) != (unsigned short)(p->defValue)){
                    char buf[32];
                    snprintf(buf, sizeof(buf), "%u", *((unsigned short*)(obj + p->offs)));
                    value = buf;
                }
                break;
            case PARAM_SHORT:
                if (*((short*)(obj + p->offs)) != (short)(p->defValue)){
                    char buf[32];
                    snprintf(buf, sizeof(buf), "%i", *((short*)(obj + p->offs)));
                    value = buf;
                }
                break;
            case PARAM_STRING:
                s = (string*)(obj + p->offs);
                v = "";
                if (p->defValue)
                    v = (const char*)(p->defValue);
                if (strcmp(s->c_str(), v.c_str())){
                    value = *s;
                    writeEmpty = true;
                }
                break;
            case PARAM_CHARS:
                p_str = (char**)(obj + p->offs);
                if (*p_str){
                    if ((p->defValue == 0) ||
                            strcmp(*p_str, (const char*)(p->defValue)))
                        value = *p_str;
                }else{
                    if (p->defValue){
                        value = "";
                        writeEmpty = true;
                    }
                }
                break;
            case PARAM_BOOL:
                if (*((bool*)(obj + p->offs)) != (bool)(p->defValue))
                    value = *((bool*)(obj + p->offs)) ? "true" : "false";
                break;
            case PARAM_I18N:{
                    p_str = (char**)(obj + p->offs);
                    if (*p_str){
                        QString vStr;
                        if (p->defValue){
                            vStr = i18n((const char*)(p->defValue));
                            if (strcmp(*p_str, vStr.local8Bit()))
                                value = *p_str;
                        }else{
                            value = *p_str;
                        }
                    }else{
                        if (p->defValue){
                            value = "";
                            writeEmpty = true;
                        }
                    }
                    break;
                }
            case PARAM_ULONGS:{
                    l = (list<unsigned long>*)(obj + p->offs);
                    if (l->size()){
                        for (it = l->begin(); it != l->end(); ++it){
                            char b[15];
                            snprintf(b, sizeof(b), "%lu", *it);
                            if (value.length()) value += ",";
                            value += b;
                        }
                    }
                    break;
                }
            case PARAM_LIST:{
                    pp = *((unsigned long**)(obj + p->offs));
                    if (pp){
                        unsigned n = *(pp++);
                        for (unsigned i = 0; i < n; i++, pp++){
                            char b[15];
                            snprintf(b, sizeof(b), "%lu", *pp);
                            if (value.length()) value += ",";
                            value += b;
                        }
                    }
                    break;
                }
            case PARAM_CHAR:
                if (obj[p->offs] != (char)(p->defValue)){
                    char buf[32];
                    snprintf(buf, sizeof(buf), "%u", *((char*)(obj + p->offs)));
                    value = buf;
                }
                break;
            default:
                l = (list<unsigned long>*)(obj + p->offs);
                for (it = l->begin(); it != l->end(); ++it){
                    writeStr(out, "[");
                    writeStr(out, p->name);
                    writeStr(out, "]\n");
                    save((void*)(*it), (const cfgParam*)(p->defValue), out);
                }
                if (l->size())
                    writeStr(out, "[]\n");
            }
            if ((value.size() == 0) && !writeEmpty) continue;
            value = quoteString(value);
            writeStr(out, p->name);
            writeStr(out, "=");
            writeStr(out, value.c_str());
            writeStr(out, "\n");
        }
    }
}

void save(void *_obj, const cfgParam *params, QFile &out)
{
    DICT dict;
    save(_obj, params, out, dict);
}

string quoteString(const string &value)
{
    string quoted;
    for (unsigned i = 0; i < value.size(); i++){
        switch (value[i]){
        case '\\':
            quoted += "\\\\";
            break;
        case '\n':
            quoted += "\\n";
            break;
        default:
            if ((unsigned char)value[i] >= ' '){
                quoted += value[i];
            }else if (value[i]){
                quoted += "\\x";
                quoted += toHex(value[i] >> 4);
                quoted += toHex(value[i]);
            }
        }
    }
    return quoted;
}

typedef void *createObj();

static char fromHex(char c)
{
    if ((c >= '0') && (c <= '9')) return c - '0';
    if ((c >= 'A') && (c <= 'F')) return c + 10 - 'A';
    if ((c >= 'a') && (c <= 'f')) return c + 10 - 'a';
    return 0;
}

bool loadParam(void *_obj, const cfgParam *params, const char *name, const char *value, QFile &sin, string &nextPart)
{
    char *obj = (char*)_obj;
    const cfgParam *p;
    paramProc *proc;
    for (p = params; p->type; p++){
        switch (p->type){
        case PARAM_PROC:
            proc = (paramProc*)(p->offs);
            if (loadParam(proc(obj), (cfgParam*)(p->defValue), name, value, sin, nextPart))
                return true;
            break;
        case PARAM_OFFS:
            if (loadParam(obj + p->offs, (cfgParam*)(p->defValue), name, value, sin, nextPart))
                return true;
            break;
        default:
            if (!strcmp(name, p->name)){
                if (value){
                    const char *pp;
                    string *s;
                    list<unsigned long> *l;
                    switch (p->type){
                    case PARAM_ULONG:
                        *((unsigned long*)(obj + p->offs)) = strtoul(value, NULL, 10);
                        return true;
                    case PARAM_USHORT:
                        *((unsigned short*)(obj + p->offs)) = atol(value);
                        return true;
                    case PARAM_SHORT:
                        *((short*)(obj + p->offs)) = atoi(value);
                        return true;
                    case PARAM_CHARS:
                    case PARAM_I18N:
                        set_str((char**)(obj + p->offs), value);
                        return true;
                    case PARAM_STRING:
                        s = (string*)(obj + p->offs);
                        *s = value;
                        return true;
                    case PARAM_BOOL:
                        *((bool*)(obj + p->offs)) =
                            (strcasecmp(value, "false") != 0) &&
                            (strcmp(value, "0") != 0) &&
                            (*value != 0);
                        return true;
                    case PARAM_ULONGS:
                        l = (list<unsigned long>*)(obj + p->offs);
                        l->clear();
                        for (pp = value; *pp; ){
                            l->push_back(atol(pp));
                            pp = strchr(pp, ',');
                            if (pp == NULL) break;
                            pp++;
                        }
                        return true;
                    case PARAM_LIST:{
                            unsigned long **pl = (unsigned long**)(obj + p->offs);
                            if (*pl){
                                free(*pl);
                                *pl = NULL;
                            }
                            list<unsigned long> l;
                            for (pp = value; *pp; ){
                                l.push_back(atol(pp));
                                pp = strchr(pp, ',');
                                if (pp == NULL) break;
                                pp++;
                            }
                            if (l.size()){
                                *pl = (unsigned long*)malloc((l.size() + 1) * sizeof(unsigned long));
                                unsigned long *p = *pl;
                                *(p++) = l.size();
                                for (list<unsigned long>::iterator it = l.begin(); it != l.end(); ++it){
                                    *(p++) = *it;
                                }
                            }
                            return true;
                        }
                    case PARAM_CHAR:
                        obj[p->offs] = atoi(value);
                        return true;
                    }
                }else{
                    switch (p->type){
                    case PARAM_ULONG:
                    case PARAM_USHORT:
                    case PARAM_SHORT:
                    case PARAM_STRING:
                    case PARAM_BOOL:
                    case PARAM_ULONGS:
                    case PARAM_CHARS:
                    case PARAM_CHAR:
                        break;
                    default:
                        list<unsigned long> *l;
                        void *o = ((createObj*)(p->type))();
                        if (!load(o, (const cfgParam*)(p->defValue), sin, nextPart))
                            return false;
                        l = (list<unsigned long>*)(obj + p->offs);
                        l->push_back((unsigned long)o);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

unsigned long line_start = 0;

bool getLine(QFile &f, string &s)
{
    s = "";
    char b[0x4000];
    line_start = f.at();
    long res = f.readLine(b, sizeof(b));
    if (res == -1) return false;
    b[res] = 0;
    for (res--; res >= 0; res--){
        if ((b[res] != '\r') && (b[res] != '\n')) break;
        b[res] = 0;
    }
    s = b;
    return true;
}

bool load(void *_obj, const cfgParam *params, QFile &sin, string &nextPart)
{
    nextPart = "";
    for (;;){
        string line;
        if (!getLine(sin, line))
            return true;
        if (line[0] == '['){
            nextPart = line;
            for (;;){
                string name = line.c_str() + 1;
                char *p = strchr((char*)(name.c_str()), ']');
                if (p == NULL){
                    log(L_WARN, "Bad string in config %s", line.c_str());
                    nextPart = "";
                    return false;
                }
                *p = 0;
                if (*name.c_str() == 0){
                    nextPart = "";
                    return true;
                }
                if (!loadParam(_obj, params, name.c_str(), NULL, sin, nextPart))
                    return true;
                if (*nextPart.c_str() == 0) break;
                line = nextPart;
            }
            continue;
        }

        if (line.size() == 0) continue;
        char *p = (char*)strchr(line.c_str(), '=');
        if (p == NULL){
            log(L_WARN, "Bad string in config %s", line.c_str());
            continue;
        }
        *p = 0;
        p++;
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
        if (!loadParam(_obj, params, line.c_str(), unquoted.c_str(), sin, nextPart))
            log(L_WARN, "Bad key in config %s", line.c_str());
    }
    return true;
}



