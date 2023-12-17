#ifndef UDEV_MAKER_H
#define UDEV_MAKER_H

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>  // c++17

enum Type {
    READ, WRITE
};

class UdevMaker {
private:
    std::string file_path = "./ref";
    std::string udev_path = "/etc/udev/rules.d";
    std::string symlink_prefix = "tty";
    std::string symlink_suffix = ".rules";
    
    // std::string symlink_name
    // std::string dir_file_name="/etc/udev/rules.d/"
    // std::string kennel="3-6.3"
    // std::string vendor="0403"
    // std::string product="6001"
    // std::string symlink_name="ttyZltech"
    // std::string filename="98-zltech-motor.rules"
        
protected:
    std::vector<std::string> v_device_list;
    std::vector<std::string> v_symlink_list;
    std::string udev_filename = "";


public:
    UdevMaker();
    ~UdevMaker();

    std::string m_dir_file_name, m_kennel, m_vendor, m_product, m_symlink_name, m_filename;
    bool setSymlink(int v_list_index);
    bool inputSymlink();
    bool openFile2();
    bool openFile(std::fstream* fs, std::string filename, Type type_enum);
    void saveVector(std::fstream* fs);
    void printList();
    void makeScript(std::fstream* fs);
    bool copyUdev();
    int getVSize();

    void createBasicList();

};

#endif