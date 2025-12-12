#ifndef USB_INFO_CONFIRMER_HPP
#define USB_INFO_CONFIRMER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <unistd.h>
#include <sys/wait.h>
#include "device_enum.hpp"
#include "udev_maker.hpp"
#include "time_checker.hpp"

#include <regex>
#include <algorithm>

struct ResultData {
    std::string result_str;
    TTYDevice tty_device = TTYDevice::USB;
    int found_device_num = -1;
};

class UsbInfoConfirmer {
public:
    UsbInfoConfirmer(UdevMaker* udevMaker);
    ResultData findNewDevice(const int& try_count);
    std::vector<std::string> findUdevInfo(const bool& is_acm_detected);
    std::string getUsbId();

    void executeCmd(std::vector<std::string>& cmd_result_data, const std::string& cmd_str, int process_result_type);
    bool executePopen(const std::string& cmd_str);
    bool executeSimpleCmd(const std::string& cmd_str);

    std::string regexWrapper(std::string& last_message, std::string reg_str);
    std::string getDetectedTime(std::string& last_message);
    std::string getKernelIdForAcm(std::string& last_message);
    std::string getKernelId(std::string& last_message);
    std::string getVenderId(std::string& last_message);
    std::string getModelId(std::string& last_message);
    std::string getSerialId(std::string& last_message);
    void removeCharacter(std::string& str, char remove_char);

    std::string getIdsAterRegex(std::string& last_message);
    bool checkNumber(std::string& input_msg);
    
    int showResult();
    std::string getLsResult();
    bool detectUsb();

private:
    UdevMaker* ptrUdevMaker = nullptr;

    std::string m_dmesg_base_cmd = "dmesg | grep tty";
    std::string m_udev_base_cmd = "udevadm info /dev/tty";
    std::string m_usb_num_str = "-1";
    std::string ls_rule_result;
};

#endif //USB_INFO_CONFIRMER_HPP