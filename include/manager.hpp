#ifndef MANAGER_HPP
#define MANAGER_HPP

#include "device_enum.hpp"
#include "udev_maker.hpp"
#include "usb_info_confirmer.hpp"
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
#endif //MANAGER_HPP