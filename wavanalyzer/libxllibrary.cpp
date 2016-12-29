#include "StdAfx.h"
#include "libxllibrary.h"

static const TCHAR *keyFileName = TEXT("libxl.key");

xlsOperator::xlsOperator(void)
{
}

xlsOperator::~xlsOperator(void)
{
}

bool xlsOperator::CreateBook()
{
	bool bRet = false;
#ifdef UNICODE
	std::wstring name;
	std::wstring key;
	std::wifstream keyfile;
#else
	std::string name;
	std::string key;
	std::ifstream keyfile;
#endif

	keyfile.open(keyFileName);
	if(!keyfile.is_open()){
		Logger(Error, "Can not open key file :%s ", keyFileName);
		goto cleanup;
	}

	std::getline(keyfile, name);
	std::getline(keyfile, key);

	mBook = xlCreateBook();
	if(mBook){
		mBook->setKey(name.c_str(), key.c_str());
		bRet = true;
	}

cleanup:
	return bRet;
}

Sheet* xlsOperator::CreateSheet(std::string sheetName)
{
	return mBook->addSheet(sheetName.c_str());
}

bool xlsOperator::SaveAndCloseBook(std::string filename)
{
	bool ret = false;

	if(mBook){
		if(mBook->save(filename.c_str())) 
		{
			ret = true;
			Logger(Info, "save file %s", filename.c_str());
		}
		else
		{
			ret = false;
			Logger(Error, "save file %s - %s", filename.c_str(), mBook->errorMessage());
		}

		mBook->release();
	}

	return ret;
}

void xlsOperator::WritePulseAtRowCol(Sheet *&sheet, int32_t row, int32_t col, PulseDesc* desc)
{
	std::string channle;
	channle.assign(1, desc->channelName);
	sheet->writeStr(row, col++, channle.c_str());
	sheet->writeNum(row, col++, desc->index);
	sheet->writeNum(row, col++, desc->start);
	sheet->writeNum(row, col++, desc->end);
	sheet->writeNum(row, col++, desc->duration);
	sheet->writeNum(row, col++, desc->type);
}

void xlsOperator::WritePulseAtRowCol(Sheet *&sheet, int32_t row, int32_t col, PulseDesc* lDesc, PulseDesc* rDesc)
{
	if(lDesc){
		WritePulseAtRowCol(sheet, row, col, lDesc);
	}
	
	if(rDesc){
		col += 6;
		WritePulseAtRowCol(sheet, row, col, rDesc);
	}
}