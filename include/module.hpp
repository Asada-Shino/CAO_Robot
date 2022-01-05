#include <mirai.h>
#include <map>
#include <set>
using namespace std;
using namespace Cyan;

class Module {
private:
    map<GID_t, map<string, bool>> group_settings;
    set<QQ_t> super_admin_list;
public:
    Module();
    void init();
};