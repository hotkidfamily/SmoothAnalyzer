#pragma once

enum log_level {
	FileSystem,
	PulseDect,
	PulseSplit,
	Debug,
	Info,
	Warning,
	Error,
	Fatal
};

void Logger(log_level level, const char* format, ...);
void SetLoggerLvel(log_level level);
void ReportProgress(int32_t progress, int32_t total);