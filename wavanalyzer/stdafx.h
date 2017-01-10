// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <tchar.h>

#include <algorithm>
#include <list>
#include <vector>
#include <fstream>
#include <string>

#include <windows.h>

#include "stdint.h"

#include "log.h" // project common file

#ifdef UNICODE
typedef std::wstring STRING;
typedef std::wifstream IFSTREAM;
#else
typedef std::string STRING;
typedef std::ifstream IFSTREAM;
#endif

// all data in millisecond

#define ANALYZER_DURATION (1000)
#define SPLIT_PERCENT (ANALYZER_DURATION/10)

#define MINIST_PULSE_DURATION (16)
#define VALID_PULSE_DURATION (MINIST_PULSE_DURATION/2) // ??
#define SYNC_THRESHOLD (MINIST_PULSE_DURATION + 1)

#define PULSE_LEVEL(x) ((int32_t)((x+MINIST_PULSE_DURATION/2)/MINIST_PULSE_DURATION)) // filter all duration < duration/2

#define SYSTEM_RESOLUTION (4) // only can analyzer 4 different pictures


#define SAFE_DELETE(x) { if(x) delete x; x=NULL; }