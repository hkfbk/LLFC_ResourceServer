# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 复制 ini 文件到构建目录
configure_file(${CMAKE_SOURCE_DIR}/ResourceServer/config/config.toml ${CMAKE_BINARY_DIR}/ResourceServer/config/config.toml COPYONLY)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("ResourceServer")

find_package(Boost REQUIRED COMPONENTS asio)
find_package(spdlog REQUIRED COMPONENTS)
find_package(OpenSSL REQUIRED COMPONENTS)

# 包含子项目。
add_subdirectory ("ResourceServer")
