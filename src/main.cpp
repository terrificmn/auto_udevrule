#include <iostream>
#include <string>
#include <filesystem>  // c++17
#include "udev_maker.hpp"
#include "manager.hpp"
#include "device_enum.hpp"
#include "lua_config.hpp"
#include "sudo_manager.hpp"

constexpr const char* VERSION = "1.2.0";

extern "C" {
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}

void helpMsg();

int main(int argc, char** argv) {
    Mode mode = Mode::SINGLE_MODE;
    /// parameter part
    if(argc == 2) {
        std::string argv_str = argv[1];
        if(argv_str == "-s") {
            // std::cout << argv[1] << std::endl;
            std::cout << "*single mode selected*" << std::endl;
            mode = Mode::SINGLE_MODE;

        } else if(argv_str == "-m") {
            // std::cout << argv[1] << std::endl; // multiple use (like 10 times)
            std::cout << "*multi mode selected*" << std::endl;
            mode = Mode::MULTI_MODE;

        } else if(argv_str == "-d") {
            std::cout << "*delete mode selected*" << std::endl;
            mode = Mode::DELETE_MODE;

        } else if(argv_str == "-i") {
            std::cout << "*input mode selected*" << std::endl;
            mode = Mode::INPUT_MODE;

        } else if(argv_str == "-h" || argv_str == "--help" ) {
            helpMsg();
            return 1;

        } else if(argv_str == "-v" || argv_str == "--version" ) {
            std::cout << "version: " << VERSION << std::endl;
            return 1;

        } else {
            std::cout << "wrong parameter: only accept -s or -m" << std::endl;
            return 1;
        }

    } else {
        helpMsg();
        return 1;
    }

    
    /// lua config setting
    bool open_config_result = LuaConfig::initialze("config.lua");

    /// false for CLI
    UdevMaker udevMaker(false);
    bool open_list_result = udevMaker.initialize();

    if(!open_list_result || !open_config_result ) {
        std::cout << "Please execute the program again~.\n";
        return 1;
    }

    Manager manager(&udevMaker, mode);
    manager.execute();

    return 0;
}


void helpMsg() {
    std::cout << "help: add parameter -s, -m, or -d" << std::endl;
    std::cout << "-s: single use" << std::endl;
    std::cout << "-m: multiple use" << std::endl;
    std::cout << "-d: delete udevrules" << std::endl;
    std::cout << "-i: input mode manually" << std::endl;
    std::cout << "-v OR --version: version info" << std::endl;
    std::cout << "-h OR --help: help message" << std::endl;
}