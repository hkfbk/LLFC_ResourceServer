# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 复制 ini 文件到构建目录
configure_file(${CMAKE_SOURCE_DIR}/ResourceServer/config/config.toml ${CMAKE_BINARY_DIR}/ResourceServer/config/config.toml COPYONLY)

project ("ResourceServer")
find_package(Boost REQUIRED COMPONENTS asio)
find_package(spdlog REQUIRED COMPONENTS)
find_package(OpenSSL REQUIRED COMPONENTS)
find_package(tomlplusplus REQUIRED COMPONENTS)

# 包含子项目。
add_subdirectory ("ResourceServer")
