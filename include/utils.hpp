#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <string>
using namespace std;
string duration_to_string(int seconds, bool english);
int string_to_duration(string s);
string add_zero(int t);
string get_date(string seperator);
string get_time();

#endif