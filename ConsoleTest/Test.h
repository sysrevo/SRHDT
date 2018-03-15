#pragma once
#include <string>
#include <vector>

namespace Test
{
	void Init();
	void Test(const std::string& test_name, const std::vector<std::string>& cases_name);
	std::vector<std::string> GetTestCaseNames();
	void Learn(const std::string& name);
}
