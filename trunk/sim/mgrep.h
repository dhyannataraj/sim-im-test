/***************************************************************************
                          mgrep.h  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _MGREP_H

#define MAXPAT		256
#define MAXLINE		1024
#define MAXSYM		256
#define MAXMEMBER1	4096
#define MAXPATFILE	260000
#define BLOCKSIZE	8192
#define MAXHASH		8192
#define mm			8191
#define max_num		30000
#define W_DELIM		128
#define L_DELIM		10

class QString;
class QTextCodec;

class Grep
{
public:
    Grep(const QString &pat, QTextCodec *codec);
    bool grep(const char *text);
protected:
    bool add_pattern(const char *buf);
    int  prepare_engine();

    void f_prep(int pat_index, unsigned char *Pattern);
    bool m_short(unsigned char *text,int start,int end );
    bool monkey1(unsigned char *text,int start,int end );

    bool bLong;
    bool bShort;

    int p_size;

    unsigned char SHIFT1[MAXMEMBER1];
    unsigned char tr[MAXSYM];
    unsigned char tr1[MAXSYM];

    struct pat_list
    {
        int  index;
        struct pat_list *next;
    }
    *HASH[MAXHASH];

    struct patinfo
    {
        unsigned char *pText;
        unsigned int len;
    } patt[max_num];

    struct pat_list  *pt, *qt;
    unsigned char pat_spool[MAXPATFILE+2*max_num+MAXPAT];

    int num_pat;
    unsigned char *pat_ptr;
};

#endif

