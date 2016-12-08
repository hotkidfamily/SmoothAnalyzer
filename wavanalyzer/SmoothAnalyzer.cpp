#include "stdafx.h"
#include "SmoothAnalyzer.h"
#include "log.h"

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


PulseAnalyzer::PulseAnalyzer(std::string &filename)
{
	mSourceFileName = filename;
	SYSTEMTIME systime;
	char buffer[256] = {'\0'};
	GetLocalTime(&systime);
	sprintf_s(buffer, 256-1, "_%04d%02d%02d%02d%02d%02d", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

	mSourceFileName += buffer;
}

PulseAnalyzer::~PulseAnalyzer(void)
{
}

void PulseAnalyzer::RecordTimestamp(CHANNELID channelID, double start, double end)
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


double PulseAnalyzer::CacluAvgValue(std::list<FrameDesc>& durationList)
{
	double sum = 0.0f;
	double avgValue = 0.0f;
	std::list<FrameDesc>::iterator durationIt;
	for(durationIt = durationList.begin(); durationIt != durationList.end(); durationIt++){
		sum += durationIt->duration;
	}

	avgValue = sum / durationList.size();

	return avgValue;
}

double PulseAnalyzer::CacluMSE(std::list<FrameDesc>& durationList)
{
	double Sum = 0.0f;
	double MSE = 0.0f;
	double avgDuration = CacluAvgValue(durationList);
	std::list<FrameDesc>::iterator durationIt;

	for(durationIt = durationList.begin(); durationIt != durationList.end(); durationIt++)
	{
		Sum += sqrt(fabs(durationIt->duration - avgDuration));
	}

	MSE = Sum / durationList.size();
	return MSE;
}

double PulseAnalyzer::CacluAvgFps(std::list<FrameDesc> &durationList)
{
	double fps = 0.0f;
	double duraionInMs = (durationList.back().end - durationList.front().start)*1000;
	fps = durationList.size()*1.0 / duraionInMs;
	return fps;
}

double PulseAnalyzer::CacluFps(std::list<FrameDesc> &durationList)
{
	double fps = 0.0f;
	int32_t frameCnt = 0;
	std::list<FrameDesc>::reverse_iterator rit = durationList.rbegin();
	for( ; rit!=durationList.rend(); rit++){
		frameCnt++;
		if((durationList.back().end - rit->start) > 1000){
			fps = frameCnt * 1000.0 / (durationList.back().end - rit->start);
		}
	}
	
	return fps;
}

int32_t PulseAnalyzer::GetPulseType(PULSETYPE ltype, PULSETYPE rtype)
{
	for(int i = 0; i < ARRAYSIZE(pulseTable); i++){
		if((rtype == pulseTable[i].lPulseType) && (rtype == pulseTable[i].rPulseType)){
			return i;
		}
	}

	return -1;
}

BOOL PulseAnalyzer::DetectPulseWidth(double &duration)
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

	std::string filePath = mSourceFileName + ".frame.csv";
	CSVFile file(filePath);

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

		file.WriteCsvLine("%d, %0.3f, %0.3f, %f,"
			"%d, %0.3f, %0.3f, %f",
			lit->channelID, lit->start, lit->end, lit->duration * 1000, 0, 
			rit->channelID, rit->start, rit->end, rit->duration * 1000, 0);
	}

	if(durationCount){
		duration = durationSum / durationCount;
		bRet = TRUE;
		inter_log(Info, "Detect frame duration %.3f ms", duration);
	}

	return bRet;
}

void PulseAnalyzer::GetFrameInfo()
{
	double pulseDuration;
	int32_t curFrameType = 0;
	int32_t frameIndex = 0;
	double referenceTimeBase = 0.0f;
	double fps = 0.0f;

	referenceTimeBase = min(mPulseList[LCHANNEL].front().start, mPulseList[RCHANNEL].front().start);
	inter_log(Info, "Reference base time is %.3f ms", referenceTimeBase);

	if(!mPulseList[LCHANNEL].empty() || !mPulseList[RCHANNEL].empty()){

		DetectPulseWidth(pulseDuration);

		std::list<PulseDesc>::iterator lit;
		std::list<PulseDesc>::iterator rit;
		PulseDesc lBak = *mPulseList[LCHANNEL].begin();
		PulseDesc rBak = *mPulseList[RCHANNEL].begin();

		for(lit = mPulseList[LCHANNEL].begin(), rit = mPulseList[RCHANNEL].begin();
			(lit != mPulseList[LCHANNEL].end()) && (rit!=mPulseList[RCHANNEL].end());)
		{
			curFrameType = GetPulseType(lBak.type, rBak.type);
			if(!(curFrameType < 0)){
				fps = CacluFps(mFramePulse);
				FrameDesc frame(curFrameType, min(lBak.start,lBak.start), max(lBak.end, rBak.end), fps);
				mFramePulse.push_back(frame);
			}

			if(abs(lBak.duration - pulseDuration) < 0.005){ // 5ms 
				lit++;
				lBak = *lit;
			}else{
				lBak.start += pulseDuration;
				lBak.duration -= pulseDuration;
			}

			if(abs(rBak.duration - pulseDuration) < 0.005){ // 5ms
				rit++;
				rBak = *rit;
			}else{
				rBak.start += pulseDuration;
				rBak.duration -= pulseDuration;
			}
		}
	}
}

void PulseAnalyzer::WriteSyncDetail()
{
	uint32_t lindex = 0;
	uint32_t rindex = 0;
	int32_t sync = 0;
	std::string filePath = mSourceFileName + ".detail.csv";
	CSVFile file(filePath);

	file.WriteCsvLine("sync, channel 1, index, start, end, duration, interval, channel 2, index, start, end, duration, interval");

	if(!mPulseList[LCHANNEL].empty() || !mPulseList[RCHANNEL].empty())
	{
		std::list<PulseDesc>::iterator lit;
		std::list<PulseDesc>::iterator rit;

		for(lit = mPulseList[LCHANNEL].begin(), rit = mPulseList[RCHANNEL].begin();
			(lit != mPulseList[LCHANNEL].end()) || (rit!=mPulseList[RCHANNEL].end());)
		{
			if(lit != mPulseList[LCHANNEL].end() && rit!=mPulseList[RCHANNEL].end()){
				if(fabs(lit->start - rit->start) < 0.016){ // 16 ms is min value
					sync = (int32_t)((rit->start - lit->start)*1000);
					lindex ++;
					rindex ++;
					lit++;
					rit++;
				}else{
					if(lit->start < rit->start){
						lit++;
						lindex ++;
					}else{
						rit++;
						rindex ++;
					}
				}
			}else if(lit != mPulseList[LCHANNEL].end()){
				lit++;
				lindex++;
			}else if(rit != mPulseList[RCHANNEL].end()){
				rit++;
				rindex ++;
			}else{
				// do nothing
			}

			file.WriteCsvLine("%d, %d, %u, %0.3f, %0.3f, %f, %d,"
				"%d, %u, %0.3f, %0.3f, %f, %d",
				sync,
				lit->channelID, lindex, lit->start, lit->end, lit->duration * 1000, 0, 
				rit->channelID, rindex, rit->start, rit->end, rit->duration * 1000, 0);
		}
	}
}

void PulseAnalyzer::OutputResult()
{
	double MSE = 0.0f;
	double fps = 0.0f;
	std::string filePath = mSourceFileName + ".smooth.csv";
	int32_t frameIndex = 0;

	if(!mFramePulse.empty()){
		CSVFile file(filePath);

		MSE = CacluMSE(mFramePulse);
		fps = CacluAvgFps(mFramePulse);
		file.WriteCsvLine("MSE, FPS");
		file.WriteCsvLine("%f, %f", MSE, fps);

		while(!mFramePulse.empty()){
			FrameDesc frame = mFramePulse.front();
			file.WriteCsvLine("%d,"
				"%d, %.3f, %.3f"
				""
				, frameIndex++, frame.duration, frame.frameRate);
			mFramePulse.pop_front();
		}
		
	}
}
