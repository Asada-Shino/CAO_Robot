#include "module.hpp"
#include "information.hpp"
#include "utils.hpp"
#include <fstream>
#include <ctime>
#include <stack>
#include <chrono>

struct music_t {
    std::string name;
    std::string artist;
    std::string previewUrl;
    int64_t id = 0;
    std::string jumpUrl;
    std::string playUrl;
    std::string error;
};

json getJson(string songName)
{
	json returnJson;
    string cmd = "python3 test.py \""+songName+"\" > res.txt";
	system(cmd.c_str());
    fstream res("res.txt", ios::in);
    string s;
    while(!res.eof()) {
        string temp;
        getline(res, temp);
        s+=temp;
    }
	returnJson = json::parse(s);

	return returnJson;
}
MessageChain test(string songName) {
    music_t music;
	json musicJson = getJson(songName);
	json miraiapp;

	//cout << musicJson << endl;

	if (musicJson.at("/result"_json_pointer).empty())
	{
		music.error = "music not found";
	}
	else
	{
		// 歌曲标题
		music.name =
			musicJson.at("/result/songs/0/name"_json_pointer).get<string>();
		// 歌曲作者
		music.artist =
			musicJson.at(
				"/result/songs/0/artists/0/name"_json_pointer).get<string>();
		// 歌曲封面链接
		music.previewUrl = "http://p2.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg";
		// 歌曲id
		music.id = 
			musicJson.at("/result/songs/0/id"_json_pointer).get<int64_t>();
		// 歌曲跳转链接
		music.jumpUrl = "https://music.163.com/#/song?id=" + to_string(music.id);
		// 歌曲播放链接
		music.playUrl ="http://music.163.com/song/media/outer/url?id=" + to_string(music.id) + ".mp3";
		// 错误信息
		music.error = "none";
	}
	if (music.error == "music not found")
	{
		return MessageChain().Plain("云村中没有这首歌哟。");
	}
	else
	{
		MusicShare share;
		share.Kind(MusicShareKind::NeteaseCloudMusic);
		share.Title(music.name);
		share.Summary(music.artist);
		share.JumUrl(music.jumpUrl);
		share.PictureUrl(music.previewUrl);
		share.MusicUrl(music.playUrl);
		share.Brief("[分享]" + music.name);
		return (MessageChain().Add<MusicShare>(share));
	}
}

Module::Module() {
    opts.BotQQ = BOT_QQ;				
	opts.HttpHostname = "localhost";		
	opts.WebSocketHostname = "localhost";	
	opts.HttpPort = 8080;					
	opts.WebSocketPort = 8080;				
	opts.VerifyKey = VERIFY_CODE;
}

void Module::init() {
    logger.init();
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
    //bot.On<TempMessage>([&](TempMessage m){deal_temp_message(m);});
    bot.On<MemberJoinEvent>([&](MemberJoinEvent m){bot.SendMessage(m.NewMember.Group.GID, MessageChain().At(m.NewMember.QQ).Plain(" 欢迎加入！"));});
    bot.On<FriendMessage>([&](FriendMessage m){deal_friend_message(m);});
    bot.On<NewFriendRequestEvent>([&](NewFriendRequestEvent m){m.Accept();});
	bot.On<LostConnection>([&](LostConnection e)
		{
			cout << e.ErrorMessage << " (" << e.Code << ")" << endl;
			while (true)
			{
				try
				{
					cout << "尝试连接 mirai-api-http......" << endl;
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
    srand(time(0));
}

void Module::load_config_file() {
    fstream f("config/config.json");
    if(f.fail())
        throw runtime_error("can't find config.json");
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
    for(auto func : func_names) {
        interval[func] = (config["interval"][func] == nullptr ? (config["interval"][func] = 0) :  config["interval"][func]);
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
    logger.release();
}

void Module::deal_group_message(GroupMessage m) {
    try {
        if(enabled_group_list.count(m.Sender.Group.GID) > 0) {
            logger.info("["+m.Sender.Group.Name+"("+to_string(m.Sender.Group.GID.ToInt64())+")] " + m.Sender.MemberName+"("+to_string(m.Sender.QQ.ToInt64())+") ["+to_string(m.MessageId())+"]: "+m.MessageChain.ToString());
            vector<AppMessage> appMsgVector = m.MessageChain.GetAll<AppMessage>();
            vector<string> cmd;
            command_parser(cmd, m.MessageChain.GetPlainText());
            if(cmd.size() == 2 && (cmd[0] == "enable" || cmd[0] == "disable")) {
                int count = change_funcs(m.Sender.Group, m.Sender, cmd[1], cmd[0] == "enable");
                if(count > 0)
                    m.QuoteReply(MessageChain().Plain((cmd[0] == "enable" ? "已启用" : "已禁用") +to_string(count)+"项功能。"));
            }
            else if(group_settings[m.Sender.Group.GID]["app-parser"] == true && !appMsgVector.empty()) {
                optional<MessageChain> msg = app_parser(m.Sender.Group, appMsgVector[0].Content());
                if(msg!=nullopt)
                    m.Reply(msg.value());
            }
            
            else if(group_settings[m.Sender.Group.GID]["timer"] == true && cmd.size() >= 7 && cmd[0] == "timer" && super_admin_list.count(m.Sender.QQ) > 0) {
                optional<MessageChain> msg = timer(m.Sender.Group, m.Sender, cmd);
                if(msg!=nullopt)
                    m.Reply(msg.value());
            }
            else if(cmd.size() == 2 && cmd[0] == "check") {
                m.QuoteReply(check(m.Sender.Group, m.Sender, cmd[1]));
            }
            else if(cmd.size() >= 2 && cmd[0] == "mute" && group_settings[m.Sender.Group.GID]["mute"] == true) {
                mute(m.Sender.Group, m.Sender, m.MessageChain.GetFirst<AtMessage>().Target(),string_to_duration(cmd[1]), cmd.size() >=3?cmd[2]:"");
            }
            else if(cmd.size() >= 1 && cmd[0] == "unmute" && group_settings[m.Sender.Group.GID]["mute"] == true) {
                unmute(m.Sender.Group, m.Sender, m.MessageChain.GetFirst<AtMessage>().Target(), cmd.size() >=3?cmd[2]:"");
            }
            else if(cmd.size() >= 3 && cmd[0] == "title" && group_settings[m.Sender.Group.GID]["title"] == true && cmd[1] == "offer") {
                m.QuoteReply(title(m.Sender.Group, m.Sender, m.Sender.QQ, cmd[2]));
            }
            else if(cmd.size() >= 3 && cmd[0] == "title" && group_settings[m.Sender.Group.GID]["title"] == true && cmd[1] == "set") {
                m.QuoteReply(title(m.Sender.Group, m.Sender, m.MessageChain.GetFirst<AtMessage>().Target(), cmd[2]));
            }
            else if(cmd.size() >= 2 && cmd[0] == "offer" && group_settings[m.Sender.Group.GID]["offer"] == true) {
                offer(m.Sender.Group, m.Sender, string_to_duration(cmd[1]), cmd.size() >=3?cmd[2]:"");
            }
            else if(cmd.size() >= 1 && cmd[0] == "kick" && group_settings[m.Sender.Group.GID]["kick"] == true) {
                kick(m.Sender.Group, m.Sender, m.MessageChain.GetFirst<AtMessage>().Target(), cmd.size() >= 2? cmd[1] : "");
            }
            else if(cmd.size() >= 1 && cmd[0] == "nonsense" && group_settings[m.Sender.Group.GID]["nonsense"] == true) {
                optional<string> s = nonsense(m.Sender.Group, m.Timestamp());
                if(s!=nullopt)
                    m.Reply(MessageChain().Plain(s.value()));
            }
            else if(cmd.size() >= 1 && cmd[0] == "pica" && group_settings[m.Sender.Group.GID]["pica"] == true) {
                optional<MessageChain> s = pica(m.Sender.Group, m.Timestamp());
                if(s!=nullopt)
                    m.Reply(s.value());
            }
            else if(group_settings[m.Sender.Group.GID]["poker"] == true && cmd.size() >= 2 && cmd[0] == "poker" && cmd[1] == "rule") {
                string reply =  "斗地主指令(群聊)：\n1、上桌/下桌/销房\n2、观战/退出观战\n3、房间状态\n4、开始游戏(仅房主可用)\n"
                                "斗地主指令(私聊)：\n1、叫3分/叫2分/叫1分/不叫\n2、出/出xxx(单独回复‘出’字将出推荐牌)\n3、过/滚/爬";
                m.Reply(MessageChain().Plain(reply));
            }
            else if(group_settings[m.Sender.Group.GID]["poker"] == true && cmd.size() >= 1 && cmd[0] == "上桌") {
                if(room_map.find(m.Sender.Group.GID) != room_map.end()) {
                    Room &room = room_map[m.Sender.Group.GID];
                    if(room.player_cnt >= 3) {
                        m.Reply(MessageChain().Plain("加入失败，已满员"));
                    }
                    else {
                        for(int i = 0; i < 3; ++i) {
                            if(room.player[i] == m.Sender.QQ) {
                                return;
                            }
                        }
                        for(auto w : room.watcher) {
                            if(w == m.Sender.QQ) {
                                m.Reply(MessageChain().Plain("你在观战列表中，无法上桌"));
                                return;
                            }
                        }
                        auto v = bot.GetFriendList();
                        for(auto fri: v) {
                            if(fri.QQ == m.Sender.QQ) {
                                for(int i = 0; i < 3; ++i) {
                                    if(room.player[i] == QQ_t(0)) {
                                        room.player[i] = m.Sender.QQ;
                                        room.player_cnt++;
                                        if(room.player_cnt == 3) {
                                            m.Reply(MessageChain().Plain("三人已凑齐，等待房主开始游戏"));
                                        }
                                        return;
                                    }
                                }
                            }
                        }
                        m.Reply(MessageChain().Plain("请先添加机器人好友！"));
                        return;
                    }
                }
                else {
                    auto v = bot.GetFriendList();
                    for(auto fri: v) {
                        if(fri.QQ == m.Sender.QQ) {
                            room_map[m.Sender.Group.GID] = Room();
                            Room &room = room_map[m.Sender.Group.GID];
                            room.player[0] = m.Sender.QQ;
                            room.player[1] = room.player[2] = 0_qq;
                            room.player_cards[0] = room.player_cards[1] = room.player_cards[2] = room.landlord_cards = room.rec = 0ULL;
                            room.player_cnt++;
                            room.landlord_id = -1;
                            room.status = Waiting;
                            room.timestamp = time(0);
                            return;
                        }
                    }
                    m.Reply(MessageChain().Plain("请先添加机器人好友！"));
                }
            }
            else if(group_settings[m.Sender.Group.GID]["poker"] == true && cmd.size() >= 1 && cmd[0] == "观战") {
                if(room_map.find(m.Sender.Group.GID) != room_map.end()) {
                    Room &room = room_map[m.Sender.Group.GID];
                    for(int i = 0; i < 3; ++i) {
                        if(room.player[i] == m.Sender.QQ) {
                            m.Reply(MessageChain().Plain("不能观看自己的对局"));
                            return;
                        }
                    }
                    for(auto q:room.watcher) {
                        if(q == m.Sender.QQ) {
                            return;
                        }
                    }
                    auto v = bot.GetFriendList();
                    for(auto fri: v) {
                        if(fri.QQ == m.Sender.QQ) {
                            room.watcher.push_back(m.Sender.QQ);
                            return;
                        }
                    }
                    m.Reply(MessageChain().Plain("请先添加机器人好友！"));
                    return;
                }
            }
            else if(group_settings[m.Sender.Group.GID]["poker"] == true && cmd.size() >= 1 && cmd[0] == "退出观战") {
                if(room_map.find(m.Sender.Group.GID) != room_map.end()) {
                    Room &room = room_map[m.Sender.Group.GID];
                    auto iter = remove(room.watcher.begin(), room.watcher.end(), m.Sender.QQ);
                    room.watcher.erase(iter, room.watcher.end());
                }
            }
            else if(group_settings[m.Sender.Group.GID]["poker"] == true && cmd.size() >= 1 && cmd[0] == "下桌") {
                if(room_map.find(m.Sender.Group.GID) != room_map.end()) {
                    Room &room = room_map[m.Sender.Group.GID];
                    if(room.status!=Waiting) {
                        m.Reply(MessageChain().Plain("请等待本局游戏结束"));
                        return;
                    }
                    for(int i = 0; i < 3; ++i) {
                        if(room.player[i] == m.Sender.QQ) {
                            for(int j = i+1; j < 3; ++j)
                                room.player[j-1] = room.player[j];
                            room.player[2] = 0_qq;
                            --room.player_cnt;
                            if(room.player_cnt <= 0)
                                room_map.erase(m.Sender.Group.GID);
                            return;
                        }
                    }
                }
            }
            else if(group_settings[m.Sender.Group.GID]["poker"] == true && cmd.size() >= 1 && cmd[0] == "销房") {
                if(room_map.find(m.Sender.Group.GID) != room_map.end()) {
                    Room &room = room_map[m.Sender.Group.GID];
                    if(room.status!=Waiting && !(cmd.size() >= 2 && cmd[1] == "--force")) {
                        m.Reply(MessageChain().Plain("请等待本局游戏结束"));
                        return;
                    }
                    for(int i = 0 ;i < 3; ++i) {
                        if(room.player[i] == m.Sender.QQ) {
                            m.Reply(MessageChain().Plain("销房成功"));
                            room_map.erase(m.Sender.Group.GID);
                            return;
                        }
                    }
                }
            }
            else if(group_settings[m.Sender.Group.GID]["poker"] == true && cmd.size() >= 1 && cmd[0] == "房间状态") {
                if(room_map.find(m.Sender.Group.GID) == room_map.end()) {
                    m.Reply(MessageChain().Plain("本群暂无房间"));
                }
                else {
                    Room &room = room_map[m.Sender.Group.GID];
                    // if(time(0) - room.timestamp > 600 && room.status == Waiting) {
                    //     m.Reply(MessageChain().Plain("长时间未开始，房间已销毁。"));
                    //     room_map.erase(m.Sender.Group.GID);
                    //     return;
                    // }
                    string s = "本房间创建于" + duration_to_string(time(0)-room.timestamp, false)+"前\n";
                    s+="房间状态：";
                    switch(room.status) {
                        case Waiting: 
                            s+="准备中";
                            for(int i = 0; i < 3; ++i) {
                                s+="\n玩家"+to_string(i+1)+"：";
                                s+= (room.player[i] != 0_qq )?bot.GetGroupMemberInfo(m.Sender.Group.GID, room.player[i]).MemberName : "(虚位以待)";
                                s+= (i == 0)?"(房主)":"";
                            }
                            break;
                        case Yelling: 
                            s+="叫地主中"; 
                            for(int i = 0; i < 3; ++i) {
                                s+="\n玩家"+to_string(i+1)+"：";
                                s+= (room.player[i] != 0_qq )?bot.GetGroupMemberInfo(m.Sender.Group.GID, room.player[i]).MemberName : "(虚位以待)";
                                s+= (i == 0)?"(房主)":"";
                            }
                            break;
                        case Gaming: 
                            s+= "游戏中\n";
                            s+= "地主牌：" + cards_to_string(room.landlord_cards) + "\n";
                            s+= "地主："+bot.GetGroupMemberInfo(m.Sender.Group.GID, room.player[room.landlord_id]).MemberName+
                                "(余"+to_string(count_all_cards(room.player_cards[room.landlord_id])) + "张牌)\n";
                            s+= "农民1："+bot.GetGroupMemberInfo(m.Sender.Group.GID, room.player[(room.landlord_id+1)%3]).MemberName+
                                "(余"+to_string(count_all_cards(room.player_cards[(room.landlord_id+1)%3])) + "张牌)\n";
                            s+= "农民2："+bot.GetGroupMemberInfo(m.Sender.Group.GID, room.player[(room.landlord_id+2)%3]).MemberName+
                                "(余"+to_string(count_all_cards(room.player_cards[(room.landlord_id+2)%3])) + "张牌)";
                            break;
                    }
                    if(room.watcher.size() > 0) {
                        s+="\n观众("+to_string(room.watcher.size())+"人):";
                        for(int i = 0; i < room.watcher.size(); ++i) {
                            s+="\n"+to_string(i+1)+"："+bot.GetGroupMemberInfo(m.Sender.Group.GID, room.watcher[i]).MemberName;
                        }
                    }
                    
                    m.Reply(MessageChain().Plain(s));
                }
            }
            else if(group_settings[m.Sender.Group.GID]["poker"] == true && cmd.size() >= 1 && cmd[0] == "开始游戏") {
                if(room_map.find(m.Sender.Group.GID) == room_map.end()) {
                    return;
                }
                else {
                    Room &room = room_map[m.Sender.Group.GID];
                    if(m.Sender.QQ == room.player[0]) {
                        if(room.player_cnt < 3) {
                            m.Reply(MessageChain().Plain("人数不足，请等待其他玩家上桌"));
                            return;
                        }
                        if(room.status != Waiting) {
                            m.Reply(MessageChain().Plain("游戏已经开始！"));
                            return;
                        }
                        m.Reply(MessageChain().Plain("游戏开始！请关注私聊消息"));
                        room.status = Yelling;
                        shuffle(room.player_cards, &room.landlord_cards);
                        room.turn = 0;
                        // TODO: change 上家下家 to names for(int i = 0; i )
                        //bot.GetGroupMemberInfo(gid, m.Sender.QQ).MemberName;
                        bot.SendMessage(room.player[room.turn], MessageChain().Plain("叫地主阶段！\n你当前牌为"+cards_to_string(room.player_cards[room.turn])+"\n请你叫分："));
                        bot.SleepSeconds(1);
                        bot.SendMessage(room.player[(room.turn+1)%3], MessageChain().Plain("叫地主阶段！\n你当前牌为"+cards_to_string(room.player_cards[(room.turn+1)%3])+"\n等待上家叫分中......"));
                        bot.SleepSeconds(1);
                        bot.SendMessage(room.player[(room.turn+2)%3], MessageChain().Plain("叫地主阶段！\n你当前牌为"+cards_to_string(room.player_cards[(room.turn+2)%3])+"\n等待下家叫分中......"));
                        //
                    }
                    else {
                        for(int i = 1; i < 3; ++i) {
                            if(room.player[i] == m.Sender.QQ) {
                                m.Reply(MessageChain().Plain("你不是房主！"));
                                return;
                            }
                        }
                    }
                }
            }
            // else if(cmd.size() >= 2 && cmd[0] == "python" && group_settings[m.Sender.Group.GID]["python"] == true) {
            //     m.Reply(python(m.Sender.Group,m.Sender, m.MessageChain.GetPlainText().substr(m.MessageChain.GetPlainText().find("python")+6), m.Timestamp()));
            // }
            // else if(cmd.size() >= 2 && cmd[0] == "c" && group_settings[m.Sender.Group.GID]["c"] == true) {
            //     m.Reply(c(m.Sender.Group,m.Sender, m.MessageChain.GetPlainText().substr(m.MessageChain.GetPlainText().find("c")+1), m.Timestamp()));
            // }
            else if(group_settings[m.Sender.Group.GID]["song"] == true && cmd.size() >= 2 && cmd[0] == "点歌") {
                m.Reply(song(m.Sender.Group, cmd[1], m.Timestamp()));
            }
            else if(group_settings[m.Sender.Group.GID]["wiki"] == true && cmd.size() >= 2 && cmd[0] == "百科") {
                m.Reply(baike(m.Sender.Group, cmd[1], m.Timestamp()));
            }
            else if(group_settings[m.Sender.Group.GID]["translate"] == true && cmd.size() >= 2 && cmd[0] == "中译英") {
                m.Reply(translate(m.Sender.Group, cmd[1], 1, m.Timestamp()));
            }
            else if(group_settings[m.Sender.Group.GID]["translate"] == true && cmd.size() >= 2 && cmd[0] == "英译中") {
                m.Reply(translate(m.Sender.Group, cmd[1], 2, m.Timestamp()));
            }
            else if(group_settings[m.Sender.Group.GID]["translate"] == true && cmd.size() >= 2 && cmd[0] == "中译日") {
                m.Reply(translate(m.Sender.Group, cmd[1], 3, m.Timestamp()));
            }
            else if(group_settings[m.Sender.Group.GID]["translate"] == true && cmd.size() >= 2 && cmd[0] == "日译中") {
                m.Reply(translate(m.Sender.Group, cmd[1], 4, m.Timestamp()));
            }
            else if(group_settings[m.Sender.Group.GID]["translate"] == true && cmd.size() >= 2 && cmd[0] == "翻译") {
                m.Reply(translate(m.Sender.Group, cmd[1], 0, m.Timestamp()));
            }
            else if(group_settings[m.Sender.Group.GID]["admin"] == true && cmd.size() >= 2 && cmd[0] == "admin" && cmd[1] == "set") {
                setadmin(m.Sender.Group, m.Sender, m.MessageChain.GetFirst<AtMessage>().Target());
            }
            else if(group_settings[m.Sender.Group.GID]["admin"] == true && cmd.size() >= 2 && cmd[0] == "admin" && cmd[1] == "remove") {
                removeadmin(m.Sender.Group, m.Sender, m.MessageChain.GetFirst<AtMessage>().Target());
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

int Module::change_funcs(Group_t& group, GroupMember& sender, string func_name, bool enable) {
    if(super_admin_list.count(sender.QQ) > 0) {
        if(func_name == "all") {
            int count = 0;
            for(auto func : func_names)
                if(group_settings[group.GID][func] == !enable) {
                    group_settings[group.GID][func] = enable;
                    config["group_settings"][to_string(group.GID.ToInt64())][func] = enable;
                    ++count;
                }
            return count;
        }
        else if(func_names.count(func_name) > 0 && group_settings[group.GID][func_name] == !enable) {
            group_settings[group.GID][func_name] = enable;
            config["group_settings"][to_string(group.GID.ToInt64())][func_name] = enable;
            return 1;
        }  
    }
    return 0;
}

void Module::command_parser(vector<string>& cmd, string plain_text) {
    int start = 0;
    bool blank = true;
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

void Module::offer(Group_t& group, GroupMember& sender, int seconds, string reason) {
    if(sender.Permission == GroupPermission::Member) {
        if(seconds > 0 && seconds <= 43200*60)
            bot.Mute(group.GID, sender.QQ, seconds);
    }
}

optional<string> Module::nonsense(Group_t& group, time_t timestamp) {
    static time_t last_time = 0;
    if(timestamp - last_time >= 10) {
        last_time = timestamp;
        fstream f("text/nonsense.txt");
        string sentence = "";
        int n = rand()%2437 + 1;
        for(int i = 0; i < n; ++i) {
            getline(f, sentence, '\n');
        }
        f.close();
        if(sentence != "") {
            return sentence+" [id:"+to_string(n)+"]";
        }
    }
    return nullopt;
    
}

void Module::mute(Group_t& group, GroupMember& sender, QQ_t target, int seconds, string reason) {
    if((sender.Permission >= GroupPermission::Administrator || super_admin_list.count(sender.QQ) > 0) && super_admin_list.count(target) == 0) {
        if(seconds > 0 && seconds <= 43200*60)
            bot.Mute(group.GID, target, seconds);
    }
}

void Module::unmute(Group_t& group, GroupMember& sender, QQ_t target, string reason) {
    if(sender.Permission >= GroupPermission::Administrator || super_admin_list.count(sender.QQ) > 0) {
        bot.UnMute(group.GID, target);
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
    // TODO: 排除Marketface 不支持的表情
    if(repeat_count.find(group.GID) == repeat_count.end())
        repeat_count[group.GID] = set<QQ_t>();
    auto &qq_set = repeat_count[group.GID];
    if(last_text_map.find(group.GID) == last_text_map.end()) {
        last_text_map[group.GID] = pair<MessageChain, time_t>(msg, timestamp);
        qq_set.insert(sender.QQ);
        return nullopt;
    }
    if(last_text_map[group.GID].first == msg) {
        if(qq_set.find(sender.QQ) == qq_set.end())
            qq_set.insert(sender.QQ);
    }
    else {
        static GroupImage img = bot.UploadGroupImage("img/break.jpg");
        MessageChain mc;
        if(qq_set.size() >= 3) {
            mc = MessageChain().Image(img).Plain("本次接龙内容：")+last_text_map[group.GID].first+
            MessageChain().Plain("\n共计"+to_string(qq_set.size())+"人参与\n历时"+duration_to_string(timestamp - last_text_map[group.GID].second, false));
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

optional<MessageChain> Module::pica(Group_t& group, time_t timestamp) {
    static time_t last_time = 0;
    static time_t last_info_time = 0;
    if(timestamp - last_time >= 60) {
        last_time = timestamp;
        int n = rand()%1000+1;
        GroupImage img;
        try {
            img = bot.UploadGroupImage("img/"+to_string(n)+".jpg");
            return MessageChain().Image(img);
        }
        catch(...) {
            return MessageChain().Plain("图片上传失败！["+to_string(n)+"]");
        }
    }
    else if(timestamp - last_info_time > 10) {
        last_info_time = timestamp;
        return MessageChain().Plain("技能正在冷却中......还有"+duration_to_string(60 - (timestamp - last_time), false));
    }
    return nullopt;
}

// MessageChain Module::python(Group_t& group, GroupMember& sender, string code, time_t timestamp) {
//     static time_t last_time = 0;
//     static time_t last_info_time = 0;
//     if(timestamp - last_time >= 10) {
//         fstream f("test", ios::out);
//         f << code;
//         f.close();
//         f.open("res.txt", ios::out);
//         f.close();
//         system("python3 test > res.txt 2>&1");
//         f.open("res.txt", ios::in);
//         int line_cnt= 0, word_cnt = 0;
//         char ch;
//         string s="";
//         while(word_cnt < 300 && line_cnt < 11) {
//             f.get(ch);
//             if(f.eof())
//                 break;
//             s+=ch;
//             if(ch == '\n')
//                 ++line_cnt;
//             else
//                 ++word_cnt;
//         }
//         if(s[s.size()-1] == '\n') {
//             --line_cnt;
//             s.pop_back();
//         }
//         if(word_cnt >= 300 || line_cnt >= 11) {
//             s+="\n............\n输出内容过多";
//         }
//         if(s == "")
//             s = "无输出";
//         return MessageChain().Plain(s);
//     }
//     else if(timestamp - last_info_time > 10) {
//         last_info_time = timestamp;
//         return MessageChain().Plain("技能正在冷却中......还有"+duration_to_string(10 - (timestamp - last_time), false));
//     }
// }

// MessageChain Module::c(Group_t& group, GroupMember& sender, string code, time_t timestamp) {
//     static time_t last_time = 0;
//     static time_t last_info_time = 0;
//     if(timestamp - last_time >= 10) {
//         fstream f("test.c", ios::out);
//         f << code;
//         f.close();
//         f.open("c_res.txt", ios::out);
//         f.close();
//         system("gcc test.c -o test.out > c_res.txt 2>&1 && ./test.out < null.txt > c_res.txt 2>&1");
//         f.open("c_res.txt", ios::in);
//         int line_cnt= 0, word_cnt = 0;
//         char ch;
//         string s="", buffer;
//         while(word_cnt < 200 && line_cnt < 11) {
//             f.get(ch);
//             if(f.eof())
//                 break;
//             s+=ch;
//             if(ch == '\n')
//                 ++line_cnt;
//             else
//                 ++word_cnt;
//         }
//         if(word_cnt >= 200 || line_cnt >= 11) {
//             s+="............\n输出内容过多";
//         }
//         if(s == "")
//             s = "无输出";
//         return MessageChain().Plain(s);
//     }
//     else if(timestamp - last_info_time > 10) {
//         last_info_time = timestamp;
//         return MessageChain().Plain("技能正在冷却中......还有"+duration_to_string(10 - (timestamp - last_time), false));
//     }
// }

MessageChain Module::check(Group_t& group, GroupMember& sender, string func) {
    if(func_names.count(func) > 0) {
        if(group_settings[group.GID][func] == true)
            return MessageChain().Plain("该模块目前为启用状态");
        else
            return MessageChain().Plain("该模块目前为禁用状态");
    }
    return MessageChain().Plain("找不到相关模块");
}


void Module::deal_friend_message(FriendMessage m) {
    GID_t gid = GID_t(0);
    for(auto iter:room_map) {
        for(int i = 0; i < 3; ++i) {
            if(iter.second.player[i] == m.Sender.QQ)
                gid = iter.first;
        }
    }
    if(gid == GID_t(0)) {
        // string s = m.MessageChain.GetPlainText(); 
        // if(s!="") {
        //     m.Reply(MessageChain().Plain(s));
        // }
        m.Reply(MessageChain().Plain("爬"));
    }
    else if(enabled_group_list.count(gid) > 0) {
        logger.info("[Friend]["+m.Sender.NickName+"("+to_string(gid.ToInt64())+")] " + m.Sender.NickName+"("+to_string(m.Sender.QQ.ToInt64())+") ["+to_string(m.MessageId())+"]: "+m.MessageChain.ToString());
        vector<string> cmd;
        command_parser(cmd, m.MessageChain.GetPlainText());
        if(group_settings[gid]["poker"] == true && cmd.size() >= 1 && cmd[0].find("出") == 0) {
            if(room_map.find(gid) != room_map.end()) {
                Room &room = room_map[gid];
                if(room.status!=Gaming) {
                    m.Reply(MessageChain().Plain("不在游戏阶段"));
                    return;
                }
                if(room.player[room.turn] != m.Sender.QQ) {
                    m.Reply(MessageChain().Plain("暂未轮到你出牌"));
                    return;
                }
                CardsGroup dealed = 0, copyhand = room.player_cards[room.turn];
                if(!deal(cmd[0].substr(string("出").length()), &copyhand, &dealed)) {
                    m.Reply(MessageChain().Plain("此牌组合不在你的手牌中！"));
                }
                else {
                    if(room.player[room.last_player_id] == m.Sender.QQ)
                        room.last_cards = 0ULL;
                    if(cmd[0] == "出" && dealed == 0) {
                        if(room.rec == 0) {
                            m.MessageChain = MessageChain().Plain("过");
                            deal_friend_message(m);
                            return;
                        }
                        dealed = room.rec;
                        take_cards(&copyhand, dealed);
                    }
                    if(get_cards_info(dealed).type == cg_Invalid) {
                        m.Reply(MessageChain().Plain("你出的牌不合法，请重出："));
                        return;
                    }
                    if(room.last_cards != 0ULL) {
                        switch(judge(room.last_cards, dealed)) {
                            case jr_Smaller:
                                m.Reply(MessageChain().Plain("你出的牌压不过当前牌面，请重出："));
                                return;
                            case jr_InvalidInput:
                            case jr_WrongMatch:
                                m.Reply(MessageChain().Plain("错误牌型！请重出："));
                                return;
                            case jr_Bigger:
                                break;
                            case jr_Unexpected:
                                m.Reply(MessageChain().Plain("未知错误"));
                                return;
                        }
                    }
                    room.player_cards[room.turn] = copyhand;
                    room.last_cards = dealed;
                    for(int i = 0; i < 3; ++i) {
                        if(room.player[i] == m.Sender.QQ)
                            room.last_player_id = i;
                    }
                    int remain = count_all_cards(copyhand);
                    string order = string(room.landlord_id == room.turn?"(地主)":"(农民)")+"出牌：\n"+cards_to_string(dealed)+"（"+group_type_to_string(get_cards_info(dealed).type)+"，还剩"+to_string(remain)+"张牌）";
                    if(remain == 0) {
                        string res = "";
                        m.Reply(MessageChain().Plain("你"+order));
                        bot.SleepSeconds(1);
                        bot.SendMessage( room.player[(room.turn+1)%3], MessageChain().Plain("上家"+order));
                        bot.SleepSeconds(1);
                        bot.SendMessage( room.player[(room.turn+2)%3], MessageChain().Plain("下家"+order));
                        string identity;
                        if(room.turn == room.landlord_id)
                            identity = "地主";
                        else
                            identity = (room.turn == (room.landlord_id+1)%3) ? "农民1" : "农民2";
                        identity+="：";
                        for(auto w:room.watcher) {
                            bot.SleepSeconds(1);
                            bot.SendMessage(w, MessageChain().Plain(identity+cards_to_string(dealed)));
                        }
                        room.history.push_back(dealed);
                        string review;
                        string ids[3] = {"地主", "农民1", "农民2"};
                        int turn = 0;
                        review+="地主(@"+bot.GetGroupMemberInfo(gid, room.player[room.landlord_id]).MemberName+")初始牌："+cards_to_string(room.player_cards_copy[room.landlord_id])+"\n\n";
                        review+="农民1(@"+bot.GetGroupMemberInfo(gid, room.player[(room.landlord_id+1)%3]).MemberName+")初始牌："+cards_to_string(room.player_cards_copy[(room.landlord_id+1)%3])+"\n\n";
                        review+="农民2(@"+bot.GetGroupMemberInfo(gid, room.player[(room.landlord_id+2)%3]).MemberName+")初始牌："+cards_to_string(room.player_cards_copy[(room.landlord_id+2)%3])+"\n\n";
                        review+="出牌流程：";
                        for(int i = 0; i < room.history.size(); ++i) {
                            if(room.history[i] != 0ULL) {
                                review+="\n"+ids[turn]+"："+cards_to_string(room.history[i]);
                            }
                            turn = (turn + 1)%3;
                        }
                        if(room.turn == room.landlord_id) {
                            res="地主（@"+bot.GetGroupMemberInfo(gid, m.Sender.QQ).MemberName+"）";
                        }
                        else {
                            res = "农民（@"+bot.GetGroupMemberInfo(gid, room.player[(room.landlord_id+1)%3]).MemberName+"、@"
                                    +bot.GetGroupMemberInfo(gid, room.player[(room.landlord_id+2)%3]).MemberName+"）";
                        }
                        res+="赢了！";
                        for(int i = 0 ; i < 3; ++i) {
                            bot.SleepSeconds(1);
                            bot.SendMessage( room.player[i], MessageChain().Plain(res));
                        }
                        for(auto w:room.watcher) {
                            bot.SleepSeconds(1);
                            bot.SendMessage(w, MessageChain().Plain(res));
                        }
                        for(int i = 0 ; i < 3; ++i) {
                            bot.SleepSeconds(1);
                            bot.SendMessage( room.player[i], MessageChain().Plain("本局复盘：\n"+review));
                        }
                        for(auto w:room.watcher) {
                            bot.SleepSeconds(1);
                            bot.SendMessage(w, MessageChain().Plain("本局复盘：\n"+review));
                        }
                        room_map.erase(gid);
                        return;
                    }
                    room.rec = recommend_cards(room.last_cards, room.player_cards[(room.turn+1)%3], 1);
                    m.Reply(MessageChain().Plain("你"+order+"\n等待下家"+((room.landlord_id == (room.turn+1)%3)?"(地主)":"(农民)")+"出牌中......"));
                    bot.SleepSeconds(1);
                    bot.SendMessage( room.player[(room.turn+1)%3], MessageChain().Plain("上家"+order+"\n你当前牌为："+cards_to_string(room.player_cards[(room.turn+1)%3])+
                            "\n轮到你"+((room.landlord_id == (room.turn+1)%3)?"(地主)":"(农民)")+"出牌：" + 
                            "\n(推荐出牌：" + (room.rec == 0 ? "过" : cards_to_string(room.rec)) + ")"));
                    bot.SleepSeconds(1);
                    bot.SendMessage( room.player[(room.turn+2)%3], MessageChain().Plain("下家"+order+"\n等待上家"+((room.landlord_id == (room.turn+1)%3)?"(地主)":"(农民)")+"出牌中......"));
                    string identity;
                    if(room.turn == room.landlord_id)
                        identity = "地主";
                    else
                        identity = (room.turn == (room.landlord_id+1)%3) ? "农民1" : "农民2";
                    identity+="：";
                    for(auto w:room.watcher) {
                        bot.SleepSeconds(1);
                        bot.SendMessage(w, MessageChain().Plain(identity+cards_to_string(dealed)));
                    }
                    room.history.push_back(dealed);
                    room.turn = (room.turn+1)%3;
                    return;
                }
            }
        }
        else if(group_settings[gid]["poker"] == true && cmd.size() >= 1 && (cmd[0].find("叫") == 0 || cmd[0] == "不叫")) {
            if(room_map.find(gid) != room_map.end()) {
                Room &room = room_map[gid];
                if(room.status!=Yelling) {
                    m.Reply(MessageChain().Plain("不在叫地主阶段"));
                    return;
                }
                if(room.player[room.turn] != m.Sender.QQ) {
                    m.Reply(MessageChain().Plain("暂未轮到你叫牌"));
                    return;
                }
                if(cmd[0] == "叫3分") {
                    for(int i = 0; i < 3; ++i) {
                        if(room.player[i] == m.Sender.QQ) {
                            room.landlord_id = room.turn = i;
                            room.status = Gaming;
                            room.last_player_id = i;
                            string res = "叫3分，成为地主！\n地主牌："+cards_to_string(room.landlord_cards);
                            CardsGroup cds = room.landlord_cards;
                            add_cards(&room.player_cards[room.landlord_id], &cds);
                            for(int i = 0; i < 3; ++i)
                                room.player_cards_copy[i] = room.player_cards[i];
                            
                            room.rec = recommend_cards(room.last_cards, room.player_cards[room.turn], 1);

                            m.Reply(MessageChain().Plain("你"+res+"\n你当前牌："+cards_to_string(room.player_cards[room.turn])+"\n轮到你出牌："+
                                "\n(推荐出牌：" + (room.rec == 0 ? "过" : cards_to_string(room.rec)) + ")"));
                            bot.SleepSeconds(1);
                            bot.SendMessage( room.player[(room.turn+1)%3], MessageChain().Plain("上家"+res+"\n等待上家出牌中......"));
                            bot.SleepSeconds(1);
                            bot.SendMessage( room.player[(room.turn+2)%3], MessageChain().Plain("下家"+res+"\n等待下家出牌中......"));
                            for(auto w:room.watcher) {
                                bot.SleepSeconds(1);
                                bot.SendMessage(w, MessageChain().Plain("(@"+bot.GetGroupMemberInfo(gid, room.player[room.turn]).MemberName+")"+res));
                            }
                            return ;
                        }
                    }
                }
                else if(cmd[0] == "叫2分") {
                    if(room.score >= 2) {
                        m.Reply(MessageChain().Plain("有人叫分比你大！请选择更大叫分或者不叫"));
                        return;
                    }
                    room.score = 2;
                    for(int i = 0; i < 3; ++i) {
                        if(room.player[i] == m.Sender.QQ) {
                            room.landlord_id = room.turn = i;
                            break;
                        }
                    }
                    if(room.turn == 2) {
                        room.status = Gaming;
                        room.last_player_id = room.turn;
                        string res = "在本轮叫分中叫分最高(2分)，成为地主！\n地主牌："+cards_to_string(room.landlord_cards);
                        CardsGroup cds = room.landlord_cards;
                        add_cards(&room.player_cards[room.landlord_id], &cds);
                        for(int i = 0; i < 3; ++i)
                            room.player_cards_copy[i] = room.player_cards[i];

                        room.rec = recommend_cards(room.last_cards, room.player_cards[room.turn], 1);

                        m.Reply(MessageChain().Plain("你"+res+"\n你当前牌："+cards_to_string(room.player_cards[room.turn])+"\n轮到你出牌："+
                            "\n(推荐出牌：" + (room.rec == 0 ? "过" : cards_to_string(room.rec)) + ")"));
                        bot.SleepSeconds(1);
                        bot.SendMessage( room.player[(room.turn+1)%3], MessageChain().Plain("上家"+res+"\n等待上家出牌中......"));
                        bot.SleepSeconds(1);
                        bot.SendMessage( room.player[(room.turn+2)%3], MessageChain().Plain("下家"+res+"\n等待下家出牌中......"));
                        for(auto w:room.watcher) {
                            bot.SleepSeconds(1);
                            bot.SendMessage(w, MessageChain().Plain("@"+bot.GetGroupMemberInfo(gid, room.player[room.turn]).MemberName+res));
                        }
                        return;
                    }
                    else{
                        m.Reply(MessageChain().Plain("你叫2分，等待下家叫分中......"));
                        bot.SleepSeconds(1);
                        bot.SendMessage( room.player[(room.turn+1)%3], MessageChain().Plain("上家叫2分。轮到你叫分："));
                        bot.SleepSeconds(1);
                        bot.SendMessage( room.player[(room.turn+2)%3], MessageChain().Plain("下家叫2分。等待上家叫分中......"));
                        for(auto w:room.watcher) {
                            bot.SleepSeconds(1);
                            bot.SendMessage(w, MessageChain().Plain("(@"+bot.GetGroupMemberInfo(gid, room.player[room.turn]).MemberName+")"+"叫2分"));
                        }
                        ++room.turn;
                    }
                }
                else if(cmd[0] == "叫1分") {
                    if(room.score >= 1) {
                        m.Reply(MessageChain().Plain("有人叫分比你大！请选择更大叫分或者不叫"));
                        return;
                    }
                    room.score = 1;
                    for(int i = 0; i < 3; ++i) {
                        if(room.player[i] == m.Sender.QQ) {
                            room.landlord_id = room.turn = i;
                            break;
                        }
                    }
                    if(room.turn == 2) {
                        room.status = Gaming;
                        room.last_player_id = room.turn;
                        string res = "在本轮叫分中叫分最高(1分)，成为地主！\n地主牌："+cards_to_string(room.landlord_cards);
                        CardsGroup cds = room.landlord_cards;
                        add_cards(&room.player_cards[room.landlord_id], &cds);
                        for(int i = 0; i < 3; ++i)
                            room.player_cards_copy[i] = room.player_cards[i];
                            
                        room.rec = recommend_cards(room.last_cards, room.player_cards[room.turn], 1);

                        m.Reply(MessageChain().Plain("你"+res+"\n你当前牌："+cards_to_string(room.player_cards[room.turn])+"\n轮到你出牌："+
                            "\n(推荐出牌：" + (room.rec == 0 ? "过" : cards_to_string(room.rec)) + ")"));
                        bot.SleepSeconds(1);
                        bot.SendMessage( room.player[(room.turn+1)%3], MessageChain().Plain("上家"+res+"\n等待上家出牌中......"));
                        bot.SleepSeconds(1);
                        bot.SendMessage( room.player[(room.turn+2)%3], MessageChain().Plain("下家"+res+"\n等待下家出牌中......"));
                        for(auto w:room.watcher) {
                            bot.SleepSeconds(1);
                            bot.SendMessage(w, MessageChain().Plain("(@"+bot.GetGroupMemberInfo(gid, room.player[room.turn]).MemberName+")"+res));
                        }
                        return;
                    }
                    else{
                        m.Reply(MessageChain().Plain("你叫1分，等待下家叫分中......"));
                        bot.SleepSeconds(1);
                        bot.SendMessage( room.player[(room.turn+1)%3], MessageChain().Plain("上家叫1分。轮到你叫分："));
                        bot.SleepSeconds(1);
                        bot.SendMessage( room.player[(room.turn+2)%3], MessageChain().Plain("下家叫1分。等待上家叫分中......"));
                        for(auto w:room.watcher) {
                            bot.SleepSeconds(1);
                            bot.SendMessage(w, MessageChain().Plain("(@"+bot.GetGroupMemberInfo(gid, room.player[room.turn]).MemberName+")"+"叫1分"));
                        }
                        ++room.turn;
                    }
                }
                else if(cmd[0] == "不叫") {
                    if(room.turn == 2) {
                        if(room.score > 0) {
                            room.status = Gaming;
                            room.turn = room.last_player_id = room.landlord_id;
                            string res = "在本轮叫分中叫分最高("+to_string(room.score)+"分)，成为地主！\n地主牌："+cards_to_string(room.landlord_cards);
                            CardsGroup cds = room.landlord_cards;
                            add_cards(&room.player_cards[room.landlord_id], &cds);
                            for(int i = 0; i < 3; ++i)
                                room.player_cards_copy[i] = room.player_cards[i];
                            
                            room.rec = recommend_cards(room.last_cards, room.player_cards[room.turn], 1);
                            
                            bot.SendMessage( room.player[room.turn], MessageChain().Plain("你"+res+"\n你当前牌："+cards_to_string(room.player_cards[room.turn])+"\n轮到你出牌："+
                            "\n(推荐出牌：" + (room.rec == 0 ? "过" : cards_to_string(room.rec)) + ")"));
                            bot.SleepSeconds(1);
                            bot.SendMessage( room.player[(room.turn+1)%3], MessageChain().Plain("上家"+res+"\n等待上家出牌中......"));
                            bot.SleepSeconds(1);
                            bot.SendMessage( room.player[(room.turn+2)%3], MessageChain().Plain("下家"+res+"\n等待下家出牌中......"));
                            for(auto w:room.watcher) {
                                bot.SleepSeconds(1);
                                bot.SendMessage(w, MessageChain().Plain("(@"+bot.GetGroupMemberInfo(gid, room.player[room.turn]).MemberName+")"+res));
                            }
                        }
                        else {
                            shuffle(room.player_cards, &room.landlord_cards);
                            room.turn = 0;
                            bot.SendMessage(room.player[room.turn], MessageChain().Plain("无人叫地主，已重新发牌！\n你当前牌为"+cards_to_string(room.player_cards[room.turn])+"\n请你叫分："));
                            bot.SleepSeconds(1);
                            bot.SendMessage(room.player[(room.turn+1)%3], MessageChain().Plain("无人叫地主，已重新发牌！\n你当前牌为"+cards_to_string(room.player_cards[(room.turn+1)%3])+"\n等待上家叫分中......"));
                            bot.SleepSeconds(1);
                            bot.SendMessage(room.player[(room.turn+2)%3], MessageChain().Plain("无人叫地主，已重新发牌！\n你当前牌为"+cards_to_string(room.player_cards[(room.turn+2)%3])+"\n等待下家叫分中......"));
                            for(auto w:room.watcher) {
                                bot.SleepSeconds(1);
                                bot.SendMessage(w, MessageChain().Plain("无人叫地主，已重新发牌！"));
                            }
                        }
                    }
                    else {
                        m.Reply(MessageChain().Plain("你不叫分，等待下家叫分中......"));
                        bot.SleepSeconds(1);
                        bot.SendMessage( room.player[(room.turn+1)%3], MessageChain().Plain("上家不叫分。轮到你叫分："));
                        bot.SleepSeconds(1);
                        bot.SendMessage( room.player[(room.turn+2)%3], MessageChain().Plain("下家不叫分。等待上家叫分中......"));
                        for(auto w:room.watcher) {
                            bot.SleepSeconds(1);
                            bot.SendMessage(w, MessageChain().Plain("(@"+bot.GetGroupMemberInfo(gid, room.player[room.turn]).MemberName+")"+"不叫分"));
                        }
                        ++room.turn;
                    }
                }
                else {
                    m.Reply(MessageChain().Plain("违规叫分，请重新叫分："));
                    return;
                }
            }
        }
        else if(group_settings[gid]["poker"] == true && cmd.size() >= 1 && (cmd[0] == "过" ||  cmd[0] == "滚" || cmd[0] == "爬")) {
            if(room_map.find(gid) != room_map.end()) {
                Room &room = room_map[gid];
                if(room.status!=Gaming) {
                    m.Reply(MessageChain().Plain("不在游戏阶段"));
                    return;
                }
                if(room.player[room.turn] != m.Sender.QQ) {
                    m.Reply(MessageChain().Plain("暂未轮到你出牌"));
                    return;
                }
                if(room.player[room.last_player_id] == m.Sender.QQ) {
                    m.Reply(MessageChain().Plain("你不能跳过自己回合，请重新出牌："));
                    return;
                }
                int remain = count_all_cards(room.player_cards[room.turn]);
                string order = "跳过出牌(还剩"+to_string(remain)+"张牌)";
                if(room.last_player_id == (room.turn+1)%3) {
                    room.last_cards = 0;
                }
                room.rec = recommend_cards(room.last_cards, room.player_cards[(room.turn+1)%3], 1);
                m.Reply(MessageChain().Plain("你"+string(room.landlord_id == room.turn?"(地主)":"(农民)")+order+"\n等待下家"+string(room.landlord_id == (room.turn+1)%3?"(地主)":"(农民)")+"出牌中......"));
                bot.SleepSeconds(1);
                bot.SendMessage( room.player[(room.turn+1)%3], MessageChain().Plain("上家"+string(room.landlord_id == room.turn?"(地主)":"(农民)")+order+"\n你当前牌为："+cards_to_string(room.player_cards[(room.turn+1)%3])+
                    "\n轮到你"+string(room.landlord_id == (room.turn+1)%3?"(地主)":"(农民)")+"出牌：" + 
                    "\n(推荐出牌：" + (room.rec == 0 ? "过" : cards_to_string(room.rec)) + ")"));
                bot.SleepSeconds(1);
                bot.SendMessage( room.player[(room.turn+2)%3], MessageChain().Plain("下家"+string(room.landlord_id == room.turn?"(地主)":"(农民)")+order+"\n等待上家"+string(room.landlord_id == (room.turn+1)%3?"(地主)":"(农民)")+"出牌......"));
                string identity;
                if(room.turn == room.landlord_id)
                    identity = "地主";
                else
                    identity = (room.turn == (room.landlord_id+1)%3) ? "农民1" : "农民2";
                identity+="：";
                for(auto w:room.watcher) {
                    bot.SleepSeconds(1);
                    bot.SendMessage(w, MessageChain().Plain(identity+cmd[0]));
                }
                room.history.push_back(0ULL);
                room.turn = (room.turn+1)%3;
            }
        }
    }

}

MessageChain Module::song(Group_t& group, string songName, time_t timestamp) {
    static time_t last_time = 0;
    static time_t last_info_time = 0;
    if(timestamp - last_time >= 60) {
        last_time = timestamp;
        try {
            return test(songName);
        }
        catch(...) {
            return MessageChain().Plain("云村中没有这首歌哟~");
        }
    }
    else if(timestamp - last_info_time > 10) {
        last_info_time = timestamp;
        return MessageChain().Plain("技能正在冷却中......还有"+duration_to_string(60 - (timestamp - last_time), false));
    }
    else
        return MessageChain();
}

void Module::setadmin(Group_t& group, GroupMember& sender, QQ_t target) {
    if(super_admin_list.count(sender.QQ) > 0) {
        bot.SetGroupAdmin(group.GID, target, 1);
    }
}

void Module::removeadmin(Group_t& group, GroupMember& sender, QQ_t target) {
    if(super_admin_list.count(sender.QQ) > 0 || sender.QQ == target) {
        bot.SetGroupAdmin(group.GID, target, 0);
    }
}

MessageChain Module::title(Group_t& group, GroupMember& sender, QQ_t target, string t) {
    if(target != sender.QQ && super_admin_list.count(sender.QQ) <= 0)
        return MessageChain().Plain("你没有权限设置他人头衔哦~");
    try {
        bot.SetGroupMemberSpecialTitle(group.GID, target, t);
    }
    catch(...) {
        return MessageChain().Plain("设置失败！违规头衔");
    }
    return MessageChain().Plain("已设置头衔\""+t+"\"。");

}

json getTranslateJson(string word, int mode)
{
	json returnJson;
    string cmd = "python3 translate.py \""+word+"\" " + to_string(mode) + " > translate_res.txt";
	system(cmd.c_str());
    fstream res("translate_res.txt", ios::in);
    string s;
    while(!res.eof()) {
        string temp;
        getline(res, temp);
        s+=temp;
    }
	returnJson = json::parse(s);
	return returnJson;
}

// MessageChain translate(string word) {
//     json res = getTranslateJson(word);
//     if(!res.at("/error_code"_json_pointer).empty()) {
//         return MessageChain().Plain(res.at("/error_msg").get<string>());
//     }
//     else {
//         return MessageChain().Plain("\""+res.at("/trans_res/src"_json_pointer).get<string>()+"\"的意思是"
//                                     +"\""+res.at("/trans_result/dst"_json_pointer).get<string>()+"\"");
//     }
// }

MessageChain Module::translate(Group_t& group, string word, int mode, time_t timestamp) {
    static time_t last_time = 0;
    static time_t last_info_time = 0;
    if(timestamp - last_time >= 10) {
        last_time = timestamp;
        try {
            json res = getTranslateJson(word, mode);
            if(res.contains("error_code")) {
                return MessageChain().Plain(res["error_msg"].get<string>());
            }
            else {
                return MessageChain().Plain("\""+res["trans_result"][0]["src"].get<string>()+"\"的意思是"
                                            +"\""+res["trans_result"][0]["dst"].get<string>()+"\"");
            }
        }
        catch(...) {
            return MessageChain().Plain("发生未知错误。");
        }
    }
    else if(timestamp - last_info_time > 0) {
        last_info_time = timestamp;
        return MessageChain().Plain("技能正在冷却中......还有"+duration_to_string(10 - (timestamp - last_time), false));
    }
    else
        return MessageChain();
}

MessageChain Module::baike(Group_t& group, string word, time_t timestamp) {
    static time_t last_time = 0;
    static time_t last_info_time = 0;
    if(timestamp - last_time >= 60) {
        last_time = timestamp;
        system(("python3 baike.py "+word + " > baike.txt").c_str());
        fstream res("baike.txt", ios::in);
        string s;
        while(!res.eof()) {
            string temp;
            getline(res, temp);
            s+="\n"+temp;
        }
        if(s == "\n")
            return MessageChain().Plain("找不到这个词条呢~");
        else
            return MessageChain().Plain(word+"：\n"+s);

    }
    else if(timestamp - last_info_time > 10) {
        last_info_time = timestamp;
        return MessageChain().Plain("技能正在冷却中......还有"+duration_to_string(60 - (timestamp - last_time), false));
    }
    else
        return MessageChain();
}

optional<MessageChain> Module::app_parser(Group_t& group, string s) {
    json js = json::parse(s);
    if(js["meta"]["detail_1"]["appid"] == "1110081493") {
        string url = js["meta"]["detail_1"]["qqdocurl"].get<string>();
        url = url.substr(0, url.find_first_of("?"));
        string cmd = "python3 zhihu.py "+url+ " > zhihu.txt";
        system(cmd.c_str());
        fstream res("zhihu.txt", ios::in);
        string s;
        while(!res.eof()) {
            string temp;
            getline(res, temp);
            s+=temp+"\n";
        }
        return MessageChain().Plain(s+"详细链接："+url);
    }
    else if(js["meta"]["detail_1"]["appid"] == "1109937557") {
        string url = js["meta"]["detail_1"]["qqdocurl"].get<string>();
        url = url.substr(0, url.find_first_of("?"));
        string cmd = "python3 bilibili.py "+url+ " > bilibili.txt";
        system(cmd.c_str());
        fstream res("bilibili.txt", ios::in);
        string s;
        while(!res.eof()) {
            string temp;
            getline(res, temp);
            s+=temp+"\n";
        }
        return MessageChain().Plain(s+"详细链接："+url);
    }
    else if(js["meta"]["detail_1"]["appid"] == "1111264064") {
        // string url = js["meta"]["detail_1"]["qqdocurl"].get<string>();
        // url = url.substr(0, url.find_first_of("?"));
        // string cmd = "python3 tieba.py "+url+ " > tieba.txt";
        // system(cmd.c_str());
        // fstream res("tieba.txt", ios::in);
        // string s;
        // while(!res.eof()) {
        //     string temp;
        //     getline(res, temp);
        //     s+=temp+"\n";
        // }
        // return MessageChain().Plain(s+"详细链接："+url);
    }
    else{
        
    }
    return nullopt;
}

optional<MessageChain> Module::timer(Group_t& group, GroupMember& sender, vector<string> &cmds) {
    try {
        schedule sc;
        sc.at = stoi(cmds[1]);
        int mode = stoi(cmds[2]);
        if(mode == 0) {
            sc.trigger_time = datetime_to_timestamp(cmds[3]);
        }
        else {
            sc.trigger_time = time(0) + string_to_duration(cmds[3]);
        }
        sc.group = group.GID;
        sc.qq = sender.QQ;
        sc.interval_time = string_to_duration(cmds[4]);
        sc.repeat_cnt = stoi(cmds[5]);
        sc.msg = "";
        for(int i = 6; i < cmds.size() - 1; ++i) {
            sc.msg += cmds[i] + " ";
        }
        sc.msg += cmds[cmds.size() - 1];
        if(sc.trigger_time < time(0)) {
            return MessageChain().Plain("起始时间不能小于当前时间");
        }
        if(sc.trigger_time > time(0) + 24*3600*30) {
            return MessageChain().Plain("时长不得超过三十天");
        }
        if(sc.repeat_cnt < 0 || sc.repeat_cnt > 30) {
            return MessageChain().Plain("重复次数需要在1~30之间");
        }
        if((sc.repeat_cnt > 1 && sc.interval_time < 60) || sc.interval_time > 24*3600*30) {
            return MessageChain().Plain("间隔时长需控制在60s~30d之间");
        }
        if(tasks.find(group.GID) == tasks.end())
            tasks[group.GID] = priority_queue<schedule>();
        tasks[group.GID].push(sc);
        return MessageChain().Plain("设置完成，将于"+ duration_to_string(sc.trigger_time - time(0), false)+"后执行");

    }catch(...) {
        return nullopt;
    }
}

optional<MessageChain> Module::timer_check(Group_t& group, GroupMember& sender) {
    if(tasks.find(group.GID) == tasks.end())
        tasks[group.GID] = priority_queue<schedule>();
    auto &q = tasks[group.GID];
    if(q.size() > 0) {
        return nullopt; //TODO
    }
}



void Module::checkSchedule() {
    static chrono::time_point last = chrono::system_clock::now();
    if(chrono::system_clock::now() - last > chrono::milliseconds(1000)) {
        auto iter = tasks.begin();
        while(iter != tasks.end()) {
            priority_queue<schedule> &q = iter->second;
            while(!q.empty()) {
                schedule sc = q.top();
                if(sc.trigger_time < time(0)) {
                    MessageChain res;
                    if(sc.at == true)
                        res=MessageChain().At(sc.qq).Plain(" ").Plain(sc.msg);
                    else
                        res=(MessageChain().Plain(sc.msg));
                    q.pop();
                    if(sc.repeat_cnt > 1) {
                        sc.trigger_time += sc.interval_time;
                        sc.repeat_cnt --;
                        q.push(sc);
                        res = res.Plain("\n[定时任务]\n["+duration_to_string(sc.interval_time, false)
                                            +"后再次触发]\n[剩余"+to_string(sc.repeat_cnt)+"次]");
                    }
                    else {
                        res = res.Plain("\n[定时任务结束]");
                    }
                    bot.SendMessage(sc.group, res);
                }
                else
                    break;
            }
            iter++;
        }
        last = chrono::system_clock::now();
    }
}