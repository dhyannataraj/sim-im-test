#ifndef _SIM_EXPORT_H
#define _SIM_EXPORT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_GCC_VISIBILITY      // @linux: all gcc >= 4.0 have visibility support - please add a check for configure
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

#endif // _SIM_EXPORT_H
