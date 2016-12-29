#pragma once 


class iDump{
public:
	iDump(STRING &filepath){
		//dumpfile.open(filepath.c_str(), std::ios::binary);
	}

	int32_t DumpData(std::string &data)
	{
		if(dumpfile.is_open())
			dumpfile.write(data.c_str(), data.size());

		return 0;
	}

protected:
	std::ofstream dumpfile;
};
