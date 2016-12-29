#pragma once

#include "pulse.h"
#include "..\libxl-3.7.2.0\include_cpp\libxl.h"

#pragma comment(lib, "..\\libxl-3.7.2.0\\lib\\libxl")
using namespace libxl;

class xlsOperator
{
public:
	xlsOperator(void);
	~xlsOperator(void);

	bool CreateBook();
	Sheet* CreateSheet(std::string sheetName);
	bool SaveAndCloseBook(std::string filename);
	void WritePulseAtRowCol(Sheet *&,int32_t, int32_t, PulseDesc *, PulseDesc *);

protected:
	void WritePulseAtRowCol(Sheet *&sheet, int32_t row, int32_t col, PulseDesc* lDesc);

private:
	Book *mBook;
};
