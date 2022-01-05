#include <iostream>
#include "robot.hpp"
#include <map>
#include <mirai.h>
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
	return 0;
	MiraiBot bot;
	SessionOptions opts;
	opts.BotQQ = 1706866519_qq;				// 请修改为你的机器人QQ
	opts.HttpHostname = "localhost";		// 请修改为和 mirai-api-http 配置文件一致
	opts.WebSocketHostname = "localhost";	// 同上
	opts.HttpPort = 8080;					// 同上
	opts.WebSocketPort = 8080;				// 同上
	opts.VerifyKey = "cao_robot";			// 同上
	
	while (true)
	{
		try
		{
			cout << "尝试与 mirai-api-http 建立连接..." << endl;
			bot.Connect(opts);
			break;
		}
		catch (const std::exception& ex)
		{
			cout << ex.what() << endl;
		}
		MiraiBot::SleepSeconds(1);
	}
	cout << "Bot Working..." << endl;

	// 用map记录哪些群启用了“反撤回”功能
	map<GID_t, bool> groups;

	bot.On<GroupMessage>(
		[&](GroupMessage m)
		{
			try
			{
				string plain = m.MessageChain.GetPlainText();
				if (plain == "/anti-recall enabled." || plain == "撤回没用")
				{
					groups[m.Sender.Group.GID] = true;
					m.Reply(MessageChain().Plain("撤回也没用，我都看到了"));
					return;
				}
				if (plain == "/anti-recall disabled." || plain == "撤回有用")
				{
					groups[m.Sender.Group.GID] = false;
					m.Reply(MessageChain().Plain("撤回有用"));
					return;
				}
			}
			catch (const std::exception& ex)
			{
				cout << ex.what() << endl;
			}
		});


	bot.On<GroupRecallEvent>(
		[&](GroupRecallEvent e)
		{
			try
			{
				if (!groups[e.Group.GID]) return;
				auto recalled_mc = bot.GetGroupMessageFromId(e.MessageId).MessageChain;
				auto mc = "刚刚有人撤回了: " + recalled_mc;
				bot.SendMessage(e.Group.GID, mc);
			}
			catch (const std::exception& ex)
			{
				cout << ex.what() << endl;
			}
		});

	// 在失去与mah的连接后重连
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
				MiraiBot::SleepSeconds(1);
			}
		});

	string cmd;
	while (cin >> cmd)
	{
		if (cmd == "exit")
		{
			// 程序结束前必须调用 Disconnect，否则 mirai-api-http 会内存泄漏。
			bot.Disconnect();
			break;
		}
	}

	return 0;
}