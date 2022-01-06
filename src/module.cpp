#include "module.hpp"
#include <fstream>

Module::Module() {}

void Module::init() {
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
    save_config_file();
}

void Module::deal_group_message(GroupMessage m) {
    
}