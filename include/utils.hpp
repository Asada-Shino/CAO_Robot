#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <string>
#include "poker.hpp"
using namespace std;
string duration_to_string(int seconds, bool english);
int string_to_duration(string s);
string add_zero(int t);
string get_date(string seperator);
string get_time();

string cards_to_string(cards cs);
string group_type_to_string(CardGroupType cgt);

int deal(string s, cards *hand, cards *dealed);
#endif