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
	Sheet* CreateSheet(STRING);
	Sheet* InsertSheet(STRING);
	bool SaveAndCloseBook(STRING);

	Format *CreateFormat();

	void WriteMultiplePulseAtRowCol(Sheet *&,int32_t, int32_t, PulseDesc *, PulseDesc *);
	void Printf(Sheet *&,int32_t, int32_t, TCHAR *format, ...);
	void WriteLineWithString(Sheet *&sheet, int32_t row, int32_t col, TCHAR *str);

protected:
	void WritePulseAtRowCol(Sheet *&, int32_t , int32_t , PulseDesc* );

private:
	Book *mBook;
};
