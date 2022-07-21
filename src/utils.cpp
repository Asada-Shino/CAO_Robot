/*** 
 * @Author       : KnightZJ
 * @LastEditTime : 2022-05-20 23:05:37
 * @LastEditors  : Please set LastEditors
 * @Description  : 
 */
#include "utils.hpp"
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>

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

char* cds = "3456789xJQKA2";

string cards_to_string(CardsGroup cs) {
    string str="";
    for(int i = 0; i < 15; ++i) {
        CardsGroup res = (0x1000100010001ULL << i) & cs;
        if(res) {
            string cd="";
            switch(i) {
                case 7: cd = "10"; break;
                case 13: cd = "小王"; break;
                case 14: cd = "大王"; break;
                default: cd.push_back(cds[i]);
            }
            int repeat = 0;
            if(res < 0x8000)
                repeat = 1;
            else if(res < 0x80000000)
                repeat = 2;
            else if(res < 0x800000000000)
                repeat = 3;
            else
                repeat = 4;
            for(int j = 0; j < repeat; ++j) {
                str+="["+cd+"]";
            }
        }
    }
    return str;
}

string group_type_to_string(CardsGroupType cgt) {
    return string(strCardGroupType[cgt]);
}

int deal(string s, CardsGroup *hand, CardsGroup *dealed) {
    CardsGroup copyhand = *hand;
    for(int i = 0; i < s.length();++i) {
        int offset = 0;
        switch(s[i]) {
            case '3': offset = 0; break;
            case '4': offset = 1; break;
            case '5': offset = 2; break;
            case '6': offset = 3; break;
            case '7': offset = 4; break;
            case '8': offset = 5; break;
            case '9': offset = 6; break;
            case 'J':
            case 'j': offset = 8; break;
            case 'Q':
            case 'q': offset = 9; break;
            case 'K':
            case 'k': offset = 10; break;
            case 'A':
            case 'a': offset = 11; break;
            case '2': offset = 12; break;
            default:
                string sub = s.substr(i);
                if(sub.find("10") == 0) { offset = 7; i+=string("10").length()-1;}
                else if(sub.find("小王") == 0) { offset = 13; i+=string("小王").length()-1;}
                else if(sub.find("大王") == 0) { offset = 14; i+=string("大王").length()-1;}
                else
                    return 0;
        }
        if(!can_take_card(copyhand,(CardType)offset, 1))
            return 0;
        take_card(&copyhand, (CardType)offset, 1);
        add_card(dealed, (CardType)offset, 1);
    }
    *hand = copyhand;
    return 1;
}

time_t datetime_to_timestamp(string s) {
    std::tm t{};
    std::istringstream ss(s);
    if(s.length() == 11)
        s = "2022/"+s+":00";
    else if(s.length() == 14)
        s = "2022/" + s;
    else if(s.length() == 16)
        s += ":00";
    ss >> std::get_time(&t, "%Y/%m/%d-%H:%M:%S");
    if (ss.fail()) {
        throw std::runtime_error{"failed to parse time string"};
    }   
    return  mktime(&t);
}