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

#include <algorithm>
#include <list>
#include <fstream>
#include <string>

#include <windows.h>

#include "stdint.h"

// all data in millisecond

#define ANALYZER_DURATION (1000)
#define SPLIT_PERCENT (ANALYZER_DURATION/10)

#define MINIST_PULSE_DURATION (16)
#define SYNC_THRESHOLD (MINIST_PULSE_DURATION)

#define BASE_SCREEN_DURATION (MINIST_PULSE_DURATION)
#define PULSE_LEVEL(x) ((x+BASE_SCREEN_DURATION/2)/BASE_SCREEN_DURATION) // filter all duration < duration/2
