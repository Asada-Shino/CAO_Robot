#include "robot.hpp"
#include <fstream>
#include <iostream>

CAORobot::CAORobot() {			
}

void CAORobot::init() {
	module.init();
}

void CAORobot::release() {
	module.release();
}

void CAORobot::checkSchedule() {
	module.checkSchedule();
}