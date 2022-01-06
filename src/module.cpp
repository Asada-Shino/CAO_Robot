#include "module.hpp"
#include "information.hpp"
#include <fstream>
#include <ctime>

Module::Module() {
    opts.BotQQ = BOT_QQ;				
	opts.HttpHostname = "localhost";		
	opts.WebSocketHostname = "localhost";	
	opts.HttpPort = 8080;					
	opts.WebSocketPort = 8080;				
	opts.VerifyKey = VERIFY_CODE;
}

void Module::init() {
    while(true)
	{
		try
		{
			bot.Connect(opts);
            cout << "bot working" << endl;
			break;
		}
		catch (const std::exception& ex)
		{
			cout << ex.what() << endl;
		}
		MiraiBot::SleepSeconds(1);
	}
	bot.On<GroupMessage>([&](GroupMessage m){deal_group_message(m);});
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

    load_config_file();
}

void Module::load_config_file() {
    fstream f("config/config.json");
    if(f.fail())
        throw "can't find config.json";
    string config_s;
    while(!f.eof()) {
        string s;
        f >> s;
        config_s+=s;
    }
    f.close();
    config = json::parse(config_s);
    for(auto item : config["super_admin"].items()) {
        super_admin_list.insert(QQ_t(item.value()));
    }
    for(auto item : config["black_list"].items()) {
        black_list.insert(QQ_t(item.value()));
    }
    for(auto item : config["enabled_group"].items()) {
        enabled_group_list.insert(GID_t(item.value()));
    }
    for(auto g : enabled_group_list) {
        string group_s = to_string(g.ToInt64());
        map<string, bool>& group = (group_settings[g] = map<string, bool>());
        for(auto func : func_names) {
            if(config["group_settings"][group_s][func] == nullptr)
                config["group_settings"][group_s][func] = false;
            group[func] = config["group_settings"][group_s][func];
        }
    }
}

void Module::save_config_file() {
    fstream f("config/config.json", ios::out);
    if(f.fail())
        throw "can't open config.json";
    f << to_string(config);
    f.close();
}

void Module::release() {
	bot.Disconnect();
    save_config_file();
}

void Module::deal_group_message(GroupMessage m) {
    try {
        if(enabled_group_list.count(m.Sender.Group.GID) > 0) {
            vector<string> cmd;
            command_parser(cmd, m.MessageChain.GetPlainText());
            for(auto s : cmd) {
                cout << s << endl;
            }
            if(cmd.size() == 2 && cmd[0] == "enable") {
                if(enable(m.Sender.Group, m.Sender, cmd[1]))
                    m.QuoteReply(MessageChain().Plain("功能\""+cmd[1]+"\"已启用。"));
            }
            else if(cmd.size() == 2 && cmd[0] == "disable") {
                if(disable(m.Sender.Group, m.Sender, cmd[1]))
                    m.QuoteReply(MessageChain().Plain("功能\""+cmd[1]+"\"已禁用。"));
            }
            else if(cmd.size() >= 2 && cmd[0] == "ban" && group_settings[m.Sender.Group.GID]["ban"] == true) {
                ban(m.Sender.Group, m.Sender, m.MessageChain.GetFirst<AtMessage>().Target(), stol(cmd[1]), cmd.size() >=3?cmd[2]:"");
            }
            else if(cmd.size() >= 1 && cmd[0] == "kick" && group_settings[m.Sender.Group.GID]["kick"] == true) {
                kick(m.Sender.Group, m.Sender, m.MessageChain.GetFirst<AtMessage>().Target(), cmd.size() >= 2? cmd[1] : "");
            }
            else if(group_settings[m.Sender.Group.GID]["repeat-analysis"] == true) {
                optional<MessageChain> mc = repeat_analysis(m.Sender.Group, m.Sender, m.MessageChain, m.Timestamp());
                if(mc != nullopt)
                    m.Reply(mc.value());
            }
        }
    }
    catch (const std::exception& ex) {
			cout << ex.what() << endl;
		}
}

bool Module::enable(Group_t& group, GroupMember& sender, string func_name) {
    if(super_admin_list.count(sender.QQ) > 0 && func_names.count(func_name) > 0 && group_settings[group.GID][func_name] == false) {
        group_settings[group.GID][func_name] = true;
        config["group_settings"][to_string(group.GID.ToInt64())][func_name] = true;
        return true;
    }
    return false;
}

bool Module::disable(Group_t& group, GroupMember& sender, string func_name) {
    if(super_admin_list.count(sender.QQ) > 0 && func_names.count(func_name) > 0 && group_settings[group.GID][func_name] == true) {
        group_settings[group.GID][func_name] = false;
        config["group_settings"][to_string(group.GID.ToInt64())][func_name] = false;
        return true;
    }
    return false;
}

void Module::command_parser(vector<string>& cmd, string plain_text) {
    int start = 0;
    bool blank = false;
    for(int i = 0 ; i < plain_text.length(); ++i) {
        if(plain_text[i] == ' ' || plain_text[i] == '\n') {
            if(blank == false) {
                cmd.push_back(plain_text.substr(start, i-start));
                blank = true;
            }
        }
        else {
            if(blank == true) {
                start = i;
                blank = false;
            }
        }
    }
    if(blank == false)
        cmd.push_back(plain_text.substr(start));
}

void Module::ban(Group_t& group, GroupMember& sender, QQ_t target, int minute, string reason) {
    if((sender.Permission >= GroupPermission::Administrator || super_admin_list.count(sender.QQ) > 0) && super_admin_list.count(target) == 0) {
        if(minute == 0)
            bot.UnMute(group.GID, target);
        if(minute > 0 && minute <= 43200)
        bot.Mute(group.GID, target, minute*60);
    }
}

void Module::kick(Group_t& group, GroupMember& sender, QQ_t target, string reason) {
    if((sender.Permission >= GroupPermission::Administrator || super_admin_list.count(sender.QQ) > 0) && super_admin_list.count(target) == 0) {
        bot.Kick(group.GID, target);
    }
}

optional<MessageChain> Module::repeat_analysis(Group_t& group, GroupMember& sender, MessageChain msg, time_t timestamp) {
    static map<GID_t, pair<MessageChain, time_t>> last_text_map;
    static map<GID_t, set<QQ_t>> repeat_count; 
    if(repeat_count.find(group.GID) == repeat_count.end())
        repeat_count[group.GID] = set<QQ_t>();
    auto &qq_set = repeat_count[group.GID];
    if(last_text_map.find(group.GID) == last_text_map.end()) {
        last_text_map[group.GID] = pair<MessageChain, time_t>(msg, timestamp);
        qq_set.insert(sender.QQ);
        return nullopt;
    }
    cout << "last: "<< last_text_map[group.GID].first.ToString() <<"\nnow: " << msg.ToString() << "\nCompare: " << (last_text_map[group.GID].first == msg) << endl;
    
    if(last_text_map[group.GID].first == msg) {
        if(qq_set.find(sender.QQ) == qq_set.end())
            qq_set.insert(sender.QQ);
    }
    else {
        MessageChain mc;
        if(qq_set.size() >= 3) {
            mc = MessageChain().Plain("本次接龙内容：")+last_text_map[group.GID].first+
            MessageChain().Plain("\n共计"+to_string(qq_set.size())+"人参与\n历时"+to_string(timestamp - last_text_map[group.GID].second)+"秒");
            last_text_map[group.GID] = pair<MessageChain, time_t>(msg, timestamp);
            qq_set.clear();
            qq_set.insert(sender.QQ);
            return mc;
        }
        last_text_map[group.GID] = pair<MessageChain, time_t>(msg, timestamp);
        qq_set.clear();
        qq_set.insert(sender.QQ);
    }
    return nullopt;
}
