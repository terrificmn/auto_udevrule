#ifndef UDEV_MAKER_HPP
#define UDEV_MAKER_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>  // c++17
#include <unordered_map>
#include <cstdio>  // popen
#include "lua_config.hpp"

enum Type {
    READ, WRITE, DELETE
};

struct UdevInfo {
    std::string kernel;
    std::string vendor; 
    std::string product;
    std::string serial;
    std::string symlink_name;
};

class UdevMaker {
public:
    UdevMaker(bool policy_kit_use);
    ~UdevMaker();

    bool initialize();
    bool getIsPolicyKitNeeded();
    UdevInfo udevInfo;
    std::string m_dir_file_name, m_filename;
    bool setSymlink(int v_list_index);
    void setSymlinkNameByType(const std::string& user_input);
    std::string makeSymlinkFilename(const std::string& user_input_str);
    std::string veritySymlinkName(const std::string& user_input_str);
    bool openFile(std::fstream* fs, std::string filename, Type type_enum);
    void saveVector(std::fstream* fs);
    void printList();
    std::vector<std::string> getDeviceList();
    void makeScript(std::fstream* fs);
    bool getSerialWarn();
    void makeContent(std::string& udev_str);
    bool copyUdev();
    bool executeUdevadmControl();
    int makeUdevruleFile();
    int deleteUdevruleFile(const std::string& input_str);
    bool getUdevFilename(std::string* return_filename, int list_index);
    std::string& getUdevRuleFilename();
    int getVSize();

    void createBasicList(std::string list_full_path="");
    bool copyHelper(const std::string& helper_path);
    void createConfigLua();
    bool inputDevInfo();
    void assignInfoByInput();

    UdevInfo getUdevInfo();

private:
    std::string file_path = "./ref";
    std::string udev_path = "/etc/udev/rules.d";
    std::string symlink_prefix = "tty";
    std::string symlink_suffix = ".rules";
    std::string prefix_udevrule_number = "90";
    std::unordered_map<std::string, std::string> un_dev_info;
    const char* HELPER_WRITER_FILENAME = "helper_writer";
    std::string HELPER_WRITER_FULL_PATH;
    // std::string symlink_name
    // std::string dir_file_name="/etc/udev/rules.d/"
    // std::string kerel="3-6.3"
    // std::string vendor="0403"
    // std::string product="6001"
    // std::string symlink_name="ttyZltech"
    // std::string filename="98-zltech-motor.rules"
        
protected:
    std::vector<std::string> v_device_list;
    std::vector<std::string> v_symlink_list;
    std::string udev_filename = "";
    bool is_policy_kit_needed;

};

#endif //UDEV_MAKER_HPP