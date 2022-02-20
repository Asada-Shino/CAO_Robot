#include <iostream>
#include "robot.hpp"
#include "utils.hpp"
using namespace std;
using namespace Cyan;

int main()
{
	CAORobot robot;
	try {
		robot.init();
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