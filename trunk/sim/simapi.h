/***************************************************************************
                          simapi.h  -  description
                             -------------------
    begin                : Sun Mar 17 2002
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

#ifndef _SIMAPI_H
#define _SIMAPI_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
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

#ifdef Q_CC_MSVC
// "conditional expression is constant" (W4)
# pragma warning(disable: 4127)
// "'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'" (W1)
# pragma warning(disable: 4251)
// "function' : function not inlined" (W4)4786
# pragma warning(disable: 4710)
#endif

#ifdef Q_CC_MSVC
# define DEPRECATED __declspec(deprecated)
#elif defined Q_CC_GNU
# define DEPRECATED __attribute__ ((deprecated))
#else
# define DEPRECATED
#endif

#include "sim_export.h"

#ifndef COPY_RESTRICTED
# define COPY_RESTRICTED(A) \
    private: \
        A(const A&); \
        A &operator = (const A&);
#endif

#include "event.h"
#include "cfg.h"
#include "cmddef.h"
#include "contacts.h"
#include "country.h"
#include "log.h"
#include "message.h"
#include "misc.h"
#include "plugins.h"
#include "unquot.h"


#endif
