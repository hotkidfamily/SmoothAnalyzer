// wavanalyzer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "waveAnalyzer.h"
#include "fileEnum.h"
#include "SmoothAnalyzer.h"

#define TEST 5

#ifdef _DEBUG
TCHAR* debug_args[] ={
	_T(""),
	_T("-i"),
#if (TEST == 1)
	_T("e:\\smooth\\ios-sony-entertainment.wav"),
#elif (TEST == 2)
	_T("e:\\smooth\\xiaomi4-sony-entertainment.wav"),
	_T("-offset"),
	_T("-3"),
#elif (TEST == 3)
	_T("e:\\smooth\\mpc-hc-sony-entertainment-default-render.wav"),
#elif (TEST == 4)
	_T("C:\\workspace\\SmoothAnalyzer\\samples\\ios-sony-entertainment.wav"),
	_T("-offset"),
	_T("-8"),
#elif (TEST == 5)
	_T("C:\\workspace\\SmoothAnalyzer\\samples\\xiaomi4-sony-entertainment.wav"),
	_T("-offset"),
	_T("-4"),
#else
	_T("C:\\workspace\\SmoothAnalyzer\\samples\\mac-sony-entertainment.wav"),
#endif
	_T("-fps"),
	_T("30"),
	_T("-loglevel"),
	_T("3")
};
#endif

typedef struct programContext{
	programContext(){
		ZeroMemory(this, sizeof(struct programContext));
	}
	ANALYZER_PARAMS analyzerParams;
	STRING targetPath;
	FileEnumer *fileFinder;
	int32_t logLevel;
}SMOOTH_CONTEXT, *PSMOOTH_CONTEXT;

static void print_usage(const TCHAR *name)
{
	const TCHAR *help = _T("\n\tAnazlyer a smooth steam wave file.\n\n")
		_T("Usage:\t%s -i <PATH>/<File Name> -loglevlel -pulse -offset -fps\n")
		_T("e.g., \t%s a.wav -loglevel 3 -pulse 16 -offset -8 -fps 30\n\n")
		_T("Tips:\tIf you want indicate channels different Please fill [offset] in millisecond.\n")
		_T("     \tAnd if you know frame rate Please fill [fps].\n")
		_T("     \t[i]         - Set input file name.\n")
		_T("     \t[loglevel]  - Set log level, default is 3(debug), minist is 0, biggest is 7.\n")
		_T("     \t[pulse]     - Set Minist Pulse width for detect valid frame.\n")
		_T("     \t[offset]    - Negative indicate ahead, Positive indicate behind.\n")
		_T("     \t[fps]	   - Frame rate of analyzed view, if you want inut frame rate alone, you should make offset to 0.\n")
		_T("     \t             e.g., frame rate is 30fps then input \"0 0 30\" .\n\n")
		_T("Warning:Only Support .WAV File With 2 Channels.\n\n");

	_tprintf(help, name, name);	
}

static int32_t parse_parameters(SMOOTH_CONTEXT* ctx, const int32_t argc, TCHAR* argv[])
{
	int32_t ret = 0;
	if(argc < 2){
		print_usage(argv[0]);
		ret = -1;
		goto cleanup;
	}

	for(int i = 1; i<argc; i++)
	{
		if (_tcscmp(_T("-i"), argv[i]) == 0 && (i + 1) < argc){
			ctx->targetPath = argv[++i];
		}
		else if (_tcscmp(_T("-loglevel"), argv[i]) == 0 && (i + 1) < argc){
			ctx->logLevel = _ttoi(argv[++i]);
			if(ctx->logLevel>=FileSystem && ctx->logLevel<=Fatal){
				SetLoggerLvel((log_level)ctx->logLevel);
			}
		}
		else if (_tcscmp(_T("-pulse"), argv[i]) == 0 && (i + 1) < argc){
			ctx->analyzerParams.validPulseWidth = _ttoi(argv[++i]);
		}
		else if (_tcscmp(_T("-fps"), argv[i]) == 0 && (i + 1) < argc){
			double frameRate = 0.0f;
			frameRate = _tstof(argv[++i]);
			if(frameRate != 0.0f){
				ctx->analyzerParams.sampleFrameRate = frameRate;
			}
		}
		else if (_tcscmp(_T("-offset"), argv[i]) == 0 && (i + 1) < argc){
			ctx->analyzerParams.channelOffset = _tstof(argv[++i]);
		}
	}

cleanup:
	return ret;
}

static int analyzeFile(PSMOOTH_CONTEXT ctx, STRING file)
{
	std::string ChannelData;
	int32_t ret = 0;
	PulseAnalyzer *smoothAnalyzer = NULL;
	std::string fileNameAsc;
	fileNameAsc.assign(file.begin(), file.end());
	smoothAnalyzer = new PulseAnalyzer(file);
	WaveAnalyzer *wavAnalyzer = new WaveAnalyzer(file);

	Logger(Info, "File %s, channel offset %f", fileNameAsc.c_str(), ctx->analyzerParams.channelOffset);

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

static int GetAbsolutlyPath(const TCHAR* path, STRING &rPath)
{
	DWORD retVal = 0;
	STRING cupath = path;
	std::string fileNameAsc;
	fileNameAsc.assign(cupath.begin(), cupath.end());
	TCHAR buffer[MAX_PATH] = {_T('\0')};
	TCHAR *retPath = {NULL};
	retVal = GetFullPathName(path, MAX_PATH, buffer, &retPath);
	if (retVal == 0) {
		Logger(Error ,"Invalid file path %d, code %d\n", fileNameAsc.c_str(), GetLastError());
		return -1;
	}

	rPath.clear();
	rPath = buffer;
	rPath.insert(rPath.size(), _T("\\"));
	return 0;
}

int main(int argc, TCHAR* argv[])
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
		STRING file;
		if(GetAbsolutlyPath(argv[1], pCtx->targetPath) < 0){
			goto cleanup;
		}
		pCtx->fileFinder->EnumDirectory(pCtx->targetPath + _T("*"), _T(".wav"));
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

