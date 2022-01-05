#include "module.hpp"
#include <fstream>

Module::Module() {}

void Module::init() {
    fstream f("config.json");
    if(f.fail())
        throw "can't find config.json";
    string config_s;
    while(!f.eof()) {
        string s;
        f >> s;
        config_s+=s;
    }
    f.close();
    json config = json::parse(config_s);
    for(auto item:config["super_admin"].items()) {
        super_admin_list.insert(QQ_t(item.value()));
    }
    for(auto item:config["group_settings"].items()) {
        map<string, bool>& group = (group_settings[GID_t(stoll(item.key()))] = map<string, bool>());
        for(auto func : item.value().items()) {
            group[func.key()] = func.value();
        }
    }
}