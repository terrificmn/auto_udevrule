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
        ///product category TODO: print
        for(auto& pc : LuaConfig::luaParam.v_product_category) {
            std::cout << "vendor is " << pc.vendor << std::endl;
            std::cout << "model is " << pc.model << std::endl;
        }
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
    
    product_category = {
        { vendor = "1", model = "hello"},
        { vendor = "2", model = "new"},
        { vendor = "3", model = "world"},
        { vendor = "4", model = "good"},
        { vendor = "5", model = "bye"}
    }
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

    lua_getfield(LuaConfig::s_L, -1, "product_category");
    if(!lua_istable(LuaConfig::s_L, -1)) {
        std::cerr << "product_category is not a table." << std::endl;
        lua_pop(LuaConfig::s_L, 1);
        return false;
    }

    ///TODO: vector (product) 만들기

    int product_count = lua_rawlen(LuaConfig::s_L, -1); /// length of an array
    std::cout << "product_count: " << product_count << std::endl;
    for(int i =1; i <= product_count; i++) { /// start from 1
        ///FYI: get an array element by index
        lua_rawgeti(LuaConfig::s_L, -1, i); /// get product

        // if(lua_istable(LuaConfig::s_L, -1)) {
        // }
        ProductCategory pc;
        lua_getfield(LuaConfig::s_L, -1, "vendor");
        if(lua_isstring(LuaConfig::s_L, -1)) {
            // std::cout << "vendor: " << lua_tostring(LuaConfig::s_L, -1) << std::endl;
            pc.vendor = lua_tostring(LuaConfig::s_L, -1);
        }
        lua_pop(LuaConfig::s_L, 1);

        lua_getfield(LuaConfig::s_L, -1, "model");
        if(lua_isstring(LuaConfig::s_L, -1)) {
            // std::cout << "model: " << lua_tostring(LuaConfig::s_L, -1) << std::endl;
            pc.model = lua_tostring(LuaConfig::s_L, -1);
        }
        lua_pop(LuaConfig::s_L, 1);

        /// pop for stacked lua_rawgeti
        lua_pop(LuaConfig::s_L, 1);

        LuaConfig::luaParam.v_product_category.push_back(pc);
    }

    lua_pop(LuaConfig::s_L, 1); // remove the table from the stack
    return true;
}
