#include "Test.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::vector;

int main()
{
	int input;
	cout << "Training? ";
	cin >> input;
	bool learn = input != 0;
	cout << "#Training: " << (learn == 0 ? "False" : "True") << endl;;

    Test::Init();


	cout << "Select name from below:" << endl;
	vector<string> names = Test::GetTestCaseNames();
	for (const auto& name : names) cout << name << endl;
	string name;
	cin >> name;
	cout << "#Name: " << name << endl;

    if (learn)
    {
        Test::Learn(name);
    }
    else
    {
		cout << "Input test case names, end with #" << endl;
		vector<string> imgs_name;
		std::sort(imgs_name.begin(), imgs_name.end());

		string tmp;
		while (true)
		{
			cin >> tmp;
			if (tmp.empty() || tmp[0] == '#')
				break; 
			imgs_name.push_back(tmp);
		}
        Test::Test(name, imgs_name);
    }
    return 0;
}