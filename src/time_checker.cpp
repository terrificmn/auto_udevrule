#include "time_checker.hpp"

    /// TEST Code
    // TimeChecker timeC;
    // auto boot_t = timeC.getBootTime();
    // double dmesg_time = 281.385257;
    // timeC.dmesgToRealTime(dmesg_time);
    // double boot_since_time = timeC.getBootTimeAsDouble();
    // timeC.compareDmesgTime(281.385);


TimeChecker::TimeChecker() {}

/// @brief This reads uptime (boot time). Dmesg's timeline is started from the uptime. 
/// @return std::chrono::system_clock::time_point
std::chrono::system_clock::time_point TimeChecker::getBootTime() {
    std::ifstream uptimeFile("/proc/uptime");
    double uptime_seconds = 0;
    uptimeFile >> uptime_seconds;
    // std::cout << "uptime_secs: " << uptime_seconds << std::endl;
    auto now = std::chrono::system_clock::now();
    auto boot_time = now - std::chrono::duration<double>(uptime_seconds);
    auto boot_time_tp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(boot_time);

    /// for readable print
    // std::time_t t_for_print = std::chrono::system_clock::to_time_t(boot_time_tp);
    // std::cout << "boot time: " << std::ctime(&t_for_print) << std::endl;
    return boot_time_tp;
}

/// @brief get boot time and return it as double
/// @return 
double TimeChecker::getBootTimeAsDouble() {
    std::ifstream uptimeFile("/proc/uptime");
    double uptime_seconds = 0;
    uptimeFile >> uptime_seconds;

    double now_since_boot = uptime_seconds;
    // std::cout << "boot time(double): " << now_since_boot << std::endl;

    return std::move(now_since_boot);
}

/// @brief figure the time out which device is detected, by boot time
/// @param dmesg_time 
/// @return 
std::chrono::system_clock::time_point TimeChecker::dmesgToRealTime(const double& dmesg_time) {
    std::chrono::system_clock::time_point boot_time_tp = this->getBootTime();
    /// boot time + current time for duration
    auto duration_t = boot_time_tp + std::chrono::duration<double>(dmesg_time);
    auto duration_t_tp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(duration_t);
    
    /// for readable print
    std::time_t t_for_print = std::chrono::system_clock::to_time_t(duration_t_tp);
    ///DEBUG
    ///FYI: The dmesg's timeline is started from the boot time.
    std::cout << "detected time (figured out by boot time): " << std::ctime(&t_for_print); /// \n is included.

    return duration_t_tp;
}

bool TimeChecker::compareDmesgTime(const double& dmesg_time) {
    double boot_time = this->getBootTimeAsDouble();
    double delta = boot_time - dmesg_time;
    // std::cout << "Seconds since dmesg event: " << delta << " seconds\n";

    /// limit
    if(delta > LuaConfig::timeout_sec) {
        std::cout << "More than " << LuaConfig::timeout_sec << " seconds have passed: ";
        double curr = delta / 60.0;
        std::cout << curr << " minites have passed." << std::endl;
        return false;
    }

    return true;
}

