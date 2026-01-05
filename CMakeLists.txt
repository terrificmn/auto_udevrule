cmake_minimum_required(VERSION 3.10)
project(auto_udevrule VERSION 1.3.0)
add_definitions(-DVERSION="${PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
## lua static path
set(LUA_ROOT /opt/lua-5.3-static)

option(PUBLIC_BUILD "Build for public version" ON)
if(PUBLIC_BUILD)
    add_compile_definitions(PUBLIC_BUILD=1)
    message("-- PUBLIC_BUILD is ON")
else()
    add_compile_definitions(PROJECT_BUILD=1)
    message("-- PROJECT_BUILD is ON")
endif()

add_library(lua_static STATIC IMPORTED)
set_target_properties(lua_static PROPERTIES
    IMPORTED_LOCATION ${LUA_ROOT}/lib/liblua.a
    INTERFACE_INCLUDE_DIRECTORIES ${LUA_ROOT}/include
)

# Create a shared library
add_library(auto_udevrule.so.${PROJECT_VERSION} SHARED 
    src/lua_config.cpp
    src/sub_process_writer.cpp
    src/sudo_manager.cpp
    src/time_checker.cpp
    src/udev_maker.cpp
    src/usb_info_confirmer.cpp
)

### FYI: uncomment the below and comment out the above(add_library) if you want to built an normal executable file.
### FYI: 또한, target_link_libraries 이름(getudev)도 변경해준다. 
# add_executable(getudev
#     src/main.cpp
#     src/lua_config.cpp
#     src/manager.cpp
#     src/sub_process_writer.cpp
#     src/sudo_manager.cpp
#     src/time_checker.cpp
#     src/udev_maker.cpp
#     src/usb_info_confirmer.cpp
# )

include_directories(${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/include
)

# target_link_libraries(getudev PRIVATE ### for executable
target_link_libraries(auto_udevrule.so.${PROJECT_VERSION} PRIVATE
    lua_static
    dl
    m
)
