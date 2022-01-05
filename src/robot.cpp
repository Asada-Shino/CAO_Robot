#include "robot.hpp"
#include "information.hpp"
#include <fstream>
#include <iostream>

CAORobot::CAORobot() {
    opts.BotQQ = QQ;				
	opts.HttpHostname = "localhost";		
	opts.WebSocketHostname = "localhost";	
	opts.HttpPort = 8080;					
	opts.WebSocketPort = 8080;				
	opts.VerifyKey = VERIFY_CODE;			
}

void CAORobot::init() {
    module.init();
	for(int i = 0; i < 3; ++i)
	{
		try
		{
			bot.Connect(opts);
			break;
		}
		catch (const std::exception& ex)
		{
			cout << ex.what() << endl;
		}
		MiraiBot::SleepSeconds(1);
	}
}