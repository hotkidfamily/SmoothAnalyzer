#include "StdAfx.h"
#include <math.h>
#include "log.h"
#include "csvFileMaker.h"

const struct tagPulseType{
	PULSETYPE lPulseType;
	PULSETYPE rPulseType;
} pulseTable[] = {
	{ PULSE_HIGH,	PULSE_LOW	},
	{ PULSE_HIGH,	PULSE_HIGH	},
	{ PULSE_LOW,	PULSE_HIGH  },
	{ PULSE_LOW,	PULSE_LOW	},
};

#define PULSETABLECOUNT (ARRAYSIZE(pulseTable))

csvOutput::csvOutput(void)
{
}

csvOutput::~csvOutput(void)
{
}

csvOutput::csvOutput(std::string &filename)
{
	mSourceFileName.assign(filename);
}

void csvOutput::makeRecordFileName()
{
	SYSTEMTIME systime;
	char buffer[256] = "";
	GetLocalTime(&systime);
	sprintf_s(buffer, 256-1, "_%04d%02d%02d%02d%02d%02d", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);
	
	mResultFileName = mSourceFileName + buffer + ".csv";
	mRawPulseFileName = mSourceFileName + buffer + ".raw.csv";
	mRawFramePulseFileName = mSourceFileName + buffer + ".pulse.csv";
}

void csvOutput::RecordTimestamp(CHANNELID channelID, double start, double end)
{
	PulseDesc time(channelID, start, end, PULSE_HIGH);

	if(!mPulseList[channelID].empty()){
		PulseDesc &lastTime = mPulseList[channelID].back();
		
		if(lastTime.type != PULSE_HIGH ) {
			inter_log(Error, "wtf ?? last pulse is \"%s\", not high pulse.", lastTime.type==PULSE_LOW?"low":"high");
			return ;
		}

		PulseDesc timeInsert(channelID, lastTime.end, start, PULSE_LOW);
		mPulseList[channelID].push_back(timeInsert);
		
		// filter 
#if 0
		if(((time.start - lastTime.end) < 0.500)
			|| (end - start < 0.003)){ // remove less than 3ms pulse 
				return;
		}
#endif
	}

	mPulseList[channelID].push_back(time);	
}

#if 0
void csvOutput::GenerateLowHighDurationList(std::list<PulseTimestamp>& startTimeList, std::list<PulseTimestamp>& EndTimeList, std::list<double>& lowDurationList, std::list<double>& highDurationList)
{
	std::list<PulseTimestamp>::iterator startTimeIterator = startTimeList.begin();	
	std::list<PulseTimestamp>::iterator endTimeIterator = EndTimeList.begin();
	double preEndTime = startTimeIterator->time;
	while(startTimeIterator != startTimeList.end() && endTimeIterator != EndTimeList.end()){
		double lowDuration =  startTimeIterator->time - preEndTime;
		double highDuration = endTimeIterator->time - startTimeIterator->time;
		lowDurationList.push_back(lowDuration);
		highDurationList.push_back(highDuration);
		
		preEndTime = endTimeIterator->time;
		startTimeIterator++;
		endTimeIterator++;
	}
}

bool csvOutput::ReadLChannelListInfo(double &startTime, double &endTime, double &lowDuration, double &highDuration)
{
	bool result = false;
	if(m_lowDurationListLChannel.size() && m_highDurationListLChannel.size())
	{
		startTime = m_startTimeListLChannel.front().time;
		endTime = m_endTimeListLChannel.front().time;
		lowDuration = m_lowDurationListLChannel.front();
		highDuration = m_highDurationListLChannel.front();
		m_startTimeListLChannel.pop_front();
		m_endTimeListLChannel.pop_front();
		m_lowDurationListLChannel.pop_front();
		m_highDurationListLChannel.pop_front();
		result = true;
	}
	return result;
}

bool csvOutput::ReadRChannelListInfo(double &startTime, double &endTime, double &lowDuration, double &highDuration)
{
	bool result = false;
	if(m_lowDurationListRChannel.size() && m_highDurationListRChannel.size())
	{
		startTime = m_startTimeListRChannel.front().time;
		endTime = m_endTimeListRChannel.front().time;
		lowDuration = m_lowDurationListRChannel.front();
		highDuration = m_highDurationListRChannel.front();
		m_startTimeListRChannel.pop_front();
		m_endTimeListRChannel.pop_front();
		m_lowDurationListRChannel.pop_front();
		m_highDurationListRChannel.pop_front();
		result = true;
	}
	return result;
}

void csvOutput::outputResult()
{
	double Lfps = 0;
	double Rfps = 0;
	Lfps = (m_startTimeListLChannel.size() + m_endTimeListLChannel.size()) / m_endTimeListLChannel.back().time * 1000;	
	Rfps = (m_startTimeListRChannel.size() + m_endTimeListRChannel.size()) / m_endTimeListRChannel.back().time * 1000;
	inter_log(Info, "fps %0.3f, %0.3f.", Lfps, Rfps);
	
	//L/R duration list
	GenerateLowHighDurationList(m_startTimeListLChannel, m_endTimeListLChannel, m_lowDurationListLChannel, m_highDurationListLChannel);	
	GenerateLowHighDurationList(m_startTimeListRChannel, m_endTimeListRChannel, m_lowDurationListRChannel, m_highDurationListRChannel);

	// L/R Mean Square Error
	double L_MSE = CacluMSE(m_lowDurationListLChannel, m_highDurationListLChannel);	
	double R_MSE = CacluMSE(m_lowDurationListRChannel, m_highDurationListRChannel);

	csvFile.open(csvFilePath.c_str());
	if(csvFile.is_open())
	{
		inter_log(Info, "Create file %s.", csvFilePath.c_str());
		writeCsvLine("Lchannel, fps, MSE, Rchanle, fps, MSE");
		writeCsvLine("%u, %0.3f, %0.3f,"
			"%u, %0.3f, %0.3f",
			0, Lfps, L_MSE,
			1, Rfps, R_MSE);

		writeCsvLine("Lchannel, index, start, end, lowduration, highduration, Rchannel, index, start, end, lowduration, highduration");

		uint32_t lindex = 0;
		uint32_t rindex = 0;
		double lStartTime = 0, lEndTime = 0, lLowDuration = 0, lHighDuration = 0;
		double rStartTime = 0, rEndTime = 0, rLowDuration = 0, rHighDuration = 0;
		while(1)
		{
			if (ReadLChannelListInfo(lStartTime, lEndTime, lLowDuration, lHighDuration)
				&& ReadRChannelListInfo(rStartTime, rEndTime, rLowDuration, rHighDuration))
			{
				lindex++;
				rindex++;
			}
			else if(ReadLChannelListInfo(lStartTime, lEndTime, lLowDuration, lHighDuration))
			{
				lindex++;
			}
			else if(ReadRChannelListInfo(rStartTime, rEndTime, rLowDuration, rHighDuration))
			{
				rindex++;
			}
			else
			{
				break;
			}

			writeCsvLine("%d, %u, %0.3f, %0.3f, %f, %f,"
				"%d, %u, %0.3f, %0.3f, %f, %f",
				0, lindex, lStartTime, lEndTime, lLowDuration, lHighDuration, 
				1, rindex, rStartTime, rEndTime, rLowDuration, rHighDuration);
		}
		csvFile.close();
	}
}
#endif

double csvOutput::CacluAvgValue(std::list<PulseDesc>& durationList)
{
	double sum = 0.0f;
	double avgValue = 0.0f;
	std::list<PulseDesc>::iterator durationIt;
	for(durationIt = durationList.begin(); durationIt != durationList.end(); durationIt++){
		sum += durationIt->duration;
	}

	avgValue = sum / durationList.size();

	return avgValue;
}

double csvOutput::CacluMSE(std::list<PulseDesc>& durationList)
{
	double Sum = 0.0f;
	double MSE = 0.0f;
	double avgDuration = CacluAvgValue(durationList);
	std::list<PulseDesc>::iterator durationIt;
	
	for(durationIt = durationList.begin(); durationIt != durationList.end(); durationIt++)
	{
		Sum += sqrt(fabs(durationIt->duration - avgDuration));
	}

	MSE = Sum / durationList.size();
	return MSE;
}

int32_t csvOutput::GetPulseType(PULSETYPE ltype, PULSETYPE rtype)
{
	for(int i = 0; i < ARRAYSIZE(pulseTable); i++){
		if((rtype == pulseTable[i].lPulseType) && (rtype == pulseTable[i].rPulseType)){
			return i;
		}
	}
	
	return 0;
}

BOOL csvOutput::DetectPulseWidth(double &duration)
{
	int32_t exceptNextType = 0;
	int32_t continueTypeCount = 0;
	int32_t curFrameType = 0;
	double durationSum = 0.0f; // sum of list suitable duration
	double durationFrames = 0.0f; // 
	int32_t durationCount = 0;
	BOOL bRet = FALSE;

	std::list<PulseDesc>::iterator lit;
	std::list<PulseDesc>::iterator rit;

	for(lit = mPulseList[LCHANNEL].begin(), rit = mPulseList[RCHANNEL].begin();
		(lit != mPulseList[LCHANNEL].end()) && (rit!=mPulseList[RCHANNEL].end());
		lit++, rit++)
	{
		curFrameType = GetPulseType(lit->type, rit->type);
		if(curFrameType == exceptNextType){

			continueTypeCount++;
			durationFrames += (lit->duration + rit->duration);
			exceptNextType = (exceptNextType+1)%PULSETABLECOUNT;

			if(continueTypeCount == PULSETABLECOUNT){
				durationCount += PULSETABLECOUNT*2;
				durationSum += durationFrames;
				durationFrames = 0.0f;
			}
		}else{
			continueTypeCount = 0;
			exceptNextType = curFrameType; // restart calculate
			durationFrames = 0.0f;
		}
	}

	if(durationCount){
		duration = durationSum / durationCount;
		bRet = TRUE;
		inter_log(Info, "Detect frame duration %.2fms", duration);
	}

	return bRet;
}

void csvOutput::GetFrameInfo()
{
	double duration;
	int32_t curFrameType = 0;

	if(!mPulseList[LCHANNEL].empty() || !mPulseList[RCHANNEL].empty()){

		DetectPulseWidth(duration);

		std::list<PulseDesc>::iterator lit;
		std::list<PulseDesc>::iterator rit;

		for(lit = mPulseList[LCHANNEL].begin(), rit = mPulseList[RCHANNEL].begin();
			(lit != mPulseList[LCHANNEL].end()) && (rit!=mPulseList[RCHANNEL].end());
			lit++, rit++)
		{
			curFrameType = GetPulseType(lit->type, rit->type);

			WriteCsvLine("%d, %0.3f, %0.3f, %f,"
				"%d, %0.3f, %0.3f, %f",
				lit->channelID, lit->start, lit->end, lit->duration * 1000, 0, 
				rit->channelID, rit->start, rit->end, rit->duration * 1000, 0);
		}
	}
}

void csvOutput::WriteCsvLine(const char* format, ...)
{    
	char csvLine[1024] = "";
	va_list args;
	va_start(args, format);
	vsprintf_s(csvLine, format, args);
	va_end(args);

	csvFile << csvLine << '\n';
}

void csvOutput::WriteDetail()
{
	uint32_t lindex = 0;
	uint32_t rindex = 0;
	
	WriteCsvLine("sync, channel 1, index, start, end, duration, interval, channel 2, index, start, end, duration, interval");

	while(!mPulseList[LCHANNEL].empty() || !mPulseList[RCHANNEL].empty()){
		int32_t sync = 0;
		PulseDesc lPulse;
		PulseDesc rPulse;

		if(!mPulseList[LCHANNEL].empty() && !mPulseList[RCHANNEL].empty()){
			lPulse = mPulseList[LCHANNEL].front();
			rPulse = mPulseList[RCHANNEL].front();

			if(fabs(lPulse.start - rPulse.start) < 0.500){
				mPulseList[LCHANNEL].pop_front();
				mPulseList[RCHANNEL].pop_front();
				sync = (int32_t)((rPulse.start - lPulse.start)*1000);
				lindex ++;
				rindex ++;
			}else{
				if(lPulse.start < rPulse.start){
					mPulseList[LCHANNEL].pop_front();
					rPulse.reset();
					lindex ++;
				}else{
					mPulseList[RCHANNEL].pop_front();
					lPulse.reset();
					rindex ++;
				}
			}
		}else if(!mPulseList[LCHANNEL].empty()){
			lPulse = mPulseList[LCHANNEL].front();
			mPulseList[LCHANNEL].pop_front();
			lindex++;
		}else if(!mPulseList[RCHANNEL].empty()){
			rPulse = mPulseList[RCHANNEL].front();
			mPulseList[RCHANNEL].pop_front();
			rindex ++;
		}else{
			// do nothing
		}

		WriteCsvLine("%d, %d, %u, %0.3f, %0.3f, %f, %d,"
			"%d, %u, %0.3f, %0.3f, %f, %d",
			sync,
			lPulse.channelID, lindex, lPulse.start, lPulse.end, (lPulse.end - lPulse.start) * 1000, 0, 
			rPulse.channelID, rindex, rPulse.start, rPulse.end, (rPulse.end - rPulse.start) * 1000, 0);
	}
}

void csvOutput::OutputResult()
{
	csvFile.open(mSourceFileName.c_str());
	if(!csvFile.is_open()){
		inter_log(Info, "Can not create file %s.", mSourceFileName.c_str());
		return ;
	}

	inter_log(Info, "Create result file %s.", mSourceFileName.c_str());

	WriteDetail();

	csvFile.close();
}
