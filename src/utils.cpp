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
        res+=num*60;
    return res;
}