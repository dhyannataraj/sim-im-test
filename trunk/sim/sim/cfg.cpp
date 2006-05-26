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
#include <qregexp.h>
#include <qdir.h>

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

EXPORT QString app_file(const char *f)
{
    QString app_file_name = "";
    QString fname = QFile::decodeName(f);
#ifdef WIN32
    if ((fname[1] == ':') || (fname.left(2) == "\\\\"))
        return f;
    char buff[256];
    GetModuleFileNameA(NULL, buff, sizeof(buff));
    char *p = strrchr(buff, '\\');
    if (p) *p = 0;
    app_file_name = buff;
    if (app_file_name.length() && (app_file_name.right(1) != "\\") && (app_file_name.right(1) != "/"))
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
    return QDir::convertSeparators(app_file_name);
}

// ______________________________________________________________________________________

EXPORT QString user_file(const char *f)
{
    QString res;
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

// deprecated
EXPORT std::string number(unsigned n)
{
    return QString::number(n).latin1();
}

// deprecated
EXPORT std::string trim(const char *from)
{
    return QString(from).stripWhiteSpace();
}

// deprecated
EXPORT QString trim(const QString &from)
{
    return from.stripWhiteSpace();
}

char fromHex(char c)
{
    if ((c >= '0') && (c <= '9')) return (char)(c - '0');
    if ((c >= 'A') && (c <= 'F')) return (char)(c + 10 - 'A');
    if ((c >= 'a') && (c <= 'f')) return (char)(c + 10 - 'a');
    return (char)0;
}

EXPORT string getToken(char const *&p, char c, bool bUnEscape)
{
    string res;
    char const *start = p;
    for (; *p; p++){
        if (*p == c)
            break;
        if (*p == '\\'){
            p++;
            if (*p == 0)
                break;
            if (!bUnEscape)
                continue;
            char c = *p;
            int d = 0;
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
            case 'x':
                if (p[1] && p[2]){
                    c = (char)((fromHex(p[1]) << 4) + fromHex(p[2]));
                    d = 2;
                }
                break;
            }
            if (start != p - 1){
                string part;
                part.append(start, (unsigned)(p - start - 1));
                res += part;
            }
            res += c;
            start = p + 1 + d;
            continue;
        }
    }
    if (start != p){
        string part;
        part.append(start, (unsigned)(p - start));
        res += part;
    }
    if (*p == c)
        p++;
    return res;
}

EXPORT string getToken(string &from, char c, bool bUnEscape)
{
    const char *p = from.c_str();
    string res = getToken(p, c, bUnEscape);
    from = string(p);
    return res;
}

EXPORT QString quoteChars(const QString &from, const char *chars, bool bQuoteSlash)
{
    QString     res;
    QString     quote_chars;

    quote_chars = chars;
    if (bQuoteSlash) {
        quote_chars += '\\';
    }
    for (int i = 0; i < (int) (from.length ()); i++) {
        QChar       c = from[i];
        if (quote_chars.contains (c)) {
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

EXPORT bool set_utf8(char **str, const QString &value)
{
    if ((*str == NULL) && (value.isEmpty()))
        return false;
    if (*str && !value.isEmpty() && *str == value)
        return false;
    if (*str){
        delete[] *str;
        *str = NULL;
    }
    if (!value.isEmpty()){
        QCString utf8 = value.utf8();
        *str = new char[utf8.length() + 1];
        strcpy(*str, utf8.data());
        return true;
    }
    return false;
}

EXPORT QString get_utf8(const char *str)
{
    if(!str)
        return QString();
    return QString::fromUtf8(str);
}

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
        return true;
    }
    return false;
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

QString unquoteStringInternal(const QString &p)
{
    QString unquoted;
    int pos = 0;
    if (p.startsWith("\""))
        pos = 1;
    int length = p.length();

    for (; pos < length; pos++){
        QChar c = p[pos];
        if (c != '\\'){
            unquoted += c;
            continue;
        }
        pos++;
        if ( pos >= length)
            break;
        c = p[pos];
        switch (c){
        case '\\':
            unquoted += '\\';
            break;
        case 'n':
            unquoted += '\n';
            break;
        case 't':
            unquoted += '\t';
            break;
        case 'x': {
            if ( pos + 2 >= length ) {
                pos--;
                break;
            }
            QChar c1 = p[pos+1];
            QChar c2 = p[pos+2];
            char ch;
            ch = (char)((fromHex(c1) << 4) + fromHex(c2));
            unquoted += ch;
            pos += 2;
            break;
        }
        default:
            pos--;
        }
    }
    if( unquoted.endsWith( "\"" ) )
        unquoted = unquoted.left( unquoted.length() - 1 );
    return unquoted;
}

void init_data(const DataDef *d, Data *data)
{
    for (const DataDef *def = d; def->name; def++){
        for (unsigned i = 0; i < def->n_values; i++, data++){
            data->ptr = NULL;
            switch (def->type){
            case DATA_STRING:
            case DATA_STRLIST:
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
                data->value = (unsigned long)(def->def_value);
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

EXPORT void load_data(const DataDef *d, void *_data, ConfigBuffer *cfg)
{
    Data *data = (Data*)_data;
    init_data(d, data);
    if (cfg == NULL)
        return;
    cfg->savePos();
    for (;;){
        QString line = cfg->getLine();
        if (line.isEmpty())
            break;
        int idx = line.find('=');
        if(idx == -1)
            continue;
        QString name = line.left( idx );
        QString val  = line.mid( idx + 1 );
        if(name.isEmpty() || val.isEmpty())
            continue;

        unsigned offs = 0;
        const DataDef *def = find_key(d, name, offs);
        if (def == NULL)
            continue;
        Data *ld = data + offs;
        switch (def->type){
        case DATA_IP: {
            QStringList strlist = QStringList::split( ',', val );
            const char *ip = strlist[0];
            const char *url = strlist[1];
            set_ip(ld, inet_addr(ip), url);
            break;
        }
        case DATA_STRLIST: {
            int idx = val.find( ',' );
            if( idx == -1 )
                break;
            QString cnt = val.left( idx );
            int i = cnt.toULong();
            if (i == 0)
                break;
            QString s = unquoteStringInternal(val.mid( idx + 1 ));
            set_str(ld, i, s);
            break;
        }
        case DATA_UTFLIST: {
            int idx = val.find( ',' );
            if( idx == -1 )
                break;
            QString cnt = val.left( idx );
            int i = cnt.toULong();
            if (i == 0)
                break;
            QString s = val.mid( idx + 1 );
            // no longer needed, but we want to read old configs correct...
            if( s.endsWith( "u" ) ) {
                s = unquoteStringInternal( s.left( s.length() - 1 ) );
            } else {
                s = unquoteStringInternal( s );
            }
            set_str(ld, i, s.utf8());
            break;
        }
        case DATA_UTF: {
            QStringList sl = QStringList::split( "\",\"", val );
            for (unsigned i = 0; i < def->n_values && i < sl.count(); i++, ld++){
                QString s = sl[i];
                if(s.isEmpty())
                    continue;
                // no longer needed, but we want to read old configs correct...
                if( s.endsWith( "u" ) ) {
                    s = unquoteStringInternal( s.left( s.length() - 1 ) );
                } else {
                    s = unquoteStringInternal( s );
                }
                set_str(&ld->ptr, s.utf8());
            }
            break;
        }
        case DATA_STRING: {
            QStringList sl = QStringList::split( "\",\"", val );
            for (unsigned i = 0; i < def->n_values && i < sl.count(); i++, ld++){
                QString s = sl[i];
                if(s.isEmpty())
                    continue;
                s = unquoteStringInternal(s);
                set_str(&ld->ptr, s);
            }
            break;
        }
        case DATA_LONG: {
            QStringList sl = QStringList::split(',',val,true);
            for (unsigned i = 0; i < def->n_values && i < sl.count(); i++, ld++){
                QString s = sl[i];
                if(s.isEmpty())
                    continue;
                ld->value = s.toLong();
            }
            break;
        }
        case DATA_ULONG: {
            QStringList sl = QStringList::split(',',val,true);
            for (unsigned i = 0; i < def->n_values && i < sl.count(); i++, ld++){
                QString s = sl[i];
                if(s.isEmpty())
                    continue;
                ld->value = s.toULong();
            }
            break;
        }
        case DATA_BOOL: {
            QStringList sl = QStringList::split(',',val,true);
            for (unsigned i = 0; i < def->n_values && i < sl.count(); i++, ld++){
                QString s = sl[i];
                if(s.isEmpty())
                    continue;
                if(s.lower() == "false" || s == "0")
                    ld->bValue = false;
                else
                    ld->bValue = true;
            }
            break;
        }
        }
    }
    cfg->restorePos();
}

static char toHex(char c)
{
    c &= 0x0F;
    if (c < 10) return (char)(c + '0');
    return (char)(c - 10 + 'a');
}

static QString quoteStringInternal(const QString &str)
{
    QString quoted("\"");
    int length = str.length();
    for (int i = 0; i < length; i++){
        QChar c = str[i];
        switch (c){
        case '\\':
            quoted += "\\\\";
            break;
        case '\r':
            break;
        case '\n':
            quoted += "\\n";
            break;
        case '\"':
            quoted += "\\\"";
            break;
        default:
            if (c >= ' '){
                quoted += c;
            }else if (c){
                quoted += "\\x";
                quoted += toHex((char)(c >> 4));
                quoted += toHex(c);
            }
        }
    }
    quoted += "\"";
    return quoted;
}

EXPORT QString save_data(const DataDef *def, void *_data)
{
    Data *data = (Data*)_data;
    QString res;
    for (; def->name; def++){
        QString value;
        bool bSave = false;
        unsigned i;
        if (def->type == DATA_STRUCT){
            QString s = save_data((DataDef*)(def->def_value), data);
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
                            res += QString::number((*it).first);
                            res += ",";
                            QString s = (*it).second.c_str();
                            res += quoteStringInternal(s);
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
                            res += QString::number((*it).first);
                            res += ",";
                            QString s = QString::fromUtf8((*it).second.c_str());
                            res += quoteStringInternal(s);
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
                                value += quoteStringInternal(p);
                                bSave = true;
                            }
                        }else{
                            if ((p != NULL) && *p){
                                value += quoteStringInternal(p);
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
                            value += quoteStringInternal(s);
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

EXPORT void saveGeometry(QWidget *w, Geometry geo)
{
    if (w == NULL)
        return;
    QPoint pos = w->pos();
    QSize size = w->size();
    geo[LEFT].value   = pos.x();
    geo[TOP].value    = pos.y();
    geo[WIDTH].value  = size.width();
    geo[HEIGHT].value = size.height();
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

EXPORT void restoreGeometry(QWidget *w, Geometry geo, bool bPos, bool bSize)
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

const unsigned SAVE_STATE = (unsigned)(-1);

EXPORT void saveToolbar(QToolBar *bar, Data state[7])
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
    state[0].value = SAVE_STATE;
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

EXPORT void restoreToolbar(QToolBar *bar, Data state[7])
{
    if (bar == NULL)
        return;
    if (state[0].value != SAVE_STATE){
        if (state[1].value == 0)
            state[1].value = (unsigned)(QMainWindow::Top);
        state[2].value = 0;
        state[3].value = 0;
        state[4].value = SAVE_STATE;
        state[5].value = 0;
        state[6].value = 0;
    }
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

}
