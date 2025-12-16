#include "lua_config.hpp"

lua_State* LuaConfig::s_L = nullptr;
std::string LuaConfig::config_path = getenv("HOME");
bool LuaConfig::use_kernel = true;
bool LuaConfig::use_serial = true;
double LuaConfig::timeout_sec = 20.0;
LuaParam LuaConfig::luaParam;

LuaConfig::LuaConfig() {}

bool LuaConfig::initialze(const std::string& config_name, std::string override_path) {
    LuaConfig::config_path += "/.local/share/auto_udevrule";
    std::string lua_file_path;
    bool res_parent_dir;
    if(override_path.empty()) {
        res_parent_dir = std::filesystem::exists(LuaConfig::config_path);
        lua_file_path = LuaConfig::config_path;

    } else {
        res_parent_dir = std::filesystem::exists(override_path);
        lua_file_path = override_path;
    }

    if(!res_parent_dir) {
        std::cerr << "config_path not found: " << std::endl;
        lua_file_path += "/config";
        std::cout << "diretory: " << lua_file_path << std::endl;
        std::filesystem::create_directories(lua_file_path);
        std::cout << "lua config dir has been created." << std::endl;

        lua_file_path += "/" + config_name;

        std::ofstream createFile(lua_file_path, std::ios::ate);
        if(!createFile.is_open()) {
            std::cerr << "lua file not open: " << lua_file_path << std::endl;
            return false;
        }

        LuaConfig::createLuaFile(createFile);

    } else {
        lua_file_path += "/config/" + config_name;
        // std::cout << "auto_udevrule diretory found. config path: " << lua_file_path << std::endl;
    }

    LuaConfig::s_L = luaL_newstate();
    luaL_openlibs(LuaConfig::s_L);
    
    if(luaL_dofile(LuaConfig::s_L, lua_file_path.c_str())) {
        ///FYI: 파일 여는데 실패 했을 때 true.  파일 형태가 잘못되었을 때에도 못 열수가 있다. (예, comma 등..)
        std::cerr << "failed to open a lua file." << std::endl;
        lua_close(LuaConfig::s_L);
        return false;
    }
    
    if(LuaConfig::loadLuaData("PARAM")) {
        std::cout << "use_kernel is " << std::boolalpha << LuaConfig::use_kernel << std::endl;
        std::cout << "use_serial is " << std::boolalpha << LuaConfig::use_serial << std::endl;
        std::cout << "timeout_sec is " << LuaConfig::timeout_sec << std::endl;
        ///vendor info
        std::cout << "vendor_db1 is " << LuaConfig::luaParam.vendor_db1 << std::endl;
        std::cout << "vendor_db2 is " << LuaConfig::luaParam.vendor_db2 << std::endl;
        std::cout << "vendor_db3 is " << LuaConfig::luaParam.vendor_db3 << std::endl;
        std::cout << "vendor_db4 is " << LuaConfig::luaParam.vendor_db4 << std::endl;
        std::cout << "vendor_db5 is " << LuaConfig::luaParam.vendor_db5 << std::endl;
        std::cout << "vendor_db6 is " << LuaConfig::luaParam.vendor_db6 << std::endl;
        std::cout << "------------------------\n";
    }

    /// common
    lua_close(LuaConfig::s_L);
    return true;
}

/// @brief create a basic lua file
/// @param creatFile 
void LuaConfig::createLuaFile(std::ofstream& creatFile) {
    std::string context(R"(PARAM = {
    use_kernel = true,
    use_serial = true,
    timeout_sec = 30,
    -- vendor info
    vendor_db1 = "sllidar",
    vendor_db2 = "coin",
    vendor_db3 = "esp-mini",
    vendor_db4 = "esp",
    vendor_db5 = "amr",
    vendor_db6 = "etc"
})");
    creatFile << context;
}


bool LuaConfig::loadLuaData(const char* table_name) {
    // common parent
    lua_getglobal(LuaConfig::s_L, table_name);
    if(!lua_istable(LuaConfig::s_L, -1)) {
        std::cerr << "PARAM table is wrong." << std::endl;
        lua_pop(LuaConfig::s_L, 1);
        return false;
    }

    lua_pushstring(LuaConfig::s_L, "use_kernel");
    lua_gettable(LuaConfig::s_L, -2); // Get MyParm[tableName]
    if(lua_isboolean(LuaConfig::s_L, -1)) {
        LuaConfig::use_kernel = lua_toboolean(LuaConfig::s_L, -1);
        LuaConfig::luaParam.use_kernel = LuaConfig::use_kernel;
    }
    lua_pop(LuaConfig::s_L, 1);

    ///반복
    lua_pushstring(LuaConfig::s_L, "use_serial");
    lua_gettable(LuaConfig::s_L, -2); // Get MyParm[tableName]
    if(lua_isboolean(LuaConfig::s_L, -1)) {
        LuaConfig::use_serial = lua_toboolean(LuaConfig::s_L, -1);
        LuaConfig::luaParam.use_kernel = LuaConfig::use_serial;
    }
    lua_pop(LuaConfig::s_L, 1);

    lua_pushstring(LuaConfig::s_L, "timeout_sec");
    lua_gettable(LuaConfig::s_L, -2); // Get MyParm[tableName]
    if(lua_tonumber(LuaConfig::s_L, -1)) {
        LuaConfig::timeout_sec = lua_tonumber(LuaConfig::s_L, -1);
        LuaConfig::luaParam.use_kernel = LuaConfig::timeout_sec;
    }
    lua_pop(LuaConfig::s_L, 1);

    for(int i=1; i<7; ++i) {
        std::string vendor_db = "vendor_db";
        vendor_db+= std::to_string(i);
        lua_pushstring(LuaConfig::s_L, vendor_db.c_str());
        lua_gettable(LuaConfig::s_L, -2); // Get MyParm[tableName]
        if(lua_tostring(LuaConfig::s_L, -1)) {
            switch (i) {
            case 1 :
                LuaConfig::luaParam.vendor_db1 = lua_tostring(LuaConfig::s_L, -1);
                break;
            case 2 :
                LuaConfig::luaParam.vendor_db2 = lua_tostring(LuaConfig::s_L, -1);
                break;
            case 3 :
                LuaConfig::luaParam.vendor_db3 = lua_tostring(LuaConfig::s_L, -1);
                break;
            case 4 :
                LuaConfig::luaParam.vendor_db4 = lua_tostring(LuaConfig::s_L, -1);
                break;
            case 5 :
                LuaConfig::luaParam.vendor_db5 = lua_tostring(LuaConfig::s_L, -1);
                break;
            case 6 :
                LuaConfig::luaParam.vendor_db6 = lua_tostring(LuaConfig::s_L, -1);
                break;
            default:
                break;
            }
        }
        lua_pop(LuaConfig::s_L, 1);
    }

    lua_pop(LuaConfig::s_L, 1); // remove the table from the stack
    return true;
}
