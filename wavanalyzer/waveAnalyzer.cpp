#include "StdAfx.h"
#include "waveAnalyzer.h"

WaveAnalyzer::WaveAnalyzer(STRING &filePath)
: mAnalyzerFile(filePath)
{
	ZeroMemory(mFrameId, sizeof(mFrameId));
}

WaveAnalyzer::~WaveAnalyzer(void)
{
}

int32_t WaveAnalyzer::UpdateThreshold(std::string &channelData, analyzerContext *ctx)
{
	int32_t sum = 0;
	int16_t *data = (int16_t *)channelData.c_str();

	for(size_t i=0; i<channelData.size()/mWavFormat.GetBytesPerSample(); i++){
		ctx->minThreshold = min(*data, ctx->minThreshold);
		ctx->maxThreshold = max(*data, ctx->maxThreshold);
		data ++;
	}

	if(ctx->totalSampleCount/mWavFormat.nSampleRate > 10){
		ctx->isThresholdValid = true;
		Logger(Info, "threshold is %d, min %d, max %d", ctx->GetThreshold(), ctx->minThreshold, ctx->maxThreshold);
	}

	return 0;
}

void WaveAnalyzer::FindPulse(analyzerContext *ctx, const int16_t *data, uint32_t nb_samples, uint32_t &start, uint32_t &end, uint32_t count)
{
	int32_t sum = 0;
	uint32_t startSamples = 0;
	uint32_t endSamples = 0;
	int32_t threshold = ctx->GetThreshold();
	uint32_t endSampleIndex = 0;

	sum = 0;
	for(uint32_t i = 0; i<nb_samples; i++){
		sum += *(data+i);
	}
	sum /= nb_samples;

	Logger(PulseDect, "sum = %d, threshold %d", sum, threshold);

	if(sum > threshold){
		for(uint32_t i = 0; i<nb_samples; i++){
			if(*(data+i) > threshold){
				startSamples = i;
				break;
			}
		}
		if(!ctx->bInPulse){
			ctx->bInPulse = true;
			ctx->pulseSampleIndex = ctx->totalSampleCount + count + startSamples;
		}
	}else{
		if(ctx->bInPulse){
			for(uint32_t i = 0; i<nb_samples; i++){
				if(*(data+i) < threshold){
					endSamples = i;
					break;
				}
			}

			ctx->bInPulse = false;
			endSampleIndex = ctx->totalSampleCount + count + endSamples;
			start = ctx->pulseSampleIndex;
			end = endSampleIndex;
		}
	}
}

int32_t WaveAnalyzer::SplitDataAndFindPulse(analyzerContext *ctx, std::string &channelData, std::list<SamplePos> &pulses)
{
	int16_t *data = (int16_t *)channelData.c_str();
	size_t processedSamplesCount = 0;
	uint32_t nbSampleSplitStep = 0;
	uint32_t nbProcessSamples = 0;
	uint32_t start = 0, end = 0;

	size_t nbTotalSamples = channelData.size()/mWavFormat.GetBytesPerSample();

	if(nbTotalSamples > SPLIT_PERCENT){
		nbSampleSplitStep = SPLIT_PERCENT; // 10 percent
	}else{
		nbSampleSplitStep = nbTotalSamples;
	}

	Logger(PulseDect, "analyzer size is %d", nbSampleSplitStep);

	do{
		if(nbSampleSplitStep + processedSamplesCount > nbTotalSamples){
			nbProcessSamples = nbTotalSamples - processedSamplesCount;
		}else{
			nbProcessSamples = nbSampleSplitStep;
		}

		FindPulse(ctx, data, nbProcessSamples, start, end, processedSamplesCount);
		if(start && end){
			SamplePos index(start, end);
			pulses.push_back(index);
			start = end = 0;
		}

		data += nbProcessSamples;
		processedSamplesCount += nbProcessSamples;

	}while(processedSamplesCount < nbTotalSamples);

	return 0;
}

retType WaveAnalyzer::Analyzer(analyzerContext *ctx, std::string &channelData, std::list<SamplePos> &pulses)
{
	mFilters[FILTER_RMNEGTV]->process(channelData, mWavFormat.GetBytesPerSample());
	mFilters[FILTER_SMOOTH]->process(channelData, mWavFormat.GetBytesPerSample());

// 	if(!IfThresholdValid()){
// 		UpdateThreshold(channelData);
// 	}else{
// 		SplitDataAndFindPulse(channelData, pulses);
// 	}

	SplitDataAndFindPulse(ctx, channelData, pulses);

	ctx->totalSampleCount += channelData.size()/mWavFormat.GetBytesPerSample();

	if(!pulses.empty()){
		return RET_FIND_PULSE;
	}else{
		return RET_OK;
	}	
}

void WaveAnalyzer::RecordPulse(CHANNELID channelID, double start, double end)
{
	if((end - start) < VALID_PULSE_DURATION){
		return ;
	}

	if(!mPulseList[channelID].empty()){
		PulseDesc &lastTime = mPulseList[channelID].back();

		if(lastTime.type != PULSE_HIGH ) {
			Logger(Error, "?? last pulse is \"%s\", not high pulse.", lastTime.type==PULSE_LOW?"low":"high");
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

bool WaveAnalyzer::AnalyzeFilePulse()
{
	bool bRet = true;
	if(bRet)
		bRet = AnalyzeFileByChannel(LCHANNEL);
	if(bRet)
		bRet = AnalyzeFileByChannel(RCHANNEL);

	return bRet;
}

/* open file for per channel, process one channel once */
bool WaveAnalyzer::AnalyzeFileByChannel(CHANNELID index)
{
	std::string ChannelData;
	std::string sourceData;
	int32_t ret = 0;
	DataSeparater *dataSeparater = NULL;
	CWaveReader *fileReader = NULL;
	STRING workname = mAnalyzerFile + _T(".") + chanenlIDNameList[index];
	analyzerContext ctx;

	mFilters[FILTER_RMNEGTV] = new RmNegativeFilter(workname);
	mFilters[FILTER_SMOOTH] = new SmoothFilter(workname);

	dataSeparater = new DataSeparater(workname);
	fileReader = new CWaveReader();

	if (!fileReader->Open(mAnalyzerFile.c_str())){
		return false;
	}

	dataSeparater->SetWavFormat(fileReader->GetFormat());
	SetWavFormat(fileReader->GetFormat());

	while(1){
		std::list<SamplePos> SamplePosList;
		uint32_t startSampleIndex = 0;
		uint32_t endSampleIndex = 0;

		ChannelData.clear();
		ret = fileReader->ReadData(sourceData);

		dataSeparater->GetChannelData(index, sourceData ,ChannelData);

		retType retAnalyzer = RET_OK;

		retAnalyzer = Analyzer(&ctx, ChannelData, SamplePosList);
		if(retAnalyzer == RET_FIND_PULSE){
			while(!SamplePosList.empty()){
				SamplePos &samplePos = SamplePosList.front();
				RecordPulse(index, fileReader->SampeIndexToMilliSecond(samplePos.startIndex), fileReader->SampeIndexToMilliSecond(samplePos.endIndex));
				SamplePosList.pop_front();
			}
		}

		if(ret < 0)
			break;
	}

	fileReader->Close();

	delete fileReader;
	delete dataSeparater;
	delete static_cast<RmNegativeFilter*>(mFilters[FILTER_RMNEGTV]);
	delete static_cast<SmoothFilter*>(mFilters[FILTER_SMOOTH]);

	return ret == EOF;
}