/***************************************************************************
                          translator.cpp  -  description
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

#include "icqclient.h"
#include "log.h"

#include <errno.h>
#include <string.h>

#ifdef HAVE_ICONV

#include <iconv.h>

bool ICQClient::translate(const char *to, const char *from, string &str)
{
    string res(str.size() * 4, '\x00');
    iconv_t cnv = iconv_open(to, from);
    if (cnv == (iconv_t)(-1)){
        log(L_WARN, "Can't convert %s -> %s", from, to);
        return false;
    }
    ICONV_CONST char *in_ptr = (ICONV_CONST char*)str.c_str();
    char *out_ptr = (char*)res.c_str();
    size_t in_bytes = str.size();
    size_t out_bytes = str.size() * 4;
    size_t out_bytes_init = out_bytes;
    size_t cnv_res = iconv(cnv, &in_ptr, &in_bytes, &out_ptr, &out_bytes);
    if (cnv_res == (size_t)(-1)){
        log(L_WARN, "Can't convert %s->%s [%s] %s", from, to, str.c_str(), strerror(errno));
        iconv_close(cnv);
        return false;
    }
    str.erase();
    str.append(res.c_str(), out_bytes_init - out_bytes);
    iconv_close(cnv);
    return true;
}

#else

#include <qstring.h>
#include <qtextcodec.h>

#ifndef HAVE_STRCASECMP
int strcasecmp(const char *a, const char *b);
#endif

static QTextCodec *codecForName(const char *name)
{
    if (!strcasecmp(name, "utf-8"))
        return QTextCodec::codecForName("utf8");
    if (!strcasecmp(name, "ucs-2"))
        return QTextCodec::codecForName("utf16");
    string n = name;
    for (char *p = (char*)n.c_str(); *p; p++)
        *p = toupper(*p);
    return QTextCodec::codecForName(n.c_str());
}

bool ICQClient::translate(const char *to, const char *from, string &str)
{
    if (*str.c_str() == 0)
        return true;
    if (!strcasecmp(from, to))
        return true;
    QTextCodec *fromCodec = codecForName(from);
    QTextCodec *toCodec = codecForName(to);
    if ((fromCodec == NULL) && (toCodec == NULL)){
        if ((*from && strcmp(from, "ascii")) || (*to && strcmp(to, "ascii")))
            log(L_WARN, "Codec for %s -> %s not found", from, to);
        return true;
    }
    QString s;
    if (fromCodec == NULL){
        if (*from && strcmp(from, "ascii"))
            log(L_WARN, "Codec for %s not found", from);
        s = QString::fromLocal8Bit(str.c_str());
    }else{
        QTextDecoder *decoder = fromCodec->makeDecoder();
        s = decoder->toUnicode(str.c_str(), strlen(str.c_str()));
    }
    if (s.length() == 0){
        s = "";
        return true;
    }
    if (toCodec == NULL){
        if (*to && strcmp(to, "ascii"))
            log(L_WARN, "Codec for %s not found", to);
        str = s.local8Bit();
    }else{
        QTextEncoder *encoder = toCodec->makeEncoder();
        int size = s.length();
        str = encoder->fromUnicode(s, size);
    }
    return true;
}

#endif

void ICQClient::fromUTF(string &str)
{
    translate(localCharset(), "UTF-8", str);
}

void ICQClient::toUTF(string &str)
{
    translate("UTF-8", localCharset(), str);
}

void ICQClient::fromServer(string &str)
{
    translate(localCharset(), serverCharset(), str);
}

void ICQClient::toServer(string &str)
{
    translate(serverCharset(), localCharset(), str);
}

const char *ICQClient::localCharset()
{
    char *p = getenv("LANG");
    if (p) {
        p = strchr(p, '.');
        if (p) p++;
    }
    if (p) return p;
    return "ascii";
}

const char *ICQClient::serverCharset()
{
    const char *p = localCharset();
    if (!strcmp(p, "KOI8-R")) return "CP1251";
    return p;
}


