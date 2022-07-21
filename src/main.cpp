#include <iostream>
#include <thread>
#include "robot.hpp"
#include "utils.hpp"
using namespace std;
using namespace Cyan;

void is_to_exit(int *flag) {
	string cmd;
	while(!*flag) {
		cin >> cmd;
		if(cmd == "exit")
			*flag = 1;
	}
}

int main()
{
	CAORobot robot;
	try {
		robot.init();
	}
	catch(...) {
		cout << "sth wrong" << endl;
	}
	int flag = 0;
	
	thread t(is_to_exit, &flag);
	t.detach();
	while (true)
	{
		if(flag == 1)
			break;
		else {
			robot.checkSchedule();
		}
	}
	robot.release();
	return 0;
}