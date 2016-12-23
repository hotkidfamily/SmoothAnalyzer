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
: mChannelOffset(0)
, mSourceFileName(filename)
{
	SYSTEMTIME systime;
	char buffer[256] = {'\0'};
	GetLocalTime(&systime);
	sprintf_s(buffer, 256-1, "-%04d-%02d%02d-%02d%02d-%02d", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

	mSourceFileName += buffer;

	ZeroMemory(mFrameHistograms, sizeof(mFrameHistograms));

}

PulseAnalyzer::~PulseAnalyzer(void)
{
}

void PulseAnalyzer::ReportProgress(int32_t progress, int32_t total)
{
	if(total)
		fprintf(stderr, "\t progress %.3f\r", progress*100.0 / total);
}

void PulseAnalyzer::SetAnalyzerData(std::list<PulseDesc>* dataPtr)
{
	mPulseList[LCHANNEL] = dataPtr[LCHANNEL];
	mPulseList[RCHANNEL] = dataPtr[RCHANNEL];
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

void PulseAnalyzer::MergeOffset()
{
	std::list<PulseDesc> &longChannel = mPulseList[LCHANNEL].size()>mPulseList[RCHANNEL].size()?mPulseList[LCHANNEL]:mPulseList[RCHANNEL];
	std::list<PulseDesc>::iterator it = longChannel.begin();

	while(it != longChannel.end()){
		it->start += mChannelOffset;
		it->end += mChannelOffset;
		it++;
	}
}

void PulseAnalyzer::PulseFilter()
{
	PulseLowFilter(mPulseList[LCHANNEL]);
	PulseLowFilter(mPulseList[RCHANNEL]);

	// merge offfset
	if(fabs(mChannelOffset) >= 0.0001f){
		MergeOffset();
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

			if((itPreLong == longChannel.begin()) || (itPosLong == longChannel.end())){
				// 
				inter_log(Error, "pulse duration is to small at begin or end.");
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

void PulseAnalyzer::GetFrameInfoByChannel(const double &duration)
{
	int32_t curFrameType = 0;
	double fps = 0.0f;
	double stdevp = 0.0f;
	double avg = 0.0f;
	int32_t index = 0;
	std::list<PulseDesc>::iterator rChannelIT;
	std::list<PulseDesc>::iterator lChannelIT;
	std::list<PulseDesc> &lChannel = mPulseList[LCHANNEL];
	std::list<PulseDesc> &rChannel = mPulseList[RCHANNEL];

	lChannelIT = lChannel.begin();
	rChannelIT = rChannel.begin();

	for(; lChannelIT != lChannel.end(); )
	{
		curFrameType = GetPulseType(lChannelIT->type, rChannelIT->type);
		{
			mStdevpAlgorithm.CalcAvgStdAndFps(mFramePulse, avg, stdevp, fps);

			if(!mFramePulse.empty()){
				double pre_start = mFramePulse.back().end;
				FrameDesc frame(curFrameType, pre_start, rChannelIT->start, fps, avg, stdevp, index++);
				mFramePulse.push_back(frame);
			}else{
				FrameDesc frame(curFrameType, 0, rChannelIT->start, fps, avg, stdevp, index++);
				mFramePulse.push_back(frame);
			}

			fps = stdevp = 0.0f;
		}

		ReportProgress(lChannelIT->index, rChannel.size());

		rChannelIT++;
		lChannelIT++;
	}
}


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
void PulseAnalyzer::ProcessSyncDetail(double pulseWidth)
{
	std::list<PulseDesc> shortChannel; // short list
	std::list<PulseDesc> longChannel; // long list

	std::list<PulseDesc> lChannel; // new left channel list
	std::list<PulseDesc> rChannel; // new right channel list

	int32_t sync = 0;
	double firstDiff = 0.0f;
	double secondDiff = 0.0f;
	double firstSLdiff = 0.0f;
	double secondSLdiff = 0.0f;
	bool bShouldDrop = false;

	std::list<PulseDesc>::iterator itShort;
	std::list<PulseDesc>::iterator itShortNext;
	std::list<PulseDesc>::iterator itLong;
	std::list<PulseDesc>::iterator itLongNext;
	pulseWidth /= 1000;

	if (mPulseList[LCHANNEL].size() >= mPulseList[RCHANNEL].size()){
		longChannel = mPulseList[LCHANNEL];
		shortChannel = mPulseList[RCHANNEL];
	} else{
		longChannel = mPulseList[RCHANNEL];
		shortChannel = mPulseList[LCHANNEL];
	}

	itShort = shortChannel.begin();
	itLong = longChannel.begin();

	while(1){
		PulseDesc shortPulse, longPulse;
		bShouldDrop = false;
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
			itShortNext = itShort;
			itShortNext++;
			if(itShortNext != shortChannel.end())
				secondSLdiff = (itShortNext->start - itLong->start)*1000;
			
			if(firstDiff == 0.0f){
				longPulse = *itLong;
				shortPulse = *itShort;
				sync = (int32_t)firstDiff;
			} else if(secondDiff == 0.0f){
				longPulse = *itLong;
			}else if(firstDiff > 0 && secondDiff < 0){
				longPulse = *itLong;
				if((fabs(firstDiff) <= fabs(secondDiff)) && (fabs(firstDiff) < SYNC_THRESHOLD)){
					shortPulse = *itShort;
					sync = (int32_t)firstDiff;
				}
			} else if (firstDiff > 0 && secondDiff > 0){
				if(firstDiff >= secondDiff){
					longPulse = *itLong;
				}else{
					inter_log(Error, "can not happend 1.");
				}
			}else if(firstDiff < 0 && secondDiff > 0){
				inter_log(Error, "can not happend 2.");
			}else if(firstDiff < 0 && secondDiff < 0){
				if(firstDiff < secondDiff){
					inter_log(Error, "can not happend 3.");
				}else{
					// go to next short list element
					shortPulse = *itShort;
					if(fabs(secondSLdiff) > fabs(firstDiff)){
						if(fabs(firstDiff) < SYNC_THRESHOLD){
							longPulse = *itLong;
							sync = (int32_t)firstDiff;
						}
					}else{
						//inter_log(Info, "debg");
					}
					
				}
			}			
		}else{ // drop all reset samples.
			break;
		}

		if(!longPulse.IsInvalid()){
			itLong++;
		}else{
			if(itLong != longChannel.begin()){
				itLong--;
				longPulse = *itLong;
				itLong++;
			}
		}

		rChannel.push_back(longPulse);

		if(!shortPulse.IsInvalid()){
			PulseDesc pos(shortPulse.channelID, shortPulse.start, shortPulse.end, shortPulse.type, longPulse.index);
			lChannel.push_back(pos);
			itShort++;
		}else{
			if(itShort != shortChannel.begin()){
				itShort --;
				shortPulse = *itShort;
				itShort++;
				shortPulse.start += pulseWidth;
			}
			PulseDesc pos(shortPulse.channelID, shortPulse.start, shortPulse.end, shortPulse.type, longPulse.index);
			lChannel.push_back(pos);
		}

		ReportProgress(longPulse.index, longChannel.size());
	}

	mPulseList[LCHANNEL].clear();
	mPulseList[LCHANNEL] = lChannel;
	mPulseList[RCHANNEL].clear();
	mPulseList[RCHANNEL] = rChannel;
}

void PulseAnalyzer::WriteSyncDetail()
{
	int32_t sync = 0;
	std::list<PulseDesc>::iterator lit = mPulseList[LCHANNEL].begin();
	std::list<PulseDesc>::iterator rit = mPulseList[RCHANNEL].begin();

	std::string filePath = mSourceFileName + ".sync.detail.csv";
	CSVFile file(filePath);


	file.WriteCsvLine("Left Right Sync Detail.,");

	file.WriteCsvLine("sync, channel 1, index, start, end, duration, type, interval, "
		"channel 2, index, start, end, duration, type, interval, ");

	while((lit != mPulseList[LCHANNEL].end()) && (rit != mPulseList[RCHANNEL].end()))
	{
		sync = (int32_t)((lit->start - rit->start)*1000);

		file.WriteCsvLine("%d, "
			"%c, %u, %.3f, %.3f, %.3f, %d, %d, "
			"%c, %u, %.3f, %.3f, %.3f, %d, %d, ",
			sync,
			lit->channelName, lit->index, lit->start, lit->end, lit->duration, lit->type, 0, 
			rit->channelName, rit->index, rit->start, rit->end, rit->duration, rit->type, 0);

		ReportProgress(lit->index, mPulseList[LCHANNEL].size());

		lit++;
		rit++;
	}
}


void PulseAnalyzer::HistogramInfo(const double &pulseWidth)
{
	std::list<FrameDesc>::iterator it = mFramePulse.begin();
	int32_t NormalLevel = PULSE_LEVEL(pulseWidth);

	inter_log(Info, "Normal Level is %d.", NormalLevel);

	while(it != mFramePulse.end()){
		if((it->level < SYSTEM_RESOLUTION) && (it->level>0)){
			mFrameHistograms[it->level]++;
		}else{
			mFrameHistograms[BAD_RESOLUTION]++;
		}

		if(it->level == NormalLevel){
			mFrameHistograms[NORMATL_RESOLUTION]++;
		} 

		mFrameHistograms[TOTAL_RESOLUTION]++;

		it++;
	}
}

void PulseAnalyzer::JudgetDropFrame()
{
	int32_t exceptNextType = 0;
	int32_t curFrameType = 0;
	bool bSkip = false;

	std::list<FrameDesc>::iterator it;
	std::list<FrameDesc>::iterator itPre;
	std::list<FrameDesc> &channel = mFramePulse;

	it = channel.begin();
	exceptNextType = it->frameType;

	for(; it != channel.end(); )
	{
		curFrameType = it->frameType;
		if(curFrameType == exceptNextType){
			exceptNextType = (exceptNextType+1)%PULSETABLECOUNT;
		}else{
			exceptNextType = (curFrameType+1)%PULSETABLECOUNT; // restart calculate
			itPre = it;
			itPre--;

			{
				int32_t drops = abs(exceptNextType - curFrameType)%PULSETABLECOUNT;
				double threashold = drops*MINIST_PULSE_DURATION;
				double duration = abs((it->start - itPre->start)*1000);

				if(duration < threashold){
					mFrameHistograms[BAD_RESOLUTION]++;
					bSkip = true;
				}else{
					mFrameHistograms[DROP_RESOLUTION] += drops;
				}
			}
		}

		if(!bSkip){
			if(it->IsLevelInvalid()){
				mFrameHistograms[BAD_RESOLUTION]++;
			}
		}

		bSkip = false;

		ReportProgress(it->index, channel.size());

		it++;
	}

	//inter_log(Error, "drop %d frames, bad %d frames.\n", dropFrame, badFrame);
}

void PulseAnalyzer::AnalyzerSmoooth(const double &pulseWidth)
{
	HistogramInfo(pulseWidth);
	JudgetDropFrame();
}

void PulseAnalyzer::WriteSmoothDetail()
{
	double stdevp = 0.0f;
	double fps = 0.0f;
	std::string filePath = mSourceFileName + ".smooth.csv";

	if(!mFramePulse.empty()){
		CSVFile file(filePath);

		double avg = mStdevpAlgorithm.CalcAvgValue(mFramePulse);
		stdevp = mStdevpAlgorithm.CalcSTDEVP(mFramePulse, avg);
		fps = mStdevpAlgorithm.CalcFps(mFramePulse);
		file.WriteCsvLine("STDEVP, FPS, Avg, Frames, Duration, ");
		file.WriteCsvLine("%.3f, %.3f, %.3f, %d, %.3f,", stdevp, fps, avg, mFramePulse.size(), (mFramePulse.back().end - mFramePulse.front().start)*1000);

		file.WriteCsvLine(",");

		file.WriteCsvLine("Total, Normal, Percent, 1Pulse, 2Pulse, 3Pulse, Bad, Drops,");
		file.WriteCsvLine("%u, %u,"
			"%.3f, "
			" %u, %u, %u, %u, %u,", 
			mFrameHistograms[TOTAL_RESOLUTION], mFrameHistograms[NORMATL_RESOLUTION], 
			100.0 * mFrameHistograms[NORMATL_RESOLUTION]/mFrameHistograms[TOTAL_RESOLUTION],
			mFrameHistograms[1], mFrameHistograms[2], mFrameHistograms[3], 
			mFrameHistograms[BAD_RESOLUTION], mFrameHistograms[DROP_RESOLUTION]);

		file.WriteCsvLine(",");
		file.WriteCsvLine("All data in millisecond,");
		file.WriteCsvLine(",");

		file.WriteCsvLine("Index, Start, End, Duration, Average, Delta, STDEVP, FPS, Type, level, ");
		while(!mFramePulse.empty()){
			FrameDesc frame = mFramePulse.front();
			file.WriteCsvLine("%d, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %d, %d, "
				, frame.index, frame.start, frame.end, frame.duration, frame.AVG, frame.offset, frame.STDEVP, frame.frameRate, frame.frameType, frame.level);
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

	inter_log(Info, "Filter Raw Pulse... ");
	PulseFilter();

	WriteRawPulseDetail();

	inter_log(Info, "Detect Pulse Width... ");
	DetectPulseWidth(pulseWidth);

	inter_log(Info, "Process Sync Data... ");
	ProcessSyncDetail(pulseWidth);

	inter_log(Info, "Write Sync Data... ");
	WriteSyncDetail();

	inter_log(Info, "Detect Frame Info... ");
	GetFrameInfoByChannel(pulseWidth);

	AnalyzerSmoooth(pulseWidth);
	
	inter_log(Info, "Write Smooth Data... ");
	WriteSmoothDetail();

	inter_log(Info, "end.");
}
