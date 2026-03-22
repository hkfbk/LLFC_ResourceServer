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
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/LuaJit/include")
find_package(Boost REQUIRED COMPONENTS asio)
find_package(spdlog REQUIRED COMPONENTS)
find_package(OpenSSL REQUIRED COMPONENTS)

# 搜索 libluajit.a 或 luajit.lib
find_library(LUAJIT_LIBRARY
    NAMES lua51
    PATHS "${CMAKE_SOURCE_DIR}/LuaJit/lib"
    NO_DEFAULT_PATH   # 可选，只搜索给定路径
)
# 包含子项目。
add_subdirectory ("ResourceServer")

# 拷贝目录并保持目录结构
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/LuaJit/dll/lua51.dll
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ResourceServer
)