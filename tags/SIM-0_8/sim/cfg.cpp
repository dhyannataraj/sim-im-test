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

#include "defs.h"
#include "cfg.h"
#include "log.h"

#include <stdio.h>

void init(void *_obj, const cfgParam *params)
{
    char *obj = (char*)_obj;
    for (;;){
        const cfgParam *p;
        for (p = params; *p->name; p++){
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
            case PARAM_BOOL:
                *((bool*)(obj + p->offs)) = (bool)p->defValue;
                break;
            case PARAM_I18N:
                if (p->defValue){
                    string *s = (string*)(obj + p->offs);
                    if (*((const char*)(p->defValue))){
                        *s = i18n((const char*)(p->defValue)).local8Bit();
                    }else{
                        *s = "";
                    }
                }
                break;
            case PARAM_CHAR:
                obj[p->offs] = p->defValue;
                break;
            }
        }
        if (p->defValue == 0) break;
        params = (const cfgParam*)(p->defValue);
        if (p->offs) obj = *((char**)(obj + p->offs));
    }
}

static char toHex(char c)
{
    c &= 0x0F;
    if (c < 10) return c + '0';
    return c - 10 + 'a';
}

void save(void *_obj, const cfgParam *params, ostream &out)
{
    const cfgParam *saveParam = params;
    char *obj = (char*)_obj;
    string *s;
    string v;
    list<unsigned long> *l;
    list<unsigned long>::iterator it;
    for (;;){
        const cfgParam *p;
        for (p = params; *p->name; p++){
            const cfgParam *pp = saveParam;
            for (;;){
                for (; *pp->name; pp++)
                    if (strcmp(pp->name, p->name) == 0) break;
                if (*pp->name) break;
                if (pp->defValue == 0) break;
                pp = (const cfgParam*)(pp->defValue);
            }
            if (pp != p) continue;
            bool writeEmpty = false;
            string value;
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
                    snprintf(buf, sizeof(buf), "%u", *((short*)(obj + p->offs)));
                    value = buf;
                }
                break;
            case PARAM_STRING:
                s = (string*)(obj + p->offs);
                v = "";
                if (p->defValue)
                    v = (const char*)(p->defValue);
                if (*s != v){
                    value = *s;
                    writeEmpty = true;
                }
                break;
            case PARAM_BOOL:
                if (*((bool*)(obj + p->offs)) != (bool)(p->defValue))
                    value = *((bool*)(obj + p->offs)) ? "true" : "false";
                break;
            case PARAM_I18N:
                s = (string*)(obj + p->offs);
                v = "";
                if (p->defValue)
                    v = i18n((const char*)(p->defValue)).local8Bit();
                if (*s != v) value = v;
                break;
            case PARAM_ULONGS:
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
                    out << "[" << p->name << "]\n";
                    save((void*)(*it), (const cfgParam*)(p->defValue), out);
                }
                if (l->size())
                    out << "[]\n";
            }
            if ((value.size() == 0) && !writeEmpty) continue;
            value = quoteString(value);
            out << p->name << "=" << value << "\n";
        }
        if (p->defValue == 0) break;
        params = (const cfgParam*)(p->defValue);
        if (p->offs) obj = *((char**)(obj + p->offs));
    }
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

bool load(void *_obj, const cfgParam *params, istream &sin, string &nextPart)
{
    nextPart = "";
    list<unsigned long> *l;
    string *s;
    for (;;){
        if (sin.eof() || sin.fail())
            return true;
        string line;
        getline(sin, line);
        if (line[0] == '['){
            for (;;){
                nextPart = line;
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
                const cfgParam *pp = params;
                char *obj = (char*)_obj;
                for (;;){
                    for (; *pp->name; pp++)
                        if (!strcmp(name.c_str(), pp->name)) break;
                    if (*pp->name) break;
                    if (pp->defValue == 0){
                        return true;
                    }
                    if (pp->offs) obj = *((char**)(obj + pp->offs));
                    pp = (const cfgParam*)(pp->defValue);
                }
                switch (pp->type){
                case PARAM_ULONG:
                case PARAM_USHORT:
                case PARAM_SHORT:
                case PARAM_STRING:
                case PARAM_BOOL:
                case PARAM_I18N:
                case PARAM_ULONGS:
                case PARAM_CHAR:
                    return true;
                }
                void *o = ((createObj*)(pp->type))();
                if (!load(o, (const cfgParam*)(pp->defValue), sin, nextPart))
                    return false;
                l = (list<unsigned long>*)(obj + pp->offs);
                l->push_back((unsigned long)o);
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
        const cfgParam *pp = params;
        char *obj = (char*)_obj;
        for (;;){
            for (; *pp->name; pp++)
                if (!strcmp(line.c_str(), pp->name)) break;
            if (*pp->name) break;
            if (pp->defValue == 0) break;
            if (pp->offs) obj = *((char**)(obj + pp->offs));
            pp = (const cfgParam*)(pp->defValue);
        }
        if (*pp->name == 0){
            log(L_WARN, "Bad key in config %s", line.c_str());
            continue;
        }
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
        switch (pp->type){
        case PARAM_ULONG:
            *((unsigned long*)(obj + pp->offs)) = atol(unquoted.c_str());
            break;
        case PARAM_USHORT:
            *((unsigned short*)(obj + pp->offs)) = atol(unquoted.c_str());
            break;
        case PARAM_SHORT:
            *((short*)(obj + pp->offs)) = atoi(unquoted.c_str());
            break;
        case PARAM_STRING:
        case PARAM_I18N:
            s = (string*)(obj + pp->offs);
            *s = unquoted;
            break;
        case PARAM_BOOL:
            *((bool*)(obj + pp->offs)) = ((unquoted != "false") && (unquoted != "0") && (unquoted != ""));
            break;
        case PARAM_ULONGS:
            l = (list<unsigned long>*)(obj + pp->offs);
            for (p = (char*)unquoted.c_str(); *p; ){
                l->push_back(atol(p));
                p = strchr(p, ',');
                if (p == NULL) break;
                p++;
            }
            break;
        case PARAM_CHAR:
            obj[pp->offs] = atoi(unquoted.c_str());
            break;
        default:
            log(L_WARN, "Bad key in config %s", line.c_str());
        }
    }
    return true;
}



