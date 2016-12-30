#include "stdafx.h"
#include "SmoothAnalyzer.h"
#include "libxllibrary.h"

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
#define INVALID_FRAMETYPE (-1)

#define RAW_PULSE_SHEET_NAME (_T("raw-pulse"))
#define SYNC_SHEET_NAME (_T("sync"))
#define RAW_SYNC_SHEET_NAME (_T("raw-sync"))
#define RESULT_SHEET_NAME (_T("Result"))
#define FRAME_SHEET_NAME (_T("frame-detail"))

PulseAnalyzer::PulseAnalyzer(STRING &filename)
:xlsMachine(new xlsOperator)
{
	SYSTEMTIME systime;
	TCHAR buffer[256] = {_T('\0')};
	GetLocalTime(&systime);
	_stprintf_s(buffer, 256-1, _T("-%04d-%02d%02d-%02d%02d-%02d"), systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

	mWorkParams.mSourceFileName = filename + buffer;

	ZeroMemory(mFrameHistograms, sizeof(mFrameHistograms));

	xlsMachine->CreateBook();
}

PulseAnalyzer::~PulseAnalyzer(void)
{
	xlsMachine->SaveAndCloseBook(mWorkParams.mSourceFileName + _T(".smooth.analyzer.xlsx"));
}

void PulseAnalyzer::SetWorkingParam(ANALYZER_PARAMS &params)
{
	mWorkParams.sampleFrameRate = params.sampleFrameRate;
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

	return INVALID_FRAMETYPE;
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
		if(!it->IsPulseValid()){
			// link the sample before and after together
			itPre = it;
			itPos = it;
			itPre --;
			itPos ++;

			if((itPre == sourceChannel.begin()) || (itPos == sourceChannel.end())){
				// 
				Logger(Error, "pulse duration is to small at begin or end.");
			}

			if(itPre->type == itPos->type){
				index --;
				PulseDesc timeInsert(itPre->channelID, itPre->start, itPos->end, itPre->type, index);
				it = itPos;
				Logger(Debug, "Merge (%.2f,%.2f) and (%.2f,%.2f) to (%.2f,%.2f)",
					itPre->start, itPre->end, itPos->start, itPos->end, timeInsert.start, timeInsert.end);
				newChannel.pop_back();
				newChannel.push_back(timeInsert);
			}else{
				Logger(Error, "Can not merge two pulse, pre and post type is not equal.");
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
	Logger(Info, "Filter Raw Pulse... ");

	for(int32_t i =0; i<MAX_CHANNEL; i++){
		PulseLowFilter(mPulseList[i]);
	}
}

BOOL PulseAnalyzer::GetPulseWidthByInput(double &duration)
{
	BOOL bRet = TRUE;
	bRet = (mWorkParams.sampleFrameRate > 0);
	if(bRet)
		duration = 1000.0/mWorkParams.sampleFrameRate;

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

	Logger(Info, "Calculate Pulse Width... ");

	bRet = GetPulseWidthByInput(duration);
	if(!bRet){
		bRet = DetectPulseWidth(duration);
	}

	Logger(Info, "Frame duration %.3f ms", duration);

	return bRet;
}

void PulseAnalyzer::WriteRawPulseDetail()
{
	PulseList::iterator lit;
	PulseList::iterator rit;
	int32_t rowIndex = 0;
	int32_t colIndex = 0;

	Logger(Info, "Write Raw Data... ");

	Sheet* rawPulseSheet = xlsMachine->CreateSheet(RAW_PULSE_SHEET_NAME);

	// write header 
	xlsMachine->WriteLineWithString(rawPulseSheet, rowIndex++, colIndex, _T(" channel, index, start, end, duration, type, channel, index, start, end, duration, type, "));

	lit = mPulseList[LCHANNEL].begin();
	rit = mPulseList[RCHANNEL].begin();

	while((lit != mPulseList[LCHANNEL].end()) && (rit!=mPulseList[RCHANNEL].end())){
		colIndex = 0;
		xlsMachine->WriteMultiplePulseAtRowCol(rawPulseSheet, rowIndex++, colIndex, &(*lit), &(*rit));

		lit++;
		rit++;
	}

	while(lit!= mPulseList[LCHANNEL].end()){
		colIndex = 5;
		xlsMachine->WriteMultiplePulseAtRowCol(rawPulseSheet, rowIndex++, colIndex, &(*lit), NULL);
		lit++;
	}

	while(rit != mPulseList[RCHANNEL].end()){
		colIndex = 0;
		xlsMachine->WriteMultiplePulseAtRowCol(rawPulseSheet, rowIndex++, colIndex, NULL, &(*rit));

		rit++;
	}
}

void PulseAnalyzer::WriteSyncDetail()
{
	int32_t sync = 0;
	int32_t row = 0; 
	int32_t col = 0;
	PulseList::iterator lit = mPulseList[LCHANNEL].begin();
	PulseList::iterator rit = mPulseList[RCHANNEL].begin();

	Logger(Info, "Write Sync Data... ");

	Sheet* syncSheet = xlsMachine->CreateSheet(SYNC_SHEET_NAME);
	
	xlsMachine->WriteLineWithString(syncSheet, row++, col, _T("sync,")
		_T("channel, index, start, end, duration, type, interval, ")
		_T("channel, index, start, end, duration, type, interval, "));

	while((lit != mPulseList[LCHANNEL].end()) && (rit != mPulseList[RCHANNEL].end())){
		col = 0;
		sync = (int32_t)(lit->start - rit->start);

		xlsMachine->Printf(syncSheet, row++, col, _T("%d, ")
			_T("%c, %u, %.3f, %.3f, %.3f, %d, %d, ")
			_T("%c, %u, %.3f, %.3f, %.3f, %d, %d, "),
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
	PulseList::iterator itShort;
	PulseList::iterator itLong;
	int32_t row = 0;
	int32_t col = 0;

	Sheet* rawSyncSheet = xlsMachine->CreateSheet(RAW_SYNC_SHEET_NAME);

	PulseList &shortChannel = mPulseList[LCHANNEL];
	PulseList &longChannel = mPulseList[RCHANNEL];

	itShort = shortChannel.begin();
	itLong = longChannel.begin();

	xlsMachine->WriteLineWithString(rawSyncSheet, row++, col, _T("sync, ")
		_T("channel, index, start, end, duration, type, interval, ")
		_T("channel, index, start, end, duration, type, interval, "));

	while(1)
	{
		PulseDesc shortPulse, longPulse;

		if((itShort != shortChannel.end()) && (itLong != longChannel.end())){
			// find most suitable pulse
			sync = 0;

			syncRet ret = ifStartSync(itShort,shortChannel.end(), itLong, longChannel.end());
			if(ret == ALLSYNC){
				shortPulse = *itShort;
				longPulse = *itLong;
				sync = shortPulse.start - longPulse.start;
			}else if(ret == LEFTAHEAD){
				shortPulse = *itShort;
			}else {
				longPulse = *itLong;
			}
		}else{
			break;
		}

		if(!longPulse.Empty() && !shortPulse.Empty()){
			col = 0;
			xlsMachine->Printf(rawSyncSheet, row++, col, 
				_T("%.3f, ")
				_T("%c, %u, %.3f, %.3f, %.3f, %d, %d, ")
				_T("%c, %u, %.3f, %.3f, %.3f, %d, %d, "),
				sync,
				shortPulse.channelName, shortPulse.index, shortPulse.start, shortPulse.end, shortPulse.duration, shortPulse.type, 0, 
				longPulse.channelName, longPulse.index, longPulse.start, longPulse.end, longPulse.duration, longPulse.type, 0);

			itShort++;
			itLong++;
		}else if (!longPulse.Empty()){
			col = 8;
			xlsMachine->Printf(rawSyncSheet, row++, col, _T("%c, %u, %.3f, %.3f, %.3f, %d, %d,"), 
				longPulse.channelName, longPulse.index, longPulse.start, longPulse.end, longPulse.duration, longPulse.type, 0);

			itLong++;
		}else if(!shortPulse.Empty()){
			col = 1;
			xlsMachine->Printf(rawSyncSheet, row++, col, _T("%c, %u, %.3f, %.3f, %.3f, %d, %d,"), 
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

void PulseAnalyzer::JudgetDropFrame(const int32_t &NormalLevel, const double &pulseWidth)
{
	int32_t exceptNextType = 0;
	int32_t shouldNextType = 0;
	int32_t curFrameType = 0;
	std::size_t i = 0;

	FrameVector frameVect;
	frameVect.assign(mFramePulse.begin(), mFramePulse.end());

	exceptNextType = frameVect[0].frameType;
	shouldNextType = exceptNextType;

	while(i<frameVect.size())
	{
		curFrameType = frameVect[i].frameType;
		if(curFrameType == exceptNextType){
			exceptNextType = (exceptNextType+1)%PULSETABLECOUNT;
		}else{
			{
				int32_t drops = abs(exceptNextType - curFrameType)%PULSETABLECOUNT;
				double threashold = drops*pulseWidth;
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

		shouldNextType = (shouldNextType+1)%PULSETABLECOUNT;
		ReportProgress(i, frameVect.size());
		i++;
	}
}

void PulseAnalyzer::AnalyzerSmoooth(const double &pulseWidth)
{
	int32_t NormalLevel = PULSE_LEVEL(pulseWidth);
	HistogramInfo(NormalLevel, pulseWidth);
	JudgetDropFrame(NormalLevel, pulseWidth);
}

void PulseAnalyzer::WriteSmoothDetail()
{
	int32_t row = 0;
	int32_t col = 0;

	if(!mFramePulse.empty())
	{
		Sheet* resultSheet = xlsMachine->InsertSheet(RESULT_SHEET_NAME);

		double avg = mStdevpAlgorithm.CalcAvgValue(mFramePulse);
		double stdevp = mStdevpAlgorithm.CalcSTDEVP(mFramePulse, avg);
		double fps = mStdevpAlgorithm.CalcFps(mFramePulse);
		double detectfps = (mFrameHistograms[FH_TOTAL] - mFrameHistograms[FH_BAD])*1000.0 / (mFramePulse.back().end - mFramePulse.front().start);
		xlsMachine->WriteLineWithString(resultSheet, row++, col, _T("STDEVP, Avg, Frames, Duration, "));
		xlsMachine->Printf(resultSheet, row++, col, _T("%.3f, %.3f, %d, %.3f, "),
			stdevp, avg, mFramePulse.size(), mFramePulse.back().end - mFramePulse.front().start);

		xlsMachine->WriteLineWithString(resultSheet, row++, col, _T("  , "));

		xlsMachine->WriteLineWithString(resultSheet, row++, col, _T("StaticFps, DetectFps, RealFps, "));
		xlsMachine->Printf(resultSheet, row++, col, _T("%.3f, %.3f, %.3f, "), fps, detectfps, mWorkParams.sampleFrameRate);

		xlsMachine->WriteLineWithString(resultSheet, row++, col, _T("  , "));

		xlsMachine->WriteLineWithString(resultSheet, row++, col, _T("Total, Normal, Percent, 1Pulse, 2Pulse, 3Pulse, Bad, Drops,"));
		xlsMachine->Printf(resultSheet, row++, col, _T("%u, %u,")
			_T("%.3f, ")
			_T(" %u, %u, %u, %u, %u,"),
			mFrameHistograms[FH_TOTAL], mFrameHistograms[FH_NORMAL], 
			100.0 * mFrameHistograms[FH_NORMAL]/mFrameHistograms[FH_TOTAL],
			mFrameHistograms[1], mFrameHistograms[2], mFrameHistograms[3], 
			mFrameHistograms[FH_BAD], mFrameHistograms[FH_DROP]);
	}
}

void PulseAnalyzer::WriteFrameDetail()
{
	std::size_t i=0;
	int32_t row = 0;
	int32_t col = 0;

	Logger(Info, "Write Smooth Data... ");

	if(!mFramePulse.empty()){
		Sheet *frameSheet = xlsMachine->CreateSheet(FRAME_SHEET_NAME);
		row = 0;
		col = 0;
		xlsMachine->WriteLineWithString(frameSheet, row++, col, _T("Index, Start, End, Duration, Average, Delta, STDEVP, FPS, Type, level, "));
		while( i < mFramePulse.size()){
			FrameDesc frame = mFramePulse[i];

			xlsMachine->Printf(frameSheet, row++, col, _T("%d, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %d, %d, "), 
				frame.index, frame.start, frame.end, frame.duration, frame.AVG, frame.offset, frame.STDEVP, frame.frameRate, frame.frameType, frame.level);

			ReportProgress(i, mFramePulse.size());
			i++;
		}	
	}
}

inline bool PulseAnalyzer::IsPosSync(const double &diff)
{
	return (fabs(diff) < SYNC_THRESHOLD);
}

/*
 * if left and right element sync
 *
 * return value:
 *				Zero - sync
 *				Negative - left ahead
 *				Positive - right
 */
syncRet PulseAnalyzer::ifStartSync(PulseList::iterator &left, PulseList::iterator &leftEnd, 
							  PulseList::iterator &right, PulseList::iterator &rightEnd)
{
	PulseList::iterator leftNext, rightNext;
	PulseDesc shortPulse, longPulse;
	syncRet ret = ALLSYNC;

	double firstDiff = 0.0f;
	double secondDiff = 0.0f;
	double secondSLdiff = 0.0f;

	rightNext = right;
	rightNext++;

	firstDiff = (left->start - right->start);
	if(rightNext != rightEnd)
		secondDiff = (left->start - rightNext->start);
	leftNext = left;
	leftNext++;
	if(leftNext != leftEnd)
		secondSLdiff = leftNext->start - right->start;

	if(IsPosSync(firstDiff)){
		if(fabs(secondDiff) > fabs(firstDiff)){
			if(fabs(secondSLdiff) > fabs(firstDiff)){
				shortPulse = *left;
				longPulse = *right;
			}else{
				shortPulse = *left;
				if(leftNext == leftEnd){
					longPulse = *right;
				}
			}
		}else{
			longPulse = *right;
			if(rightNext == rightEnd){
				shortPulse = *left;
			}
		}
	}else{
		if(firstDiff < 0){
			shortPulse = *left;
		}else{
			longPulse = *right;
		}
	}

	if(!shortPulse.Empty() && !longPulse.Empty()){
		ret = ALLSYNC;
	}else if (!longPulse.Empty()){
		ret = RIGHTAHEAD;
	}else {
		ret = LEFTAHEAD;
	}

	return ret;
}

inline bool IsBigger(double left, double right)
{
	return ((left - right) >= MINIST_PULSE_DURATION);
}

inline bool IsEqual(double left, double right)
{
	return (fabs(left - right) < MINIST_PULSE_DURATION);
}

//----------------------------------------------------------------------
//   I      II      III     IV       V   
// -       - -      -	    _ -     - _ 
// | -     | |      |       | |     | |   
// | |     | -      | -     | |     | |
// | -     |        | |     | |     | |
// -       -        - -     _ -     - _
//----------------------------------------------------------------------
inline bool IfleftContainRight(PulseDesc *left, PulseDesc *right)// I II III
{
	return ((left->duration >= right->duration) && IsEqual(right->start, left->start));
}

inline bool ifLeftAheadRight(PulseDesc* left, PulseDesc *right)
{
	return ((right->start >= left->end));
}

splitType PulseAnalyzer::ifNeedSplitPulse(PulseDesc *left, PulseDesc *right)
{
	splitType ret = splitNone;

	if(ifLeftAheadRight(left,right)){  // IV, V
		ret = splitSkipLeft;
	}else if(ifLeftAheadRight(right, left)){
		ret = splitSkipRight;
	}else if(IfleftContainRight(left, right)){ // I II III
		ret = splitLeft;

		if (IsEqual(left->start, right->start)){
			if (!IsEqual(left->end, right->end)) {
				if (IsBigger(left->end, right->end)) {
					ret = splitLeftRefRight;
				} else {
					ret = splitSkipLeft;
				}
			} else {
				ret = splitSkipAll;
			}
		}
	}else if(IfleftContainRight(right, left)){
		ret = splitRight;

		if (IsEqual(right->start, left->start)){
			if (!IsEqual(left->end , right->end)) {
				if (IsBigger(left->end, right->end)) {
					ret = splitRightRefLeft;
				} else {
					ret = splitSkipRight;
				}
			} else {
				ret = splitSkipAll;
			}
		}
	} else {
		ret = ret;
	}

	return ret;
}

#define to_str(x) #x
/* compare short list to long list and write value */
void PulseAnalyzer::CreateFrameInfo(double frameDuration)
{
	int32_t index = 0;
	int32_t curFrameType = 0;

	PulseList &shortChannel = mPulseList[LCHANNEL];
	PulseList &longChannel = mPulseList[RCHANNEL];

	PulseList::iterator itShort = shortChannel.begin();
	PulseList::iterator itLong = longChannel.begin();

	Logger(Info, "Process Sync Data... ");

	while (!(itLong == longChannel.end() || itShort == shortChannel.end())) 
	{
		double start, end;
		char *prePost = "Unknow";
		PulseDesc shortOrg, longOrg;
		start = end = 0;
		curFrameType = GetPulseType(itShort->type, itLong->type);

		ReportProgress(itLong->index, longChannel.size());

		splitType ret = ifNeedSplitPulse(&(*itShort), &(*itLong));
		switch(ret)
		{
			case splitLeft:
				{
					prePost = to_str(splitLeft);
					shortOrg = *itShort;
					longOrg = *itLong;

					start = itShort->start;
					end = itLong->start;
					Logger(Debug, "splitLeft: (%.2f,%.2f) & (%.2f,%.2f) => (%.2f, %.2f)",
						itShort->start, itShort->end, itLong->start, itLong->end, start, end);
					itShort->SetStart(end);
					if (itShort->duration <= 0) {
						itShort++;
					}
				}
				break;
			case splitLeftRefRight:
				{
					prePost = to_str(splitLeftRefRight);
					shortOrg = *itShort;
					longOrg = *itLong;

					start = (itLong->start + itShort->start)/2;
					end = itLong->end;
					itShort->SetStart(end);
					if (itShort->duration <= 0) {
						itShort++;
					}
					itLong++;
				}
				break;
			case  splitRight:
				{
					prePost = to_str(splitRight);
					shortOrg = *itShort;
					longOrg = *itLong;

					start = itLong->start;
					end = itShort->start;
					itLong->SetStart(end);
					if (itLong->duration <= 0) {
						itLong++;
					}
				}
				break;
			case splitRightRefLeft:
				{
					prePost = to_str(splitRightRefLeft);
					shortOrg = *itShort;
					longOrg = *itLong;

					start = (itLong->start + itShort->start)/2;
					end = itShort->end;
					itLong->SetStart(end);
					if (itLong->duration <= 0) {
						itLong++;
					}
					itShort++;
				}
				break;
			case splitSkipLeft:
				{
					prePost = to_str(splitSkipLeft);
					shortOrg = *itShort;
					longOrg = *itLong;

					start = itShort->start;
					end = itShort->end;
					itShort++;
				}
				break;
			case splitSkipRight:
				{
					prePost = to_str(splitSkipRight);
					shortOrg = *itShort;
					longOrg = *itLong;

					start = itLong->start;
					end = itLong->end;
					itLong++;
				}
				break;
			case splitSkipAll:
				{
					prePost = to_str(splitSkipAll);
					shortOrg = *itShort;
					longOrg = *itLong;

					start = (itLong->start + itShort->start)/2;
					end = (itLong->end + itShort->end)/2;
					itLong++;
					itShort++;
				}
				break;
			default:
				Logger(Error, "Unknow Pulse Split type.");
				break;
		}

		Logger(Debug, "%s: (%.2f,%.2f) & (%.2f,%.2f) => (%.2f, %.2f)", prePost,
			shortOrg.start, shortOrg.end, longOrg.start, longOrg.end, start, end);

		{
			double fps = 0.0f;
			double stdevp = 0.0f;
			double avg = 0.0f;
			Pulse OutPulse(start, end);

			mStdevpAlgorithm.CalcAvgStdAndFps(mFramePulse, avg, stdevp, fps);
			FrameDesc frame(curFrameType, OutPulse.start, OutPulse.end, fps, avg, stdevp, index++);
			mFramePulse.push_back(frame);
		}
	}
}
#undef to_str

void PulseAnalyzer::OutputResult()
{
	double pulseWidth = 0.0f;

	WriteRawPulseDetail();

	PulseFilter();
	MergeOffset();

	WriteRawSyncDetail();

	GetPulseWidth(pulseWidth);

	CreateFrameInfo(pulseWidth);
	WriteFrameDetail();

	AnalyzerSmoooth(pulseWidth);
	WriteSmoothDetail();

	Logger(Info, "end.");
}
