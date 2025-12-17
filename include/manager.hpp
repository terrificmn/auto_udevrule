#ifndef MANAGER_HPP
#define MANAGER_HPP

#include <thread>
#include <memory>
#include "device_enum.hpp"
#include "udev_maker.hpp"
#include "usb_info_confirmer.hpp"

class Manager {
public:
    Manager(UdevMaker* udevMaker, const Mode& mode);
    void execute();
    
    std::string inputList(const std::string& str_print);
    bool singleMode();
    bool allDetectMode();
    bool detectUsb();
    bool detectUsbs();
    void proceedByVendor();
    int makeUdevRule(const std::string& input_str);
    int makeUdevRuleByProductCategory(const std::string product_category_name);
    
    bool inputMode();
    void inputSymlinkInManualMode();
    bool deleteMode();
    int removeUdevRule(int input_num);
    
public:
    std::shared_ptr<TtyUdevInfo> ttyUdevInfo;

private:
    UdevMaker* ptrUdevMaker = nullptr;
    UsbInfoConfirmer mUsbInfoConfirmer;
    Mode m_mode;

};
#endif //MANAGER_HPP