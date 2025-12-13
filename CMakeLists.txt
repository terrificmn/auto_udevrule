cmake_minimum_required(VERSION 3.10)
project(auto_udevrule VERSION 1.2.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
## lua static path
set(LUA_ROOT /opt/lua-5.3-static)

add_library(lua_static STATIC IMPORTED)
set_target_properties(lua_static PROPERTIES
    IMPORTED_LOCATION ${LUA_ROOT}/lib/liblua.a
    INTERFACE_INCLUDE_DIRECTORIES ${LUA_ROOT}/include
)

add_executable(getudev
    src/main.cpp
    src/lua_config.cpp
    src/manager.cpp
    src/sub_process_writer.cpp
    src/sudo_manager.cpp
    src/time_checker.cpp
    src/udev_maker.cpp
    src/usb_info_confirmer.cpp
)

include_directories(${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(getudev PRIVATE
    lua_static
    dl
    m
)
