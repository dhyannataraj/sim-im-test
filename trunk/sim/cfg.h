/***************************************************************************
                          cfg.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#ifndef _CFG_H
#define _CFG_H

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

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4530)
#endif
#include <winsock.h>
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif

#include <list>
#include <string>

using namespace std;
#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4786)
#endif
#endif

typedef struct cfgParam
{
    const char	*name;
    unsigned	offs;
    unsigned	type;
    unsigned	defValue;
} cfgParam;

const unsigned PARAM_ULONG	 = 1;
const unsigned PARAM_USHORT	 = 2;
const unsigned PARAM_SHORT	 = 3;
const unsigned PARAM_CHARS	 = 4;
const unsigned PARAM_BOOL	 = 5;
const unsigned PARAM_I18N	 = 6;
const unsigned PARAM_ULONGS	 = 7;
const unsigned PARAM_CHAR	 = 8;
const unsigned PARAM_OFFS	 = 9;
const unsigned PARAM_PROC	 = 10;
const unsigned PARAM_STRING	 = 11;

class QFile;

void init(void *obj, const cfgParam *params);
void free(void *obj, const cfgParam *params);
void save(void *obj, const cfgParam *params, QFile &f);
bool load(void *obj, const cfgParam *params, QFile &f, string &nextPart);
string quoteString(const string &value);

void writeStr(QFile &f, const char *str);
bool getLine(QFile &f, string &s);
extern unsigned long line_start;
extern const char *empty_str;
void set_str(char **a, const char *r);

#define PROP_BOOL(A)								\
	bool get##A() { return data.A; }					\
	void set##A(bool r) { data.A = r; }

#define PROP_ULONG(A)                                                           \
	unsigned long get##A() { return data.A; }                               \
	void set##A(unsigned long r) { data.A = r; }

#define PROP_STR(A)                                                             \
	const char *get##A() { return data.A ? data.A : empty_str; }            \
	void set##A(const char *r) { ::set_str(&data.A, r); }			\
	char **_##A() { return &data.A; }

#define OFFSET_OF(type, field)                                               \
            (reinterpret_cast <size_t>                                       \
                (reinterpret_cast <char *>                                   \
                    (&(reinterpret_cast <type *> (0))->field)))

#endif

