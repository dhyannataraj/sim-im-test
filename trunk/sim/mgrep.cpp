/***************************************************************************
                          client.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru

Original mgrep.c 
Copyright (c) 1991 Sun Wu and Udi Manber.  All Rights Reserved.

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
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#else
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <qstring.h>
#include <qtextcodec.h>

#include "cfg.h"
#include "mgrep.h"

// _____________________________________________________________________________________________________________________

Grep::Grep(const QString &pat, QTextCodec *codec)
{
    bLong = false;
    bShort = false;

    p_size = 0;
    num_pat = 0;
    pat_ptr = pat_spool;

    int i;
    unsigned Mask = 15;

    pat_ptr = pat_spool;
    num_pat = 0;

    for (i=0; i< MAXSYM; i++)
        tr[i] = i;

    for (i=0; i< MAXSYM; i++)
        tr1[i] = tr[i]&Mask;

    p_size = 0;

    bShort = bLong = 0;

    int lenOut = pat.length();
    if (lenOut){
        string uStr;
        uStr = pat.utf8();
        string lStr;
        lStr = codec->makeEncoder()->fromUnicode(pat, lenOut);
        add_pattern(quoteString(uStr).c_str());
        if (strcmp(uStr.c_str(), lStr.c_str()))
            add_pattern(quoteString(lStr).c_str());
    }
    prepare_engine();
}

bool Grep::add_pattern(const char *buf)
{
    unsigned char *pStart = (unsigned char*)buf;
    unsigned char *pEnd = pStart + strlen((char*)pStart);

    if (pEnd <= pStart)
        return false;

    if (num_pat >= max_num)
        return false;

    patt[num_pat].pText = pat_ptr;
    patt[num_pat].len = pEnd - pStart;
    num_pat++;

    /* Copy pattern to the buffer */
    while (pStart < pEnd)
        *pat_ptr++ = *pStart++;
    *pat_ptr++ = '\0';
    return true;
}

/* Prepare engine for search */
int Grep::prepare_engine()
{
    int i;
    unsigned minlen, maxlen;

    if (num_pat < 1)
        return 0;

    minlen = maxlen = patt[0].len;

    for(i=1; i < num_pat; i++)
    {
        if (patt[i].len < minlen)
            minlen = patt[i].len;
        else if (patt[i].len > maxlen)
            maxlen = patt[i].len;
    }

    if (minlen == 0)
        return 0;

    if(maxlen > 400 && minlen > 2)
        bLong = true;
    else if(minlen == 1)
        bShort = true;

    p_size = minlen;
    for(i=0; i<MAXMEMBER1; i++)
        SHIFT1[i] = p_size - 2;

    for(i=0; i<MAXHASH; i++)
        HASH[i] = 0;

    for(i=0; i < num_pat; i++)
        f_prep(i, patt[i].pText);

    return 1;
}


// _____________________________________________________________________________________________________________________

bool Grep::grep(const char *text)
{
    if(bShort)
        return m_short((unsigned char*)text, 0, strlen(text));
    return monkey1((unsigned char*)text, 0, strlen(text));
}

bool Grep::monkey1(unsigned char *text, int start, int end )
{
    register unsigned char *textend;
    register unsigned hash;
    register unsigned char shift;
    register int  m1, i, j;
    bool Long = bLong;
    int pat_index, m=p_size;
    register unsigned char *qx;
    register struct pat_list *p;
    unsigned char *lastout;

    textend = text + end;
    m1 = m - 1;
    lastout = text+start+1;
    text = text + start + m1 ;
    while (text <= textend)
    {
        hash=tr1[*text];
        hash=(hash<<4)+(tr1[*(text-1)]);
        if(Long) hash=(hash<<4)+(tr1[*(text-2)]);
        shift = SHIFT1[hash];
        if(shift == 0)
        {
            hash=0;
            for(i=0;i<=m1;i++)
            {
                hash=(hash<<4)+(tr1[*(text-i)]);
            }
            hash=hash&mm;
            p = HASH[hash];
            while(p != 0)
            {
                pat_index = p->index;
                p = p->next;
                qx = text-m1;
                j = 0;
                while(tr[ patt[pat_index].pText[j] ] == tr[ *(qx++) ]) j++;
                if (j > m1 )
                {
                    if(patt[pat_index].len <= (unsigned)j)
                    {
                        if(text > textend) return false;
                        return true;
                    }
                }
            }
            shift = 1;
        }
        text = text + shift;
    }
    return false;
}

bool Grep::m_short(unsigned char *text, int start, int end )
{
    register unsigned char *textend;
    register int  j;
    register struct pat_list *p;
    register int pat_index;
    unsigned char *lastout;
    unsigned char *qx;

    textend = text + end;
    lastout = text + start + 1;
    text = text + start - 1 ;
    while (++text <= textend)
    {
        p = HASH[*text];
        while(p != 0)
        {
            pat_index = p->index;
            p = p->next;
            qx = text;
            j = 0;
            while(tr[ patt[pat_index].pText[j] ] == tr[ *(qx++) ]) j++;
            if(patt[pat_index].len <= (unsigned)j)
            {
                if(text >= textend) return false;
                return true;
            }
        }
    }
    return false;
}

void Grep::f_prep(int pat_index, unsigned char *Pattern)
{
    int i, m;
    register unsigned hash, Mask=15;

    m = p_size;
    for (i = m-1; i>= (bLong ? 2 : 1); i--)
    {
        hash = (Pattern[i] & Mask);
        hash = (hash << 4) + (Pattern[i-1]& Mask);
        if(bLong) hash = (hash << 4) + (Pattern[i-2] & Mask);
        if(SHIFT1[hash] >= m-1-i) SHIFT1[hash] = m-1-i;
    }
    if(bShort) Mask = 255;  /* 011111111 */
    hash = 0;
    for(i = m-1; i>=0; i--)
    {
        hash = (hash << 4) + (tr[Pattern[i]]&Mask);
    }
    hash=hash&mm;
    qt = (struct pat_list *) malloc(sizeof(struct pat_list));
    qt->index = pat_index;
    pt = HASH[hash];
    qt->next = pt;
    HASH[hash] = qt;
}


