/***************************************************************************
                          log.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "log.h"
//#include "buffer.h"
#include "event.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string>

#include <QApplication>
#include <QString>
#ifdef QT_DLL
//Added by qt3to4:
#endif

namespace SIM
{

using std::string;

const char *level_name(unsigned short n)
{
    if (n & L_ERROR) return "ERR";
    if (n & L_WARN) return "WRN";
    if (n & L_DEBUG) return "DBG";
    return "???";
}

#ifdef WIN32
#define vsnprintf _vsnprintf
#endif

#if defined __GNUC__
void vformat(string &s, const char *fmt, va_list ap)
__attribute__ ((__format__ (printf, 2, 0)));
void format(string &s, const char *fmt, ...)
__attribute__ ((__format__ (printf, 2, 3)));
#else
void vformat(string &s, const char *fmt, va_list ap);
void format(string &s, const char *fmt, ...);
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
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    QString m = QString("%1/%2/%3 %4:%5:%6 [%7] ").arg(tm->tm_mday).arg(tm->tm_mon + 1)
			.arg(tm->tm_year + 1900).arg(tm->tm_hour).arg(tm->tm_min).arg(tm->tm_sec).arg(level_name(l));
	m += s;
    EventLog e(l, m);
    e.process();
}

void log(unsigned short l, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    string m;
    vformat(m, fmt, ap);
    log_string(l, m.c_str());
    va_end(ap);
}

void log(unsigned short l, const QString &str)
{
    log_string(l, str.local8Bit().data());
}

}
