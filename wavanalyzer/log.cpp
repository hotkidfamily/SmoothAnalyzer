#include "stdafx.h"
#include <stdarg.h>
#include "log.h"

#define to_str(x) #x

const char* level_strs[] = {
	to_str(FileSystem),
	to_str(Pulse),
	to_str(Debug),
	to_str(Info),
	to_str(Warning),
	to_str(Error),
	to_str(Fatal)
};

#undef to_str

#ifdef _DEBUG
static log_level log_level_base = Debug;
#else
static log_level log_level_base = Info;
#endif

void inter_log(log_level level, const char* format, ...)
{
	char str[4096] = "";
	va_list vl = NULL;
	va_start(vl, format);
	_vsnprintf_s(str, 4096 - 1, _TRUNCATE, format, vl);
	va_end(vl);

	if (level >= log_level_base)
	{
		fprintf(stderr, "%s: %s\n", level_strs[level], str);
	}
}