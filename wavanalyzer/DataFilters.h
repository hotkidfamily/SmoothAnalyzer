#pragma once
#include "stdafx.h"

enum FILTERS_INDEX{
	FILTER_ABS,
	FILTER_SMOOTH,
	FILTER_UPDOWN,
	FILTER_COUNT,
};

class IFilter{
protected:
	~IFilter(){};

public:
	virtual int32_t process(std::string &samples, int32_t bytesPerSample) = 0;
};

class iDump{
public:
	iDump(std::string &filepath);
	int32_t DumpData(std::string &data);
protected:
	std::ofstream dumpfile;
};

class ABSFilter
	: public IFilter
	, public iDump
{
public:
	ABSFilter(std::string &filePath);
	~ABSFilter(){};

	virtual int32_t process(std::string &samples, int32_t Bps);
};

class UDFilter
	: public IFilter
	, public iDump
{
public:
	UDFilter(std::string &filePath);
	~UDFilter(){};

	virtual int32_t process(std::string &samples, int32_t Bps);
};

class SmoothFilter
	: public IFilter
	, public iDump
{
public:
	SmoothFilter(std::string &filePath);
	~SmoothFilter(){};

	virtual int32_t process(std::string &samples, int32_t Bps);
};
