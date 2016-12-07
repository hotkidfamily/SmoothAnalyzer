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
	"."
};
#endif

static void print_usage(const char *name)
{
	std::string program_name = name;
	std::string helpString = "\nUsage: Analyze AV sync result recording wav files, " + program_name + " <input file>\n"
		"\nExample1 : " + program_name + " result.wav\n"
		"\tIt will analyze result.wav file\n"
		"Example2 : " + program_name + " result1.wav result2.wav\n"
		"\tIt will analyze result1.wav and result2.wav file\n"
		"\nNote: Can only analyze wav files in current folder, and no recursive search.\n"
		"If you want to analyze all wav files in current folder, use: " + program_name + " <Path>\n";
	printf("%s", helpString.c_str());
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

int analyzeFile(std::string file)
{
	std::string lChannelData;
	std::string rChannelData;
	std::string statiticsFile;
	
	WAVFileParse *parse = new WAVFileParse(DEBUG_CHANNEL_DATA);
	if(!parse->openWavFile(file.c_str())){
		delete parse;
		return -1;
	}
	makeRecordFileName(file, statiticsFile);
	inter_log(Info, "Analyze file %s", file.c_str());
	
	int wavSampleRate = parse->getSampleRate();
	csvOutput *csvFile = new csvOutput(statiticsFile.c_str());
	waveAnalyzer *lChannelAnalyzer = new waveAnalyzer("lchannel");
	waveAnalyzer *rChannelAnalyzer = new waveAnalyzer("rchannel");
	lChannelAnalyzer->setWaveSampleRate(wavSampleRate);
	rChannelAnalyzer->setWaveSampleRate(wavSampleRate);
	
	int times = 0;
	while(1){
		std::list<int> startTimingL; //ms		
		std::list<int> endTimingL; //ms
		int readTiming = 500; //ms
		int baseTiming = readTiming * times;
		
		lChannelData.clear();
		rChannelData.clear();
		bool ret = parse->getLRChannelDataSeperately(lChannelData, rChannelData, readTiming);
		
		retType retAnalyzer = RET_OK;
		retAnalyzer = lChannelAnalyzer->analyze(lChannelData, startTimingL, endTimingL);
		if(retAnalyzer == RET_FIND_START){
			csvFile->recordTimestamp(Timestamp::LCHANNEL, startTimingL, endTimingL, baseTiming);
		}

		std::list<int> startTimingR; //ms		
		std::list<int> endTimingR; //ms
		retAnalyzer = rChannelAnalyzer->analyze(rChannelData, startTimingR, endTimingR);
		if(retAnalyzer == RET_FIND_START){
			csvFile->recordTimestamp(Timestamp::RCHANNEL, startTimingR, endTimingR, baseTiming);
		}
		times++;
		if(!ret){
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

int main(int argc, char* argv[])
{
#ifdef _DEBUG
	argc = sizeof(debug_args)/sizeof(debug_args[0]);
	debug_args[0] = argv[0];
	argv = debug_args;
#endif

	if(argc < 2){
		print_usage();
		return 0;
	}

	std::string currentPath;
	char buffer[MAX_PATH] = {0,};
	if(GetCurrentDirectoryA(MAX_PATH, buffer) == 0){
		inter_log(Fatal, "No support path %s, error code %d", currentPath.c_str(), GetLastError());
		return 0;
	}
	currentPath.clear();
	currentPath.assign(buffer);
	currentPath.append("\\");
	
	std::string firstFile = argv[1];
	if (argc == 2 && firstFile == ".")
	{
		fileEnum *fileFinder = new fileEnum();
		fileFinder->enumDirectory(currentPath + "*", ".wav");
		std::string fileName;
		
		while(!fileFinder->getFile(fileName)){
			fileName.insert(0, currentPath);
			analyzeFile(fileName);
		}
		delete fileFinder;
	}
	else
	{
		int fileNum = argc - 1;
		while(fileNum > 0)
		{
			std::string fileName = argv[fileNum];
			fileName.insert(0, currentPath);
			if (0 == _access(fileName.c_str(), 0))
			{
				analyzeFile(fileName);
			}
			fileNum--;
		}
	}
	return 0;
}

