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

	if(m_dataListLChannel.size() || m_dataListRChannel.size()){
		csvFile.open(csvFilePath.c_str());
		inter_log(Info, "Create file %s.", csvFilePath.c_str());
		writeCsvFile("sync, channel 1, index, start, end, duration, interval, channel 2, index, start, end, duration, interval");

		if(csvFile.is_open()){
			while(m_dataListLChannel.size() || m_dataListRChannel.size()){
				int32_t sync = 0;
				syncTimestamp lChannelTime;
				syncTimestamp rChannelTime;

				if(m_dataListLChannel.size() && m_dataListRChannel.size()){
					lChannelTime = m_dataListLChannel.front();
					rChannelTime = m_dataListRChannel.front();

					if(fabs(lChannelTime.start - rChannelTime.start) < 0.500){
						m_dataListLChannel.pop_front();
						m_dataListRChannel.pop_front();
						sync = (int32_t)((lChannelTime.start - rChannelTime.start)*1000);
						lindex ++;
						rindex ++;
					}else{
						if(lChannelTime.start < rChannelTime.start){
							m_dataListLChannel.pop_front();
							rChannelTime.reset();
							lindex ++;
						}else{
							m_dataListRChannel.pop_front();
							lChannelTime.reset();
							rindex ++;
						}
					}
				}else if(m_dataListLChannel.size()){
					lChannelTime = m_dataListLChannel.front();
					m_dataListLChannel.pop_front();
					lindex++;
				}else {
					rChannelTime = m_dataListRChannel.front();
					m_dataListRChannel.pop_front();
					rindex ++;
				}

				writeCsvFile("%d, %d, %u, %0.3f, %0.3f, %0.3f, %d,"
					"%d, %u, %0.3f, %0.3f, %0.3f, %d",
					sync,
					lChannelTime.channelID, lindex, lChannelTime.start, lChannelTime.end, (lChannelTime.end - lChannelTime.start) * 1000, 0, 
					rChannelTime.channelID, rindex, rChannelTime.start, rChannelTime.end, (rChannelTime.end - rChannelTime.start) * 1000, 0);
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