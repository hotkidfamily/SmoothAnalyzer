// wavanalyzer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "log.h"
#include "wavFileParse.h"
#include "csvFileMaker.h"
#include "waveAnalyzer.h"
#include "fileEnum.h"
#include "SmoothAnalyzer.h"

#ifdef _DEBUG
char* debug_args[] ={
	"",
	"..\\Debug"
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

static int32_t parse_parameters(int32_t argc, char* argv[])
{
	int32_t ret = 0;
	if(argc < 2){
		print_usage(argv[0]);
		ret = -1;
	}
	
	return ret;
}

int analyzeFile(std::string file)
{
	std::string lChannelData;
	std::string rChannelData;
	int32_t ret = 0;

	WAVFileParse *parse = NULL;
	WaveAnalyzer *lWaveAnalyzer = NULL;
	WaveAnalyzer *rWaveAnalyzer = NULL;
	PulseAnalyzer *smoothAnalyzer = NULL;
	
	parse = new WAVFileParse(DEBUG_CHANNEL_DATA);
	if(!parse->openWavFile(file.c_str())){
		delete parse;
		return -1;
	}

	lWaveAnalyzer = new WaveAnalyzer("lchannel");
	rWaveAnalyzer = new WaveAnalyzer("rchannel");
	smoothAnalyzer = new PulseAnalyzer(file);

	lWaveAnalyzer->setWavFormat(parse->getWavFormat());
	rWaveAnalyzer->setWavFormat(parse->getWavFormat());
	
	int times = 0;
	while(1){
		uint32_t startSampleIndex = 0;
		uint32_t endSampleIndex = 0;
		int readTiming = 500; //ms
		int baseTiming = readTiming * times;
		
		lChannelData.clear();
		rChannelData.clear();
		ret = parse->getLRChannelData(lChannelData, rChannelData);
		
		retType retAnalyzer = RET_OK;

		retAnalyzer = lWaveAnalyzer->analyzer(lChannelData, startSampleIndex, endSampleIndex);
		if(retAnalyzer == RET_FIND_PULSE){
			smoothAnalyzer->RecordTimestamp(LCHANNEL, parse->convertIndexToMS(startSampleIndex), parse->convertIndexToMS(endSampleIndex));
		}

		startSampleIndex = 0;
		endSampleIndex = 0;
		retAnalyzer = rWaveAnalyzer->analyzer(rChannelData, startSampleIndex, endSampleIndex);
		if(retAnalyzer == RET_FIND_PULSE){
			smoothAnalyzer->RecordTimestamp(RCHANNEL, parse->convertIndexToMS(startSampleIndex), parse->convertIndexToMS(endSampleIndex));
		}

		if(ret == EOF){
			break;
		}
	}

	parse->closeWavFile();
	smoothAnalyzer->OutputResult();

	if(parse){
		delete parse;
	}

	if(smoothAnalyzer){
		delete smoothAnalyzer;
	}

	if(lWaveAnalyzer){
		delete lWaveAnalyzer;
	}

	if(rWaveAnalyzer){
		delete rWaveAnalyzer;
	}

	return 0;
}

static int GetAbsolutlyPath(const char* path, std::string &rPath)
{
	DWORD retVal = 0;
	std::string cupath = path;
	char buffer[MAX_PATH] = {'\0'};
	LPSTR retPath = {NULL};
	retVal = GetFullPathNameA(path, MAX_PATH, buffer, &retPath);
	if (retVal == 0) {
		inter_log(Error ,"Invalid file path %d, code %d\n", path, GetLastError());
		return -1;
	}

	rPath.append(buffer);
	rPath.insert(rPath.size(), "\\");
	return 0;
}

int main(int argc, char* argv[])
{
	std::string absPath;
	fileEnum *fileFinder = NULL;
	int32_t ret = 0;

#ifdef _DEBUG
	argc = sizeof(debug_args)/sizeof(debug_args[0]);
	debug_args[0] = argv[0];
	argv = debug_args;
#endif

	if(parse_parameters(argc, argv) < 0){
		goto cleanup;
	}

	if(GetAbsolutlyPath(argv[1], absPath) < 0){
		goto cleanup;
	}

	fileFinder = new fileEnum();
	if(!fileFinder){
		inter_log(Error, "Can not enum file.");
		goto cleanup;
	}

	ret = fileFinder->isDirectory(absPath);
	if(ret < 0){
		inter_log(Fatal, "path %s is invalid.", argv[1]);
	}else if (ret > 0){
		std::string file;
		fileFinder->enumDirectory(absPath + "*", ".wav");
		while(!fileFinder->getFile(file)){
			file.insert(0, absPath);
			analyzeFile(file);
		}
	}else {
		analyzeFile(absPath);
	}

cleanup:
	if(fileFinder)
		delete fileFinder;

	return 0;
}

