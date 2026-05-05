#ifndef _DEBUG
//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif // _DEBUG
#include "ResourceServer.h"


#ifdef _WIN32
#include <Windows.h>
void set_console_to_utf8 ()
{
	SetConsoleOutputCP (CP_UTF8);
	SetConsoleCP (CP_UTF8);
}
#else
void set_console_to_utf8 ()
{ }
#endif // _Win32

static void run ()
{
	try {
		std::clog << "资源服务器准备启动\n";
		std::clog << "ResourceServer run..." << std::endl;
		auto&& server_name = cfg["SelfServer"]["Name"].value_or<std::string> ("failed");
		auto&& port = cfg["SelfServer"]["Port"].value_or<int> (9090);
		std::clog << server_name << std::endl;
		asio::io_context ioc;
		std::make_shared<CServer> (ioc, port)->start ();
		asio::signal_set stop_token (ioc, SIGTERM, SIGINT);
		stop_token.async_wait ([&ioc] (auto, auto) {
			ioc.stop ();
			spdlog::info (std::format ("ResourceServer stop!!!"));
			});
		ioc.run ();
	}
	catch (toml::ex::parse_error e_)
	{
		std::cerr << "解析失败:" << e_.what () << std::endl;
	}
}

int main ()
{
	set_console_to_utf8 ();
	std::cout << BOOST_ASIO_VERSION << std::endl;
	run ();
	
	(void) getchar ();
}
