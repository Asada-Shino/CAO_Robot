#include <mirai.h>
#include <map>
#include <set>
using namespace std;
using namespace Cyan;

const set<string> func_names = {"anti-recall", "repeat-analysis", "ban", "kick"};
class Module {
private:
    map<GID_t, map<string, bool>> group_settings;
    set<QQ_t> super_admin_list;
    set<QQ_t> black_list;
    set<GID_t> enabled_group_list;
    json config;
    MiraiBot bot;
    SessionOptions opts;
private:
    void load_config_file();
    void save_config_file();
    int change_funcs(Group_t& group, GroupMember& sender, string func_name, bool enable);
    void kick(Group_t& group, GroupMember& sender, QQ_t target, string reason);
    void ban(Group_t& group, GroupMember& sender, QQ_t target, int time, string reason);
    void command_parser(vector<string>& cmd, string plain_text);
    optional<MessageChain> repeat_analysis(Group_t& group, GroupMember& sender, MessageChain msg, time_t timestamp);
public:
    Module();
    void init();
    void release();
    void deal_group_message(GroupMessage m);
};