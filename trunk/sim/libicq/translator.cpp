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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef NOUSE_QT
#include <qtextcodec.h>
#include <qstringlist.h>
#endif

#include "icqclient.h"
#include "icqprivate.h"
#include "log.h"

#include <errno.h>
#include <string.h>

#ifndef HAVE_STRCASECMP
int strcasecmp(const char *a, const char *b);
#endif

#ifndef NOUSE_QT

static QTextCodec *codecForName(const char *name)
{
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
    QString s;
    if (strcasecmp(from, "utf-8") == 0){
        s = QString::fromUtf8(str.c_str());
    }else{
        QTextCodec *fromCodec = (*from) ? codecForName(from) : QTextCodec::codecForLocale();
        if (fromCodec == NULL){
            if (strcasecmp(from, "ascii"))
                log(L_WARN, "Codec for %s not found", from);
            s = QString::fromLocal8Bit(str.c_str());
        }else{
            QTextDecoder *decoder = fromCodec->makeDecoder();
            s = decoder->toUnicode(str.c_str(), strlen(str.c_str()));
        }
    }
    if (s.length() == 0){
        s = "";
        return true;
    }
    QCString r;
    if (strcasecmp(to, "utf-8") == 0){
        r = s.utf8();
    }else{
        QTextCodec *toCodec = (*to) ? codecForName(to) : QTextCodec::codecForLocale();
        if (toCodec == NULL){
            if (*to && strcmp(to, "ascii"))
                log(L_WARN, "Codec for %s not found", to);
            r = s.local8Bit();
        }else{
            QTextEncoder *encoder = toCodec->makeEncoder();
            int size = s.length();
            r = encoder->fromUnicode(s, size);
        }
    }
    if (r.length()){
        str = (const char*)r;
    }else{
        str = "";
    }
    return true;
}

const char *ICQClient::localCharset()
{
    return QTextCodec::codecForLocale()->name();
}

#else

bool ICQClient::translate(const char*, const char*, string&)
{
    return true;
}

const char *ICQClient::localCharset()
{
    return "system";
}

#endif

char const *ICQClient::localCharset(ICQUser *u)
{
    if (u && *u->Encoding.c_str())
        return u->Encoding.c_str();
    if (*owner->Encoding.c_str())
        return owner->Encoding.c_str();
    return localCharset();
}

bool ICQClient::fromUTF(string &str, const char *encoding)
{
    return translate(encoding, "UTF-8", str);
}

bool ICQClient::toUTF(string &str, const char *encoding)
{
    return translate("UTF-8", encoding, str);
}

void ICQClient::fromServer(string &str, ICQUser *u)
{
    fromServer(str, localCharset(u));
}

void ICQClient::fromServer(string &str, const char *lclCharset)
{
    if (lclCharset == NULL) lclCharset = localCharset();
    translate(lclCharset, serverCharset(lclCharset), str);
}

void ICQClient::toServer(string &str, ICQUser *u)
{
    toServer(str, localCharset(u));
}

void ICQClient::toServer(string &str, const char *lclCharset)
{
    if (lclCharset == NULL) lclCharset = localCharset();
    translate(serverCharset(lclCharset), lclCharset, str);
}

const char *ICQClient::serverCharset(const char *p)
{
    if (p == NULL) p = localCharset();
    if (!strcasecmp(p, "KOI8-R")) return "CP1251";
    if (!strcasecmp(p, "KOI8-U")) return "CP1251";
    if (!strcasecmp(p, "ISO 8859-5")) return "CP1251";
    return p;
}

bool ICQClientPrivate::utf16to8(unsigned short c, string &s)
{
    if (c <= 0x7F)
    {
        /* Leave ASCII encoded */
        s += (char)c;
    }
    else if (c <= 0x07FF)
    {
        /* 110xxxxx 10xxxxxx */
        s += (char)(0xC0 | (c >> 6));
        s += (char)(0x80 | (c & 0x3F));
    }
    else if (c <= 0xFFFF)
    {
        /* 1110xxxx + 2 */
        s += (char)(0xE0 | (c >> 12));
        s += (char)(0x80 | ((c >> 6) & 0x3F));
        s += (char)(0x80 | (c & 0x3F));
    }
    else if (c <= 0x1FFFFF)
    {
        /* 11110xxx + 3 */
        s += (char)(0xF0 | (c >> 18));
        s += (char)(0x80 | ((c >> 12) & 0x3F));
        s += (char)(0x80 | ((c >> 6) & 0x3F));
        s += (char)(0x80 | (c & 0x3F));
    }
    else if (c <= 0x3FFFFFF)
    {
        /* 111110xx + 4 */
        s += (char)(0xF8 | (c >> 24));
        s += (char)(0x80 | ((c >> 18) & 0x3F));
        s += (char)(0x80 | ((c >> 12) & 0x3F));
        s += (char)(0x80 | ((c >> 6) & 0x3F));
        s += (char)(0x80 | (c & 0x3F));
    }
    else if (c <= 0x7FFFFFFF)
    {
        /* 1111110x + 5 */
        s += (char)(0xFC | (c >> 30));
        s += (char)(0x80 | ((c >> 24) & 0x3F));
        s += (char)(0x80 | ((c >> 18) & 0x3F));
        s += (char)(0x80 | ((c >> 12) & 0x3F));
        s += (char)(0x80 | ((c >> 6) & 0x3F));
        s += (char)(0x80 | (c & 0x3F));
    }
    else
    {
        return false;
    }
	unsigned char *p = (unsigned char*)(s.c_str());
    return true;
}

#ifndef NOUSE_QT

class WordIterator
{
public:
    WordIterator(const QString &s);
    QString operator *();
    bool operator++();
    bool getNext();
    void back();
protected:
    QString getNextWord();
    QString str;
    int pos;
    QStringList words;
    unsigned nWord;
};

WordIterator::WordIterator(const QString &s)
{
    str = s;
    pos = 0;
    nWord = 0;
    ++(*this);
}

QString WordIterator::operator *()
{
    if (nWord) return words[nWord-1];
    return str;
}

void WordIterator::back()
{
    nWord = 0;
}

bool WordIterator::operator ++()
{
    if (words.count()){
        QStringList::Iterator it = words.begin();
        str = *it;
        words.remove(it);
        nWord = 0;
        return true;
    }
    str = getNextWord();
    return !str.isEmpty();
}

QString WordIterator::getNextWord()
{
    QString res;
    for (; pos < (int)str.length(); pos++){
        QChar c = str[pos];
        if (c.isLetterOrNumber()) break;
    }
    for (; pos < (int)str.length(); pos++){
        if (!str[pos].isLetterOrNumber()) break;
        res += str[pos];
    }
    return res;
}

bool WordIterator::getNext()
{
    if (nWord < words.count()){
        nWord++;
        return true;
    }
    QString w = getNextWord();
    if (w.isEmpty()) return false;
    words.append(w);
    nWord++;
    return true;
}

static bool match(const QString &str, const QString &pat, int strPos, int patPos)
{
    for (; (strPos < (int)str.length()) && (patPos < (int)pat.length()); strPos++, patPos++){
        if (pat[patPos] == '?') continue;
        if (pat[patPos] == '*'){
            for (patPos++; (patPos < (int)pat.length()) && (pat[patPos] == '*'); patPos++);
            for (int sp = strPos; sp < (int)str.length(); sp++){
                if (match(str, pat, sp, patPos)) return true;
            }
            return false;
        }
        if (str[strPos].lower() != pat[patPos].lower()) return false;
    }
    for (; (patPos < (int)pat.length()) && (pat[patPos] == '*'); patPos++);
    return (strPos == (int)str.length()) && (patPos == (int)pat.length());
}

typedef QValueList<QStringList> PatList;

static void splitPat(PatList &pats, const QString &pat)
{
    bool inQuote = false;
    QStringList l;
    for (int n = 0; n < (int)pat.length(); n++){
        QChar c = pat[n];
        if (c == '\"'){
            inQuote = !inQuote;
            if (l.count()){
                pats.append(l);
                l.clear();
            }
            continue;
        }
        if ((c != '?') && (c != '*') && !c.isLetterOrNumber())
            continue;
        QString s;
        for (; n < (int)pat.length(); n++){
            QChar c = pat[n];
            if ((c != '?') && (c != '*') && !c.isLetterOrNumber()) break;
            s += c;
        }
        if (!s.isEmpty()){
            if (!inQuote && l.count()){
                pats.append(l);
                l.clear();
            }
            l.append(s);
        }
        n--;
    }
    if (l.count())
        pats.append(l);
}

bool ICQClientPrivate::match(const char *sStr, const char *sPat)
{
    QString str = QString::fromLocal8Bit(sStr);
    QString pat = QString::fromLocal8Bit(sPat);
    PatList pats;
    splitPat(pats, pat);
    for (WordIterator it(str); !(*it).isEmpty(); ++it){
        for (PatList::Iterator itPat = pats.begin(); itPat != pats.end(); ++itPat){
            QStringList::Iterator itStr;
            for (itStr = (*itPat).begin(); itStr != (*itPat).end(); ++itStr, it.getNext())
                if (!::match(*it, *itStr, 0, 0)) break;
            if (itStr == (*itPat).end()) return true;
            it.back();
        }
    }
    return false;
}

void ICQClient::setRejectFilter(const char *fltr)
{
    RejectFilter = "";
    if (fltr == NULL){
        return;
    }
    PatList pats;
    splitPat(pats, QString::fromLocal8Bit(fltr));
    bool bFirstPat = true;
    for (PatList::Iterator itPat = pats.begin(); itPat != pats.end(); ++itPat){
        if (!bFirstPat){
            RejectFilter += ' ';
        }else{
            bFirstPat = false;
        }
        if ((*itPat).count() > 1) RejectFilter += '\"';
        bool bFirstWord = true;
        for (QStringList::Iterator itStr = (*itPat).begin(); itStr != (*itPat).end(); ++itStr){
            if (!bFirstWord){
                RejectFilter += ' ';
            }else{
                bFirstWord = false;
            }
            RejectFilter += (*itStr).local8Bit();
        }
        if ((*itPat).count() > 1) RejectFilter += '\"';
    }
}

#else

bool ICQClient::match(const char*, const char*)
{
    return false;
}

#endif
