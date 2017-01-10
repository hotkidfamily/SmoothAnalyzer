#pragma once

enum log_level {
	Fatal,
	Error,
	Warning,
	Info,
	Debug,
	PulseSplit,
	PulseDect,
	FileSystem,
};

void Logger(log_level level, const char* format, ...);
void SetLoggerLevel(log_level level);
void ReportProgress(int32_t progress, int32_t total);