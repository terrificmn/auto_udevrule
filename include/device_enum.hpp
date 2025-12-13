#ifndef DEVICE_ENUM_HPP
#define DEVICE_ENUM_HPP
#include <string>

enum TTYDevice {
    USB, ACM
};

enum ResultType {
    LAST_RESULT_ONLY, WHOLE_RESULT, EXECUTE_ONLY
};

enum Mode {
    SINGLE_MODE, MULTI_MODE, DELETE_MODE, INPUT_MODE
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
    /// MODEL_FROM_DATABASE=FT232 Serial (UART) IC
    /// VENDOR_FROM_DATABASE=Future Technology Devices International, Ltd
};

struct ResultData {
    std::string result_str;
    TTYDevice tty_device = TTYDevice::USB;
    int found_device_num = -1;
};

#endif // DEVICE_ENUM_HPP
