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

#include <stdio.h>

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
#include <kwin.h>
#include "kdeisversion.h"
#endif

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(push)
#pragma warning(disable: 4512)  
#endif
#endif

#include <map>
using namespace std;

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(pop)
#endif
#endif

namespace SIM
{

void save_state()
{
    Event e(EventSaveState, NULL);
    e.process();
}

// ______________________________________________________________________________________

#ifdef WIN32

static bool isWindowsNT()
{
    OSVERSIONINFO ovi;

    ZeroMemory(&ovi, sizeof(ovi));
    ovi.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
    GetVersionEx(&ovi);

    return (ovi.dwPlatformId==VER_PLATFORM_WIN32_NT);
}

EXPORT bool makedir(char *p)
{
    char *r = strrchr(p, '\\');
    if (r == NULL) return true;
    *r = 0;
    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;
    ZeroMemory(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    if(isWindowsNT()){
        InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
        sa.lpSecurityDescriptor = &sd;
    }
    CreateDirectoryA(p, &sa);
    DWORD dwAttr = GetFileAttributesA(p);
    if (dwAttr & FILE_ATTRIBUTE_READONLY)
        SetFileAttributesA(p, dwAttr & ~FILE_ATTRIBUTE_READONLY);
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
            if (mkdir(p, 0700)){
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
    QString fname = QFile::decodeName(f);
#ifdef WIN32
    if ((fname[1] == ':') || (fname.left(2) == "\\\\"))
        return f;
    char buff[256];
    GetModuleFileNameA(NULL, buff, sizeof(buff));
    char *p = strrchr(buff, '\\');
    if (p) *p = 0;
    app_file_name = buff;
    if (app_file_name.length() && (app_file_name[app_file_name.length()-1] != '\\'))
        app_file_name += "\\";
#else
    if (fname[0] == '/')
        return f;
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
        if (from[i] == '\\'){
            i++;
            if (i >= (int)from.length())
                break;
            if (!bUnEscape)
                res += "\\";
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

bool set_ip(Data *p, unsigned long value, const char *host)
{
    IP **ip = (IP**)(&p->ptr);
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
        if (host == NULL)
            (*ip)->resolve();
        return false;
    }
    (*ip)->set(value, host);
    return true;
}

unsigned long get_ip(Data &p)
{
    IP *ip = (IP*)p.ptr;
    if (ip)
        return ip->ip();
    return 0;
}

const char *get_host(Data &p)
{
    IP *ip = (IP*)p.ptr;
    if (ip && ip->host())
        return ip->host();
    return "";
}

// _______________________________________________________________________________________

typedef map<unsigned, string> STRING_MAP;

EXPORT void clear_list(Data *d)
{
    STRING_MAP **strlist = (STRING_MAP**)(&d->ptr);
    if (*strlist == NULL)
        return;
    delete *strlist;
    *strlist = NULL;
}

EXPORT const char *get_str(const Data &d, unsigned index)
{
    STRING_MAP *strlist = (STRING_MAP*)(d.ptr);
    if (strlist == NULL)
        return "";
    STRING_MAP::iterator it = strlist->find(index);
    if (it == strlist->end())
        return "";
    return (*it).second.c_str();
}

EXPORT void set_str(Data *d, unsigned index, const char *value)
{
    STRING_MAP **strlist = (STRING_MAP**)(&d->ptr);
    if ((value == NULL) || (*value == 0)){
        if (*strlist == NULL)
            return;
        STRING_MAP::iterator it = (*strlist)->find(index);
        if (it == (*strlist)->end())
            return;
        (*strlist)->erase(it);
        return;
    }
    if (*strlist == NULL)
        *strlist = new STRING_MAP;
    STRING_MAP::iterator it = (*strlist)->find(index);
    if (it == (*strlist)->end()){
        (*strlist)->insert(STRING_MAP::value_type(index, value));
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

EXPORT void free_data(const DataDef *def, void *d)
{
    Data *data = (Data*)d;
    for (; def->name; def++){
        unsigned type = def->type;
        for (unsigned i = 0; i < def->n_values; i++, data++){
            switch (type){
            case DATA_STRING:
            case DATA_UTF:
                set_str(&data->ptr, NULL);
                break;
            case DATA_STRLIST:
            case DATA_UTFLIST:
                clear_list(data);
                break;
            case DATA_OBJECT:
                if (data->ptr){
                    delete (QObject*)(data->ptr);
                    data->ptr = NULL;
                }
                break;
            case DATA_IP:
                if (data->ptr){
                    delete (IP*)(data->ptr);
                    data->ptr = NULL;
                }
                break;
            case DATA_STRUCT:
                free_data((DataDef*)(def->def_value), data);
                i    += (def->n_values - 1);
                data += (def->n_values - 1);
                break;
            }
        }
    }
}

char fromHex(char c)
{
    if ((c >= '0') && (c <= '9')) return (char)(c - '0');
    if ((c >= 'A') && (c <= 'F')) return (char)(c + 10 - 'A');
    if ((c >= 'a') && (c <= 'f')) return (char)(c + 10 - 'a');
    return (char)0;
}

string unquoteString(const char *p)
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
                c = (char)((fromHex(p[1]) << 4) + fromHex(p[2]));
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

void init_data(const DataDef *d, Data *data)
{
    for (const DataDef *def = d; def->name; def++){
        for (unsigned i = 0; i < def->n_values; i++, data++){
            data->ptr = NULL;
            switch (def->type){
            case DATA_STRING:
                set_str(&data->ptr, def->def_value);
                break;
            case DATA_UTF:
                if (def->def_value){
                    QString  value = i18n(def->def_value);
                    QCString v = value.utf8();
                    set_str(&data->ptr, v);
                }
                break;
            case DATA_ULONG:
            case DATA_LONG:
                data->value = (unsigned)(def->def_value);
                break;
            case DATA_BOOL:
                data->bValue = (def->def_value != NULL);
                break;
            case DATA_STRUCT:
                init_data((DataDef*)(def->def_value), data);
                data += (def->n_values - 1);
                i += (def->n_values - 1);
                break;
            }
        }
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
        offs += def->n_values;
    }
    return NULL;
}

EXPORT void load_data(const DataDef *d, void *_data, const char *config)
{
    Data *data = (Data*)_data;
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
        string v;
        Data *ld = data + offs;
        switch (def->type){
        case DATA_IP:
            v = getToken(line, ',');
            set_ip(ld, inet_addr(v.c_str()), line.c_str());
            break;
        case DATA_STRLIST:
            v = getToken(line, ',');
            i = strtoul(v.c_str(), NULL, 10);
            if (i){
                getToken(line, '\"');
                v = getToken(line, '\"');
                set_str(ld, i, v.c_str());
            }
            break;
        case DATA_UTFLIST:
            v = getToken(line, ',');
            i = strtoul(v.c_str(), NULL, 10);
            if (i){
                getToken(line, '\"');
                v = getToken(line, '\"', false);
                v = unquoteString(v.c_str());
                if (line.length() && (line[0] == 'u')){
                    set_str(ld, i, v.c_str());
                }else{
                    QString s = QString::fromLocal8Bit(v.c_str());
                    set_str(ld, i, s.utf8());
                }
            }
            break;
        case DATA_UTF:
            for (i = 0; i < def->n_values; i++, ld++){
                getToken(line, '\"');
                if (line.length() == 0)
                    break;
                v = getToken(line, '\"', false);
                v = unquoteString(v.c_str());
                if (line.length() && (line[0] == 'u')){
                    set_str(&ld->ptr, v.c_str());
                }else{
                    QString s = QString::fromLocal8Bit(v.c_str());
                    set_str(&ld->ptr, s.utf8());
                }
                getToken(line, ',');
            }
            if (i < def->n_values)
                set_str(&ld->ptr, NULL);
            break;
        case DATA_STRING:
            for (i = 0; i < def->n_values; i++, ld++){
                unsigned n;
                for (n = 0; n < line.length(); n++)
                    if (line[n] != ' ') break;
                if (n >= line.length())
                    break;
                line = line.substr(n);
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
                    set_str(&ld->ptr, v.c_str());
                }else{
                    set_str(&ld->ptr, NULL);
                }
                for (n = 0; n < line.length(); n++)
                    if (line[n] == ',') break;
                line = line.substr(n);
            }
            if (i < def->n_values)
                set_str(&ld->ptr, NULL);
            break;
        case DATA_LONG:
            for (i = 0; i < def->n_values; i++, ld++){
                if (line.length() == 0)
                    break;
                v = getToken(line, ',');
                ld->value = atol(v.c_str());
            }
            break;
        case DATA_ULONG:
            for (i = 0; i < def->n_values; i++, ld++){
                if (line.length() == 0)
                    break;
                v = getToken(line, ',');
                ld->value = strtoul(v.c_str(), NULL, 10);
            }
            break;
        case DATA_BOOL:
            for (i = 0; i < def->n_values; i++, ld++){
                if (line.length() == 0)
                    break;
                v = getToken(line, ',');
                if (v.length()){
                    if (!strcasecmp(v.c_str(), "false") || !strcmp(v.c_str(), "0")){
                        ld->bValue = false;
                    }else{
                        ld->bValue = true;
                    }
                }
            }
            break;
        }
    }
}

static char toHex(char c)
{
    c &= 0x0F;
    if (c < 10) return (char)(c + '0');
    return (char)(c - 10 + 'a');
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
                    quoted += toHex((char)(*p >> 4));
                    quoted += toHex(*p);
                }
            }
        }
    }
    quoted += "\"";
    return quoted;
}

EXPORT string save_data(const DataDef *def, void *_data)
{
    Data *data = (Data*)_data;
    string res;
    for (; def->name; def++){
        string value;
        bool bSave = false;
        unsigned i;
        if (def->type == DATA_STRUCT){
            string s = save_data((DataDef*)(def->def_value), data);
            if (s.length()){
                if (res.length())
                    res += "\n";
                res += s;
            }
        }else  if (*def->name){
            Data *d = data;
            switch (def->type){
            case DATA_IP:{
                    IP *p = (IP*)(d->ptr);
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
                    STRING_MAP *p = (STRING_MAP*)(d->ptr);
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
                    STRING_MAP *p = (STRING_MAP*)(d->ptr);
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
                    for (i = 0; i < def->n_values; i++, d++){
                        char *p = d->ptr;
                        if (value.length())
                            value += ",";
                        if (def->def_value){
                            if ((p == NULL) || strcmp(p, def->def_value)){
                                value += quoteString(p);
                                bSave = true;
                            }
                        }else{
                            if ((p != NULL) && *p){
                                value += quoteString(p);
                                bSave = true;
                            }
                        }
                    }
                    break;
                }
            case DATA_UTF:{
                    for (i = 0; i < def->n_values; i++, d++){
                        char *p = d->ptr;
                        if (value.length())
                            value += ",";
                        QString s;
                        if (p != NULL)
                            s = QString::fromUtf8(p);
                        if (def->def_value){
                            if (s != i18n(def->def_value))
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
                    for (i = 0; i < def->n_values; i++, d++){
                        bool p = d->bValue;
                        if (value.length())
                            value += ",";
                        if (p != (def->def_value != 0)){
                            if (p){
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
                    for (i = 0; i < def->n_values; i++, d++){
                        long p = d->value;
                        if (value.length())
                            value += ",";
                        if (p != (long)(def->def_value)){
                            char b[32];
                            snprintf(b, sizeof(b), "%li", p);
                            value += b;
                            bSave = true;
                        }
                    }
                    break;
                }
            case DATA_ULONG:{
                    for (i = 0; i < def->n_values; i++, d++){
                        unsigned long p = d->value;
                        if (value.length())
                            value += ",";
                        if (p != (unsigned long)(def->def_value)){
                            char b[32];
                            snprintf(b, sizeof(b), "%lu", p);
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
        data += def->n_values;
    }
    return res;
}

// ______________________________________________________________________________________

#ifdef WIN32
#ifndef SM_CYSMCAPTION
#define SM_CYSMCAPTION          51
#endif
#endif

EXPORT void saveGeometry(QWidget *w, Data geo[5])
{
    if (w == NULL)
        return;
    QPoint pos = w->pos();
    QSize size = w->size();
    geo[0].value = pos.x();
    geo[1].value = pos.y();
    geo[2].value = size.width();
    geo[3].value = size.height();
#ifdef WIN32
    if (GetWindowLongA(w->winId(), GWL_EXSTYLE) & WS_EX_TOOLWINDOW){
        int dc = GetSystemMetrics(SM_CYCAPTION);
        int ds = GetSystemMetrics(SM_CYSMCAPTION);
        geo[1].value += dc - ds;
        geo[3].value -= (dc - ds) * 2;
    }
#endif
#ifdef USE_KDE
#if KDE_IS_VERSION(3,2,0)
    KWin::WindowInfo info = KWin::windowInfo(w->winId());
    geo[4].value = info.desktop();
    if (info.onAllDesktops())
        geo[4].value = (unsigned)(-1);
#else
KWin::Info info = KWin::info(w->winId());
    geo[4].value = info.desktop;
    if (info.onAllDesktops)
        geo[4].value = (unsigned)(-1);
#endif
#endif
}

EXPORT void restoreGeometry(QWidget *w, Data geo[5], bool bPos, bool bSize)
{
    if (w == NULL)
        return;
    QRect rc = screenGeometry();
    if ((int)geo[WIDTH].value > rc.width())
        geo[WIDTH].value = rc.width();
    if ((int)geo[HEIGHT].value > rc.height())
        geo[HEIGHT].value = rc.height();
    if ((int)geo[LEFT].value + (int)geo[WIDTH].value > rc.width())
        geo[LEFT].value = rc.width() - geo[WIDTH].value;
    if ((int)geo[TOP].value + (int)geo[HEIGHT].value > rc.height())
        geo[TOP].value = rc.height() - geo[HEIGHT].value;
    if ((int)geo[LEFT].value < 0)
        geo[LEFT].value = 0;
    if ((int)geo[TOP].value < 0)
        geo[TOP].value = 0;
    if (bPos)
        w->move(geo[LEFT].value, geo[TOP].value);
    if (bSize)
        w->resize(geo[WIDTH].value, geo[HEIGHT].value);
#ifdef USE_KDE
    if (geo[4].value == (unsigned)(-1)){
        KWin::setOnAllDesktops(w->winId(), true);
    }else{
        KWin::setOnAllDesktops(w->winId(), false);
        KWin::setOnDesktop(w->winId(), geo[4].value);
    }
#endif
}

EXPORT void saveToolbar(QToolBar *bar, Data state[8])
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
    state[0].value = 1;
    state[1].value = (long)dock;
    state[2].value = index;
    state[3].value = nl ? 1 : 0;
    state[4].value = extraOffset;
    if (dock == QMainWindow::TornOff){
        QPoint pos = bar->geometry().topLeft();
        state[5].value = pos.x();
        state[6].value = pos.y();
    }
}

EXPORT void restoreToolbar(QToolBar *bar, Data state[8])
{
    if (bar == NULL)
        return;
    if (state[0].value == 0)
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
    QMainWindow::ToolBarDock dock = (QMainWindow::ToolBarDock)state[1].value;
    main->moveToolBar(bar, dock, state[2].value != 0, state[3].value != 0, state[4].value);
    if (dock == QMainWindow::TornOff)
        bar->move(state[5].value, state[6].value);
}

EXPORT bool cmp(char *s1, char *s2)
{
    if (s1 == NULL)
        return s2 != NULL;
    if (s2 == NULL)
        return true;
    return strcmp(s1, s2) != 0;
}

};
