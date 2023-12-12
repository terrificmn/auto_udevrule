#ifndef USB_CHECKER_H
#define USB_CHECKER_H

#include <iostream>
#include <string>
#include <vector>
#include "device_enum.h"
#include "udev_maker.h"

#include <regex>
#include <algorithm>


class UsbChecker {
private:
    std::string m_dmesg_cmd = "dmesg | grep ttyUSB";
    std::string m_udev_cmd = "udevadm info /dev/ttyUSB";
    std::string m_usb_num_str = "0";
    UdevMaker* ptrUdevMaker = nullptr;

public:
    UsbChecker(UdevMaker* udevMaker);
    std::string findNewDevice();
    std::vector<std::string> findUdevInfo();
    std::string getUsbId();

    void getCmdResult(std::vector<std::string>& cmd_data, int cmd_type);

    std::string regexWrapper(std::string& last_message, std::string reg_str);
    std::string getKernelId(std::string& last_message);
    std::string getVenderId(std::string& last_message);
    std::string getModelId(std::string& last_message);
    void removeCharacter(std::string& str, char remove_char);

    std::string getIdsAterRegex(std::string& last_message);
    bool checkNumber(std::string& input_msg);

    bool detectUsb();

};

#endif