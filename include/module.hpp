#include <mirai.h>
#include <map>
#include <set>
#include "logger.hpp"

using namespace std;
using namespace Cyan;

const set<string> func_names = {"repeat-analysis", "ban", "kick", "offer", "nonsense", "pica"};
class Module {
private:
    map<GID_t, map<string, bool>> group_settings;
    set<QQ_t> super_admin_list;
    set<QQ_t> black_list;
    set<GID_t> enabled_group_list;
    map<string, int> interval;
    json config;
    MiraiBot bot;
    SessionOptions opts;
    Logger logger;
private:
    void load_config_file();
    void save_config_file();
    int change_funcs(Group_t& group, GroupMember& sender, string func_name, bool enable);
    void kick(Group_t& group, GroupMember& sender, QQ_t target, string reason);
    void ban(Group_t& group, GroupMember& sender, QQ_t target, int seconds, string reason);
    void offer(Group_t& group, GroupMember& sender, int seconds, string reason);
    optional<MessageChain> pica(Group_t& group, time_t timestamp);
    void command_parser(vector<string>& cmd, string plain_text);
    optional<string> nonsense(Group_t& group, time_t timestamp);
    optional<MessageChain> repeat_analysis(Group_t& group, GroupMember& sender, MessageChain msg, time_t timestamp);
public:
    Module();
    void init();
    void release();
    void deal_group_message(GroupMessage m);
};

/*
    TODO:
    一个函数
    每个群不同的冷却时间
    冷却还有较长时间会有提醒
    提醒不会多次提醒
    每次提醒冷却中的失败调用计数
    每次成员成功调用计数
    成员调用频率过高时熔断





*/