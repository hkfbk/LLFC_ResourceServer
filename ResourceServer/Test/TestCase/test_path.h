#include "global.hpp"
#include <filesystem>

namespace Test::path {



	void test ()
	{
		std::string pre_path = cfg["Path"]["ResourceRoot"].value_or<std::string> ("failed");
		std::cout << std::format ("pre_path:{}\n", pre_path);
	}
}