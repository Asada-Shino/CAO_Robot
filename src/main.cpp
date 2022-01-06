#include <iostream>
#include "robot.hpp"
#include <map>
#include <mirai.h>
#include <fstream>
using namespace std;
using namespace Cyan;

int main()
{
	CAORobot robot;
	try {
		robot.init();
	}
	catch(const char* s) {
		cout << s << endl;
	}
	catch(...) {
		cout << "sth wrong" << endl;
	}
	string cmd;
	while (cin >> cmd)
	{
		if (cmd == "exit")
		{
			robot.release();
			break;
		}
	}

	return 0;
}