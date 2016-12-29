#include "StdAfx.h"
#include "fileEnum.h"

FileEnumer::FileEnumer(void)
{
}

FileEnumer::~FileEnumer(void)
{
}

int32_t FileEnumer::GetFile(STRING &filename)
{
	int32_t ret = 0;
	STRING listFile;
	if(m_filesList.size()){
		listFile = m_filesList.front();
		m_filesList.pop_front();
		filename = listFile;
	}else{
		ret = -1;
	}

	return ret;
}

int32_t FileEnumer::IsDirectory(STRING &path)
{
	int32_t bRet = 0;
	DWORD fileAttr = 0;
	fileAttr = GetFileAttributes(path.c_str());
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

int32_t FileEnumer::EnumDirectory(STRING path, STRING postPix)
{
	STRING filename;
	try{
		WIN32_FIND_DATA fd = {0};
		HANDLE hFindFile = FindFirstFile(path.c_str(), &fd);
		if(hFindFile == INVALID_HANDLE_VALUE){
			Logger(Error, "Open path %s", path.c_str());
			FindClose(hFindFile);
			return 0;
		}

		do{
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
				filename.assign(fd.cFileName);
				char *pFile = (char*)filename.c_str();
				if(memcmp((pFile+filename.size()-postPix.size()), postPix.c_str(), postPix.size()) == 0){
					m_filesList.push_back(filename);
				}
			}
		}while (FindNextFile(hFindFile, &fd) != 0);

		FindClose(hFindFile);
		
	}catch(...){ 
		Logger(Fatal, "open %s.", path.c_str());
		return -1; 
	}
	return 0;
}