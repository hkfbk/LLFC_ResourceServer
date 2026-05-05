# CMakeList.txt: ResourceServer 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#

# 将源代码添加到此项目的可执行文件。
include_directories("include")
file(GLOB SRC "src/*.cpp")
file(GLOB TestScr "Test/*.cpp")
add_executable (ResourceServer ${SRC})
add_executable (ResourceServerTest ${TestScr})


target_link_libraries("ResourceServer" PRIVATE Boost::asio
											   spdlog::spdlog_header_only
											   OpenSSL::SSL 
											   OpenSSL::Crypto
											   tomlplusplus::tomlplusplus
# end						
)
target_link_libraries("ResourceServerTest" PRIVATE Boost::asio
											   spdlog::spdlog_header_only
											   OpenSSL::SSL 
											   OpenSSL::Crypto
											   tomlplusplus::tomlplusplus
# end						
)
