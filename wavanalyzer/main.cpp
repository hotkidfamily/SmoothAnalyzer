// wavanalyzer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "log.h"
#include "waveAnalyzer.h"
#include "fileEnum.h"
#include "SmoothAnalyzer.h"

#define TEST 3

#ifdef _DEBUG
char* debug_args[] ={
	"",
#if (TEST == 1)
	"e:\\smooth\\ios-sony-entertainment.wav",
	"-8",
#elif (TEST == 2)
	"e:\\smooth\\xiaomi4-sony-entertainment.wav",
	"-14",
#else
	"e:\\smooth\\mpc-hc-sony-entertainment-default-render.wav",
	"0",
#endif
	"30"
};
#endif

typedef struct programContext{
	programContext(){
		ZeroMemory(this, sizeof(struct programContext));
	}
	ANALYZER_PARAMS analyzerParams;
	std::string targetPath;
	FileEnumer *fileFinder;
}SMOOTH_CONTEXT, *PSMOOTH_CONTEXT;

static void print_usage(const char *name)
{
	const char *help = "\n\tAnazlyer a smooth steam wave file.\n\n"
		"Usage:\t%s <PATH>/<File Name> [Offset] [fps]\n"
		"e.g., \t%s a.wav -8 30\n\n"
		"Tips:\tIf you want indicate channels different Please fill [offset] in millisecond.\n"
		"     \tAnd if you know frame rate Please fill [fps].\n"
		"     \t[offset]- Negative indicate ahead, Positive indicate behind.\n"
		"     \t[fps]	- Frame rate of analyzed view, if you want inut frame rate alone, you should make offset to 0.\n"
		"     \t          e.g., frame rate is 30fps then input \"0 30\" .\n\n"
		"Warning:Only Support .WAV File With 2 Channels.\n\n";

	printf(help, name, name);
}

static int32_t parse_parameters(SMOOTH_CONTEXT* ctx, const int32_t argc, char* argv[])
{
	int32_t ret = 0;
	if(argc < 2){
		print_usage(argv[0]);
		ret = -1;
		goto cleanup;
	}

	ctx->targetPath = argv[1];

	/* convert input millisecond to second */

	if(argc >= 3){
		ctx->analyzerParams.channelOffset = atof(argv[2]);
	}

	if(argc >= 4){
		double frameRate = 0.0f;
		frameRate = atof(argv[3]);
		if(frameRate != 0.0f){
			ctx->analyzerParams.sampleFrameRate = frameRate;
		}
	}
	
cleanup:
	return ret;
}

static int analyzeFile(PSMOOTH_CONTEXT ctx, std::string file)
{
	std::string ChannelData;
	int32_t ret = 0;
	PulseAnalyzer *smoothAnalyzer = NULL;
	smoothAnalyzer = new PulseAnalyzer(file);
	WaveAnalyzer *wavAnalyzer = new WaveAnalyzer(file);

	Logger(Info, "File %s, channel offset %f", file.c_str(), ctx->analyzerParams.channelOffset);

	smoothAnalyzer->SetWorkingParam(ctx->analyzerParams);
	wavAnalyzer->AnalyzeFilePulse();
	smoothAnalyzer->SetAnalyzerData(wavAnalyzer->GetPulseData());

	Logger(Info, "\nAnalyzer... ");

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
		Logger(Error ,"Invalid file path %d, code %d\n", path, GetLastError());
		return -1;
	}

	rPath.clear();
	rPath = buffer;
	rPath.insert(rPath.size(), "\\");
	return 0;
}

int main(int argc, char* argv[])
{
	int32_t ret = 0;
	PSMOOTH_CONTEXT pCtx = new SMOOTH_CONTEXT;

#ifdef _DEBUG
	argc = sizeof(debug_args)/sizeof(debug_args[0]);
	debug_args[0] = argv[0];
	argv = debug_args;
#endif

	if(parse_parameters(pCtx, argc, argv) < 0){
		goto cleanup;
	}

	pCtx->fileFinder = new FileEnumer();
	if(!pCtx->fileFinder){
		Logger(Error, "Can not enum file.");
		goto cleanup;
	}

	ret = pCtx->fileFinder->IsDirectory(pCtx->targetPath);
	if(ret < 0){
		Logger(Fatal, "path %s is invalid.", argv[1]);
	}else if (ret > 0){
		std::string file;
		if(GetAbsolutlyPath(argv[1], pCtx->targetPath) < 0){
			goto cleanup;
		}
		pCtx->fileFinder->EnumDirectory(pCtx->targetPath + "*", ".wav");
		while(!pCtx->fileFinder->GetFile(file)){
			file.insert(0, pCtx->targetPath);
			analyzeFile(pCtx, file);
		}
	}else {
		analyzeFile(pCtx, pCtx->targetPath);
	}

	ret = 0;

cleanup:
	if(pCtx){
		SAFE_DELETE(pCtx->fileFinder);
		SAFE_DELETE(pCtx);
	}

	return ret;
}

