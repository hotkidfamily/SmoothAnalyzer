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
	csvFilePath.assign(filename);
}

void csvOutput::recordTimestamp(Timestamp::CHANNELID channelID, std::list<int>& start, std::list<int>& end, int baseTime)
{
	while(start.size())
	{
		Timestamp ts;
		ts.channelID = channelID;
		ts.time = baseTime + start.front();

		if(channelID == Timestamp::LCHANNEL){
			m_startTimeListLChannel.push_back(ts);
		} else {
			m_startTimeListRChannel.push_back(ts);
		}
		start.pop_front();
	}
	
	while(end.size())
	{
		Timestamp ts;
		ts.channelID = channelID;
		ts.time = baseTime + end.front();

		if(channelID == Timestamp::LCHANNEL){
			m_endTimeListLChannel.push_back(ts);
		} else {
			m_endTimeListRChannel.push_back(ts);
		}
		end.pop_front();
	}
}

void csvOutput::GenerateLowHighDurationList(std::list<Timestamp>& startTimeList, std::list<Timestamp>& EndTimeList, std::list<double>& lowDurationList, std::list<double>& highDurationList)
{
	std::list<Timestamp>::iterator startTimeIterator = startTimeList.begin();	
	std::list<Timestamp>::iterator endTimeIterator = EndTimeList.begin();
	double preEndTime = startTimeIterator->time;
	while(startTimeIterator != startTimeList.end() && endTimeIterator != EndTimeList.end()){
		double lowDuration =  startTimeIterator->time - preEndTime;
		double highDuration = endTimeIterator->time - startTimeIterator->time;
		lowDurationList.push_back(lowDuration);
		highDurationList.push_back(highDuration);
		
		preEndTime = endTimeIterator->time;
		startTimeIterator++;
		endTimeIterator++;
	}
}

double csvOutput::CacluMeanValue(std::list<double>& durationList)
{
	double sum = 0.0f;
	double meanValue = 0.0f;
	std::list<double>::iterator durationIterator;
	for(durationIterator = durationList.begin(); durationIterator != durationList.end(); durationIterator++){
		sum += *durationIterator;
	}

	meanValue = sum / durationList.size();

	return meanValue;
}


double csvOutput::CacluMSE(std::list<double>& lowDurationList, std::list<double>& highDurationList)
{
	double meanValue = (CacluMeanValue(lowDurationList) + CacluMeanValue(highDurationList)) / 2;
	std::list<double>::iterator lowDurationIterator ;
	double lowSquareSum = 0;
	for(lowDurationIterator = lowDurationList.begin(); lowDurationIterator != lowDurationList.end(); lowDurationIterator++)
	{
		lowSquareSum += sqrt(fabs(*lowDurationIterator - meanValue));
	}
	double highSquareSum = 0;
	for(std::list<double>::iterator highDurationIterator = highDurationList.begin(); highDurationIterator != highDurationList.end(); highDurationIterator++)
	{
		highSquareSum += sqrt(fabs(*highDurationIterator - meanValue));
	}
	double MSE = (lowSquareSum + highSquareSum) / (lowDurationList.size() + highDurationList.size());
	return MSE;
}

bool csvOutput::ReadLChannelListInfo(double &startTime, double &endTime, double &lowDuration, double &highDuration)
{
	bool result = false;
	if(m_lowDurationListLChannel.size() && m_highDurationListLChannel.size())
	{
		startTime = m_startTimeListLChannel.front().time;
		endTime = m_endTimeListLChannel.front().time;
		lowDuration = m_lowDurationListLChannel.front();
		highDuration = m_highDurationListLChannel.front();
		m_startTimeListLChannel.pop_front();
		m_endTimeListLChannel.pop_front();
		m_lowDurationListLChannel.pop_front();
		m_highDurationListLChannel.pop_front();
		result = true;
	}
	return result;
}

bool csvOutput::ReadRChannelListInfo(double &startTime, double &endTime, double &lowDuration, double &highDuration)
{
	bool result = false;
	if(m_lowDurationListRChannel.size() && m_highDurationListRChannel.size())
	{
		startTime = m_startTimeListRChannel.front().time;
		endTime = m_endTimeListRChannel.front().time;
		lowDuration = m_lowDurationListRChannel.front();
		highDuration = m_highDurationListRChannel.front();
		m_startTimeListRChannel.pop_front();
		m_endTimeListRChannel.pop_front();
		m_lowDurationListRChannel.pop_front();
		m_highDurationListRChannel.pop_front();
		result = true;
	}
	return result;
}

void csvOutput::outputResult()
{
	double Lfps = 0;
	double Rfps = 0;
	Lfps = (m_startTimeListLChannel.size() + m_endTimeListLChannel.size()) / m_endTimeListLChannel.back().time * 1000;	
	Rfps = (m_startTimeListRChannel.size() + m_endTimeListRChannel.size()) / m_endTimeListRChannel.back().time * 1000;
	inter_log(Info, "fps %0.3f, %0.3f.", Lfps, Rfps);
	
	//L/R duration list
	GenerateLowHighDurationList(m_startTimeListLChannel, m_endTimeListLChannel, m_lowDurationListLChannel, m_highDurationListLChannel);	
	GenerateLowHighDurationList(m_startTimeListRChannel, m_endTimeListRChannel, m_lowDurationListRChannel, m_highDurationListRChannel);

	// L/R Mean Square Error
	double L_MSE = CacluMSE(m_lowDurationListLChannel, m_highDurationListLChannel);	
	double R_MSE = CacluMSE(m_lowDurationListRChannel, m_highDurationListRChannel);

	csvFile.open(csvFilePath.c_str());
	if(csvFile.is_open())
	{
		inter_log(Info, "Create file %s.", csvFilePath.c_str());
		writeCsvLine("Lchannel, fps, MSE, Rchanle, fps, MSE");
		writeCsvLine("%u, %0.3f, %0.3f,"
			"%u, %0.3f, %0.3f",
			0, Lfps, L_MSE,
			1, Rfps, R_MSE);

		writeCsvLine("Lchannel, index, start, end, lowduration, highduration, Rchannel, index, start, end, lowduration, highduration");

		uint32_t lindex = 0;
		uint32_t rindex = 0;
		double lStartTime = 0, lEndTime = 0, lLowDuration = 0, lHighDuration = 0;
		double rStartTime = 0, rEndTime = 0, rLowDuration = 0, rHighDuration = 0;
		while(1)
		{
			if (ReadLChannelListInfo(lStartTime, lEndTime, lLowDuration, lHighDuration)
				&& ReadRChannelListInfo(rStartTime, rEndTime, rLowDuration, rHighDuration))
			{
				lindex++;
				rindex++;
			}
			else if(ReadLChannelListInfo(lStartTime, lEndTime, lLowDuration, lHighDuration))
			{
				lindex++;
			}
			else if(ReadRChannelListInfo(rStartTime, rEndTime, rLowDuration, rHighDuration))
			{
				rindex++;
			}
			else
			{
				break;
			}

			writeCsvLine("%d, %u, %0.3f, %0.3f, %f, %f,"
				"%d, %u, %0.3f, %0.3f, %f, %f",
				0, lindex, lStartTime, lEndTime, lLowDuration, lHighDuration, 
				1, rindex, rStartTime, rEndTime, rLowDuration, rHighDuration);
		}
		csvFile.close();
	}
}

#if 0

void csvOutput::recordAVSyncTimestamp(syncTimestamp::CHANNELID channelID, double start, double end)
{
	syncTimestamp time;
	time.channelID = channelID;
	time.start = start;
	time.end = end;

	if(channelID == syncTimestamp::LCHANNEL){
		if(m_dataListLChannel.size()){
			syncTimestamp &lastTime = m_dataListLChannel.back();
			if(((time.start - lastTime.end) < 0.500)
				|| (end - start < 0.003)){ // remove less than 3ms pulse 
					return;
			}
		}
		m_dataListLChannel.push_back(time);
	}else{
		if(m_dataListRChannel.size()){
			syncTimestamp &lastTime = m_dataListRChannel.back();
			if(((time.start - lastTime.end) < 0.500)
				|| (end - start < 0.003)){ // remove less than 3ms pulse 
					return;
			}
		}
		m_dataListRChannel.push_back(time);
	}
}

void csvOutput::outputAVSyncResult()
{
	uint32_t lindex = 0;
	uint32_t rindex = 0;

	if(m_dataListLChannel.size() || m_dataListRChannel.size()){
		csvFile.open(csvFilePath.c_str());
		inter_log(Info, "Create file %s.", csvFilePath.c_str());
		writeCsvLine("sync, channel 1, index, start, end, duration, interval, channel 2, index, start, end, duration, interval");

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
						sync = (int32_t)((rChannelTime.start - lChannelTime.start)*1000);
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

				writeCsvLine("%d, %d, %u, %0.3f, %0.3f, %f, %d,"
					"%d, %u, %0.3f, %0.3f, %f, %d",
					sync,
					lChannelTime.channelID, lindex, lChannelTime.start, lChannelTime.end, (lChannelTime.end - lChannelTime.start) * 1000, 0, 
					rChannelTime.channelID, rindex, rChannelTime.start, rChannelTime.end, (rChannelTime.end - rChannelTime.start) * 1000, 0);
			}

			csvFile.close();
		}
	}
}
#endif
void csvOutput::writeCsvLine(const char* format, ...)
{    
	char csvLine[1024] = "";
	va_list args;
	va_start(args, format);
	vsprintf_s(csvLine, format, args);
	va_end(args);

	csvFile << csvLine << '\n';
}
