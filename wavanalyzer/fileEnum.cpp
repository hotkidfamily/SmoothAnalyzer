#include "StdAfx.h"
#include "fileEnum.h"

fileEnum::fileEnum(void)
{
}

fileEnum::~fileEnum(void)
{
}

int fileEnum::getFile(std::string &filename)
{
	int ret = 0;
	std::string listFile;
	if(m_filesList.size()){
		listFile = m_filesList.front();
		m_filesList.pop_front();
		filename = listFile;
	}else{
		ret = -1;
	}

	return ret;
}

int fileEnum::isDirectory(std::string path)
{
	int bRet = 0;
	DWORD fileAttr = 0;
	fileAttr = GetFileAttributesA(path.c_str());
	if(fileAttr & FILE_ATTRIBUTE_DIRECTORY){
		bRet = 1;
	}else{
		bRet = 0;
	}

	if(fileAttr == INVALID_FILE_ATTRIBUTES){
		bRet = -1;
	}

	return bRet;
}

int fileEnum::enumDirectory(std::string path, std::string postPix)
{
	LARGE_INTEGER filesize;
	std::string filename;
	try{
		WIN32_FIND_DATAA fd = {0};
		HANDLE hFindFile = FindFirstFileA(path.c_str(), &fd);
		if(hFindFile == INVALID_HANDLE_VALUE){
			inter_log(Error, "Error open path %s", path.c_str());
			FindClose(hFindFile);
			return 0;
		}

		do{
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
				filesize.LowPart = fd.nFileSizeLow;
				filesize.HighPart = fd.nFileSizeHigh;
				filename.assign(fd.cFileName);
				char *pFile = (char*)filename.c_str();
				if(memcmp((pFile+filename.size()-postPix.size()), postPix.c_str(), postPix.size()) == 0){
					m_filesList.push_back(filename);
				}
			}
		}while (FindNextFileA(hFindFile, &fd) != 0);

		FindClose(hFindFile);
		
	}catch(...){ 
		inter_log(Fatal, "open %s.", path.c_str());
		return -1; 
	}
	return 0;
}