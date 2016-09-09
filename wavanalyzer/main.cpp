// wavanalyzer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "log.h"
#include "wavFileParse.h"
#include "csvFileMaker.h"
#include "waveAnalyzer.h"

#ifdef _DEBUG
char* debug_args[] ={
	"",
	//"e:/Resources/iphone.wav",
	"e:/avsync_test_result/movie test with devices/VIVO XPLAY5 24fps.wav",
};
#endif

static void print_usage(const char *program_name)
{
	printf("usage: %s <input file>\n", program_name);
}

static void makeRecordFileName(const char *filename, std::string &recordFilePath)
{
	SYSTEMTIME systime;
	char buffer[256] = "";

	GetLocalTime(&systime);
	recordFilePath.assign(filename, strlen(filename));
	sprintf_s(buffer, 256-1, "_%04d%02d%02d%02d%02d%02d.csv", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

	recordFilePath.insert(strlen(filename), buffer);
}

static int parse_parameters(int argc, char* argv[])
{
	int ret = 0;
	if(argc < 2){
		print_usage(argv[0]);
		ret = -1;
	}
	return ret;
}

int main(int argc, char* argv[])
{	
	std::string recordFileName;
	std::string lChannelData;
	std::string rChannelData;

#ifdef _DEBUG
	argc = sizeof(debug_args)/sizeof(debug_args[0]);
	debug_args[0] = argv[0];
	argv = debug_args;
#endif

	if(parse_parameters(argc, argv) < 0){
		goto cleanup;
	}
	
	WAVFileParse *parse = new WAVFileParse(DEBUG_CHANNEL_DATA);
	makeRecordFileName(argv[1], recordFileName);
	csvOutput *csvFile = new csvOutput(recordFileName.c_str());
	waveAnalyzer *lChannelAnalyzer = new waveAnalyzer("lchannel");
	waveAnalyzer *rChannelAnalyzer = new waveAnalyzer("rchannel");

	if(parse->openWavFile(argv[1]) < 0){
		goto cleanup;
	}

	while(1){
		int32_t ms = 0;
		uint32_t startSampleIndex = 0;
		uint32_t endSampleIndex = 0;
		int ret = parse->getLRChannelData(lChannelData, rChannelData);
		retType retAnalyzer = RET_OK;
		
		retAnalyzer = lChannelAnalyzer->analyzer(lChannelData, startSampleIndex, endSampleIndex);
		if(retAnalyzer == RET_FIND_START){
			csvFile->recordTimestamp(syncTimestamp::LCHANNEL, parse->covertSampleToMS(startSampleIndex), parse->covertSampleToMS(endSampleIndex));
		}

		startSampleIndex = 0;
		endSampleIndex = 0;
		retAnalyzer = rChannelAnalyzer->analyzer(rChannelData, startSampleIndex, endSampleIndex);
		if(retAnalyzer == RET_FIND_START){
			csvFile->recordTimestamp(syncTimestamp::RCHANNEL, parse->covertSampleToMS(startSampleIndex), parse->covertSampleToMS(endSampleIndex));
		}

		if(ret < 0 ){
			break;
		}
	}
	
	parse->closeWavFile();
	csvFile->outputResult();

	if(parse){
		delete parse;
	}
	if(csvFile){
		delete csvFile;
	}
	if(lChannelAnalyzer){
		delete lChannelAnalyzer;
	}

	if(rChannelAnalyzer){
		delete rChannelAnalyzer;
	}

cleanup:
	return 0;
}

