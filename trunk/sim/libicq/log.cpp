/***************************************************************************
                          log.cpp  -  description
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

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#ifndef WIN32
#include <syslog.h>
#endif

#ifdef QT_DLL
#include <qapplication.h>
#endif

#include "log.h"

unsigned short log_level = L_ERROR;

static const char *level_name(unsigned short n)
{
    if (n & L_ERROR) return "ERR";
    if (n & L_WARN) return "WRN";
    if (n & L_DEBUG) return "DBG";
    if (n & L_PACKET) return "PKT";
    return "???";
}

void log(unsigned short l, const char *fmt, ...)
{
    if ((l & log_level) == 0) return;
    time_t now;
    time(&now);
    struct tm *tm = localtime(&now);
    va_list ap;
    va_start(ap, fmt);
#ifdef QT_DLL
    char time[128] = "";
    char msg[1024];
    if (!(l & L_SILENT)){
        _snprintf(time, sizeof(time), "%02u:%02u:%02u [%s] ", tm->tm_hour, tm->tm_min, tm->tm_sec, level_name(l));
        _vsnprintf(msg, sizeof(msg), fmt, ap);
        qWarning("%s%s", time, msg);
    }
#else
if (!(l & L_SILENT)){
    fprintf(stderr, "%02u:%02u:%02u [%s] ", tm->tm_hour, tm->tm_min, tm->tm_sec, level_name(l));
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
}
#endif
    va_end(ap);
}

