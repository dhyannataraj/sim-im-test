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
#include <qstringlist.h>

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
    return QTextCodec::codecForLocale()->name();
}

const char *ICQClient::serverCharset()
{
    const char *p = localCharset();
    if (!strcasecmp(p, "KOI8-R")) return "CP1251";
    if (!strcasecmp(p, "KOI8-U")) return "CP1251";
    if (!strcasecmp(p, "ISO8859-5")) return "CP1251";
    return p;
}

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

bool ICQClient::match(const char *sStr, const char *sPat)
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

