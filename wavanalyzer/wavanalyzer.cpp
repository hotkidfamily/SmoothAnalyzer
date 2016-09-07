// wavanalyzer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "log.h"
#include "wavFileParse.h"

#ifdef _DEBUG
char* debug_args[] ={
	"",
	"e:/Resources/iphone.wav",
};
#endif

void print_usage()
{
	inter_log(Info, "usage: %s input");
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
		goto cleanup;
	}

	wavFileParse *parse = new wavFileParse;
	parse->openWavFile(argv[1]);
	parse->parseLRSync();
	parse->closeWavFile();
cleanup:
	return 0;
}

