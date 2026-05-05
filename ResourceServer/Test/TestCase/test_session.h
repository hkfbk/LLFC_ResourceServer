#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <thread>

namespace asio = boost::asio;
using boost::asio::ip::tcp;
using namespace std::chrono_literals;
namespace Test::session {
	int test1 (std::string msg)
	{
		char buffer[1024]{ 0 };
		try {
			asio::io_context io_context;

			// 创建socket并连接
			tcp::socket socket (io_context);
			tcp::endpoint endpoint (asio::ip::make_address ("127.0.0.1"), 9091);
			socket.connect (endpoint);
			std::cout << "Connected to server." << std::endl;

			// 1. 准备消息体（JSON数据）
			std::string message_body = msg;

			// 2. 准备消息头部
			uint16_t message_id = 1019;  // 消息ID
			uint16_t net_message_id = asio::detail::socket_ops::host_to_network_short (message_id);  // 转为网络字节序

			uint32_t body_length = static_cast<uint32_t>(message_body.size ());
			uint32_t net_body_length = asio::detail::socket_ops::host_to_network_short (body_length);  // 转为网络字节序

			while (true) {

				// 3. 发送消息头部（6字节）
				// 发送2字节消息ID
				asio::write (socket, asio::buffer (&net_message_id, sizeof (net_message_id)));

				// 发送4字节消息体长度
				asio::write (socket, asio::buffer (&net_body_length, sizeof (net_body_length)));

				// 4. 发送消息体
				asio::write (socket, asio::buffer (message_body));

				std::cout << "Message sent:" << std::endl;
				std::cout << "  ID: " << message_id << std::endl;
				std::cout << "  Body length: " << body_length << " bytes" << std::endl;
				std::cout << "  Body: " << message_body << std::endl;
				std::this_thread::sleep_for (0.1s);
				std::cout << "Read...\n";
				uint16_t respond_message_id;

				asio::read (socket, asio::buffer (&respond_message_id, sizeof (respond_message_id)));
				respond_message_id = asio::detail::socket_ops::network_to_host_short (respond_message_id);
				std::cout << std::format ("Respond id:{}\n", respond_message_id);
				uint32_t respond_body_length;
				asio::read (socket, asio::buffer (&respond_body_length, sizeof (respond_body_length)));
				respond_body_length = asio::detail::socket_ops::network_to_host_short (respond_body_length);
				std::cout << std::format ("Respond body lenght:{}\n", respond_body_length);
				asio::read (socket, asio::buffer (buffer, respond_body_length));
				std::cout << std::format ("Respond:{}\n", std::string (buffer, respond_body_length));
				std::this_thread::sleep_for (0.1s);


			}

			socket.close ();
			std::cout << "Connection closed." << std::endl;

		}
		catch (std::exception& e) {
			std::cerr << "Error: " << e.what () << std::endl;
			return 1;
		}

		return 0;
	}

	void thr_test ()
	{
		std::thread th1 (test1, R"({"data":"hello,c++"})");
		std::thread th2 (test1, R"({"data":"helloworld"})");
		th1.join ();
		th2.join ();
	}

	int test ()
	{
		thr_test ();
		return 0;
	}
}