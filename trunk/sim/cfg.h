/***************************************************************************
                          cfg.h  -  description
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

#include <iostream>
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

const unsigned PARAM_ULONG	 = 0;
const unsigned PARAM_USHORT	 = 1;
const unsigned PARAM_SHORT	 = 2;
const unsigned PARAM_STRING	 = 3;
const unsigned PARAM_BOOL	 = 4;
const unsigned PARAM_I18N	 = 5;
const unsigned PARAM_ULONGS	 = 6;
const unsigned PARAM_CHAR	 = 7;

void init(void *obj, const cfgParam *params);
void save(void *obj, const cfgParam *params, ostream &s);
bool load(void *obj, const cfgParam *params, istream &s, string &nextPart);
string quoteString(const string &value);

#endif

