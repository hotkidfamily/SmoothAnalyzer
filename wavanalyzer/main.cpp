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
	"e:/Resources/iphone.wav",
};
#endif

static void print_usage()
{
	inter_log(Info, "usage: %s input");
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
		print_usage();
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
	waveAnalyzer *analyzer = new waveAnalyzer;

	parse->openWavFile(argv[1]);
	while(1){
		int32_t ms = 0;
		int ret = parse->getLRChannelData(lChannelData, rChannelData);
		retType retAnalyzer = RET_OK;
		
		retAnalyzer = analyzer->analyzer(lChannelData, ms);
		if(retAnalyzer == RET_FIND_START){
			csvFile->recordTimestamp(2, ms, 0);
		}
		retAnalyzer = analyzer->analyzer(rChannelData, ms);
		if(retAnalyzer == RET_FIND_START){
			csvFile->recordTimestamp(2, ms, 0);
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
	if(analyzer){
		delete analyzer;
	}

cleanup:
	return 0;
}

