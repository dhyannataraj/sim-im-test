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

#include "simapi.h"
#include "sockfactory.h"
#include "socket.h"

#include <errno.h>

#include <qfile.h>
#include <qtoolbar.h>
#include <qmainwindow.h>
#include <qstringlist.h>
#include <qapplication.h>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#ifdef USE_KDE
#include <qapplication.h>
#include <kglobal.h>
#include <kstddirs.h>
#endif

#include <map>
using namespace std;

namespace SIM
{

void save_state()
{
    Event e(EventSaveState, NULL);
    e.process();
}

// ______________________________________________________________________________________

#ifdef WIN32

EXPORT bool makedir(char *p)
{
    char *r = strrchr(p, '\\');
    if (r == NULL) return true;
    *r = 0;
    CreateDirectoryA(p, NULL);
    *r = '\\';
    return true;
}

#else

EXPORT bool makedir(char *p)
{
    bool res = true;
    char *r = strrchr(p, '/');
    if (r == NULL) return res;
    *r = 0;
    struct stat st;
    if (stat(p, &st)){
        if (makedir(p)){
            if (mkdir(p, 0755)){
                log(L_ERROR, "Can't create %s: %s", p, strerror(errno));
                res = false;
            }
        }else{
            res = false;
        }
    }else{
        if ((st.st_mode & S_IFMT) != S_IFDIR){
            log(L_ERROR, "%s no directory", p);
            res = false;
        }
    }
    *r = '/';
    return res;
}

#endif

// _____________________________________________________________________________________

EXPORT string app_file(const char *f)
{
    string app_file_name = "";
#ifdef WIN32
    char buff[256];
    GetModuleFileNameA(NULL, buff, sizeof(buff));
    char *p = strrchr(buff, '\\');
    if (p) *p = 0;
    app_file_name = buff;
    if (app_file_name.length() && (app_file_name[app_file_name.length()-1] != '\\'))
        app_file_name += "\\";
#else
#ifdef USE_KDE
    if (qApp){
        QStringList lst = KGlobal::dirs()->findDirs("data", "sim");
        for (QStringList::Iterator it = lst.begin(); it != lst.end(); ++it){
            QFile fi(*it + f);
            if (fi.exists()){
                app_file_name = (const char*)QFile::encodeName(fi.name());
                return app_file_name;
            }
        }
    }
#endif
    app_file_name = PREFIX "/share/apps/sim/";
#endif
    app_file_name += f;
#ifdef WIN32
    for (p = (char*)app_file_name.c_str(); *p; p++)
        if (*p == '/')
            *p = '\\';
#endif
    return app_file_name;
}

// ______________________________________________________________________________________

EXPORT string user_file(const char *f)
{
    string res;
    if (f) {
        res = f;
    } else {
        res = "";
    }
    Event e(EventHomeDir, &res);
    if (e.process())
        return res;
    return app_file(f);
}

// ______________________________________________________________________________________

EXPORT string number(unsigned n)
{
    char b[32];
    snprintf(b, sizeof(b), "%u", n);
    return string(b);
}

EXPORT string trim(const char *from)
{
    string res;
    if (from == NULL)
        return res;
    res = from;
    int i;
    for (i = 0; i < (int)(res.length()); i++){
        if (res[i] != ' ')
            break;
    }
    if (i)
        res = res.substr(i);
    for (i = res.length() - 1; i >= 0; i--){
        if (res[i] != ' ')
            break;
    }
    if (i < (int)(res.length() - 1))
        res = res.substr(0, i + 1);
    return res;
}

EXPORT QString trim(const QString &from)
{
    QString res;
    res = from;
    int i;
    for (i = 0; i < (int)(res.length()); i++){
        if (res[i] != ' ')
            break;
    }
    if (i)
        res = res.mid(i);
    for (i = res.length() - 1; i >= 0; i--){
        if (res[i] != ' ')
            break;
    }
    if (i < (int)(res.length() - 1))
        res = res.mid(0, i + 1);
    return res;
}

EXPORT string quoteChars(const char *from, const char *chars)
{
    string res;
    for (; *from; from++){
        char c[2];
        c[0] = *from;
        c[1] = 0;
        if ((c[0] == '\\') || strstr(c, chars))
            res += '\\';
        res += c[0];
    }
    return res;
}

EXPORT string getToken(string &from, char c, bool bUnEscape)
{
    string res;
    unsigned i;
    for (i = 0; i < from.length(); i++){
        if (from[i] == c)
            break;
        if (from[i] == '\\'){
            if (!bUnEscape)
                res += '\\';
            i++;
            if (i >= from.length())
                break;
            if (bUnEscape){
                char c = from[i];
                switch (c){
                case 'n':
                    c = '\n';
                    break;
                case 'r':
                    c = '\r';
                    break;
                case 't':
                    c = '\t';
                    break;
                }
                res += c;
            }else{
                res += from[i];
            }
            continue;
        }
        res += from[i];
    }
    if (i < from.length()){
        from = from.substr(i + 1);
    }else{
        from = "";
    }
    return res;
}

EXPORT QString quoteChars(const QString &from, const char *chars, bool bQuoteSlash)
{
    QString res;
    for (int i = 0; i < (int)(from.length()); i++){
        QChar c = from[i];
        if (c){
            char s[2];
            s[0] = c;
            s[1] = 0;
            if ((bQuoteSlash && (s[0] == '\\')) || strstr(s, chars))
                res += '\\';
        }
        res += c;
    }
    return res;
}

EXPORT QString getToken(QString &from, char c, bool bUnEscape)
{
    QString res;
    int i;
    for (i = 0; i < (int)from.length(); i++){
        if (from[i] == c)
            break;
        if (bUnEscape && (from[i] == '\\')){
            i++;
            if (i >= (int)from.length())
                break;
            res += from[i];
            continue;
        }
        res += from[i];
    }
    if (i < (int)from.length()){
        from = from.mid(i + 1);
    }else{
        from = "";
    }
    return res;
}

// ______________________________________________________________________________________

EXPORT bool getLine(QFile &f, string &s)
{
    s = "";
    char b[0x4000];
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

// _______________________________________________________________________________________

bool set_ip(void **p, unsigned long value)
{
    IP **ip = (IP**)p;
    if (value == 0){
        if (*ip == NULL)
            return false;
        delete *ip;
        *ip = NULL;
        return true;
    }
    if (*ip == NULL)
        *ip = new IP;
    if ((*ip)->ip() == value){
        (*ip)->resolve();
        return false;
    }
    (*ip)->set(value);
    return true;
}

unsigned long get_ip(void *p)
{
    IP *ip = (IP*)p;
    if (ip)
        return ip->ip();
    return 0;
}

const char *get_host(void *p)
{
    IP *ip = (IP*)p;
    if (ip && ip->host())
        return ip->host();
    return "";
}

// _______________________________________________________________________________________

typedef map<unsigned, string> STRING_MAP;

EXPORT void clear_list(void **strlist)
{
    if (strlist == NULL)
        return;
    STRING_MAP *s_list = static_cast<STRING_MAP*>(*strlist);
    delete s_list;
    *strlist = NULL;
}

EXPORT const char *get_str(void *strlist, unsigned index)
{
    if (strlist == NULL)
        return "";
    STRING_MAP *s_list = static_cast<STRING_MAP*>(strlist);
    STRING_MAP::iterator it = s_list->find(index);
    if (it == s_list->end())
        return "";
    return (*it).second.c_str();
}

EXPORT void set_str(void **strlist, unsigned index, const char *value)
{
    if ((value == NULL) || (*value == 0)){
        if (*strlist == NULL)
            return;
        STRING_MAP *s_list = static_cast<STRING_MAP*>(*strlist);
        STRING_MAP::iterator it = s_list->find(index);
        if (it == s_list->end())
            return;
        s_list->erase(it);
        return;
    }
    if (*strlist == NULL)
        *strlist = new STRING_MAP;
    STRING_MAP *s_list = static_cast<STRING_MAP*>(*strlist);
    STRING_MAP::iterator it = s_list->find(index);
    if (it == s_list->end()){
        s_list->insert(STRING_MAP::value_type(index, value));
        return;
    }
    (*it).second = value;
}

// _______________________________________________________________________________________

EXPORT bool set_str(char **str, const char *value)
{
    if ((*str == NULL) && (value == NULL))
        return false;
    if (*str && value && !strcmp(*str, value))
        return false;
    if (*str){
        delete[] *str;
        *str = NULL;
    }
    if (value && *value){
        *str = new char[strlen(value) + 1];
        strcpy(*str, value);
    }
    return true;
}

EXPORT void free_data(const DataDef *def, void *data)
{
    unsigned offs = 0;
    for (; def->name; def++){
        unsigned type = def->type;
        if ((type == DATA_STRING) || (type == DATA_UTF)){
            char **p = (char**)(((char*)data) + offs);
            for (unsigned i = 0; i < def->n_values; i++, p++)
                set_str(p, NULL);
        }
        if ((type == DATA_STRLIST) || (type == DATA_UTFLIST)){
            clear_list((void**)(((char*)data) + offs));
        }
        if (type == DATA_OBJECT){
            QObject **p = (QObject**)(((char*)data) + offs);
            if (*p){
                delete *p;
                *p = NULL;
            }
        }
        if (type == DATA_IP){
            IP **p = (IP**)(((char*)data) + offs);
            if (*p)
                delete *p;
        }
        if (type == DATA_STRUCT){
            free_data((DataDef*)(def->def_value), ((char*)data) + offs);
        }
        offs += sizeof(void*) * def->n_values;
    }
}

static char fromHex(char c)
{
    if ((c >= '0') && (c <= '9')) return c - '0';
    if ((c >= 'A') && (c <= 'F')) return c + 10 - 'A';
    if ((c >= 'a') && (c <= 'f')) return c + 10 - 'a';
    return 0;
}

static string unquoteString(const char *p)
{
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
    return unquoted;
}

void init_data(const DataDef *d, void *data)
{
    unsigned offs = 0;
    for (const DataDef *def = d; def->name; def++){
        unsigned i;
        switch (def->type){
        case DATA_IP:{
                IP **p = (IP**)(((char*)data) + offs);
                *p = NULL;
                break;
            }
        case DATA_UTFLIST:
        case DATA_STRLIST:{
                STRING_MAP **p = (STRING_MAP**)(((char*)data) + offs);
                *p = NULL;
                break;
            }
        case DATA_OBJECT:{
                QObject **p = (QObject**)(((char*)data) + offs);
                *p = NULL;
                break;
            }
        case DATA_STRING:{
                char **p = (char**)(((char*)data) + offs);
                for (i = 0; i < def->n_values; i++, p++){
                    *p = NULL;
                    set_str(p, (char*)(def->def_value));
                }
                break;
            }
        case DATA_UTF:{
                char **p = (char**)(((char*)data) + offs);
                for (i = 0; i < def->n_values; i++, p++)
                    *p = NULL;
                if (def->def_value){
                    QString  value = i18n((const char*)(def->def_value));
                    QCString v = value.utf8();
                    p = (char**)(((char*)data) + offs);
                    for (i = 0; i < def->n_values; i++, p++)
                        set_str(p, v);
                }
                break;
            }
        case DATA_LONG:{
                long *p = (long*)(((char*)data) + offs);
                for (i = 0; i < def->n_values; i++, p++)
                    *p = def->def_value;
                break;
            }
        case DATA_ULONG:{
                unsigned long *p = (unsigned long*)(((char*)data) + offs);
                for (i = 0; i < def->n_values; i++, p++)
                    *p = def->def_value;
                break;
            }
        case DATA_BOOL:{
                unsigned long *p = (unsigned long*)(((char*)data) + offs);
                for (i = 0; i < def->n_values; i++, p++)
                    *p = (def->def_value != 0) ? -1 : 0;
                break;
            }
        case DATA_STRUCT:
            init_data((DataDef*)(def->def_value), ((char*)data) + offs);
        }
        offs += sizeof(void*) * def->n_values;
    }
}

const DataDef *find_key(const DataDef *def, const char *name, unsigned &offs)
{
    for (; def->name; def++){
        if (!strcmp(def->name, name))
            return def;
        if (def->type == DATA_STRUCT){
            unsigned save_offs = offs;
            const DataDef *res = find_key((DataDef*)(def->def_value), name, offs);
            if (res)
                return res;
            offs = save_offs;
        }
        offs += sizeof(void*) * def->n_values;
    }
    return NULL;
}

EXPORT void load_data(const DataDef *d, void *data, const char *config)
{
    init_data(d, data);
    if (config == NULL)
        return;
    string cfg = config;
    while (cfg.length()){
        string line = getToken(cfg, '\n', false);
        string key = getToken(line, '=');
        unsigned offs = 0;
        const DataDef *def = find_key(d, key.c_str(), offs);
        if (def == NULL)
            continue;
        unsigned i;
        switch (def->type){
        case DATA_IP:{
                string b = getToken(line, ',');
                IP **p = (IP**)(((char*)data) + offs);
                if (*p == NULL)
                    *p = new IP;
                (*p)->set(inet_addr(b.c_str()), line.c_str());
                break;
            }
        case DATA_STRLIST:{
                string v = getToken(line, ',');
                unsigned index = strtoul(v.c_str(), NULL, 10);
                if (index){
                    getToken(line, '\"');
                    v = getToken(line, '\"');
                    set_str((void**)(((char*)data) + offs), index, v.c_str());
                }
                break;
            }
        case DATA_UTFLIST:{
                string v = getToken(line, ',');
                unsigned index = strtoul(v.c_str(), NULL, 10);
                if (index){
                    getToken(line, '\"');
                    v = getToken(line, '\"', false);
                    v = unquoteString(v.c_str());
                    if (line[0] == 'u'){
                        set_str((void**)(((char*)data) + offs), index, v.c_str());
                    }else{
                        QString s = QString::fromLocal8Bit(v.c_str());
                        set_str((void**)(((char*)data) + offs), index, s.utf8());
                    }
                }
                break;
            }
        case DATA_UTF:{
                char **p = (char**)(((char*)data) + offs);
                for (i = 0; i < def->n_values; i++, p++){
                    getToken(line, '\"');
                    if (line.length() == 0)
                        break;
                    string v = getToken(line, '\"', false);
                    v = unquoteString(v.c_str());
                    if (line[0] == 'u'){
                        set_str(p, v.c_str());
                    }else{
                        QString s = QString::fromLocal8Bit(v.c_str());
                        set_str(p, s.utf8());
                    }
                    getToken(line, ',');
                }
                if (i < def->n_values)
                    set_str(p, NULL);
                break;
            }
        case DATA_STRING:{
                char **p = (char**)(((char*)data) + offs);
                for (i = 0; i < def->n_values; i++, p++){
                    unsigned n;
                    for (n = 0; n < line.length(); n++)
                        if (line[n] != ' ') break;
                    if (n >= line.length())
                        break;
                    line = line.substr(n);
                    string v;
                    if (line[n] == '\"'){
                        for (n = 1; n < line.length(); n++){
                            if (line[n] == '\"')
                                break;
                            if (line[n] == '\\')
                                n++;
                        }
                        v = line.substr(1, n - 1);
                        line = line.substr(n);
                    }else{
                        for (n = 0; n < line.length(); n++){
                            if ((line[n] == ',') && (i + 1 < def->n_values))
                                break;
                            if (line[n] == '\\')
                                n++;
                        }
                        v = line.substr(0, n - 1);
                        line = line.substr(n);
                    }
                    if (v.length()){
                        v = unquoteString(v.c_str());
                        set_str(p, v.c_str());
                    }else{
                        set_str(p, NULL);
                    }
                    for (n = 0; n < line.length(); n++)
                        if (line[n] == ',') break;
                    line = line.substr(n);
                }
                if (i < def->n_values)
                    set_str(p, NULL);
                break;
            }
        case DATA_LONG:{
                long *p = (long*)(((char*)data) + offs);
                for (i = 0; i < def->n_values; i++, p++){
                    if (line.length() == 0)
                        break;
                    string v = getToken(line, ',');
                    *p = atol(v.c_str());
                }
                break;
            }
        case DATA_ULONG:{
                unsigned long *p = (unsigned long*)(((char*)data) + offs);
                for (i = 0; i < def->n_values; i++, p++){
                    if (line.length() == 0)
                        break;
                    string v = getToken(line, ',');
                    *p = strtoul(v.c_str(), NULL, 10);
                }
                break;
            }
        case DATA_BOOL:{
                unsigned long *p = (unsigned long*)(((char*)data) + offs);
                for (i = 0; i < def->n_values; i++, p++){
                    if (line.length() == 0)
                        break;
                    string v = getToken(line, ',');
                    if (v.length()){
                        if (!strcasecmp(v.c_str(), "false") || !strcmp(v.c_str(), "0")){
                            *p = 0;
                        }else{
                            *p = (unsigned)(-1);
                        }
                    }
                }
                break;
            }
        }
    }
}

static char toHex(char c)
{
    c &= 0x0F;
    if (c < 10) return c + '0';
    return c - 10 + 'a';
}

static string quoteString(const char *str)
{
    string quoted;
    quoted = "\"";
    if (str){
        for (unsigned char *p = (unsigned char*)str; *p; p++){
            switch (*p){
            case '\\':
                quoted += "\\\\";
                break;
            case '\n':
                quoted += "\\n";
                break;
            case '\"':
                quoted += "\\\"";
                break;
            default:
                if (*p >= ' '){
                    quoted += *p;
                }else if (*p){
                    quoted += "\\x";
                    quoted += toHex(*p >> 4);
                    quoted += toHex(*p);
                }
            }
        }
    }
    quoted += "\"";
    return quoted;
}

EXPORT string save_data(const DataDef *def, void *data)
{
    string res;
    unsigned offs = 0;
    for (; def->name; def++){
        string value;
        bool bSave = false;
        unsigned i;
        if (def->type == DATA_STRUCT){
            string s = save_data((DataDef*)(def->def_value), ((char*)data) + offs);
            if (s.length()){
                if (res.length())
                    res += "\n";
                res += s;
            }
        }else  if (*def->name){
            switch (def->type){
            case DATA_IP:{
                    IP *p = *((IP**)(((char*)data) + offs));
                    if (p && p->ip()){
                        struct in_addr inaddr;
                        inaddr.s_addr = p->ip();
                        value = inet_ntoa(inaddr);
                        const char *host = p->host();
                        if (host && *host){
                            value += ",";
                            value += host;
                        }
                        bSave = true;
                    }
                    break;
                }
            case DATA_STRLIST:{
                    STRING_MAP *p = *((STRING_MAP**)(((char*)data) + offs));
                    if (p){
                        for (STRING_MAP::iterator it = p->begin(); it != p->end(); ++it){
                            if (res.length())
                                res += "\n";
                            res += def->name;
                            res += "=";
                            res += number((*it).first);
                            res += ",";
                            res += quoteString((*it).second.c_str());
                        }
                    }
                    break;
                }
            case DATA_UTFLIST:{
                    STRING_MAP *p = *((STRING_MAP**)(((char*)data) + offs));
                    if (p){
                        for (STRING_MAP::iterator it = p->begin(); it != p->end(); ++it){
                            if (res.length())
                                res += "\n";
                            res += def->name;
                            res += "=";
                            res += number((*it).first);
                            res += ",";
                            QString s = QString::fromUtf8((*it).second.c_str());
                            QCString ls = s.local8Bit();
                            if (QString::fromLocal8Bit(ls) == s){
                                res += quoteString((const char*)ls);
                            }else{
                                res += quoteString((const char*)(s.utf8()));
                                res += "u";
                            }
                        }
                    }
                    break;
                }
            case DATA_STRING:{
                    char **p = (char**)(((char*)data) + offs);
                    for (i = 0; i < def->n_values; i++, p++){
                        if (value.length())
                            value += ",";
                        if (def->def_value){
                            if ((*p == NULL) || strcmp(*p, (char*)(def->def_value))){
                                value += quoteString(*p);
                                bSave = true;
                            }
                        }else{
                            if ((*p != NULL) && **p){
                                value += quoteString(*p);
                                bSave = true;
                            }
                        }
                    }
                    break;
                }
            case DATA_UTF:{
                    char **p = (char**)(((char*)data) + offs);
                    for (i = 0; i < def->n_values; i++, p++){
                        if (value.length())
                            value += ",";
                        QString s;
                        if (*p != NULL)
                            s = QString::fromUtf8(*p);
                        if (def->def_value){
                            if (s != i18n((const char*)(def->def_value)))
                                bSave = true;
                        }else{
                            if (s.length())
                                bSave = true;
                        }
                        if (bSave){
                            QCString ls = s.local8Bit();
                            if (QString::fromLocal8Bit(ls) == s){
                                value += quoteString((const char*)ls);
                            }else{
                                value += quoteString((const char*)(s.utf8()));
                                value += "u";
                            }
                        }
                    }
                    break;
                }
            case DATA_BOOL:{
                    unsigned long *p = (unsigned long*)(((char*)data) + offs);
                    for (i = 0; i < def->n_values; i++, p++){
                        if (value.length())
                            value += ",";
                        if ((*p != 0) != (def->def_value != 0)){
                            if (*p){
                                value += "true";
                            }else{
                                value += "false";
                            }
                            bSave = true;
                        }
                    }
                    break;
                }
            case DATA_LONG:{
                    long *p = (long*)(((char*)data) + offs);
                    for (i = 0; i < def->n_values; i++, p++){
                        if (value.length())
                            value += ",";
                        if (*p != (long)(def->def_value)){
                            char b[32];
                            snprintf(b, sizeof(b), "%li", *p);
                            value += b;
                            bSave = true;
                        }
                    }
                    break;
                }
            case DATA_ULONG:{
                    unsigned long *p = (unsigned long*)(((char*)data) + offs);
                    for (i = 0; i < def->n_values; i++, p++){
                        if (value.length())
                            value += ",";
                        if (*p != (unsigned long)(def->def_value)){
                            char b[32];
                            snprintf(b, sizeof(b), "%lu", *p);
                            value += b;
                            bSave = true;
                        }
                    }
                    break;
                }
            }
            if (bSave){
                if (res.length())
                    res += "\n";
                res += def->name;
                res += "=";
                res += value;
            }
        }
        offs += sizeof(void*) * def->n_values;
    }
    return res;
}

// ______________________________________________________________________________________

#ifdef WIN32
#ifndef SM_CYSMCAPTION
#define SM_CYSMCAPTION          51
#endif
#endif

EXPORT void saveGeometry(QWidget *w, long geo[4])
{
    if (w == NULL)
        return;
    QPoint pos = w->pos();
    QSize size = w->size();
    geo[0] = pos.x();
    geo[1] = pos.y();
    geo[2] = size.width();
    geo[3] = size.height();
#ifdef WIN32
    if (!(GetWindowLongA(w->winId(), GWL_EXSTYLE) & WS_EX_APPWINDOW)){
        int dc = GetSystemMetrics(SM_CYCAPTION);
        int ds = GetSystemMetrics(SM_CYSMCAPTION);
        geo[1] += dc - ds;
        geo[3] -= (dc - ds);
    }
#endif
}

EXPORT void restoreGeometry(QWidget *w, long geo[4], bool bPos, bool bSize)
{
    if (w == NULL)
        return;
    QRect rc = screenGeometry();
    if (geo[WIDTH] > rc.width())
        geo[WIDTH] = rc.width();
    if (geo[HEIGHT] > rc.height())
        geo[HEIGHT] = rc.height();
    if (geo[LEFT] + geo[WIDTH] > rc.width())
        geo[LEFT] = rc.width() - geo[WIDTH];
    if (geo[TOP] + geo[HEIGHT] > rc.height())
        geo[TOP] = rc.height() - geo[HEIGHT];
    if (geo[LEFT] < 0)
        geo[LEFT] = 0;
    if (geo[TOP] < 0)
        geo[TOP] = 0;
    if (bPos)
        w->move(geo[LEFT], geo[TOP]);
    if (bSize)
        w->resize(geo[WIDTH], geo[HEIGHT]);
}

EXPORT void saveToolbar(QToolBar *bar, long state[7])
{
    memset(state, 0, sizeof(state));
    if (bar == NULL)
        return;
    QMainWindow *main = NULL;
    for (QWidget *w = bar->parentWidget(); w; w = w->parentWidget()){
        if (w->inherits("QMainWindow")){
            main = static_cast<QMainWindow*>(w);
            break;
        }
    }
    if (main == NULL)
        return;
    QMainWindow::ToolBarDock dock;
    int  index;
    bool nl;
    int  extraOffset;
    main->getLocation(bar, dock, index, nl, extraOffset);
    state[0] = 1;
    state[1] = (long)dock;
    state[2] = index;
    state[3] = nl ? 1 : 0;
    state[4] = extraOffset;
    if (dock == QMainWindow::TornOff){
        QPoint pos = bar->geometry().topLeft();
        state[5] = pos.x();
        state[6] = pos.y();
    }
}

EXPORT void restoreToolbar(QToolBar *bar, long state[7])
{
    if (bar == NULL)
        return;
    if (state[0] == 0)
        return;
    QMainWindow *main = NULL;
    for (QWidget *w = bar->parentWidget(); w; w = w->parentWidget()){
        if (w->inherits("QMainWindow")){
            main = static_cast<QMainWindow*>(w);
            break;
        }
    }
    if (main == NULL)
        return;
    QMainWindow::ToolBarDock dock = (QMainWindow::ToolBarDock)state[1];
    main->moveToolBar(bar, dock, state[2], state[3] != 0, state[4]);
    if (dock == QMainWindow::TornOff)
        bar->move(state[5], state[6]);
}

EXPORT bool cmp(char *s1, char *s2)
{
    if (s1 == NULL)
        return s2 != NULL;
    if (s2 == NULL)
        return true;
    return strcmp(s1, s2);
}

};
