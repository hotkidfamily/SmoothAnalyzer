// wavanalyzer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "log.h"
#include "wavFileParse.h"
#include "waveAnalyzer.h"
#include "fileEnum.h"
#include "SmoothAnalyzer.h"

#ifdef _DEBUG
char* debug_args[] ={
	"",
	"e:\\smooth\\sony_entertainment.wav"
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

static int32_t analyzeFileByChannel(CHANNELID index, std::string file, PulseAnalyzer* &analyzer)
{
	std::string ChannelData;
	std::string sourceData;
	DataSeparater *dataSeparater = new DataSeparater(file + "." + chanenlIDNameList[index]);
	CWaveReader *fileReader = new CWaveReader();
	WaveAnalyzer *channelAnalyzer = new WaveAnalyzer(index, file);
	int32_t ret = 0;

	if (!fileReader->Open(file.c_str())){
		return -1;
	}

	channelAnalyzer->SetWavFormat(fileReader->GetFormat());
	dataSeparater->SetWavFormat(fileReader->GetFormat());

	while(1){
		uint32_t startSampleIndex = 0;
		uint32_t endSampleIndex = 0;

		ChannelData.clear();
		ret = fileReader->ReadData(sourceData);

		dataSeparater->GetChannelData(index, sourceData ,ChannelData);

		retType retAnalyzer = RET_OK;

		retAnalyzer = channelAnalyzer->Analyzer(ChannelData, startSampleIndex, endSampleIndex);
		if(retAnalyzer == RET_FIND_PULSE){
			analyzer->RecordTimestamp(index, fileReader->SampeIndexToMS(startSampleIndex), fileReader->SampeIndexToMS(endSampleIndex));
		}

		if(ret < 0)
			break;
	}

	fileReader->Close();

	delete fileReader;
	delete channelAnalyzer;
	delete dataSeparater;

	return ret == EOF;
}

static int analyzeFile(std::string file)
{
	std::string ChannelData;
	int32_t ret = 0;
	PulseAnalyzer *smoothAnalyzer = NULL;
	smoothAnalyzer = new PulseAnalyzer(file);

	analyzeFileByChannel(LCHANNEL, file, smoothAnalyzer);
	analyzeFileByChannel(RCHANNEL, file, smoothAnalyzer);

	inter_log(Info, "\nAnalyzer... ");

	smoothAnalyzer->OutputResult();

	if(smoothAnalyzer){
		delete smoothAnalyzer;
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

	rPath.clear();
	rPath = buffer;
	rPath.insert(rPath.size(), "\\");
	return 0;
}

int main(int argc, char* argv[])
{
	std::string absPath;
	FileEnumer *fileFinder = NULL;
	int32_t ret = 0;

#ifdef _DEBUG
	argc = sizeof(debug_args)/sizeof(debug_args[0]);
	debug_args[0] = argv[0];
	argv = debug_args;
#endif

	if(parse_parameters(argc, argv) < 0){
		goto cleanup;
	}

	absPath = argv[1];

	fileFinder = new FileEnumer();
	if(!fileFinder){
		inter_log(Error, "Can not enum file.");
		goto cleanup;
	}

	ret = fileFinder->IsDirectory(absPath);
	if(ret < 0){
		inter_log(Fatal, "path %s is invalid.", argv[1]);
	}else if (ret > 0){
		std::string file;
		if(GetAbsolutlyPath(argv[1], absPath) < 0){
			goto cleanup;
		}
		fileFinder->EnumDirectory(absPath + "*", ".wav");
		while(!fileFinder->GetFile(file)){
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

