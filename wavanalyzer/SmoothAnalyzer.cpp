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

	ZeroMemory(mFrameId, sizeof(mFrameId));
}

PulseAnalyzer::~PulseAnalyzer(void)
{
}

void PulseAnalyzer::RecordTimestamp(CHANNELID channelID, double start, double end)
{
	if(!mPulseList[channelID].empty()){
		PulseDesc &lastTime = mPulseList[channelID].back();

		if(lastTime.type != PULSE_HIGH ) {
			inter_log(Error, "?? last pulse is \"%s\", not high pulse.", lastTime.type==PULSE_LOW?"low":"high");
			return ;
		}

		PulseDesc timeInsert(channelID, lastTime.end, start, PULSE_LOW, mFrameId[channelID]++);
		mPulseList[channelID].push_back(timeInsert);

		// filter 
#if 0
		if(((time.start - lastTime.end) < 0.500)
			|| (end - start < 0.003)){ // remove less than 3ms pulse 
				return;
		}
#endif
	}
	PulseDesc time(channelID, start, end, PULSE_HIGH, mFrameId[channelID]++);
	mPulseList[channelID].push_back(time);	
}


double PulseAnalyzer::CacluAvgValue(std::list<FrameDesc>& durationList)
{
	double sum = 0.0f;
	double avgValue = 0.0f;
	std::list<FrameDesc>::iterator it;

	if (!durationList.empty()){
		for (it = durationList.begin(); it != durationList.end(); it++){
			sum += it->duration;
		}

		avgValue = sum / durationList.size();
	}

	return avgValue;
}

double PulseAnalyzer::CacluMSE(std::list<FrameDesc>& durationList)
{
	double Sum = 0.0f;
	double MSE = 0.0f;
	double avgDuration = 0.0f;
	std::list<FrameDesc>::iterator it;

	avgDuration = CacluAvgValue(durationList);

	if (!durationList.empty()){
		for(it = durationList.begin(); it != durationList.end(); it++){
			Sum += sqrt(fabs(it->duration - avgDuration));
		}

		MSE = Sum / durationList.size();
	}
	return MSE;
}


double PulseAnalyzer::CacluMSEInOneSecond(std::list<FrameDesc>& frameList)
{
	double sum = 0.0f;
	double MSE = 0.0f;
	std::list<FrameDesc>::reverse_iterator rit;

	for (rit = frameList.rbegin(); rit != frameList.rend(); rit++){
		if((frameList.back().end - rit->start)*1000.0 > 1000.0f){
			std::list<FrameDesc> frameListSplit;

			frameListSplit.assign(frameList.rbegin(), rit);
			MSE = CacluMSE(frameList);

			break;
		}
	}

	return MSE;
}

double PulseAnalyzer::CacluFps(std::list<FrameDesc> &frameList)
{
	double fps = 0.0f;
	double duraionInMs = 0.0;

	if (!frameList.empty()){
		duraionInMs = frameList.back().end - frameList.front().start;
		fps = frameList.size()*1000.0 / duraionInMs;
	}

	return fps;
}

double PulseAnalyzer::CacluFrameRate(std::list<FrameDesc> &frameList)
{
	double fps = 0.0f;
	int32_t frameCnt = 0;
	std::list<FrameDesc>::reverse_iterator rit;

	for (rit = frameList.rbegin(); rit != frameList.rend(); rit++){
		if ((frameList.back().end - rit->start)*1000.0f >= 1000.0f){
			std::list<FrameDesc> frameListSplit;

			frameListSplit.assign(frameList.rbegin(), rit);
			fps = CacluFps(frameList);

			break;
		}
	}

	return fps;
}

int32_t PulseAnalyzer::GetPulseType(PULSETYPE ltype, PULSETYPE rtype)
{
	for(int i = 0; i < ARRAYSIZE(pulseTable); i++){
		if ( (ltype == pulseTable[i].lPulseType)
				&& (rtype == pulseTable[i].rPulseType) ){
			return i;
		}
	}

	return -1;
}

BOOL PulseAnalyzer::DetectPulseWidth(double &duration)
{
	int32_t exceptNextType = 0;
	int32_t continueCount = 0;
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
			continueCount++;
			durationFrames += (lit->duration + rit->duration);
			exceptNextType = (exceptNextType+1)%PULSETABLECOUNT;

			if(continueCount == PULSETABLECOUNT){
				durationCount += PULSETABLECOUNT*2;
				durationSum += durationFrames;
				durationFrames = 0.0f;
			}
		}else{
			continueCount = 0;
			exceptNextType = curFrameType; // restart calculate
			durationFrames = 0.0f;
		}
	}

	if(durationCount){
		duration = durationSum / durationCount;
		bRet = TRUE;
		inter_log(Info, "Detect frame duration %.3f ms", duration);
	}else{
		inter_log(Error, "Detect frame duration %.3f ms", duration);
	}

	return bRet;
}

void PulseAnalyzer::GetFrameInfo()
{
	double pulseDuration = 0.0f;
	int32_t curFrameType = 0;
	double referenceTimeBase = 0.0f;
	double fps = 0.0f;
	double MSE = 0.0f;
	int32_t index = 0;

	DetectPulseWidth(pulseDuration);

	referenceTimeBase = min(mPulseList[LCHANNEL].front().start, mPulseList[RCHANNEL].front().start);
	inter_log(Info, "Reference base time is %.3f ms", referenceTimeBase);

	if(!mPulseList[LCHANNEL].empty() || !mPulseList[RCHANNEL].empty()){
		std::list<PulseDesc>::iterator lit;
		std::list<PulseDesc>::iterator rit;
		PulseDesc lBak = *mPulseList[LCHANNEL].begin();
		PulseDesc rBak = *mPulseList[RCHANNEL].begin();

		for(lit = mPulseList[LCHANNEL].begin(), rit = mPulseList[RCHANNEL].begin();
			(lit != mPulseList[LCHANNEL].end()) && (rit!=mPulseList[RCHANNEL].end());)
		{
			curFrameType = GetPulseType(lBak.type, rBak.type);
			if(!(curFrameType < 0)){
				fps = CacluFrameRate(mFramePulse);
				MSE = CacluMSEInOneSecond(mFramePulse);
				FrameDesc frame(curFrameType, min(lBak.start,rBak.start), max(lBak.end, rBak.end), fps, MSE, index++);
				mFramePulse.push_back(frame);
			}

			if(pulseDuration > 10.0f){
				if(fabs(lBak.duration - pulseDuration) < 0.005){ // 5ms 
					lit++;
					lBak = *lit;
				}else{
					lBak.start += pulseDuration;
					lBak.duration -= pulseDuration;
				}

				if(fabs(rBak.duration - pulseDuration) < 0.005){ // 5ms
					rit++;
					rBak = *rit;
				}else{
					rBak.start += pulseDuration;
					rBak.duration -= pulseDuration;
				}
			}else{
				lit++;
				rit++;
			}
		}
	}
}

void PulseAnalyzer::WriteRawPulseDetail()
{
	std::list<PulseDesc>::iterator lit;
	std::list<PulseDesc>::iterator rit;

	std::string filePath = mSourceFileName + ".raw.pulse.csv";
	CSVFile file(filePath);

	file.WriteCsvLine(" channel, start, end, duration, channel, start, end, duration, ");

	for(lit = mPulseList[LCHANNEL].begin(), rit = mPulseList[RCHANNEL].begin();
		(lit != mPulseList[LCHANNEL].end()) || (rit!=mPulseList[RCHANNEL].end());)
	{
		PulseDesc lPulse, rPulse;

		if(lit != mPulseList[LCHANNEL].end()){
			lPulse = *lit;
			lit++;
		}

		if(rit != mPulseList[RCHANNEL].end()){
			rPulse = *rit;
			rit++;
		}

		if(!lPulse.IsInvalid() && !rPulse.IsInvalid()){
			file.WriteCsvLine(
				" %d, %d, %.3f, %.3f, %.3f, "
				" %d, %d, %.3f, %.3f, %.3f, ",
				lPulse.channelID, lPulse.index, lPulse.start, lPulse.end, lPulse.duration * 1000,
				rPulse.channelID, rPulse.index, rPulse.start, rPulse.end, rPulse.duration * 1000);
		}else if(!lPulse.IsInvalid()){
			file.WriteCsvLine(
				" %d, %d, %.3f, %.3f, %.3f, "
				" ,,,,, ",
				lPulse.channelID, lPulse.index, lPulse.start, lPulse.end, lPulse.duration * 1000);
		}else if(!rPulse.IsInvalid()){
			file.WriteCsvLine(
				" ,,,,, "
				" %d, %d, %.3f, %.3f, %.3f, ",
				rPulse.channelID, rPulse.index, rPulse.start, rPulse.end, rPulse.duration * 1000);
		}
		
	}
}

#define SYNC_THRESHOLD (0.015)
/* compare short list to long list and write value */ 
void PulseAnalyzer::WriteSyncDetail()
{
	std::list<PulseDesc> shortChannel; // short list
	std::list<PulseDesc> longChannel; // long list
	int32_t sync = 0;
	double firstDiff = 0.0f;
	double secondDiff = 0.0f;
	std::list<PulseDesc>::iterator itShort;
	std::list<PulseDesc>::iterator itLong;
	std::list<PulseDesc>::iterator itLongNext;
	std::string filePath = mSourceFileName + ".sync.detail.csv";
	CSVFile file(filePath);

	if (mPulseList[LCHANNEL].size() >= mPulseList[RCHANNEL].size()){
		longChannel = mPulseList[LCHANNEL];
		shortChannel = mPulseList[RCHANNEL];
	} else{
		longChannel = mPulseList[RCHANNEL];
		shortChannel = mPulseList[LCHANNEL];
	}

	itShort = shortChannel.begin();
	itLong = longChannel.begin();
	
	file.WriteCsvLine("sync, channel 1, index, start, end, duration, interval, "
		"channel 2, index, start, end, duration, interval");

	while(1){
		PulseDesc shortPulse, longPulse;
		secondDiff = 0.0f;
		firstDiff = 0.0f;

		if((itShort != shortChannel.end()) && (itLong != longChannel.end())){
			// find most suitable pulse
			sync = 0;

			itLongNext = itLong;
			itLongNext++;

			firstDiff = itShort->start - itLong->start;
			if(itLongNext != longChannel.end())
				secondDiff = itShort->start - itLongNext->start;

			if(firstDiff == 0.0f){
				longPulse = *itLong;
				shortPulse = *itShort;
				sync = (int32_t)(firstDiff*1000);
			} else if(secondDiff == 0.0f){
				longPulse = *itLong;
			}else if(firstDiff > 0 && secondDiff < 0){
				longPulse = *itLong;
				if((fabs(firstDiff) <= fabs(secondDiff)) && (fabs(firstDiff) < SYNC_THRESHOLD)){
					shortPulse = *itShort;
					sync = (int32_t)(firstDiff*1000);
				}
			} else if (firstDiff > 0 && secondDiff > 0){
				if(firstDiff >= secondDiff){
					longPulse = *itLong;
				}else{
					inter_log(Error, "can not happend.");
				}
			}else if(firstDiff < 0 && secondDiff > 0){
				inter_log(Error, "can not happend.");
			}else if(firstDiff < 0 && secondDiff < 0){
				if(firstDiff < secondDiff){
					inter_log(Error, "can not happend.");
				}else{
					shortPulse = *itShort;
					if(fabs(firstDiff) <= SYNC_THRESHOLD){
						longPulse = *itLong;
						sync = (int32_t)(firstDiff*1000);
					}
				}
			}			
		}else if(itShort != shortChannel.end()){// only "L" channel
			shortPulse = *itShort;
		}else if(itLong != longChannel.end()){// only "R" channel
			longPulse = *itLong;
		}else{
			break;
		}

		if(!longPulse.IsInvalid() && !shortPulse.IsInvalid()){
			file.WriteCsvLine("%d, "
				"%d, %u, %.3f, %.3f, %.3f, %d, "
				"%d, %u, %.3f, %.3f, %.3f, %d, ",
				sync,
				shortPulse.channelID, shortPulse.index, shortPulse.start, shortPulse.end, shortPulse.duration * 1000, 0, 
				longPulse.channelID, longPulse.index, longPulse.start, longPulse.end, longPulse.duration * 1000, 0);
			itShort++;
			itLong++;
		}else if (!longPulse.IsInvalid()){
			file.WriteCsvLine(", "
				" ,  ,  ,  ,  , ,"
				"%d, %u, %.3f, %.3f, %.3f, %d,",
				longPulse.channelID, longPulse.index, longPulse.start, longPulse.end, longPulse.duration * 1000, 0);
			itLong++;
		}else if(!shortPulse.IsInvalid()){
			file.WriteCsvLine(", "
				"%d, %u, %.3f, %.3f, %.3f, %d, "
				", , , , , , ",
				shortPulse.channelID, shortPulse.index, shortPulse.start, shortPulse.end, shortPulse.duration * 1000, 0);
			itShort++;
		}
	}
}

void PulseAnalyzer::WriteSmoothDetail()
{
	double MSE = 0.0f;
	double fps = 0.0f;
	std::string filePath = mSourceFileName + ".smooth.csv";
	int32_t frameIndex = 0;

	if(!mFramePulse.empty()){
		CSVFile file(filePath);

		MSE = CacluMSE(mFramePulse);
		fps = CacluFps(mFramePulse);
		file.WriteCsvLine("MSE, FPS,");
		file.WriteCsvLine("%.3f, %.3f,", MSE, fps);

		file.WriteCsvLine("");
		file.WriteCsvLine("Index, Duration, FPS, MSE,");
		while(!mFramePulse.empty()){
			FrameDesc frame = mFramePulse.front();
			file.WriteCsvLine("%d, %.3f, %.3f, %.3f,"
				, frameIndex++, frame.duration, frame.frameRate, frame.MSE);
			mFramePulse.pop_front();
		}
	}
}

void PulseAnalyzer::OutputResult()
{
	GetFrameInfo();
	WriteRawPulseDetail();
	WriteSyncDetail();
	WriteSmoothDetail();
}