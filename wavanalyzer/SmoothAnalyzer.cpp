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
	SYSTEMTIME systime;
	char buffer[256] = {'\0'};
	GetLocalTime(&systime);
	sprintf_s(buffer, 256-1, "-%04d-%02d%02d-%02d%02d-%02d", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

	mWorkParams.mSourceFileName = filename + buffer;

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

void PulseAnalyzer::SetWorkingParam(ANALYZER_PARAMS &params)
{
	mWorkParams.pulseWidth = params.pulseWidth;
	mWorkParams.channelOffset = params.channelOffset;
}

void PulseAnalyzer::SetAnalyzerData(PulseList* dataPtr)
{
	mPulseList[LCHANNEL] = dataPtr[LCHANNEL];
	mPulseList[RCHANNEL] = dataPtr[RCHANNEL];

	// source pulse have different frequency, move high frequency to "R" channel
	if(dataPtr[LCHANNEL].size() > dataPtr[RCHANNEL].size()){
		std::swap(mPulseList[LCHANNEL], mPulseList[RCHANNEL]);
	}
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
	PulseList &adjustChannel = mPulseList[RCHANNEL];
	PulseList::iterator it = adjustChannel.begin();

	if(fabs(mWorkParams.channelOffset) < 0.0001f){
		return ;
	}

	while(it != adjustChannel.end()){
		it->start += mWorkParams.channelOffset;
		it->end += mWorkParams.channelOffset;
		it++;
	}
}

void PulseAnalyzer::PulseLowFilter(PulseList &channelPulse)
{
	PulseList::iterator it;
	PulseList::iterator itPre;
	PulseList::iterator itPos;
	PulseList &sourceChannel = channelPulse;
	PulseList newChannel;

	it = sourceChannel.begin();
	for(int32_t index = 0; it != sourceChannel.end(); index++)
	{
		if(!it->IsPulseInvalid()){
			// link the sample before and after together
			itPre = it;
			itPos = it;
			itPre --;
			itPos ++;

			if((itPre == sourceChannel.begin()) || (itPos == sourceChannel.end())){
				// 
				inter_log(Error, "pulse duration is to small at begin or end.");
			}

			if(itPre->type == itPos->type){
				index --;
				PulseDesc timeInsert(itPre->channelID, itPre->start, itPre->end, itPre->type, index);
				newChannel.pop_back();
				newChannel.push_back(timeInsert);
			}else{
				inter_log(Error, "pre and post type is not equal.");
			}
		}else{
			it->index = index;
			newChannel.push_back(*it);
		}

		ReportProgress(it->index, sourceChannel.size());

		it++;
	}

	sourceChannel.assign(newChannel.begin(), newChannel.end());
}

void PulseAnalyzer::PulseFilter()
{
	inter_log(Info, "Filter Raw Pulse... ");

	for(int32_t i =0; i<MAX_CHANNEL; i++){
		PulseLowFilter(mPulseList[i]);
	}
}

BOOL PulseAnalyzer::GetPulseWidthByInput(double &duration)
{
	BOOL bRet = TRUE;
	bRet = (mWorkParams.pulseWidth > 0.1);
	if(bRet)
		duration = mWorkParams.pulseWidth;

	return bRet;
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

	PulseList::iterator itLong;
	PulseList::iterator itShort;
	PulseList &longChannel = mPulseList[RCHANNEL];
	PulseList &shortChannel = mPulseList[LCHANNEL];

	int32_t step = 1;

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
	}

	return bRet;
}

BOOL PulseAnalyzer::GetPulseWidth(double &duration)
{
	BOOL bRet = FALSE;

	inter_log(Info, "Calculate Pulse Width... ");

	bRet = GetPulseWidthByInput(duration);
	if(!bRet){
		bRet = DetectPulseWidth(duration);
	}

	inter_log(Info, "Frame duration %.3f ms", duration);

	return bRet;
}



void PulseAnalyzer::GetFrameInfoByChannel(const double &duration)
{
	int32_t curFrameType = 0;
	double fps = 0.0f;
	double stdevp = 0.0f;
	double avg = 0.0f;
	int32_t index = 0;
	PulseList::iterator rChannelIT;
	PulseList::iterator lChannelIT;
	PulseList &lChannel = mPulseList[LCHANNEL];
	PulseList &rChannel = mPulseList[RCHANNEL];

	lChannelIT = lChannel.begin();
	rChannelIT = rChannel.begin();

	inter_log(Info, "Detect Frame Info... ");

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
	PulseList::iterator lit;
	PulseList::iterator rit;
	std::string filePath = mWorkParams.mSourceFileName + ".raw.pulse.csv";
	CSVFile file(filePath);

	inter_log(Info, "Write Raw Data... ");

	file.WriteCsvLine(" channel, index, start, end, duration, type, channel, index, start, end, duration, type, ");

	lit = mPulseList[LCHANNEL].begin();
	rit = mPulseList[RCHANNEL].begin();

	while((lit != mPulseList[LCHANNEL].end()) && (rit!=mPulseList[RCHANNEL].end()))
	{
		file.WriteCsvLine(
			" %c, %d, %.3f, %.3f, %.3f, %d, "
			" %c, %d, %.3f, %.3f, %.3f, %d, ",
			lit->channelName, lit->index, lit->start, lit->end, lit->duration, lit->type, 
			rit->channelName, rit->index, rit->start, rit->end, rit->duration, rit->type);

		lit++;
		rit++;
	}

	while(lit!= mPulseList[LCHANNEL].end()){
		file.WriteCsvLine(
			" %c, %d, %.3f, %.3f, %.3f, %d, "
			" ,,,,,, ",
			lit->channelName, lit->index, lit->start, lit->end, lit->duration, lit->type);
		lit++;
	}

	while(rit != mPulseList[RCHANNEL].end()){
		file.WriteCsvLine(
			" ,,,,,, "
			" %c, %d, %.3f, %.3f, %.3f, %d, ",
			rit->channelName, rit->index, rit->start, rit->end, rit->duration, rit->type);
		rit++;
	}
}

/* compare short list to long list and write value */ 
void PulseAnalyzer::ProcessChannelsSync(double pulseWidth)
{
	PulseList shortChannel; // short list
	PulseList longChannel; // long list

	PulseList lChannel; // new left channel list
	PulseList rChannel; // new right channel list

	double firstDiff = 0.0f;
	double secondDiff = 0.0f;
	double secondSLdiff = 0.0f;

	PulseList::iterator itShort;
	PulseList::iterator itShortNext;
	PulseList::iterator itLong;
	PulseList::iterator itLongNext;

	inter_log(Info, "Process Sync Data... ");

	shortChannel = mPulseList[LCHANNEL];
	longChannel = mPulseList[RCHANNEL];

	itShort = shortChannel.begin();
	itLong = longChannel.begin();

	while(1){
		PulseDesc shortPulse, longPulse;
		secondDiff = 0.0f;
		firstDiff = 0.0f;
		secondSLdiff = 0.0f;

		if((itShort != shortChannel.end()) && (itLong != longChannel.end())){
			// find most suitable pulse

			itLongNext = itLong;
			itLongNext++;

			firstDiff = (itShort->start - itLong->start);
			if(itLongNext != longChannel.end())
				secondDiff = (itShort->start - itLongNext->start);
			itShortNext = itShort;
			itShortNext++;
			if(itShortNext != shortChannel.end())
				secondSLdiff = (itShortNext->start - itLong->start);

			if(fabs(firstDiff) < SYNC_THRESHOLD){
				if(fabs(secondDiff) > fabs(firstDiff)){
					if(fabs(secondSLdiff) > fabs(firstDiff)){
						shortPulse = *itShort;
						longPulse = *itLong;
					}else{
						shortPulse = *itShort;
						if(itShortNext == shortChannel.end()){
							longPulse = *itLong;
						}
					}
				}else{
					longPulse = *itLong;
					if(itLongNext == longChannel.end()){
						shortPulse = *itShort;
					}
				}
			}else{
				if(firstDiff < 0.0f){
					shortPulse = *itShort;
				}else{
					longPulse = *itLong;
				}
			}
		}else{ // drop all rest samples.
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

	mPulseList[LCHANNEL].assign(lChannel.begin(), lChannel.end());
	mPulseList[RCHANNEL].assign(rChannel.begin(), rChannel.end());
}

void PulseAnalyzer::WriteSyncDetail()
{
	int32_t sync = 0;
	PulseList::iterator lit = mPulseList[LCHANNEL].begin();
	PulseList::iterator rit = mPulseList[RCHANNEL].begin();

	std::string filePath = mWorkParams.mSourceFileName + ".sync.detail.csv";
	CSVFile file(filePath);

	inter_log(Info, "Write Sync Data... ");

	file.WriteCsvLine("Left Right Sync Detail.,");

	file.WriteCsvLine("sync,"
		"channel, index, start, end, duration, type, interval, "
		"channel, index, start, end, duration, type, interval, ");

	while((lit != mPulseList[LCHANNEL].end()) && (rit != mPulseList[RCHANNEL].end()))
	{
		sync = (int32_t)(lit->start - rit->start);

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

/* compare short list to long list and write value */ 
void PulseAnalyzer::WriteRawSyncDetail()
{
	double sync = 0;
	double firstDiff = 0.0f;
	double secondDiff = 0.0f;
	double secondSLdiff = 0.0f;
	PulseList::iterator itShort;
	PulseList::iterator itShortNext;
	PulseList::iterator itLong;
	PulseList::iterator itLongNext;
	std::string filePath = mWorkParams.mSourceFileName + ".raw.sync.detail.csv";
	CSVFile file(filePath);

	PulseList &shortChannel = mPulseList[LCHANNEL];
	PulseList &longChannel = mPulseList[RCHANNEL];

	itShort = shortChannel.begin();
	itLong = longChannel.begin();

	file.WriteCsvLine("sync, "
		"channel, index, start, end, duration, type, interval, "
		"channel, index, start, end, duration, type, interval, ");

	while(1){
		PulseDesc shortPulse, longPulse;
		secondDiff = 0.0f;
		firstDiff = 0.0f;
		secondSLdiff = 0.0f;

		if((itShort != shortChannel.end()) && (itLong != longChannel.end())){
			// find most suitable pulse
			sync = 0;

			itLongNext = itLong;
			itLongNext++;

			firstDiff = itShort->start - itLong->start;
			if(itLongNext != longChannel.end())
				secondDiff = itShort->start - itLongNext->start;
			itShortNext = itShort;
			itShortNext++;
			if(itShortNext != shortChannel.end())
				secondSLdiff = itShortNext->start - itLong->start;

			if(fabs(firstDiff) < SYNC_THRESHOLD){
				if(fabs(secondDiff) > fabs(firstDiff)){
					if(fabs(secondSLdiff) > fabs(firstDiff)){
						shortPulse = *itShort;
						longPulse = *itLong;
						sync = firstDiff;
					}else{
						shortPulse = *itShort;
						if(itShortNext == shortChannel.end()){
							longPulse = *itLong;
							sync = firstDiff;
						}
					}
				}else{
					longPulse = *itLong;
					if(itLongNext == longChannel.end()){
						shortPulse = *itShort;
						sync = firstDiff;
					}
				}
			}else{
				if(firstDiff < 0.0f){
					shortPulse = *itShort;
				}else{
					longPulse = *itLong;
				}
			}
		}else{
			break;
		}
		
		if(!longPulse.IsInvalid() && !shortPulse.IsInvalid()){
			file.WriteCsvLine("%.3f, "
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


void PulseAnalyzer::HistogramInfo(const int32_t &NormalLevel, const double &pulseWidth)
{
	std::size_t i = 0;

	mFrameHistograms[FH_TOTAL] = mFramePulse.size();

	for(; i<mFramePulse.size();i++){
		if((mFramePulse[i].level < SYSTEM_RESOLUTION) && (mFramePulse[i].level>0)){
			mFrameHistograms[mFramePulse[i].level]++;
		}

		if(mFramePulse[i].level == NormalLevel){
			mFrameHistograms[FH_NORMAL]++;
		} 
	}
}

void PulseAnalyzer::JudgetDropFrame(const int32_t &NormalLevel)
{
	int32_t exceptNextType = 0;
	int32_t curFrameType = 0;
	std::size_t i = 0;

	FrameVector frameVect;
	frameVect.assign(mFramePulse.begin(), mFramePulse.end());

	exceptNextType = frameVect[0].frameType;

	while(i<frameVect.size()){
		curFrameType = frameVect[i].frameType;
		if(curFrameType == exceptNextType){
			exceptNextType = (exceptNextType+1)%PULSETABLECOUNT;
		}else{
			{
				int32_t drops = abs(exceptNextType - curFrameType)%PULSETABLECOUNT;
				double threashold = drops*MINIST_PULSE_DURATION;
				double duration = abs(frameVect[i].start - frameVect[i-1].start);

				// if the duration of frame is invalid
				if(duration >= threashold){
					mFrameHistograms[FH_DROP] += drops;
				}else{
					mFrameHistograms[FH_BAD]++;
				}
				if(frameVect[i].level == NormalLevel){
					mFrameHistograms[FH_NORMAL]--;
				}
			}
			exceptNextType = (curFrameType+1)%PULSETABLECOUNT; // restart calculate
		}

		ReportProgress(i, frameVect.size());
		i++;
	}
}

void PulseAnalyzer::AnalyzerSmoooth(const double &pulseWidth)
{
	int32_t NormalLevel = PULSE_LEVEL(pulseWidth);
	HistogramInfo(NormalLevel, pulseWidth);
	JudgetDropFrame(NormalLevel);
}

void PulseAnalyzer::WriteSmoothDetail()
{
	std::size_t i=0;
	std::string filePath = mWorkParams.mSourceFileName + ".smooth.csv";

	inter_log(Info, "Write Smooth Data... ");

	if(!mFramePulse.empty()){
		CSVFile file(filePath);

		double avg = mStdevpAlgorithm.CalcAvgValue(mFramePulse);
		double stdevp = mStdevpAlgorithm.CalcSTDEVP(mFramePulse, avg);
		//double fps = mStdevpAlgorithm.CalcFps(mFramePulse);
		double fps = (mFrameHistograms[FH_TOTAL] - mFrameHistograms[FH_BAD])*1000.0 / (mFramePulse.back().end - mFramePulse.front().start);
		file.WriteCsvLine("STDEVP, FPS, Avg, Frames, Duration, ");
		file.WriteCsvLine("%.3f, %.3f, %.3f, %d, %.3f,", stdevp, fps, avg, mFramePulse.size(), mFramePulse.back().end - mFramePulse.front().start);

		file.WriteCsvLine(",");

		file.WriteCsvLine("Total, Normal, Percent, 1Pulse, 2Pulse, 3Pulse, Bad, Drops,");
		file.WriteCsvLine("%u, %u,"
			"%.3f, "
			" %u, %u, %u, %u, %u,", 
			mFrameHistograms[FH_TOTAL], mFrameHistograms[FH_NORMAL], 
			100.0 * mFrameHistograms[FH_NORMAL]/mFrameHistograms[FH_TOTAL],
			mFrameHistograms[1], mFrameHistograms[2], mFrameHistograms[3], 
			mFrameHistograms[FH_BAD], mFrameHistograms[FH_DROP]);

		file.WriteCsvLine(",");
		file.WriteCsvLine("All data in millisecond,");
		file.WriteCsvLine(",");

		file.WriteCsvLine("Index, Start, End, Duration, Average, Delta, STDEVP, FPS, Type, level, ");
		while( i < mFramePulse.size()){
			FrameDesc frame = mFramePulse[i];

			file.WriteCsvLine("%d, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %d, %d, "
				, frame.index, frame.start, frame.end, frame.duration, frame.AVG, frame.offset, frame.STDEVP, frame.frameRate, frame.frameType, frame.level);

			ReportProgress(i, mFramePulse.size());
			i++;
		}
		
	}
}

void PulseAnalyzer::OutputResult()
{
	double pulseWidth = 0.0f;

	WriteRawPulseDetail();

	PulseFilter();

	MergeOffset();

	WriteRawSyncDetail();

	GetPulseWidth(pulseWidth);

	ProcessChannelsSync(pulseWidth);

	WriteSyncDetail();

	GetFrameInfoByChannel(pulseWidth);

	AnalyzerSmoooth(pulseWidth);
	
	WriteSmoothDetail();

	inter_log(Info, "end.");
}
