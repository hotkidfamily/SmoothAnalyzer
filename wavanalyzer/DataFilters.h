#pragma once
#include "stdafx.h"
#include "DataDumpper.h"

enum FILTERS_INDEX{
	FILTER_ABS,
	FILTER_SMOOTH,
	FILTER_RMNEGTV,
	FILTER_LOW,
	FILTER_COUNT,
};

class IFilter{
protected:
	~IFilter(){};

public:
	virtual int32_t process(std::string &samples, int32_t bytesPerSample) = 0;
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

class RmNegativeFilter
	: public IFilter
	, public iDump
{
public:
	RmNegativeFilter(std::string &filePath);
	~RmNegativeFilter(){};

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

#if 0
class LowFilter
	: public IFilter
	, public iDump
{
public:
	LowFilter(std::string &filePath);
	~LowFilter(){};

	virtual int32_t process(std::string &samples, int32_t Bps);
};
#endif