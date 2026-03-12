# CMakeList.txt: ResourceServer 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#

# 将源代码添加到此项目的可执行文件。
include_directories("include")
file(GLOB SRC "src/*.cpp")
file(GLOB TestScr "Test/*.cpp")
add_executable (ResourceServer ${SRC})
add_executable (ResourceServerTest ${TestScr})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ResourceServer PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries("ResourceServer" PRIVATE Boost::asio
											   spdlog::spdlog_header_only
											   OpenSSL::SSL 
											   OpenSSL::Crypto
# end						
)
target_link_libraries("ResourceServerTest" PRIVATE Boost::asio
											   spdlog::spdlog_header_only
											   OpenSSL::SSL 
											   OpenSSL::Crypto
# end						
)

# TODO: 如有需要，请添加测试并安装目标。
