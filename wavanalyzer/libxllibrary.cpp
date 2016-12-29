#include "StdAfx.h"
#include "libxllibrary.h"

static const TCHAR *keyFileName = TEXT("libxl.key");

xlsOperator::xlsOperator(void)
: mBook(NULL)
{
}

xlsOperator::~xlsOperator(void)
{
}

bool xlsOperator::CreateBook()
{
	bool bRet = false;
	STRING name;
	STRING key;
	IFSTREAM keyfile;
	STRING fontName(TEXT("Monaco"));

	keyfile.open(keyFileName);
	if(!keyfile.is_open()){
		Logger(Error, "Can not open key file :%s ", keyFileName);
		goto cleanup;
	}

	std::getline(keyfile, name);
	std::getline(keyfile, key);

	mBook = xlCreateXMLBook();
	if(mBook){
		mBook->setKey(name.c_str(), key.c_str());

		mBook->setDefaultFont(fontName.c_str(), 10);

		bRet = true;
	}

cleanup:
	return bRet;
}

Format *xlsOperator::CreateFormat()
{
	Format *xlsfmt = NULL;

	if(mBook){
		xlsfmt = mBook->addFormat();

		if(xlsfmt){
			xlsfmt->setNumFormat(NUMFORMAT_NUMBER_D2);
			xlsfmt->setAlignH(ALIGNH_CENTER);
			xlsfmt->setAlignV(ALIGNV_CENTER);
			xlsfmt->setPatternBackgroundColor(COLOR_GRAY25);
		}
	}

	return xlsfmt;
}

Sheet* xlsOperator::CreateSheet(STRING sheetName)
{
	Sheet *temp = NULL;
	if(mBook)
		temp = mBook->addSheet(sheetName.c_str());
	
	return temp;
}

Sheet* xlsOperator::InsertSheet(STRING sheetName)
{
	Sheet *temp = NULL;
	if(mBook){
		temp = mBook->insertSheet(0, sheetName.c_str());
		if(temp)
			mBook->setActiveSheet(0);
	}

	return temp;
}

bool xlsOperator::SaveAndCloseBook(STRING filename)
{
	bool ret = false;
	std::string str(filename.begin(), filename.end());

	if(mBook){
		if(mBook->save(filename.c_str())) {
			ret = true;

			Logger(Info, "save file %s", str.c_str());
		} else {
			ret = false;
			Logger(Error, "save file %s - %s", str.c_str(), mBook->errorMessage());
		}

		mBook->release();
	}

	return ret;
}

void xlsOperator::WritePulseAtRowCol(Sheet *&sheet, int32_t row, int32_t col, PulseDesc* desc)
{
	STRING channle;
	channle.assign(1, desc->channelName);
	if(sheet){
		sheet->writeStr(row, col++, channle.c_str());
		sheet->writeNum(row, col++, desc->index);
		sheet->writeNum(row, col++, desc->start);
		sheet->writeNum(row, col++, desc->end);
		sheet->writeNum(row, col++, desc->duration);
		sheet->writeNum(row, col++, desc->type);
	}
}

void xlsOperator::WriteMultiplePulseAtRowCol(Sheet *&sheet, int32_t row, int32_t col, PulseDesc* lDesc, PulseDesc* rDesc)
{
	if(lDesc){
		WritePulseAtRowCol(sheet, row, col, lDesc);
	}
	
	if(rDesc){
		col += 6;
		WritePulseAtRowCol(sheet, row, col, rDesc);
	}
}

void xlsOperator::WriteLineWithString(Sheet *&sheet, int32_t row, int32_t col, TCHAR *str)
{
	TCHAR *pstr = str;
	TCHAR split = TEXT(',');
	while(pstr){
		TCHAR val[256]={ _T('\0')};

		if(_stscanf_s(pstr, _T("%255[^,] "), val, _countof(val))){
			if(sheet)
				sheet->writeStr(row, col++, val);
		}

		pstr = _tcschr(pstr, split);

		pstr += !!pstr;
	}
}

#undef BUFFER_SIZE
#define BUFFER_SIZE (2048)
void xlsOperator::Printf(Sheet *&sheet, int32_t row, int32_t col, TCHAR *format, ...)
{
	va_list arg;
	int32_t colIndex = col;
	va_start(arg, format);
	int32_t length = 0;

	TCHAR ch;
	while (ch = *(format++))
	{
		length = 0;
		if (ch == _T('%'))
		{
			while(ch = *(format++)){
				if(isalpha(ch)){
					break;
				}
			}
			
			if (ch == _T('s'))
			{
				TCHAR *v = va_arg(arg, TCHAR *);
				if(sheet)
					sheet->writeStr(row, colIndex++, v);
			} else if (ch == _T('c')) {
				TCHAR v = va_arg(arg, TCHAR);
				STRING charistic;
				charistic.assign(1, v);
				if(sheet)
					sheet->writeStr(row, colIndex++, charistic.c_str());
			} else if (ch == _T('d')) {
				int v = va_arg(arg, int);
				if(sheet)
					sheet->writeNum(row, colIndex++, v);
			}else if (ch == _T('f')){
				double v = va_arg(arg, double);
				if(sheet)
					sheet->writeNum(row, colIndex++, v);
			}else if (ch == _T('u')){
				uint32_t v = va_arg(arg, uint32_t);
				if(sheet)
					sheet->writeNum(row, colIndex++, v);
			}else{
				Logger(Error, "can not parse option: %c", ch);
				continue;
			}
		}
	}

	va_end(arg);
}
#undef BUFFER_SIZE
