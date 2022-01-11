#include "utils.hpp"

const char* en[] = {"s", "m", "h", "d"};
const char* ch[] = {"秒", "分", "小时", "天"};

string duration_to_string(int seconds, bool english) {
    string s;
    static int ratio[] = {60, 60, 24, 1000000};
    int i = 0;
    while(seconds > 0 && i < 4) {
        if(seconds % ratio[i] != 0)
            s = to_string(seconds % ratio[i]) + (english ? en[i] : ch[i]) + s;
        seconds /= ratio[i];
        ++i;
    }
    return s;
}


int string_to_duration(string s) {
    int num = 0, res = 0;
    for(auto ch : s) {
        if(ch >= '0' && ch <= '9') {
            num = num*10+ch-'0';
        }
        else {
            switch(ch) {
                case 'd':
                    res+=num*24*60*60;
                    break;
                case 'h':
                    res+=num*60*60;
                    break;
                case 'm':
                    res+=num*60;
                    break;
                case 's':
                    res+=num;
                    break;
                default:
                    return -1;
            }
            num = 0;
        }
    }
    if(num > 0)
        res+=num;
    return res;
}

string add_zero(int t) {
        return t<10 ? "0"+to_string(t) : to_string(t);
    }

string get_date(string seperator) {
    time_t now_c = time(0);
    tm now_tm = *localtime(&now_c);
    return to_string(now_tm.tm_year+1900)+seperator+add_zero(now_tm.tm_mon+1)+seperator+add_zero(now_tm.tm_mday);
}

string get_time() {
    time_t now_c = time(0);
    tm now_tm = *localtime(&now_c);
    return add_zero(now_tm.tm_hour)+":"+add_zero(now_tm.tm_min)+":"+add_zero(now_tm.tm_sec);
}