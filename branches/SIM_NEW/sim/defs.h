/***************************************************************************
                          locale.h  -  description
                             -------------------
    begin                : Mon Mar 18 2002
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

#ifndef _DEFS_H
#define _DEFS_H 1

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

#ifdef USE_KDE

class QKeySequence;

#include <klocale.h>
#include <kcolorbutton.h>
#define QColorButton KColorButton
#if QT_VERSION < 300
QString i18n(const char *singular, const char *plural, unsigned long n);
#endif
#else
#include <qobject.h>
QString i18n(const char *text);
QString i18n(const char *text, const char *comment);
QString i18n(const char *singular, const char *plural, unsigned long n);
#define I18N_NOOP(A)	(A)
#endif

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4530)
#endif
#endif

#ifndef HAVE_STRCASECMP
int strcasecmp(const char *a, const char *b);
#endif

#if QT_VERSION < 300

#include <qpoint.h>

class QContextMenuEvent
{
public:
    QContextMenuEvent(const QPoint &pos) : p(pos) {}
    const QPoint &globalPos() { return p; }
    void accept() {}
protected:
    QPoint p;
};

#endif
#endif

