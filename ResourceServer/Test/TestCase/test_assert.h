#pragma once
#include "global.hpp"

namespace Test::assert {
	void test ()
	{
		Assert (true, "true");
		Assert (false, "false");
	}
}