#ifndef UDEV_MAKER_HPP
#define UDEV_MAKER_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>  // c++17
#include <cstdio>  // popen
#include "lua_config.hpp"
#include "sub_process_writer.hpp"
#include "device_enum.hpp"

class UdevMaker {
public:
    UdevMaker(bool policy_kit_use);
    ~UdevMaker();

    bool initialize();
    bool getIsPolicyKitNeeded();
    bool setSymlink(int v_list_index, std::shared_ptr<TtyUdevInfo> shared_tty_udev_info);
    void setSymlinkNameByType(const std::string& user_input, std::shared_ptr<TtyUdevInfo> shared_tty_udev_info);
    std::string getSymlinkNameFromList(int list_index);
    std::string makeSymlinkFilename(const std::string& user_input_str);
    std::string veritySymlinkName(const std::string& user_input_str);
    bool openFile(std::fstream* fs, std::string filename, Type type_enum);
    void saveVector(std::fstream* fs);
    void printList();
    std::vector<std::string> getDeviceList();
    void makeScript(std::fstream* fs);
    bool getSerialWarn(std::shared_ptr<TtyUdevInfo> shared_tty_udev_info);
    void makeContent(std::string& udev_str, std::shared_ptr<TtyUdevInfo> shared_tty_udev_info);
    bool copyUdev();
    bool executeUdevadmControl();
    int createUdevRuleFile(std::shared_ptr<TtyUdevInfo> shared_tty_udev_info);
    int createUdevRuleFileWithFork(std::shared_ptr<TtyUdevInfo> shared_tty_udev_info);
    int deleteUdevRuleFileWithFork(std::shared_ptr<TtyUdevInfo> shared_tty_udev_info);
    int deleteUdevruleFile();
    bool getUdevFilename(std::string* return_filename, int list_index);
    std::string& getUdevRuleFilename();
    int getVSize();

    void createBasicList(std::string list_full_path="");
    bool copyHelper(const std::string& helper_path);
    void createConfigLua();
    bool inputDevInfo(std::shared_ptr<TtyUdevInfo> shared_tty_udev_info);

protected:
    std::vector<std::string> v_device_list;
    std::vector<std::string> v_symlink_list;
    std::string udev_filename = "";
    bool is_policy_kit_needed;

private:
    std::string file_path = "./ref";
    std::string udev_path = "/etc/udev/rules.d";
    std::string symlink_prefix = "tty";
    std::string symlink_suffix = ".rules";
    std::string prefix_udevrule_number = "90";
    const char* HELPER_WRITER_FILENAME = "helper_writer";
    std::string HELPER_WRITER_FULL_PATH;

};

#endif //UDEV_MAKER_HPP