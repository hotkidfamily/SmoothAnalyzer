#pragma once
#include "stdafx.h"

enum CHANNELID{
	LCHANNEL,
	RCHANNEL,
	MAX_CHANNEL // support channel count
};

enum PULSETYPE{
	PULSE_LOW,
	PULSE_HIGH,
	PULSE_NONE
};

struct PulseDesc{
	PulseDesc(){
		reset();
	}

	PulseDesc(CHANNELID id, double start, double end, PULSETYPE type){
		this->start = start;
		this->end = end;
		this->channelID = id;
		this->type = type;
		this->duration = start - end;
	}

	void reset(){
		start = 0;
		end = 0;
		duration = 0.0f;
		type = PULSE_NONE;
	}

	double start;
	double end;

	double duration;
	PULSETYPE type;

	CHANNELID channelID;
};

class csvOutput
{
public:
	csvOutput(void);
	~csvOutput(void);
	csvOutput(std::string &filename);

	void RecordTimestamp(CHANNELID channelID, double start, double end);
	void OutputResult();

private:
	void makeRecordFileName();
	
	double CacluAvgValue(std::list<PulseDesc>& durationList);
	double CacluMSE(std::list<PulseDesc>& lowDurationList);
	BOOL DetectPulseWidth(double &duration);
	inline int32_t GetPulseType(PULSETYPE ltype, PULSETYPE rtype);
	void WriteDetail();
	void GetFrameInfo();

	void WriteCsvLine(const char* format,  ...);

private:
	std::list<PulseDesc> mPulseList[MAX_CHANNEL];
	
	std::ofstream csvFile;
	std::string mSourceFileName;
	std::string mResultFileName;
	std::string mRawPulseFileName;
	std::string mRawFramePulseFileName;
};
