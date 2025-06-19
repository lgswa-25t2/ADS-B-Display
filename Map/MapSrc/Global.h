//---------------------------------------------------------------------------

#ifndef GlobalH
#define GlobalH
#include "Timer.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <sys/time.h>
#else
#include <windows.h>
#include <time.h>
#define  _USE_MATH_DEFINES
#include <math.h>
#endif

#ifdef DEBUG
void debug(const char *fmt, ...);
#else
#define debug(format, args...)
#endif

void info(const char *fmt, ...);
void warning(const char *fmt, ...);
void fatal(const char *fmt, ...);

#define GoogleMaps             0
#define SkyVector_VFR          1
#define SkyVector_IFR_Low      2
#define SkyVector_IFR_High     3
#define OpenStreetMaps         4
#define SkyVector              5

//---------------------------------------------------------------------------
#endif
