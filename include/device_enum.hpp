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

enum InputCheck {
    RE_MAKE_UDEV, RE_SYMLINK, RE_MAKE_UDEV_AGAIN, RE_MAKE_CANCEL
};

enum Type {
    READ, WRITE, DELETE
};

enum RegUdevType {
    VENDOR_DB, VENDOR, MODEL
};

enum MapStatus {
    FIRST_INDEX_MATCH, SWAP_INDEX, MAP_FAILURE, MAP_OK, MAP_DEFAULT
};

struct TtyUdevInfo {
    std::string kernel;
    std::string vendor_id; 
    std::string product;
    std::string serial;
    std::string symlink_name;
    int tty_number;
    /// 제품 추정 관련 elements
    std::string vendor;
    std::string model;
    bool is_connected_now;
};

struct MapCheckList {
    int size;
    std::vector<int> original_index;
    std::vector<int> symlink_name_index;
    MapStatus map_status = MapStatus::MAP_DEFAULT;
};

struct ResultData {
    std::vector<std::string> result_v;
    std::string result_str;
    TTYDevice tty_device = TTYDevice::USB;
    int found_device_num = -1;
};

struct ProductCategory {
    std::string vendor;
    std::string model;
    std::string alias;
};

struct LuaParam {
    bool use_kernel;
    bool use_serial;
    double timeout_sec;
    std::vector<ProductCategory> v_product_category;
};

#endif // DEVICE_ENUM_HPP
