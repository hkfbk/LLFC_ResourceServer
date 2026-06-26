#ifndef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
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

static void set_log_fmt()
{
	auto server_name_p = cfg["SelfServer"]["Name"].as_string();
	std::string server_name = "Unname_server";
	if (server_name_p) server_name = (*server_name_p).get();
	auto file_sink_mt = std::make_shared<spdlog::sinks::daily_file_sink_mt>(std::format("logs/{}_log.log", server_name), 0, 0);
	file_sink_mt->set_pattern("[%Y-%m-%d %H:%M:%S] | [%l] | %v");
	// 控制台 Sink（带颜色，多线程安全）
	auto console_sink_mt = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	console_sink_mt->set_pattern("[%Y-%m-%d %H:%M:%S] | [%l] | %v");
	std::vector<spdlog::sink_ptr> sinks{ file_sink_mt, console_sink_mt };
	spdlog::set_default_logger(std::make_shared<spdlog::logger>("multi_sink_logger", sinks.begin(), sinks.end()));
	console_sink_mt->set_level(spdlog::level::trace);
	file_sink_mt->set_level(spdlog::level::trace);
	spdlog::flush_on(spdlog::level::trace);
}

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
	std::cout << DEBUGLOG_ << std::endl;
	set_log_fmt();
	run ();
	(void) getchar ();
}
