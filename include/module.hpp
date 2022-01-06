#include <mirai.h>
#include <map>
#include <set>
using namespace std;
using namespace Cyan;

const string func_names[] = {"anti-recall", "repeat-count", "ban", "kick"};
class Module {
private:
    map<GID_t, map<string, bool>> group_settings;
    set<QQ_t> super_admin_list;
    set<QQ_t> black_list;
    set<GID_t> enabled_group_list;
    json config;
private:
    void load_config_file();
    void save_config_file();
public:
    Module();
    void init();
    void release();
    void deal_group_message(GroupMessage m);
};