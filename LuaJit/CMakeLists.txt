cmake_minimum_required(VERSION 3.10)

project(lualib LANGUAGES C VERSION 0.01)

# 定义库目标，使用已有的二进制文件
add_library(lualib SHARED IMPORTED)


# 指定库文件位置
set_target_properties(lualib PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/dll/lua51.dll"
    IMPORTED_IMPLIB   "${CMAKE_CURRENT_SOURCE_DIR}/lib/lua51.lib"
)

# 指定头文件目录
target_include_directories(lualib INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)



