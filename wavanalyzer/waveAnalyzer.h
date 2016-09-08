#pragma once
#include <string>
#include <fstream>

#include "stdint.h"
typedef enum tagRetType{
	RET_OK,
	RET_NEED_MORE_DATA,
	RET_FIND_START,
}retType;

class waveAnalyzer
{
public:
	waveAnalyzer(void);
	~waveAnalyzer(void);

	retType analyzer(std::string &lChannel, int32_t &ms);

private:
	int filter(std::string &channelData);
	int findStart(std::string &channelData);
};
