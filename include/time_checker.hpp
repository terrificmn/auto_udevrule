#ifndef TIME_CHECKER_HPP
#define TIME_CHECKER_HPP

#include <iostream>
#include <chrono>
#include <fstream>
#include "lua_config.hpp"

class TimeChecker {
public:
    TimeChecker();
    std::chrono::system_clock::time_point getBootTime();
    double getBootTimeAsDouble();
    std::chrono::system_clock::time_point dmesgToRealTime(const double& dmesg_time);
    bool compareDmesgTime(const double& dmesg_time);

private:

};

#endif //TIME_CHECKER_HPP