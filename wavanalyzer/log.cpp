#include "stdafx.h"
#include "log.h"

#define to_str(x) #x

const char* level_strs[] = {
	to_str(FileSystem),
	to_str(PulseDect),
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

void Logger(log_level level, const char* format, ...)
{
	char str[4096] = "";
	va_list vl = NULL;
	va_start(vl, format);
	_vsnprintf_s(str, 4096 - 1, _TRUNCATE, format, vl);
	va_end(vl);

	if (level >= log_level_base)
	{
		if(level > Error){
			fprintf(stderr, "\n%s: %s\n\n", level_strs[level], str);
		}else{
			fprintf(stderr, "%s: %s\n", level_strs[level], str);
		}
	}
}

void ReportProgress(int32_t progress, int32_t total)
{
	if(total)
		fprintf(stderr, "\t progress %.3f\r", progress*100.0 / total);
}