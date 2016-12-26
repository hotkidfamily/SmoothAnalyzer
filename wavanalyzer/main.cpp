// wavanalyzer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "log.h"
#include "waveAnalyzer.h"
#include "fileEnum.h"
#include "SmoothAnalyzer.h"

#ifdef _DEBUG
char* debug_args[] ={
	"",
	"e:\\smooth\\ios-sony-entertainment.wav",
	"-8",
	"30"
};
#endif

double gChannelOffset = 0;
double gFps = 0;

static void print_usage(const char *name)
{
	const char *help = "\n\tAnazlyer a smooth steam wave file.\n\n"
		"Usage:\t%s <PATH>/<File Name> [Offset] [fps]\n"
		"e.g., \t%s a.wav -8 30\n\n"
		"Tips:\tIf you want indicate channels different Please fill [offset].\n"
		"     \tAnd if you know frame rate Please fill [fps].\n"
		"     \t[offset]: Negative indicate ahead, Positive indicate behind.\n"
		"     \t[fps]	: Frame rate of analyzed view, e.g., if frame rate is 30fps then input 30.\n\n"
		"Warning:Only Support .WAV File With 2 Channels.\n\n";

	printf(help, name, name);
}

static int32_t parse_parameters(int32_t argc, char* argv[])
{
	int32_t ret = 0;
	if(argc < 2){
		print_usage(argv[0]);
		ret = -1;
	}

	if(argc >= 3){
		gChannelOffset = atof(argv[2])/1000;
	}

	if(argc >= 4){
		gFps = atof(argv[3])/1000;
	}
	
	return ret;
}

static int analyzeFile(std::string file)
{
	std::string ChannelData;
	int32_t ret = 0;
	PulseAnalyzer *smoothAnalyzer = NULL;
	smoothAnalyzer = new PulseAnalyzer(file);
	WaveAnalyzer *wavAnalyzer = new WaveAnalyzer(file);

	inter_log(Info, "File %s, channel offset %f", file.c_str(), gChannelOffset);

	smoothAnalyzer->SetOffset(gChannelOffset);
	wavAnalyzer->AnalyzeFilePulse();
	smoothAnalyzer->SetAnalyzerData(wavAnalyzer->GetPulseData());

	inter_log(Info, "\nAnalyzer... ");

	smoothAnalyzer->OutputResult();

	if(wavAnalyzer){
		delete wavAnalyzer;
	}

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

