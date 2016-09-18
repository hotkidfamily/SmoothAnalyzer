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

void csvOutput::recordTimestamp(syncTimestamp::CHANNELID channelID, double start, double end)
{
	syncTimestamp time;
	time.channelID = channelID;
	time.start = start;
	time.end = end;

	if(end - start < 0.010){
		inter_log(Debug, "pulse duration(%f) is not width enough.", (end-start));
		return;
	}

	if(channelID == syncTimestamp::LCHANNEL){
		if(m_dataListLChannel.size()){
			syncTimestamp &lastTime = m_dataListLChannel.back();
			if((time.start - lastTime.end) < 0.500){ // filter 500ms 
				inter_log(Debug, "L pulse(%f, %f, %f, %f) between last and current is not enough.", time.start, time.end, lastTime.end, (time.start - lastTime.end));
				return;
			}
		}
		m_dataListLChannel.push_back(time);
	}else{
		if(m_dataListRChannel.size()){
			syncTimestamp &lastTime = m_dataListRChannel.back();
			if((time.start - lastTime.end) < 0.500){ // filter 500ms 
				inter_log(Debug, "R pulse(%f, %f, %f, %f) between last and current is not enough.", time.start, time.end, lastTime.end, (time.start - lastTime.end));
				return;
			}
		}
		m_dataListRChannel.push_back(time);
	}

	inter_log(Debug, "%s pulse start %f, end %f, length %fms", (channelID == syncTimestamp::LCHANNEL)?"L":"R", start, end, end - start);
}

void csvOutput::outputResult()
{
	uint32_t lindex = 0;
	uint32_t rindex = 0;
	syncTimestamp lastLChannelTime;
	syncTimestamp lastRChannelTime;

	if(m_dataListLChannel.size() || m_dataListRChannel.size()){
		csvFile.open(csvFilePath.c_str());
		inter_log(Info, "Create file %s.", csvFilePath.c_str());
		writeCsvFile("sync, , index, start, end, duration, interval, , index, start, end, duration, interval");

		if(csvFile.is_open()){
			while(m_dataListLChannel.size() || m_dataListRChannel.size()){
				int32_t sync = 0;
				syncTimestamp lChannelTime;
				syncTimestamp rChannelTime;
				double lInterval = 0.0;
				double rInterval = 0.0;

				if(m_dataListLChannel.size() && m_dataListRChannel.size()){
					lChannelTime = m_dataListLChannel.front();
					rChannelTime = m_dataListRChannel.front();

					if(fabs(lChannelTime.start - rChannelTime.start) < 0.500){
						sync = (int32_t)((lChannelTime.start - rChannelTime.start)*1000);
					}else{
						if(lChannelTime.start < rChannelTime.start){
							rChannelTime.reset();
						}else{
							lChannelTime.reset();
						}
					}
				}else if(m_dataListLChannel.size()){
					lChannelTime = m_dataListLChannel.front();
				}else {
					rChannelTime = m_dataListRChannel.front();					
				}

				if(lChannelTime.start != 0){
					lindex++;
					m_dataListLChannel.pop_front();
					lInterval = (lChannelTime.start - lastLChannelTime.start)*1000;
					lastLChannelTime = lChannelTime;
				}
				if(rChannelTime.start != 0){
					rindex ++;
					m_dataListRChannel.pop_front();
					rInterval = (rChannelTime.start - lastRChannelTime.start)*1000;
					lastRChannelTime = rChannelTime;
				}

				writeCsvFile("%d,, %u, %0.3f, %0.3f, %0.3f, %d,"
					",%u, %0.3f, %0.3f, %0.3f, %d",
					sync,
					lindex, lChannelTime.start, lChannelTime.end, (lChannelTime.end - lChannelTime.start) * 1000, (int)lInterval,
					rindex, rChannelTime.start, rChannelTime.end, (rChannelTime.end - rChannelTime.start) * 1000, (int)rInterval);
			}
		}
	}
}

void csvOutput::writeCsvFile(const char* format, ...)
{    
	char csvLine[1024] = "";
	va_list args;
	va_start(args, format);
	vsprintf_s(csvLine, format, args);
	va_end(args);

	csvFile << csvLine << '\n';
}