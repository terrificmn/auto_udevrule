#ifndef MANAGER_H
#define MANAGER_H

#include "device_enum.h"
#include "udev_maker.h"
#include "usb_checker.h"
#include <thread>

class Manager {
public:
    Manager(UdevMaker* udevMaker, const Mode& mode);
    void execute();
    
    std::string inputList(const std::string& str_print);
    bool singleMode();
    bool mulipleMode();
    int makeUdevRule(const std::string& input_str);
    
    bool inputMode();
    void inputSymlinkInManualMode();
    bool deleteMode();
    

private:
    UdevMaker* ptrUdevMaker = nullptr;
    Mode m_mode;

};
#endif