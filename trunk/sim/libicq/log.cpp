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

#ifdef QT_DLL
#include <qapplication.h>
#endif


#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4530)
#endif
#endif

#include <string>
using namespace std;

#ifndef WIN32
#include <syslog.h>
#endif

#include "buffer.h"
#include "log.h"

logProc *LogProc = NULL;

unsigned short log_level = L_ERROR;

void setLogProc(logProc *p)
{
    LogProc = p;
}

static const char *level_name(unsigned short n)
{
    if (n & L_ERROR) return "ERR";
    if (n & L_WARN) return "WRN";
    if (n & L_DEBUG) return "DBG";
    if (n & L_PACKET) return "PKT";
    return "???";
}

#ifdef WIN32
#define vsnprintf _vsnprintf
#endif

void vformat(string &s, const char *fmt, va_list ap)
{
    char msg[1024];
    vsnprintf(msg, sizeof(msg), fmt, ap);
    s += msg;
}

void format(string &s, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vformat(s, fmt, ap);
    va_end(ap);
}

void log_string(unsigned short l, const char *s)
{
    bool bLog = (l & log_level) != 0;
    if (!bLog && (LogProc == NULL)) return;
    time_t now;
    time(&now);
    struct tm *tm = localtime(&now);
    string m;
    format(m, "%02u:%02u:%02u [%s] ", tm->tm_hour, tm->tm_min, tm->tm_sec, level_name(l));
    m += s;
    if (bLog){
#ifdef QT_DLL
        for (char *p = (char*)m.c_str(); *p; ){
            char *r = strchr(p, '\n');
            if (r) *r = 0;
            qWarning("%s", p);
            if (r == NULL) break;
            p = r + 1;
        }
#else
        fprintf(stderr, "%s", m.c_str());
        fprintf(stderr, "\n");
#endif
    }
    if (LogProc) LogProc(l, m.c_str());
}

void log(unsigned short l, const char *fmt, ...)
{
    bool bLog = (l & log_level) != 0;
    if (!bLog && (LogProc == NULL)) return;
    va_list ap;
    va_start(ap, fmt);
    string m;
    vformat(m, fmt, ap);
    log_string(l, m.c_str());
    va_end(ap);
}

void dumpPacket(Buffer &b, unsigned long start, const char *operation)
{
    bool bLog = log_level & L_PACKET;
    if (!bLog && (LogProc == NULL)) return;
    string m;
    format(m, "%s %u bytes\n", operation, b.size() - start);
    char line[81];
    char *p1 = line;
    char *p2 = line;
    unsigned n = 20;
    unsigned offs = 0;
    for (unsigned i = start; i < b.size(); i++, n++){
        char buf[32];
        if (n == 16){
            m += line;
            m += "\n";
        }
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
    if (n <= 16) m += line;
    log_string(L_PACKET, m.c_str());
}

