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

#include "defs.h"
#include "icqclient.h"
#include "log.h"

#include <errno.h>
#include <string.h>

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
    QTextCodec *fromCodec = (*from) ? codecForName(from) : QTextCodec::codecForLocale();
    QTextCodec *toCodec = (*to) ? codecForName(to) : QTextCodec::codecForLocale();
    if ((fromCodec == NULL) && (toCodec == NULL) &&
            strcasecmp(from, "UCS-2") &&
            strcasecmp(from, "UTF-8") && strcasecmp(to, "UTF-8")){
        if ((*from && strcmp(from, "ascii")) || (*to && strcmp(to, "ascii")))
            log(L_WARN, "Codec for %s -> %s not found", from, to);
        return true;
    }
    QString s;
    if (fromCodec == NULL){
        if (strcasecmp(from, "utf-8") == 0){
            s = QString::fromUtf8(str.c_str());
        }else if (strcasecmp(from, "ucs-2") == 0){
            for (unsigned i = 0; i < str.length() / 2; i += 2)
                s += QChar(*((uint*)(str.c_str() + i)));
        }else{
            if (*from && strcmp(from, "ascii"))
                log(L_WARN, "Codec for %s not found", from);
            s = QString::fromLocal8Bit(str.c_str());
        }
    }else{
        QTextDecoder *decoder = fromCodec->makeDecoder();
        s = decoder->toUnicode(str.c_str(), strlen(str.c_str()));
    }
    if (s.length() == 0){
        s = "";
        return true;
    }
    if (toCodec == NULL){
        if (strcasecmp(to, "utf-8")){
            if (*to && strcmp(to, "ascii"))
                log(L_WARN, "Codec for %s not found", to);
            str = s.local8Bit();
        }else{
            str = s.utf8();
        }
    }else{
        QTextEncoder *encoder = toCodec->makeEncoder();
        int size = s.length();
        str = encoder->fromUnicode(s, size);
    }
    return true;
}

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
    const char *res = QTextCodec::codecForLocale()->name();
    return res;
}

const char *ICQClient::serverCharset()
{
    const char *p = localCharset();
    if (!strcasecmp(p, "KOI8-R")) return "CP1251";
    if (!strcasecmp(p, "KOI8-U")) return "CP1251";
    if (!strcasecmp(p, "ISO8859-5")) return "CP1251";
    return p;
}


