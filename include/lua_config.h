#ifndef LUA_CONFIG_H
#define LUA_CONFIG_H
extern "C" {
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
    //// 우분투 case
    // #include <lua5.3/lua.h>
    // #include <lua5.3/lualib.h>
    // #include <lua5.3/lauxlib.h>
}
#include <iostream>
#include <filesystem>
#include <fstream>

class LuaConfig {
private:


public:
    LuaConfig();
    static bool initialze(const std::string& config_name, std::string override_path="");
    static void createLuaFile(std::ofstream& creatFile);
    static bool loadLuaData(const char* table_name);
    
public:
    static lua_State* s_L;
    static std::string config_path;
    static bool use_kernel;
    static bool use_serial;
    static double timeout_sec;

};



#endif  // LUA_CONFIG_H