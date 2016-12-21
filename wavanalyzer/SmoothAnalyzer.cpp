#include "stdafx.h"
#include "SmoothAnalyzer.h"
#include "log.h"

const struct tagPulseType{
	PULSETYPE lPulseType;
	PULSETYPE rPulseType;
} pulseTable[] = {
	{ PULSE_HIGH,	PULSE_LOW	},
	{ PULSE_LOW,	PULSE_HIGH  },
	{ PULSE_LOW,	PULSE_LOW	},
	{ PULSE_HIGH,	PULSE_HIGH	},
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

void PulseAnalyzer::ReportProgress(int32_t progress, int32_t total)
{
	if(total)
		fprintf(stderr, "\t progress %.3f\r", progress*100.0 / total);
}

double PulseAnalyzer::CalcAvgValue(std::list<FrameDesc>& durationList)
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

double PulseAnalyzer::CalcAvgValueInOneSecond(std::list<FrameDesc> &frameList)
{
	double avg = 0.0f;
	std::list<FrameDesc>::reverse_iterator rit;

	for (rit = frameList.rbegin(); rit != frameList.rend(); rit++){
		if((frameList.back().end - rit->start) > 1.0){
			std::list<FrameDesc> frameListSplit;

			rit++;
			frameListSplit.assign(rit.base(), frameList.end());
			avg = CalcAvgValue(frameList);

			break;
		}
	}

	return avg;
}

double PulseAnalyzer::CalcSTDEVP(std::list<FrameDesc>& durationList, const double &avg)
{
	double Sum = 0.0f;
	double SD = 0.0f;
	std::list<FrameDesc>::iterator it;

	if (!durationList.empty()){
		for(it = durationList.begin(); it != durationList.end(); it++){
			Sum += pow(fabs(it->duration - avg), 2);
		}

		SD = 100.0 * sqrt(Sum / durationList.size()) / avg;
	}
	return SD;
}


// bugs:: 
double PulseAnalyzer::CalcSTDEVPInOneSecond(std::list<FrameDesc>& frameList, const double &avg)
{
	double sum = 0.0f;
	double stdevp = 0.0f;
	std::list<FrameDesc>::reverse_iterator rit;

	for (rit = frameList.rbegin(); rit != frameList.rend(); rit++){
		if((frameList.back().end - rit->start) > 1.0){
			std::list<FrameDesc> frameListSplit;

			rit++;
			frameListSplit.assign(rit.base(), frameList.end());
			stdevp = CalcSTDEVP(frameList, avg);

			break;
		}
	}

	return stdevp;
}

double PulseAnalyzer::CalcFps(std::list<FrameDesc> &frameList)
{
	double fps = 0.0f;
	double duraionInSecond = 0.0;

	if (!frameList.empty()){
		duraionInSecond = frameList.back().end - frameList.front().start;
		fps = frameList.size() / duraionInSecond;
	}

	return fps;
}

double PulseAnalyzer::CalcFpsInOneSecond(std::list<FrameDesc> &frameList)
{
	double fps = 0.0f;
	int32_t frameCnt = 0;
	std::list<FrameDesc>::reverse_iterator rit;

	for (rit = frameList.rbegin(); rit != frameList.rend(); rit++){
		if ((frameList.back().end - rit->start) >= 1.0){
			std::list<FrameDesc> frameListSplit;

			rit++;
			frameListSplit.assign(rit.base(), frameList.end());

			fps = CalcFps(frameListSplit);

			break;
		}
		frameCnt ++;
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

void PulseAnalyzer::RecordPulse(CHANNELID channelID, double start, double end)
{
	if(((end - start)*1000) < MINIST_PULSE_DURATION){
		return ;
	}

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

void PulseAnalyzer::PulseFilter()
{
	if (mPulseList[LCHANNEL].size() >= mPulseList[RCHANNEL].size()){
		PulseLowFilter(mPulseList[LCHANNEL]);
	}else{
		PulseLowFilter(mPulseList[RCHANNEL]);
	}
}

void PulseAnalyzer::PulseLowFilter(std::list<PulseDesc> &channelPulse)
{
	std::list<PulseDesc>::iterator itLong;
	std::list<PulseDesc>::iterator itPreLong;
	std::list<PulseDesc>::iterator itPosLong;
	std::list<PulseDesc> &longChannel = channelPulse;
	std::list<PulseDesc> newChannel;
	int32_t index = 0;

	itLong = longChannel.begin();

	for(; itLong != longChannel.end(); )
	{
		if(!itLong->IsPulseInvalid()){
			// link the sample before and after together
			itPreLong = itLong;
			itPosLong = itLong;
			itPreLong --;
			itPosLong ++;

			if((itPreLong != longChannel.begin()) && (itPosLong != longChannel.end())){
				// 
			}

			if(itPreLong->type == itPosLong->type){
				index --;
				PulseDesc timeInsert(itPreLong->channelID, itPreLong->start, itPreLong->end, itPreLong->type, index);
				newChannel.pop_back();
				newChannel.push_back(timeInsert);
			}else{
				inter_log(Error, "pre and post type is not equal.");
			}
		}else{
			itLong->index = index;
			newChannel.push_back(*itLong);
		}

		index++;

		ReportProgress(itLong->index, longChannel.size());

		itLong++;
	}

	longChannel.clear();
	longChannel = newChannel;
}


BOOL PulseAnalyzer::DetectPulseWidth(double &duration)
{
	int32_t exceptNextType = 0;
	int32_t continueCount = 0;
	int32_t curFrameType = 0;
	double durationSum = 0.0f; // sum of list suitable duration
	double frameDuration = 0.0f; // 
	int32_t durationCount = 0;
	BOOL bRet = FALSE;

	std::list<PulseDesc>::iterator itLong;
	std::list<PulseDesc>::iterator itShort;
	std::list<PulseDesc> shortChannel; // short list
	std::list<PulseDesc> longChannel; // long list

	int32_t step = 1;

	if (mPulseList[LCHANNEL].size() >= mPulseList[RCHANNEL].size()){
		longChannel = mPulseList[LCHANNEL];
		shortChannel = mPulseList[RCHANNEL];
	} else{
		longChannel = mPulseList[RCHANNEL];
		shortChannel = mPulseList[LCHANNEL];
	}

	itLong = longChannel.begin();
	itShort = shortChannel.begin();

	for(; (itLong != longChannel.end()) && (itShort != shortChannel.end()); )
	{
		curFrameType = GetPulseType(itShort->type, itLong->type);
		if(curFrameType == exceptNextType){
			continueCount++;
			frameDuration += itLong->duration;
			exceptNextType = (exceptNextType+1)%PULSETABLECOUNT;

			if(continueCount == PULSETABLECOUNT){
				durationCount += PULSETABLECOUNT;
				durationSum += frameDuration;
				frameDuration = 0.0f;
				continueCount = 0;
			}
		}else{
			continueCount = 0;
			exceptNextType = (curFrameType+1)%PULSETABLECOUNT; // restart calculate
			frameDuration = 0.0f;
		}

		ReportProgress(itLong->index, longChannel.size());

		itLong++;
		
		if(!(step % 2)){
			step = 0;
			itShort++;
		}
		step++;
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

void PulseAnalyzer::GetFrameInfoByStartTime(double &pulseDuration)
{
	int32_t curFrameType = 0;
	double fps = 0.0f;
	double stdevp = 0.0f;
	int32_t index = 0;

	std::list<PulseDesc>::iterator itLong;
	std::list<PulseDesc>::iterator itShort;
	std::list<PulseDesc> shortChannel; // short list
	std::list<PulseDesc> longChannel; // long list

	int32_t step = 1;

	if (mPulseList[LCHANNEL].size() >= mPulseList[RCHANNEL].size()){
		longChannel = mPulseList[LCHANNEL];
		shortChannel = mPulseList[RCHANNEL];
	} else{
		longChannel = mPulseList[RCHANNEL];
		shortChannel = mPulseList[LCHANNEL];
	}

	itLong = longChannel.begin();
	itShort = shortChannel.begin();

	for(; (itLong != longChannel.end()) && (itShort != shortChannel.end()); )
	{
		curFrameType = GetPulseType(itShort->type, itLong->type);
		{
			fps = CalcFpsInOneSecond(mFramePulse);
			double avg = 0.0f;
			avg = CalcAvgValueInOneSecond(mFramePulse);
			stdevp = CalcSTDEVPInOneSecond(mFramePulse, avg);
			
			if(!mFramePulse.empty()){
				double pre_start = mFramePulse.back().end;
				FrameDesc frame(curFrameType, pre_start, itLong->start, fps, avg, stdevp, index++);
				mFramePulse.push_back(frame);
			}else{
				FrameDesc frame(curFrameType, 0, itLong->start, fps, avg, stdevp, index++);
				mFramePulse.push_back(frame);
			}

			fps = stdevp = 0.0f;
		}

		ReportProgress(itLong->index, longChannel.size());

		itLong++;

		if(!(step % 2)){
			step = 0;
			itShort++;
		}
		step++;
	}
}

void PulseAnalyzer::GetFrameInfoByChannel(double &duration)
{
	int32_t curFrameType = 0;
	double fps = 0.0f;
	double stdevp = 0.0f;
	double avg = 0.0f;
	int32_t index = 0;
	std::list<PulseDesc>::iterator itLong;
	std::list<PulseDesc> longChannel;

	if (mPulseList[LCHANNEL].size() >= mPulseList[RCHANNEL].size()){
		longChannel = mPulseList[LCHANNEL];
	}else{
		longChannel = mPulseList[RCHANNEL];
	}

	itLong = longChannel.begin();

	for(; itLong != longChannel.end(); )
	{
		//curFrameType = GetPulseType(itShort->type, itLong->type);
		{
			if(index == 114){
				inter_log(Info, "teste");
			}
			fps = CalcFpsInOneSecond(mFramePulse);
			avg = CalcAvgValueInOneSecond(mFramePulse);
			stdevp = CalcSTDEVPInOneSecond(mFramePulse, avg);

			if(!mFramePulse.empty()){
				double pre_start = mFramePulse.back().end;
				FrameDesc frame(curFrameType, pre_start, itLong->start, fps, avg, stdevp, index++);
				mFramePulse.push_back(frame);
			}else{
				FrameDesc frame(curFrameType, 0, itLong->start, fps, avg, stdevp, index++);
				mFramePulse.push_back(frame);
			}

			fps = stdevp = 0.0f;
		}

		ReportProgress(itLong->index, longChannel.size());

		itLong++;
	}
}

void PulseAnalyzer::GetFrameInfoByDuration(double &pulseDuration)
{
	int32_t curFrameType = 0;
	double fps = 0.0f;
	double stdevp = 0.0f;
	int32_t index = 0;

	std::list<PulseDesc>::iterator itLong;
	std::list<PulseDesc>::iterator itShort;
	std::list<PulseDesc> shortChannel; // short list
	std::list<PulseDesc> longChannel; // long list

	int32_t step = 1;

	if (mPulseList[LCHANNEL].size() >= mPulseList[RCHANNEL].size()){
		longChannel = mPulseList[LCHANNEL];
		shortChannel = mPulseList[RCHANNEL];
	} else{
		longChannel = mPulseList[RCHANNEL];
		shortChannel = mPulseList[LCHANNEL];
	}

	itLong = longChannel.begin();
	itShort = shortChannel.begin();

	for(; (itLong != longChannel.end()) && (itShort != shortChannel.end()); )
	{
		curFrameType = GetPulseType(itShort->type, itLong->type);
		{
			fps = CalcFpsInOneSecond(mFramePulse);
			double avg = 0.0f;
			stdevp = CalcSTDEVPInOneSecond(mFramePulse, avg);
			FrameDesc frame(curFrameType, itLong->start, itLong->end, fps, avg, stdevp, index++);
			mFramePulse.push_back(frame);
			fps = stdevp = 0.0f;
		}

		ReportProgress(itLong->index, longChannel.size());
		
		itLong++;

		if(!(step % 2)){
			step = 0;
			itShort++;
		}
		step++;
	}
}
/*
void PulseAnalyzer::GetFrameInfo(double &pulseDuration)
{
	int32_t curFrameType = 0;
	double referenceTimeBase = 0.0f;
	double fps = 0.0f;
	double MSE = 0.0f;
	int32_t index = 0;
	int32_t lstep = 1;
	int32_t rstep = 1;

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

			if(pulseDuration > 0.0f){
				if((fabs(lBak.duration - pulseDuration) < 0.005) || (lBak.duration < 0.0f)){ // 5ms 
					lit++;
					if(lit != mPulseList[LCHANNEL].end())
						lBak = *lit;
				}else{
					lBak.start += pulseDuration;
					lBak.duration -= pulseDuration;
				}

				if((fabs(rBak.duration - pulseDuration) < 0.005) || (rBak.duration < 0.0f)){ // 5ms
					rit++;
					if(rit != mPulseList[RCHANNEL].end())
						rBak = *rit;
				}else{
					rBak.start += pulseDuration;
					rBak.duration -= pulseDuration;
				}
			}else{
				lit++;
				if(lit != mPulseList[LCHANNEL].end())
					lBak = *lit;

				rit++;
				if(rit != mPulseList[RCHANNEL].end())
					rBak = *rit;
			}
		}
	}
}
*/
void PulseAnalyzer::WriteRawPulseDetail()
{
	std::list<PulseDesc>::iterator lit;
	std::list<PulseDesc>::iterator rit;

	std::string filePath = mSourceFileName + ".raw.pulse.csv";
	CSVFile file(filePath);

	std::list<PulseDesc> channel = mPulseList[LCHANNEL].size() > mPulseList[RCHANNEL].size()? mPulseList[LCHANNEL] : mPulseList[RCHANNEL];

	file.WriteCsvLine(" channel, index, start, end, duration, type, channel, index, start, end, duration, type, ");

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
				" %c, %d, %.3f, %.3f, %.3f, %d, "
				" %c, %d, %.3f, %.3f, %.3f, %d, ",
				lPulse.channelName, lPulse.index, lPulse.start, lPulse.end, lPulse.duration, lPulse.type, 
				rPulse.channelName, rPulse.index, rPulse.start, rPulse.end, rPulse.duration, rPulse.type);
		}else if(!lPulse.IsInvalid()){
			file.WriteCsvLine(
				" %c, %d, %.3f, %.3f, %.3f, %d, "
				" ,,,,,, ",
				lPulse.channelName, lPulse.index, lPulse.start, lPulse.end, lPulse.duration, lPulse.type);
		}else if(!rPulse.IsInvalid()){
			file.WriteCsvLine(
				" ,,,,,, "
				" %c, %d, %.3f, %.3f, %.3f, %d, ",
				rPulse.channelName, rPulse.index, rPulse.start, rPulse.end, rPulse.duration, rPulse.type);
		}

		ReportProgress((lPulse.IsInvalid()?rPulse.index:lPulse.index), channel.size());
	}
}

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
	
	file.WriteCsvLine("sync, channel 1, index, start, end, duration, type, interval, "
		"channel 2, index, start, end, duration, type, interval, ");

	while(1){
		PulseDesc shortPulse, longPulse;
		secondDiff = 0.0f;
		firstDiff = 0.0f;

		if((itShort != shortChannel.end()) && (itLong != longChannel.end())){
			// find most suitable pulse
			sync = 0;

			itLongNext = itLong;
			itLongNext++;

			firstDiff = (itShort->start - itLong->start)*1000;
			if(itLongNext != longChannel.end())
				secondDiff = (itShort->start - itLongNext->start)*1000;

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
				"%c, %u, %.3f, %.3f, %.3f, %d, %d, "
				"%c, %u, %.3f, %.3f, %.3f, %d, %d, ",
				sync,
				shortPulse.channelName, shortPulse.index, shortPulse.start, shortPulse.end, shortPulse.duration, shortPulse.type, 0, 
				longPulse.channelName, longPulse.index, longPulse.start, longPulse.end, longPulse.duration, longPulse.type, 0);
			itShort++;
			itLong++;
		}else if (!longPulse.IsInvalid()){
			file.WriteCsvLine(", "
				" ,  ,  ,  ,  , , , "
				"%c, %u, %.3f, %.3f, %.3f, %d, %d,",
				longPulse.channelName, longPulse.index, longPulse.start, longPulse.end, longPulse.duration, longPulse.type, 0);
			itLong++;
		}else if(!shortPulse.IsInvalid()){
			file.WriteCsvLine(", "
				"%c, %u, %.3f, %.3f, %.3f, %d, %d,"
				", , , , , , , ",
				shortPulse.channelName, shortPulse.index, shortPulse.start, shortPulse.end, shortPulse.duration, shortPulse.type, 0);
			itShort++;
		}

		ReportProgress(longPulse.index, longChannel.size());
	}
}

void PulseAnalyzer::WriteSmoothDetail()
{
	double stdevp = 0.0f;
	double fps = 0.0f;
	std::string filePath = mSourceFileName + ".smooth.csv";

	if(!mFramePulse.empty()){
		CSVFile file(filePath);

		double avg = CalcAvgValue(mFramePulse);
		stdevp = CalcSTDEVP(mFramePulse, avg);
		fps = CalcFps(mFramePulse);
		file.WriteCsvLine("STDEVP, FPS, Avg, Frames, Duration, ");
		file.WriteCsvLine("%.3f, %.3f, %.3f, %d, %.3f,", stdevp, fps, avg, mFramePulse.size(), (mFramePulse.back().end - mFramePulse.front().start)*1000);

		file.WriteCsvLine("");
		file.WriteCsvLine("All data in millisecond,");
		file.WriteCsvLine("");
		file.WriteCsvLine("Index, Start, End, Duration, Average, Delta, STDEVP, FPS, Type,");
		while(!mFramePulse.empty()){
			FrameDesc frame = mFramePulse.front();
			file.WriteCsvLine("%d, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %d, "
				, frame.index, frame.start, frame.end, frame.duration, frame.AVG, frame.offset, frame.STDEVP, frame.frameRate, frame.frameType);
			mFramePulse.pop_front();

			ReportProgress(frame.index, mFramePulse.size());
		}
		
	}
}

void PulseAnalyzer::OutputResult()
{
	double pulseWidth = 0.0f;
	inter_log(Info, "Write Raw Data... ");
	WriteRawPulseDetail();

	PulseFilter();

	inter_log(Info, "Detect Pulse Width... ");
	DetectPulseWidth(pulseWidth);
	inter_log(Info, "Detect Frame Info... ");
	//GetFrameInfoByStartTime(pulseWidth);
	GetFrameInfoByChannel(pulseWidth);
	inter_log(Info, "Write Sync Data... ");
	WriteSyncDetail();
	inter_log(Info, "Write Smooth Data... ");
	WriteSmoothDetail();
	inter_log(Info, "end.");
}
