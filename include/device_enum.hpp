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

struct TtyUdevInfo {
    std::string kernel;
    std::string vendor; 
    std::string product;
    std::string serial;
    std::string symlink_name;
    /// MODEL_FROM_DATABASE=FT232 Serial (UART) IC
    /// VENDOR_FROM_DATABASE=Future Technology Devices International, Ltd
};

#endif // DEVICE_ENUM_HPP
