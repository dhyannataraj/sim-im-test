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

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4530)
#endif
#endif

#include "buffer.h"
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
    if (!(l & L_SILENT))
        _snprintf(time, sizeof(time), "%02u:%02u:%02u [%s] ", tm->tm_hour, tm->tm_min, tm->tm_sec, level_name(l));
    _vsnprintf(msg, sizeof(msg), fmt, ap);
    qWarning("%s%s", time, msg);
#else
    if (!(l & L_SILENT))
        fprintf(stderr, "%02u:%02u:%02u [%s] ", tm->tm_hour, tm->tm_min, tm->tm_sec, level_name(l));
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
#endif
    va_end(ap);
}

void dumpPacket(Buffer &b, unsigned long start, const char *operation)
{
    if ((log_level & L_PACKET) == 0) return;
    string res;
    log(L_PACKET, "%s %u bytes", operation, b.size() - start);
    char line[81];
    char *p1 = line;
    char *p2 = line;
    unsigned n = 20;
    unsigned offs = 0;
    for (unsigned i = start; i < b.size(); i++, n++){
        char buf[32];
        if (n == 16)
            log(L_PACKET | L_SILENT, "%s", line);
        if (n >= 16){
            memset(line, ' ', 80);
            line[80] = 0;
            snprintf(buf, sizeof(buf), "     %04X: ", offs);
            memcpy(line, buf, strlen(buf));
            p1 = line + strlen(buf);
            p2 = p1 + 52;
            n = 0;
            offs += 0x10;
        }
        if (n == 8) p1++;
        unsigned char c = (unsigned char)*(b.Data(i));
        *(p2++) = ((c >= ' ') && (c != 0x7F)) ? c : '.';
        snprintf(buf, sizeof(buf), "%02X ", c);
        memcpy(p1, buf, 3);
        p1 += 3;
    }
    if (n <= 16) log(L_PACKET | L_SILENT, "%s", line);
}

