#pragma once

enum log_level {
	Debug,
	Info,
	Warning,
	Error,
	Fatal
};

void inter_log(log_level level, const char* format, ...);