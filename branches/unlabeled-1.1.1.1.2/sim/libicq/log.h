/***************************************************************************
                          log.h  -  description
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

#ifndef _LOG_H
#define _LOG_H

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

extern unsigned short log_level;

const unsigned short L_ERROR 	= 0x01;
const unsigned short L_WARN  	= 0x02;
const unsigned short L_DEBUG 	= 0x04;
const unsigned short L_PACKET  	= 0x08;
const unsigned short L_SILENT 	= 0x8000;

void log(unsigned short level, const char *fmt, ...);

#endif
