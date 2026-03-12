#pragma once
#include "global.hpp"

namespace Test::defer {
	static void test ()
	{
		Defer d ([] {std::cout << "Defer\n"; });
		std::ignore = std::getchar ();
	}
}