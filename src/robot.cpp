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
    module.init();
	bot.On<GroupMessage>([&](GroupMessage m){module.deal_group_message(m);});
	bot.On<LostConnection>([&](LostConnection e)
		{
			cout << e.ErrorMessage << " (" << e.Code << ")" << endl;
			while (true)
			{
				try
				{
					cout << "尝试连接 mirai-api-http..." << endl;
					bot.Reconnect();
					cout << "与 mirai-api-http 重新建立连接!" << endl;
					break;
				}
				catch (const std::exception& ex)
				{
					cout << ex.what() << endl;
				}
				MiraiBot::SleepSeconds(5);
			}
		});

}

void CAORobot::release() {
	bot.Disconnect();
	module.release();
}