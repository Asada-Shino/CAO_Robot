#include <map>
#include <mirai.h>
#include "module.hpp"
using namespace std;
using namespace Cyan;

class CAORobot {
public:
    CAORobot();
    void init();
private:
    MiraiBot bot;
    SessionOptions opts;
    Module module;
};