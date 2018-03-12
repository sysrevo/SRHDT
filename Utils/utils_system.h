#pragma once
#include "common.h"

namespace imgsr
{
	namespace utils
	{
		namespace filesys
		{
			vector<string> GetFilesInDir(const string & directory_path);
			__int64 GetFileSize(std::ifstream & ifs);
			vector<char> ReadFileBytes(std::ifstream & ifs);
			string ReadFileString(std::ifstream & ifs);
		} // filesys
	} // utils
} // imagesr