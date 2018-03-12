#include "utils_system.h"
#include <Windows.h>
#include <codecvt>

using namespace imgsr;
using namespace imgsr::utils;
using std::ios;
using std::ifstream;
using std::ofstream;

vector<string> filesys::GetFilesInDir(const string & directory_path)
{
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> convert;

	wstring dir_path_wstr = convert.from_bytes(directory_path);

	HANDLE dir;
	WIN32_FIND_DATA file_data;
	vector<wstring> result;

	if ((dir = FindFirstFile((dir_path_wstr + L"/*").c_str(), &file_data)) == INVALID_HANDLE_VALUE)
		return vector<string>(); /* No files found */

	do {
		const wstring file_name = file_data.cFileName;
		const wstring full_file_name = dir_path_wstr + L"/" + file_name;
		const bool is_directory = (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

		if (file_name[0] == '.')
			continue;

		if (is_directory)
			continue;

		result.push_back(full_file_name);
	} while (FindNextFile(dir, &file_data));

	FindClose(dir);

	vector<string> out;

	for (const auto & str : result)
		out.push_back(convert.to_bytes(str));
	return out;
}

__int64 filesys::GetFileSize(ifstream & ifs)
{
	ifs.seekg(0, std::ios::end);
	__int64 size_file = ifs.tellg();
	ifs.seekg(0, std::ios::beg);
	return size_file;
}

vector<char> filesys::ReadFileBytes(ifstream & ifs)
{
	__int64 size = GetFileSize(ifs);
	ifs.seekg(0, std::ios::beg);

	vector<char> bytes(size);
	ifs.read(bytes.data(), size);

	return bytes;
}

string imgsr::utils::filesys::ReadFileString(std::ifstream & ifs)
{
	vector<char> buf = ReadFileBytes(ifs);
	return string(buf.begin(), buf.end());
}
