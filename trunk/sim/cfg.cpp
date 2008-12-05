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

#include "sockfactory.h"
#include "socket.h"
#include "log.h"
#include "misc.h"   // sprintf

#include <stdio.h>
#include <errno.h>
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

#include <qfile.h>
#include <qtoolbar.h>
#include <qmainwindow.h>
#include <qstringlist.h>
#include <qapplication.h>
#include <qdir.h>
#include <qstyle.h>
#ifdef _DEBUG
# include <qmessagebox.h>
#endif

#ifdef USE_KDE
#include <kglobal.h>
#include <kstddirs.h>
#include <kwin.h>
#include "kdeisversion.h"
#endif

namespace SIM
{

void save_state()
{
    EventSaveState e;
    e.process();
}

// ______________________________________________________________________________________

#ifdef WIN32

EXPORT bool makedir(const QString &p)
{
    QDir path;
    if(p.endsWith("/") || p.endsWith("\\")) {
        QFileInfo fi(p + "dummy.txt");
        path = fi.dir(true);
    } else {
        QFileInfo fi(p);
        path = fi.dir(true);
    }

    if(path.exists())
        return true;
    QString r = QDir::convertSeparators(path.absPath());

    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;
    ZeroMemory(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    if(QApplication::winVersion()&Qt::WV_NT_based){
        InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
        sa.lpSecurityDescriptor = &sd;
    }
    CreateDirectoryW((LPCWSTR)p.ucs2(), &sa);
    DWORD dwAttr = GetFileAttributesW((LPCWSTR)p.ucs2());
    if (dwAttr & FILE_ATTRIBUTE_READONLY)
        SetFileAttributesW((LPCWSTR)p.ucs2(), dwAttr & ~FILE_ATTRIBUTE_READONLY);
    return true;
}

#else

EXPORT bool makedir(const QString &p)
{
    QDir path;
	if(p.endsWith("/") || p.endsWith("\\")) {
        QFileInfo fi(p + "dummy.txt");
        path = fi.dir(true);
    } else {
        QFileInfo fi(p);
        path = fi.dir(true);
    }

    if(path.exists())
        return true;
    QString r = QDir::convertSeparators(path.absPath());

    struct stat st;
    if (stat(QFile::encodeName(r).data(), &st) != 0){
#ifdef __OS2__    
        int idx = r.findRev('\\');
#else    
        int idx = r.findRev('/');
#endif
        if(idx == -1)
            return false;
        if (makedir(r.left(idx))){
            if (mkdir(QFile::encodeName(r).data(), 0700)){
                log(L_ERROR, "Can't create %s: %s", QFile::encodeName(r).data(), strerror(errno));
                return false;
            }
        }
        return false;
    }
    if ((st.st_mode & S_IFMT) != S_IFDIR){
        log(L_ERROR, "%s no directory", p.local8Bit().data());
        return false;
    }
    return true;
}

#endif

EXPORT QString app_file(const QString &f)
{
    QString app_file_name;
    QString fname = f;
#if defined( WIN32 ) || defined( __OS2__ )
    if ((fname[1] == ':') || (fname.left(2) == "\\\\"))
        return f;
#ifdef __OS2__
    CHAR buff[MAX_PATH];
    PPIB pib;
    PTIB tib;
    DosGetInfoBlocks(&tib, &pib);
    DosQueryModuleName(pib->pib_hmte, sizeof(buff), buff);
#else	
    WCHAR buff[MAX_PATH];
    GetModuleFileNameW(NULL, buff, MAX_PATH);
#endif    
#ifdef __OS2__
    QString b = buff;
#else    
    QString b = QString::fromUcs2((unsigned short*)buff);
#endif    
    int idx = b.findRev('\\');
    if(idx != -1)
        b = b.left(idx+1);
    app_file_name = b;
    if (app_file_name.length() && (app_file_name.right(1) != "\\") && (app_file_name.right(1) != "/"))
        app_file_name += '\\';
#else
    if (fname[0] == '/')
        return f;
#ifdef USE_KDE
    if (qApp){
        QStringList lst = KGlobal::dirs()->findDirs("data", "sim");
        for (QStringList::Iterator it = lst.begin(); it != lst.end(); ++it){
            QFile fi(*it + f);
            if (fi.exists()){
                app_file_name = QDir::convertSeparators(fi.name());
                return app_file_name;
            }
        }
    }
#endif
#ifndef __OS2__
    app_file_name = PREFIX "/share/apps/sim/";
#endif
#endif
    app_file_name += f;
    return QDir::convertSeparators(app_file_name);
}

// ______________________________________________________________________________________

EXPORT QString user_file(const QString &f)
{
    QString res = f;
    EventHomeDir e(f);
    if (e.process())
        return e.homeDir();
    return app_file(f);
}

// ______________________________________________________________________________________

char fromHex(char c)
{
    if ((c >= '0') && (c <= '9')) return (char)(c - '0');
    if ((c >= 'A') && (c <= 'F')) return (char)(c + 10 - 'A');
    if ((c >= 'a') && (c <= 'f')) return (char)(c + 10 - 'a');
    return (char)0;
}

static unsigned char toHex(unsigned char c)
{
    c &= 0x0F;
    if (c < 10)
        return (unsigned char)(c + '0');
    return (unsigned char)(c - 10 + 'a');
}


QString quoteChars(const QString &from, const char *chars, bool bQuoteSlash)
{
    QString     res;
    QString     quote_chars;

    quote_chars = chars;
    if (bQuoteSlash) {
        quote_chars += '\\';
    }
    for (int i = 0; i < (int) (from.length ()); i++) {
        QChar c = from[i];
        if (quote_chars.contains (c)) {
            res += '\\';
        }
        res += c;
    }
    return res;
}

QString unquoteChars(const QString &from, const QString chars, bool bQuoteSlash)
{
    QString     res;
    QString     quote_chars;

    quote_chars = chars;
    if (bQuoteSlash) {
        quote_chars += '\\';
    }
    for (int i = 0; i < (int) (from.length()); i++) {
        if ( (from[i] == '\\') && (i+1 < (int) from.length()) ) {
          if (quote_chars.contains (from[i+1])) {
                i++; // If the char after the slash is part of quote_chars, then we will skip that slash
          } else
          {
            if (bQuoteSlash) {
                // There should not be slashes with characters other than quote_chars after it, when bQuoteSlash is true
                // So will warn about it
              log(L_WARN,"Single slash found while unquoting chars '%s' in string '%s'", chars.latin1(), from.latin1());
            }
          }
        }
        if ( bQuoteSlash && (from[i] == '\\') && (i+1 == (int) from.length()) ) {
          // There should not be slashe at the end of the string if bQuoteSlash is true
          log(L_WARN,"Single slash found at the end of string while unquoting chars '%s' in string '%s'", chars.latin1(), from.latin1());
        }
        res += from[i];
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
                res += '\\';
        }
        res += from[i];
    }
    if (i < (int)from.length()){
        from = from.mid(i + 1);
    }else{
        from = QString::null;
    }
    return res;
}

EXPORT QCString getToken(QCString &from, char c, bool bUnEscape)
{
    QCString res;
    int i;
    for (i = 0; i < (int)from.length(); i++){
        if (from[i] == c)
            break;
        if (from[i] == '\\'){
            i++;
            if (i >= (int)from.length())
                break;
            if (!bUnEscape)
                res += '\\';
        }
        res += from[i];
    }
    if (i < (int)from.length()){
        from = from.mid(i + 1);
    }else{
        from = QCString();
    }
    return res;
}

// _______________________________________________________________________________________

bool set_ip(Data *p, unsigned long value, const QString &host)
{
    IP *ip = p->ip();
    if (value == 0){
        if (ip == NULL)
            return false;
        delete ip;
        p->clear();
        return true;
    }
    if (ip == NULL)
        ip = new IP;
    p->setIP(ip);
    if (ip->ip() == value){
        if (host.isEmpty())
            ip->resolve();
        return false;
    }
    ip->set(value, host);
    return true;
}

unsigned long get_ip(const Data &p)
{
    const IP *ip = p.ip();
    if (ip)
        return ip->ip();
    return 0;
}

QString get_host(const Data &p)
{
    const IP *ip = p.ip();
    return ip ? ip->host() : QString::null;
}

// _______________________________________________________________________________________

EXPORT const QString &get_str(const Data &d, unsigned index)
{
    const Data::STRING_MAP &sm = d.strMap();
    Data::STRING_MAP::const_iterator it = sm.find(index);
    if(it != sm.end())
        return it.data();
    return QString::null;
}

EXPORT void set_str(Data *d, unsigned index, const QString &value)
{
    Data::STRING_MAP &sm = d->strMap();

    sm.replace(index, value);
}

// _______________________________________________________________________________________

EXPORT void free_data(const DataDef *def, void *d)
{
    Data *data = (Data*)d;
    for (; def->name; def++){
        unsigned type = def->type;
        for (unsigned i = 0; i < def->n_values; i++, data++){
            switch (type){
            case DATA_OBJECT:
                delete data->object();
                data->clear();
                break;
            case DATA_IP:
                delete data->ip();
                data->clear();
                break;
            case DATA_STRUCT:
                free_data((DataDef*)(def->def_value), data);
                i    += (def->n_values - 1);
                data += (def->n_values - 1);
                break;
            default:
                data->clear();
                break;
            }
        }
    }
}

void init_data(const DataDef *d, Data *data)
{
    for (const DataDef *def = d; def->name; def++){
        for (unsigned i = 0; i < def->n_values; i++, data++){
			data->clear();
            data->setName(def->name);
            data->setType(def->type);
            
            switch (def->type){
            case DATA_STRING:
                // when all our sources are utf-8, use QString::fromUtf8() here!
                data->str() = def->def_value ? QString(def->def_value) : QString::null;
                break;
            case DATA_CSTRING:
                // when all our sources are utf-8, use QString::fromUtf8() here!
                data->cstr() = def->def_value ? QCString(def->def_value) : "";
                break;
            case DATA_STRLIST: {
                // this breaks on non latin1 defaults!
                QStringList sl = QStringList::split(',',def->def_value);
                Data::STRING_MAP sm;
                for(unsigned i = 0; i < sl.count(); i++) {
                    sm.insert(i, sm[(int)i]);
                }
                data->strMap() = sm;
            }
            case DATA_UTF:
                if (def->def_value){
                    data->str() = i18n(def->def_value);
                }
                break;
            case DATA_ULONG:
                data->asULong() = (unsigned long)(def->def_value);
                break;
            case DATA_LONG:
                data->asLong() = (long)(def->def_value);
                break;
            case DATA_BOOL:
                data->asBool() = (def->def_value != NULL);
                break;
            case DATA_OBJECT:
                data->setObject(NULL);
                break;
            case DATA_IP:
                data->setIP(NULL);
                break;
            case DATA_BINARY:
                data->asBinary() = QByteArray();
                break;
            case DATA_STRUCT:
                init_data((DataDef*)(def->def_value), data);
                data += (def->n_values - 1);
                i += (def->n_values - 1);
                break;
            default:
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

static QCString quoteInternal(const QCString &str)
{
    QCString res("\"");
    if (!str.isEmpty()){
        for (unsigned i = 0; i < str.length(); i++){
            unsigned char p = str[(int)i];
            switch (p){
            case '\\':
                res += "\\\\";
                break;
            case '\r':
                break;
            case '\n':
                res += "\\n";
                break;
            case '\"': {
                res += "\\x";
                res += toHex((unsigned char)(p >> 4));
                res += toHex(p);
                break;
            }
            default:
                if (p >= ' '){
                    res += p;
                }else if (p){
                    res += "\\x";
                    res += toHex((unsigned char)(p >> 4));
                    res += toHex(p);
                }
            }
        }
    }
    res += '\"';
    return res;
}

static bool unquoteInternal(QCString &val, QCString &str)
{
    int idx1 = val.find('\"');
    if(idx1 == -1)
        return false;
    idx1++;
    int idx2 = val.findRev('\"');
    if(idx2 == -1)
        return false;
    str = val.mid(idx1, idx2 - idx1);
    val = val.mid(idx2 + 1);
    // now unquote
    idx1 = 0;
    while((idx1 = str.find('\\', idx1)) != -1) {
        char c = str[idx1 + 1];
        switch(c) {
            case '\\':
                str = str.left(idx1) + '\\' + str.mid(idx1 + 2);
                break;
            case 'n':
                str = str.left(idx1) + '\n' + str.mid(idx1 + 2);
                break;
            case 'x': {
                char c1 = str[idx1 + 2];
                char c2 = c1 ? str[idx1 + 3] : 0;
                if(!c1 || !c2)
                    return false;
                c = (fromHex(c1) << 4) | (fromHex(c2));
                str = str.left(idx1) + c + str.mid(idx1 + 4);
                break;
            }
            default:
                break;
        }
        idx1++;
    }
    return true;
}

EXPORT void load_data(const DataDef *d, void *_data, Buffer *cfg)
{
    Data *data = (Data*)_data;
    init_data(d, data);
    if (cfg == NULL)
        return;
    unsigned read_pos = cfg->readPos();
    for (;;){
        QCString line = cfg->getLine();
        if (line.isEmpty())
            break;
        int idx = line.find('=');
        if(idx == -1)
            continue;
        QCString name = line.left( idx );
        QCString val  = line.mid( idx + 1 );
        if(name.isEmpty() || val.isEmpty())
            continue;

        unsigned offs = 0;
        const DataDef *def = find_key(d, name, offs);
        if (def == NULL)
            continue;
        Data *ld = data + offs;
        ld->setType(def->type);
        switch (def->type){
        case DATA_IP: {
            int idx = val.find(',');
            QCString ip, url;
            if(idx == -1) {
                ip = val;
            } else {
                ip = val.left(idx);
                url = val.mid(idx + 1);
            }
            set_ip(ld, inet_addr(ip), url);
            break;
        }
        case DATA_UTFLIST:
        case DATA_STRLIST: {
            // <number>,"<text>"(u)
            QCString v;
            int idx1 = val.find(',');
            if(idx1 == -1)
                break;
            unsigned i = val.left(idx1).toUInt();
            if (i == 0)
                break;
            val = val.mid(idx1 + 1);
            if(!unquoteInternal(val, v)) {
                set_str(ld, i, QString::null);
                break;
            }
            if (!val.isEmpty() && val[0] == 'u'){
                set_str(ld, i, QString::fromUtf8(v));
            }else{
                set_str(ld, i,  QString::fromLocal8Bit(v));
            }
            break;
        }
        case DATA_UTF:
        case DATA_STRING:
        case DATA_CSTRING: {
            // "<text>"(u),"<text>"(u),"<text>"(u),"<text>"(u),...
            for (unsigned i = 0; i < def->n_values; ld++, i++){
                QCString v;
                if(!unquoteInternal(val, v))
                    break;

                if(def->type == DATA_CSTRING) {
                    ld->cstr() = v;
                } else {
                    if (!val.isEmpty() && val[0] == 'u')
                        ld->str() = QString::fromUtf8(v);
                    else
                        ld->str() = QString::fromLocal8Bit(v);
                }

                idx = val.find(',');
                if (idx == -1)
                    break;
                val = val.mid(idx + 1);
            }
            break;
        }
		case DATA_LONG:
        case DATA_ULONG: {
            QStringList sl = QStringList::split(',',val,true);
            for (unsigned i = 0; i < def->n_values && i < sl.count(); i++, ld++){
                QString s = sl[i];
                if(s.isEmpty())
                    continue;
                if(def->type == DATA_LONG)
                    ld->setLong(s.toLong());
                else
                    ld->setULong(s.toULong());
            }
            break;
        }
        case DATA_BOOL: {
            QStringList sl = QStringList::split(',',val,true);
            for (unsigned i = 0; i < def->n_values && i < sl.count(); i++, ld++){
                QString s = sl[i];
                if(s.isEmpty())
                    continue;
                ld->setBool(s.lower() != "false" && s != "0");
            }
            break;
        }
        case DATA_BINARY: {
            // ok here since they're only latin1 chars
            QStringList sl = QStringList::split(',', val, true);
            for (unsigned i = 0; i < def->n_values && i < sl.count(); i++, ld++){
                QString s = sl[i];
                if(s.isEmpty())
                    continue;
                int size = s.length() / 2;
                QByteArray ba(size);
                for(int cnt = 0; cnt < size; cnt++) {
                    ba.data()[cnt] = (char)s.mid(cnt * 2, 2).toShort(NULL, 16);
                }
                ld->setBinary(ba);
            }
            break;
        }
        case DATA_UNKNOWN:
        case DATA_STRUCT:
        case DATA_OBJECT:
	default:
	    break;
        }
    }
    cfg->setReadPos(read_pos);
}

EXPORT QCString save_data(const DataDef *def, void *_data)
{
    Data *data = (Data*)_data;
    QCString res;
    for (; def->name; def++){
        QCString value;
        bool bSave = false;
        if (def->type == DATA_STRUCT){
            QCString s = save_data((DataDef*)(def->def_value), data);
            if (s.length()){
                if (res.length())
                    res += '\n';
                res += s;
            }
        }else  if (*def->name){
            Data *ld = data;
            switch (def->type){
            case DATA_IP:{
                    IP *p = ld->ip();
                    if (p && p->ip()){
                        struct in_addr inaddr;
                        inaddr.s_addr = p->ip();
                        value = inet_ntoa(inaddr);
                        QString host = p->host();
                        if (!host.isEmpty()){
                            value += ',';
                            value += host;
                        }
                        bSave = true;
                    }
                    break;
                }
            case DATA_UTFLIST:
            case DATA_STRLIST:{
                    const Data::STRING_MAP &p = ld->strMap();
                    if (p.count()){
                        for (Data::STRING_MAP::ConstIterator it = p.begin(); it != p.end(); ++it){
                            if(it.data().isEmpty())
                                continue;
                            if (res.length())
                                res += '\n';
                            res += def->name;
                            res += '=';
                            res += QString::number(it.key());
                            res += ',';
                            QString s = it.data();
                            QCString ls = s.local8Bit();
                            if (QString::fromLocal8Bit(ls) == s){
                                res += quoteInternal(ls);
                            }else{
                                res += quoteInternal(s.utf8());
                                res += 'u';
                            }
                        }
                    }
                    break;
                }
            case DATA_STRING:{
                    for (unsigned i = 0; i < def->n_values; i++, ld++){
                        QString &str = ld->str();
                        if (value.length())
                            value += ',';
                        if (def->def_value){
                            if (str != QString::fromAscii(def->def_value)){
                                bSave = true;
                            }
                        }else{
                            if (str.length()){
                                bSave = true;
                            }
                        }
                        if (bSave){
                            QCString ls = str.local8Bit();
                            if (QString::fromLocal8Bit(ls) == str){
                                value += quoteInternal(ls);
                            }else{
                                value += quoteInternal(str.utf8());
                                value += 'u';
                            }
                        }
                    }
                    break;
                }
            case DATA_UTF:{
                    for (unsigned i = 0; i < def->n_values; i++, ld++){
                        QString &str = ld->str();
                        if (value.length())
                            value += ',';
                        if (def->def_value){
                            if (str != i18n(def->def_value))
                                bSave = true;
                        }else{
                            if (str.length())
                                bSave = true;
                        }
                        if (bSave){
                            QCString ls = str.local8Bit();
                            if (QString::fromLocal8Bit(ls) == str){
                                value += quoteInternal(ls);
                            }else{
                                value += quoteInternal(str.utf8());
                                value += 'u';
                            }
                        }
                    }
                    break;
                }
            case DATA_CSTRING:{
                    for (unsigned i = 0; i < def->n_values; i++, ld++){
                        QCString &str = ld->cstr();
                        if (value.length())
                            value += ',';
                        if (def->def_value){
                            if (str != def->def_value){
                                bSave = true;
                            }
                        }else{
                            if (str.length()){
                                bSave = true;
                            }
                        }
                        if (bSave)
                            value += quoteInternal(str);
                    }
                    break;
                }
            case DATA_BOOL:{
                    for (unsigned i = 0; i < def->n_values; i++, ld++){
                        bool p = ld->toBool();
                        if (value.length())
                            value += ',';
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
                    for (unsigned i = 0; i < def->n_values; i++, ld++){
                        long p = ld->toLong();
                        if (value.length())
                            value += ',';
                        if (p != (long)(def->def_value)){
                            QString s;
                            s.sprintf("%li", p);
                            value += s;
                            bSave = true;
                        }
                    }
                    break;
                }
            case DATA_ULONG:{
                    for (unsigned i = 0; i < def->n_values; i++, ld++){
                        unsigned long p = ld->toULong();
                        if (value.length())
                            value += ',';
                        if (p != (unsigned long)(def->def_value)){
                            QString s;
                            s.sprintf("%lu", p);
                            value += s;
                            bSave = true;
                        }
                    }
                    break;
                }
            case DATA_BINARY: {
                for (unsigned i = 0; i < def->n_values; i++) {
                    QByteArray &ba = ld->asBinary();
                    for(unsigned i = 0; i < ba.size(); i++) {
                        unsigned char c = ba.data()[i];
                        QString s;
                        s.sprintf("%02X", c);
                        value += s.latin1();    // ok here since they're only latin1 chars
                    }
                    if(!bSave)
                        bSave = (ba.size() != 0);
                }
                break;
            }
            case DATA_UNKNOWN:
            case DATA_STRUCT:
            case DATA_OBJECT:
            default:
                break;
            }
            if (bSave){
                if (res.length())
                    res += '\n';
                res += def->name;
                res += '=';
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
    geo[LEFT].asLong()   = pos.x();
    geo[TOP].asLong()    = pos.y();
    geo[WIDTH].asLong()  = size.width();
    geo[HEIGHT].asLong() = size.height();
    // if window is not visible QT return geometry without frame sizes
    // may work for X versions too
    if ( !w->isShown() ) {
        int th = w->style().pixelMetric( QStyle::PM_TitleBarHeight, w );
        int fw = w->style().pixelMetric( QStyle::PM_DefaultFrameWidth, w );
        geo[0].asLong() -= fw * 2; // + size of left frame border
        geo[1].asLong() -= th + fw; // + size of title and border
    }
#ifdef WIN32
    if (GetWindowLongA(w->winId(), GWL_EXSTYLE) & WS_EX_TOOLWINDOW){
        int dc = GetSystemMetrics(SM_CYCAPTION);
        int dd = GetSystemMetrics(SM_CYDLGFRAME);
        int ds = GetSystemMetrics(SM_CYSMCAPTION);
		geo[1].asLong() += dc - ds;
        geo[3].asLong() -= dd * 2;
    }
#endif
#ifdef USE_KDE
#if KDE_IS_VERSION(3,2,0)
    KWin::WindowInfo info = KWin::windowInfo(w->winId());
    geo[4].asLong() = info.desktop();
    if (info.onAllDesktops())
        geo[4].asLong() = -1;
#else
    KWin::Info info = KWin::info(w->winId());
    geo[4].asLong() = info.desktop;
    if (info.onAllDesktops)
        geo[4].asLong() = -1;
#endif
#endif
}

EXPORT void restoreGeometry(QWidget *w, Geometry geo, bool bPos, bool bSize)
{
    if (w == NULL)
        return;
    QRect rc = screenGeometry();
    if (geo[WIDTH].toLong() > rc.width())
        geo[WIDTH].asLong() = rc.width();
    if (geo[HEIGHT].toLong() > rc.height())
        geo[HEIGHT].asLong() = rc.height();
    if (geo[LEFT].toLong() + geo[WIDTH].toLong() > rc.right())
        geo[LEFT].asLong() = rc.right() - geo[WIDTH].asLong();
    if (geo[TOP].toLong() + geo[HEIGHT].toLong() > rc.bottom())
        geo[TOP].asLong() = rc.bottom() - geo[HEIGHT].asLong();
    if (geo[LEFT].toLong() < rc.left())
        geo[LEFT].asLong() = rc.left();
    if (geo[TOP].toLong() < rc.top())
        geo[TOP].asLong() = rc.top();
    if (bPos)
        w->move(geo[LEFT].toLong(), geo[TOP].toLong());
    if (bSize)
        w->resize(geo[WIDTH].asLong(), geo[HEIGHT].asLong());
#ifdef USE_KDE
    if (geo[4].toLong() == -1){
        KWin::setOnAllDesktops(w->winId(), true);
    }else{
        KWin::setOnAllDesktops(w->winId(), false);
        KWin::setOnDesktop(w->winId(), geo[4].toLong());
    }
#endif
}

const long SAVE_STATE = -1;

EXPORT void saveToolbar(QToolBar *bar, Data state[7])
{
    for(int i = 0; i < 7; i++)
        state[i].clear();
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
    state[0].asLong() = SAVE_STATE;
    state[1].asLong() = (long)dock;
    state[2].asLong() = index;
    state[3].asLong() = nl ? 1 : 0;
    state[4].asLong() = extraOffset;
    if (dock == QMainWindow::TornOff){
        QPoint pos = bar->geometry().topLeft();
        state[5].asLong() = pos.x();
        state[6].asLong() = pos.y();
    }
}

EXPORT void restoreToolbar(QToolBar *bar, Data state[7])
{
    if (bar == NULL)
        return;
    if (state[0].asLong() != SAVE_STATE){
        if (state[1].asLong() == 0)
            state[1].asLong() = (unsigned)(QMainWindow::Top);
        state[2].asLong() = 0;
        state[3].asLong() = 0;
        state[4].asLong() = SAVE_STATE;
        state[5].asLong() = 0;
        state[6].asLong() = 0;
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
    QMainWindow::ToolBarDock dock = (QMainWindow::ToolBarDock)state[1].asLong();
    main->moveToolBar(bar, dock, state[2].asLong() != 0, state[3].asLong() != 0, state[4].asLong());
    if (dock == QMainWindow::TornOff)
        bar->move(state[5].asLong(), state[6].asLong());
}

// ----------------------------
// class Data
// ----------------------------
class EXPORT DataPrivate {
public:
    unsigned long        m_dataAsValue;
    bool                 m_dataAsBool;
    QString             *m_dataAsQString;
    Data::STRING_MAP    *m_dataAsQStringMap;
    QObject             *m_dataAsObject;
    IP                  *m_dataAsIP;
    QByteArray          *m_dataAsBinary;
    QCString            *m_dataAsQCString;
    DataPrivate() : m_dataAsValue(0), m_dataAsBool(false), m_dataAsQString(NULL),
                    m_dataAsQStringMap(NULL), m_dataAsObject(NULL), m_dataAsIP(NULL),
                    m_dataAsBinary(NULL), m_dataAsQCString(NULL) {}

    static unsigned long myStaticDummyULong;
    static bool myStaticDummyBool;
    static QString myStaticDummyQString;
    static Data::STRING_MAP myStaticDummyQStringMap;
    static QByteArray myStaticDummyQByteArray;
    static QCString myStaticDummyQCString;
};

unsigned long DataPrivate::myStaticDummyULong = ~0U;
bool DataPrivate::myStaticDummyBool = false;
QString DataPrivate::myStaticDummyQString = QString("Wrong datatype!");
Data::STRING_MAP DataPrivate::myStaticDummyQStringMap = Data::STRING_MAP();
QByteArray DataPrivate::myStaticDummyQByteArray = QByteArray();
QCString DataPrivate::myStaticDummyQCString = QCString("Wrong datatype!");

Data::Data()
 : m_type(DATA_UNKNOWN), m_name("unknown"), data(NULL)
{
    clear();
}

Data::Data(const Data &d)
 : data(NULL)
{
    *this = d;
}

Data &Data::operator =(const Data &d)
{
    clear();
    m_name = d.m_name;
    m_type = d.m_type;
    switch(m_type) {
        case DATA_STRING:
            this->str() = d.str();
            break;
        case DATA_LONG:
        case DATA_ULONG:
            data->m_dataAsValue = d.data->m_dataAsValue;
            break;
        case DATA_BOOL:
            data->m_dataAsBool = d.data->m_dataAsBool;
            break;
        case DATA_STRMAP:
            this->strMap() = d.strMap();
            break;
        case DATA_IP:
            data->m_dataAsIP = d.data->m_dataAsIP;
            break;
        case DATA_OBJECT:
            data->m_dataAsObject = d.data->m_dataAsObject;
            break;
        case DATA_BINARY:
            this->asBinary() = d.toBinary();
            break;
        case DATA_CSTRING:
            this->cstr() = d.cstr();
            break;
        case DATA_UNKNOWN:
        case DATA_STRUCT:
        default:
            break;
    }
    return *this;
}

void Data::clear(bool bNew)
{
    if(data) {
        delete data->m_dataAsQString;
        delete data->m_dataAsQStringMap;
        delete data->m_dataAsBinary;
        delete data->m_dataAsQCString;
        delete data;
    }
    data = bNew ? new DataPrivate : NULL;
}

const QString &Data::str() const
{
    if(!checkType(DATA_STRING))
        return DataPrivate::myStaticDummyQString;
    if(!data->m_dataAsQString)
        data->m_dataAsQString = new QString();
    return *data->m_dataAsQString;
}

QString &Data::str()
{
    if(!checkType(DATA_STRING))
        return DataPrivate::myStaticDummyQString;
    if(!data->m_dataAsQString)
        data->m_dataAsQString = new QString();
    return *data->m_dataAsQString;
}

bool Data::setStr(const QString &s)
{
    if(!checkType(DATA_STRING))
        return false;
    if(data->m_dataAsQString && s == *data->m_dataAsQString)
        return false;
    if(!data->m_dataAsQString)
        data->m_dataAsQString = new QString(s);
    else
        *data->m_dataAsQString = s;
    return true;
}

const Data::STRING_MAP &Data::strMap() const
{
    if(!checkType(DATA_STRMAP))
        return DataPrivate::myStaticDummyQStringMap;
    if(!data->m_dataAsQStringMap)
        data->m_dataAsQStringMap = new STRING_MAP();
    return *data->m_dataAsQStringMap;
}

Data::STRING_MAP &Data::strMap()
{
    if(!checkType(DATA_STRMAP))
        return DataPrivate::myStaticDummyQStringMap;
    if(!data->m_dataAsQStringMap)
        data->m_dataAsQStringMap = new STRING_MAP();
    return *data->m_dataAsQStringMap;
}

bool Data::setStrMap(const STRING_MAP &s)
{
    if(!checkType(DATA_STRMAP))
        return false;
    if(!data->m_dataAsQStringMap)
        data->m_dataAsQStringMap = new STRING_MAP(s);
    else
        *data->m_dataAsQStringMap = s;
    return true;
}

long Data::toLong() const
{
    if(!checkType(DATA_LONG))
        return (long)DataPrivate::myStaticDummyULong;
    return (long)data->m_dataAsValue;
}

long &Data::asLong()
{
    if(!checkType(DATA_LONG))
        return (long&)DataPrivate::myStaticDummyULong;
    return (long&)data->m_dataAsValue;
}

bool Data::setLong(long d)
{
    if(!checkType(DATA_LONG))
        return false;
    if(d == (long)data->m_dataAsValue)
        return false;
    data->m_dataAsValue = (unsigned long)d;
    return true;
}

unsigned long Data::toULong() const
{
    if(!checkType(DATA_ULONG))
        return DataPrivate::myStaticDummyULong;
    return data->m_dataAsValue;
}

unsigned long &Data::asULong()
{
    if(!checkType(DATA_ULONG))
        return DataPrivate::myStaticDummyULong;
    return data->m_dataAsValue;
}

bool Data::setULong(unsigned long d)
{
    if(!checkType(DATA_ULONG))
        return false;
    if(d == data->m_dataAsValue)
        return false;
    data->m_dataAsValue = d;
    return true;
}

bool Data::toBool() const
{
    if(!checkType(DATA_BOOL))
        return DataPrivate::myStaticDummyBool;
    return data->m_dataAsBool;
}

bool &Data::asBool()
{
    if(!checkType(DATA_BOOL))
        return DataPrivate::myStaticDummyBool;
    return data->m_dataAsBool;
}

bool Data::setBool(bool d)
{
    if(!checkType(DATA_BOOL))
        return false;
    if(d == data->m_dataAsBool)
        return false;
    data->m_dataAsBool = d;
    return true;
}

const QObject* Data::object() const
{
    if(!checkType(DATA_OBJECT))
        return NULL;
    return data->m_dataAsObject;
}

QObject* Data::object()
{
    if(!checkType(DATA_OBJECT))
        return NULL;
    return data->m_dataAsObject;
}

bool Data::setObject(const QObject *d)
{
    if(!checkType(DATA_OBJECT))
        return false;
    if(d == data->m_dataAsObject)
        return false;
    data->m_dataAsObject = const_cast<QObject*>(d);
    return true;
}

const QByteArray &Data::toBinary() const
{
    if(!checkType(DATA_BINARY))
        return DataPrivate::myStaticDummyQByteArray;
    if(!data->m_dataAsBinary)
        data->m_dataAsBinary = new QByteArray();
    return *data->m_dataAsBinary;
}

QByteArray &Data::asBinary()
{
    if(!checkType(DATA_BINARY))
        return DataPrivate::myStaticDummyQByteArray;
    if(!data->m_dataAsBinary)
        data->m_dataAsBinary = new QByteArray();
    return *data->m_dataAsBinary;
}

bool Data::setBinary(const QByteArray &d)
{
    if(!checkType(DATA_BINARY))
        return false;
    if(data->m_dataAsBinary && d == *data->m_dataAsBinary)
        return false;
    if(!data->m_dataAsBinary)
        data->m_dataAsBinary = new QByteArray(d);
    else
        *data->m_dataAsBinary = d;
    return true;
}

const IP* Data::ip() const
{
    if(!checkType(DATA_IP))
        return NULL;
    return data->m_dataAsIP;
}

IP* Data::ip()
{
    if(!checkType(DATA_IP))
        return NULL;
    return data->m_dataAsIP;
}

bool Data::setIP(const IP *d)
{
    if(!checkType(DATA_IP))
        return false;
    if(d == data->m_dataAsIP)
        return false;
    data->m_dataAsIP = const_cast<IP*>(d);
    return true;
}

const QCString &Data::cstr() const
{
    if(!checkType(DATA_CSTRING))
        return DataPrivate::myStaticDummyQCString;
    if(!data->m_dataAsQCString)
        data->m_dataAsQCString = new QCString();
    return *data->m_dataAsQCString;
}

QCString &Data::cstr()
{
    if(!checkType(DATA_CSTRING))
        return DataPrivate::myStaticDummyQCString;
    if(!data->m_dataAsQCString)
        data->m_dataAsQCString = new QCString();
    return *data->m_dataAsQCString;
}

bool Data::setCStr(const QCString &s)
{
    if(!checkType(DATA_CSTRING))
        return false;
    if(data->m_dataAsQCString && s == *data->m_dataAsQCString)
        return false;
    if(!data->m_dataAsQCString)
        data->m_dataAsQCString = new QCString(s);
    else
        *data->m_dataAsQCString = s;
    return true;
}

static const char *dataType2Name(DataType type)
{
    switch(type) {
        case DATA_UNKNOWN:
            return "uninitialized";
        case DATA_UTF:
        case DATA_STRING:
            return "string";
        case DATA_LONG:
            return "long";
        case DATA_ULONG:
            return "ulong";
        case DATA_BOOL:
            return "bool";
        case DATA_UTFLIST:
        case DATA_STRMAP:
            return "stringmap";
        case DATA_IP:
            return "ip";
        case DATA_STRUCT:
            return "struct";
        case DATA_OBJECT:
            return "object";
        case DATA_BINARY:
            return "binary";
        case DATA_CSTRING:
            return "cstr";
    }
    return "unknown";
}

bool Data::checkType(DataType type) const
{
    DataType myType = m_type;
    if(myType == DATA_UTFLIST)
        myType = DATA_STRLIST;
    if(myType == DATA_UTF)
        myType = DATA_STRING;
    if(myType != type) {
        QString errString = QString("Using wrong data type %1 instead %2 for %3!")
                               .arg(dataType2Name(type))
                               .arg(dataType2Name(m_type))
                               .arg(m_name.isEmpty() ? "??" : m_name);
        log(L_ERROR, errString);
#ifdef _DEBUG
        QMessageBox::information(0, "Debug error", errString, QMessageBox::Ok);
#endif
        return false;
    }
    return true;
}

void Data::setName(const QString &name)
{
    if(!name.isEmpty() && (m_name.isEmpty() || m_name == "unknown"))
        m_name = name;
}

const QString &Data::name() const
{
    return m_name;
}

void Data::setType(DataType type)
{
    m_type = type;
}

DataType Data::type() const
{
    return m_type;
}

}   // namespace SIM
