#ifndef _SIM_EXPORT_H
#define _SIM_EXPORT_H

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

#ifdef HAVE_GCC_VISIBILITY
# define SIM_EXPORT __attribute__ ((visibility("default")))
# define SIM_IMPORT __attribute__ ((visibility("default")))
# define EXPORT_PROC extern "C" __attribute__ ((visibility("default")))
#elif defined(_WIN32) || defined(_WIN64)
# define SIM_EXPORT __declspec(dllexport)
# define SIM_IMPORT __declspec(dllimport)
# define EXPORT_PROC extern "C" __declspec(dllexport)
#else
# define SIM_EXPORT
# define SIM_IMPORT
# define EXPORT_PROC extern "C"
#endif

#ifdef SIMAPI_EXPORTS   // should be set when simapi-lib is build - please add a check for configure
# define EXPORT SIM_EXPORT
#else
# define EXPORT SIM_IMPORT
#endif

#ifdef Q_CC_MSVC
# define DEPRECATED __declspec(deprecated)
#elif defined Q_CC_GNU
# define DEPRECATED __attribute__ ((deprecated))
#else
# define DEPRECATED
#endif

#ifndef COPY_RESTRICTED
# define COPY_RESTRICTED(A) \
    private: \
        A(const A&); \
        A &operator = (const A&);
#endif

#endif // _SIM_EXPORT_H
