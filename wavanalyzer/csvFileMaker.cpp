#include "StdAfx.h"
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

	if(channelID == syncTimestamp::LCHANNEL){
		if(m_dataListLChannel.size()){
			syncTimestamp &lastTime = m_dataListLChannel.back();
			if(((time.start - lastTime.end) < 0.010)
					|| (end - start < 0.010)){ // filter 10ms 
				return;
			}
		}
		m_dataListLChannel.push_back(time);
	}else{
		if(m_dataListRChannel.size()){
			syncTimestamp &lastTime = m_dataListRChannel.back();
			if(((time.start - lastTime.end) < 0.050)
				|| (end - start < 0.050)){ // filter 10ms 
				return;
			}
		}
		m_dataListRChannel.push_back(time);
	}
}

void csvOutput::outputResult()
{
	uint32_t lindex = 0;
	uint32_t rindex = 0;
	if(m_dataListLChannel.size() || m_dataListRChannel.size()){
		csvFile.open(csvFilePath.c_str());
		inter_log(Info, "Create file %s.", csvFilePath.c_str());
		writeCsvFile("sync, channel, LIndex, start, end, duration, interval, channel, RIndex, start, end, duration, interval");

		if(csvFile.is_open()){
			while(m_dataListLChannel.size() || m_dataListRChannel.size()){
				syncTimestamp lChannelTime;
				syncTimestamp rChannelTime;

				if(m_dataListLChannel.size()){
					lChannelTime = m_dataListLChannel.front();
					lindex ++;
					m_dataListLChannel.pop_front();
				}

				if(m_dataListRChannel.size()){
					rChannelTime = m_dataListRChannel.front();
					rindex ++;
					m_dataListRChannel.pop_front();
				}

				writeCsvFile("%d, %d, %u, %0.3f, %0.3f, %f, %d,"
					"%d, %u, %0.3f, %0.3f, %f, %d",
					(int32_t)((lChannelTime.start - rChannelTime.start)*1000),
					lChannelTime.channelID, lindex, lChannelTime.start, lChannelTime.end, (lChannelTime.end - lChannelTime.start) * 1000, 0, 
					rChannelTime.channelID, rindex, rChannelTime.start, rChannelTime.end, (rChannelTime.end - rChannelTime.start) * 1000, 0);
			}
			csvFile.close();
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