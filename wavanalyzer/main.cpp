// wavanalyzer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "log.h"
#include "wavFileParse.h"
#include "csvFileMaker.h"
#include "waveAnalyzer.h"
#include "fileEnum.h"

#ifdef _DEBUG
char* debug_args[] ={
	"",
	"C:/Program Files (x86)/Microsoft Office/Office14/MEDIA/CAMERA.WAV",
	//"e:/avsync_test_result/movie test with devices/",
	//"E:/avsync_test_result/movie test with devices/pc_publish_pc_view_win7_64bit_silk.wav"
	//"."
};
#endif

static void print_usage(const char *program_name)
{
	printf("usage: %s <input file>\n", program_name);
}

static void makeRecordFileName(std::string recordFilePath, std::string &statiFile)
{
	SYSTEMTIME systime;
	char buffer[256] = "";
	GetLocalTime(&systime);
	sprintf_s(buffer, 256-1, "_%04d%02d%02d%02d%02d%02d.csv", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

	statiFile = recordFilePath;
	statiFile.insert(statiFile.size(), buffer);
}

static int32_t parse_parameters(int32_t argc, char* argv[])
{
	int32_t ret = 0;
	if(argc < 2){
		print_usage(argv[0]);
		ret = -1;
	}
	return ret;
}

int32_t analyzeFile(std::string file)
{
	std::string statiticsFile;
	WAVFileParse *parse = new WAVFileParse();
	makeRecordFileName(file, statiticsFile);
	csvOutput *csvFile = new csvOutput(statiticsFile.c_str());
	waveAnalyzer *lChannelAnalyzer = new waveAnalyzer("lchannel");
	waveAnalyzer *rChannelAnalyzer = new waveAnalyzer("rchannel");

	inter_log(Info, "Analysize file %s", file.c_str());

	if(parse->openWavFile(file.c_str()) < 0){
		return -1;
	}

	lChannelAnalyzer->setBytesPerSample(parse->getBytesPerSample());
	rChannelAnalyzer->setBytesPerSample(parse->getBytesPerSample());
	lChannelAnalyzer->setSampleRate(parse->getSampleRate());
	rChannelAnalyzer->setSampleRate(parse->getSampleRate());

	while(1){
		int32_t ms = 0;
		uint32_t startSampleIndex = 0;
		uint32_t endSampleIndex = 0;
		std::string lChannelData;
		std::string rChannelData;
		int32_t ret = parse->getLRChannelData(lChannelData, rChannelData);
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

	return 0;
}

int32_t main(int32_t argc, char* argv[])
{	
	std::string workFilePath;

#ifdef _DEBUG
	argc = sizeof(debug_args)/sizeof(debug_args[0]);
	debug_args[0] = argv[0];
	argv = debug_args;
#endif

	if(parse_parameters(argc, argv) < 0){
		goto cleanup;
	}
	workFilePath = argv[1];

	fileEnum *fileFinder = new fileEnum();
	int32_t ret = fileFinder->isDirectory(workFilePath);
	if(ret < 0){
		inter_log(Fatal, "path %s is invalid.", workFilePath.c_str());
	}else if (ret > 0){
		std::string files;
		char *buffer = (char*)workFilePath.c_str();

		// directory 
		inter_log(Info, "working in directory %s", workFilePath.c_str());

		// absolute path
		if((buffer[workFilePath.size()-1] == '\\') || (buffer[workFilePath.size()-1] == '/')){
			
		}else if(workFilePath == "."){ // relative path
			std::string currentPath;
			char buffer[MAX_PATH] = {0,};
			if(GetCurrentDirectoryA(MAX_PATH, buffer) == 0){
				inter_log(Fatal, "No support path %s, error code %d", workFilePath.c_str(), GetLastError());
				goto cleanup;
			}
			workFilePath.clear();
			workFilePath.assign(buffer);
			workFilePath.append("\\");
		}else {
			workFilePath.append("\\");
		}
		
		fileFinder->enumDirectory(workFilePath+"*", ".wav");
		while(!fileFinder->getFile(files)){
			files.insert(0, workFilePath);
			analyzeFile(files);
		}
	}else {
		analyzeFile(workFilePath);
	}

	delete fileFinder;
	
cleanup:
	return 0;
}

