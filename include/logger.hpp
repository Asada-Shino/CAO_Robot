#include <fstream>
#include <string>
#include "utils.hpp"

using namespace std;

class Logger {
public:
    Logger();
    void init();
    void info(string s);
    void error(string s);
    void release();
private:
    fstream log;
};