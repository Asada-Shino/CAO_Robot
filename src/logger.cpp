#include "logger.hpp"



Logger::Logger() {}

void Logger::init() {
    log.open("log/"+get_date("_")+".log", ios::app);
    if(log.fail())
        throw runtime_error("can't open log file");
}

void Logger::info(string s) {
    log << ("[INFO] "+ get_time() + " " + s + "\n"); 
}

void Logger::error(string s) {
    log << ("[Error] "+ get_time() + " " + s + "\n"); 
}

void Logger::release() {
    log.close();
}