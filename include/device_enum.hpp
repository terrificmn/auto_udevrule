#ifndef DEVICE_ENUM_HPP
#define DEVICE_ENUM_HPP
#include <string>
#include <vector>

enum TTYDevice {
    USB, ACM
};

enum ResultType {
    LAST_RESULT_ONLY, WHOLE_RESULT, EXECUTE_ONLY
};

enum Mode {
    LAST_DETECT_MODE, ALL_DETECT_MODE, INPUT_MODE, DELETE_MODE
};

enum Type {
    READ, WRITE, DELETE
};


struct TtyUdevInfo {
    std::string kernel;
    std::string vendor; 
    std::string product;
    std::string serial;
    std::string symlink_name;
    std::string model_db;
    std::string vendor_db;
    bool is_connected_now;
    /// MODEL_FROM_DATABASE=FT232 Serial (UART) IC
    /// VENDOR_FROM_DATABASE=Future Technology Devices International, Ltd
    /// ID_USB_DRIVER=ch341
};

struct ResultData {
    std::vector<std::string> result_v;
    std::string result_str;
    TTYDevice tty_device = TTYDevice::USB;
    int found_device_num = -1;
};

struct LuaParam {
    bool use_kernel;
    bool use_serial;
    double timeout_sec;
    std::string lidar2d_main_vendor;
    std::string lidar2d_bottom_vendor;
    std::string lidar1d_luna_vendor;
    std::string loadcell_vendor;
    std::string amrbd_vendor;
    std::string etc;
};

#endif // DEVICE_ENUM_HPP
