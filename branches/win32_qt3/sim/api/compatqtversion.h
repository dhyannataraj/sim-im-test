/***************************************************************************
                     compatqtversion.h  -  description
                             -------------------
    begin                : Wed Feb 18 2004
    copyright            : (C) 2004 by Marcel Meckel
    email                : debian@thermoman.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _COMPATQTVERSION_H
#define _COMPATQTVERSION_H

#ifndef QT_VERSION
#include <qglobal.h>
#endif

// important: make the old format of QT_VERSION comparable
//            to the new format that is used since Qt 3.0.5

#if QT_VERSION == 141
#define COMPAT_QT_VERSION 0x010401
#endif
#if QT_VERSION == 142
#define COMPAT_QT_VERSION 0x010402
#endif
#if QT_VERSION == 143
#define COMPAT_QT_VERSION 0x010403
#endif
#if QT_VERSION == 144
#define COMPAT_QT_VERSION 0x010404
#endif
#if QT_VERSION == 145
#define COMPAT_QT_VERSION 0x010405
#endif
#if QT_VERSION == 200
#define COMPAT_QT_VERSION 0x020000
#endif
#if QT_VERSION == 201
#define COMPAT_QT_VERSION 0x020001
#endif
#if QT_VERSION == 202
#define COMPAT_QT_VERSION 0x020002
#endif
#if QT_VERSION == 210
#define COMPAT_QT_VERSION 0x020100
#endif
#if QT_VERSION == 211
#define COMPAT_QT_VERSION 0x020101
#endif
#if QT_VERSION == 220
#define COMPAT_QT_VERSION 0x020200
#endif
#if QT_VERSION == 221
#define COMPAT_QT_VERSION 0x020201
#endif
#if QT_VERSION == 222
#define COMPAT_QT_VERSION 0x020202
#endif
#if QT_VERSION == 223
#define COMPAT_QT_VERSION 0x020203
#endif
#if QT_VERSION == 224
#define COMPAT_QT_VERSION 0x020204
#endif
#if QT_VERSION == 230
#define COMPAT_QT_VERSION 0x020300
#endif
#if QT_VERSION == 231
#define COMPAT_QT_VERSION 0x020301
#endif
#if QT_VERSION == 232
#define COMPAT_QT_VERSION 0x020302
#endif
#if QT_VERSION == 300
#define COMPAT_QT_VERSION 0x030000
#endif
#if QT_VERSION == 301
#define COMPAT_QT_VERSION 0x030001
#endif
#if QT_VERSION == 302
#define COMPAT_QT_VERSION 0x030002
#endif
#if QT_VERSION == 303
#define COMPAT_QT_VERSION 0x030003
#endif
#if QT_VERSION == 304
#define COMPAT_QT_VERSION 0x030004
#endif

// --- new format since Qt 3.0.5

#ifndef COMPAT_QT_VERSION
#define COMPAT_QT_VERSION QT_VERSION
#endif

#endif

