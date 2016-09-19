#include "StdAfx.h"
#include <math.h>
#include "log.h"
#include "csvFileMaker.h"


csvOutput::csvOutput(void)
{
}

csvOutput::~csvOutput(void)
{
}

csvOutput::csvOutput(const char* filename)
{
	csvFilePath.assign(filename, strlen(filename));
}

void csvOutput::recordTimestamp(uint8_t channelID, double start, double end)
{
	syncTimestamp time(channelID, start, end);

	if(end - start < 0.010){
		inter_log(Debug, "pulse duration(%f) is not width enough.", (end-start));
		return;
	}

	inter_log(Debug, "map size %d", channelMap.size());

	std::list<syncTimestamp> &channel = channelMap[channelID];
	if(channel.size()){
		syncTimestamp &lastTime = channel.back();
		if((time.start - lastTime.end) < 0.500){ // filter 500ms 
			inter_log(Debug, "channel %d pulse(%f, %f, %f, %f) between last and current is not enough.", channelID, time.start, time.end, lastTime.end, (time.start - lastTime.end));
			return;
		}
	}
	
	channel.push_back(time);	

	inter_log(Debug, "%d pulse start %f, end %f, length %fms", channelID, start, end, end - start);
}

void csvOutput::outputTwoCols(std::list<syncTimestamp> &lChannel, std::list<syncTimestamp> &rChannel)
{
	uint32_t lindex = 0;
	uint32_t rindex = 0;
	syncTimestamp lastLChannelTime;
	syncTimestamp lastRChannelTime;

	while(lChannel.size() || rChannel.size()){
		int32_t sync = 0;
		syncTimestamp lChannelTime;
		syncTimestamp rChannelTime;
		uint32_t lInterval;
		uint32_t rInterval;

		if(lChannel.size() && rChannel.size()){
			lChannelTime = lChannel.front();
			rChannelTime = rChannel.front();

			if(fabs(lChannelTime.start - rChannelTime.start) < 0.500){
				sync = (int32_t)((lChannelTime.start - rChannelTime.start)*1000);
			}else{
				if(lChannelTime.start < rChannelTime.start){
					rChannelTime.reset();
				}else{
					lChannelTime.reset();
				}
			}
		}else if(lChannel.size()){
			lChannelTime = lChannel.front();
		}else {
			rChannelTime = rChannel.front();					
		}

		if(lChannelTime.start != 0){
			lindex++;
			lChannel.pop_front();
			lInterval = (uint32_t)((lChannelTime.start - lastLChannelTime.start)*1000);
			lastLChannelTime = lChannelTime;
		}
		if(rChannelTime.start != 0){
			rindex ++;
			rChannel.pop_front();
			rInterval = (uint32_t)((rChannelTime.start - lastRChannelTime.start)*1000);
			lastRChannelTime = rChannelTime;
		}

		double lDuration = (lChannelTime.end - lChannelTime.start) * 1000;
		double rDuration = (rChannelTime.end - rChannelTime.start) * 1000;

		writeCsvLine("%d,, %u, %0.3f, %0.3f, %0.3f, %u,"
			",%u, %0.3f, %0.3f, %0.3f, %u",
			sync,
			lindex, lChannelTime.start, lChannelTime.end, lDuration, lInterval,
			rindex, rChannelTime.start, rChannelTime.end, rDuration, rInterval);
	}
}


void csvOutput::outputResult()
{
	bool bHaveData = false;
	uint32_t listCnt = 1;
	uint32_t maxLineCnt = UINT_MAX;
	uint32_t lineCount = 0;

	std::map<uint8_t, std::list<syncTimestamp>>::iterator it;
	for(it = channelMap.begin(); it!=channelMap.end(); it++){
		if(it->second.size() != 0){
			bHaveData = true;
		}
		maxLineCnt = min(maxLineCnt, it->second.size());
	}

	if(!bHaveData){
		return;
	}
	
	csvFile.open(csvFilePath.c_str());
	if(!csvFile.is_open()){
		return;
	}
	inter_log(Info, "Create file %s.", csvFilePath.c_str());

	/* make csv header */
	std::string header("sync,,");
	for(size_t i=0; i<channelMap.size(); i++){
		header.append("index, start, end, duration, interval, , ");
	}
	writeCsvLine("%s", header.c_str());

	if(channelMap.size() != 2){
		std::map<uint8_t, std::list<syncTimestamp>>::iterator it;
		for(it = channelMap.begin(); it!=channelMap.end(); it++){
			std::list<syncTimestamp> &dataList = it->second;
			lineCount = 0;

			syncTimestamp lastTime;
			while(lineCount < maxLineCnt){

				syncTimestamp curTime;
				if(dataList.size()){
					curTime = dataList.front();
					dataList.pop_front();
				}
				
				double duration = (curTime.end - curTime.start)*1000;
				uint32_t interval = (uint32_t)((curTime.start - lastTime.start)*1000);
				if(listCnt>1){
					appendToCsvLine(lineCount+1, "%d, ,%u, %0.3f, %0.3f, %0.3f, %u, , ",
						0,
						lineCount, curTime.start, curTime.end, duration, interval);
				}else{
					writeCsvLine("%d, , %u, %0.3f, %0.3f, %0.3f, %u, , ",
						0,
						lineCount, curTime.start, curTime.end, duration, interval);
				}

				lastTime = curTime;
				lineCount ++;
			}
			listCnt++;
		}
	}else{
		std::map<uint8_t, std::list<syncTimestamp>>::iterator it;
		std::list<syncTimestamp> lchannel;
		std::list<syncTimestamp> rchannel;

		it = channelMap.begin();
		lchannel = it->second;
		it++;
		rchannel = it->second;
		
		outputTwoCols(lchannel, rchannel);
	}
	
	while(csvContentList.size()){
		csvFile << csvContentList.front() << '\n';
		csvContentList.pop_front();
	}
}

void csvOutput::writeCsvLine(const char* format, ...)
{    
	std::string line;
	char csvLine[4096] = "";
	va_list args;
	va_start(args, format);
	vsprintf_s(csvLine, format, args);
	va_end(args);

	line.assign(csvLine);
	csvContentList.push_back(line);
}

void csvOutput::insertCsvLine(uint32_t lineIndex, const char*format, ...)
{
	std::string line;

	char csvLine[1024] = "";
	va_list args;
	va_start(args, format);
	vsprintf_s(csvLine, format, args);
	va_end(args);
	line.append(csvLine);

	std::list<std::string>::iterator it;
	it = csvContentList.begin();

	csvContentList.insert(it, lineIndex, line);
}

void csvOutput::appendToCsvLine(uint32_t lineIndex, const char*format, ...)
{
	char csvLine[1024] = "";
	va_list args;
	va_start(args, format);
	vsprintf_s(csvLine, format, args);
	va_end(args);

	std::list<std::string>::iterator it;
	it = csvContentList.begin();
	for(uint32_t i=0; i<lineIndex; i++){
		it++;
	}

	it->erase(it->size()-1, 1);
	it->append(csvLine);
	
}