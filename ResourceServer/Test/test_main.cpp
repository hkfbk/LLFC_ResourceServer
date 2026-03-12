#define __MYTEST__
#include <iostream>
#include "TestCase/test_defer.h"
#include "TestCase/test_singleton.h"
#include "TestCase/test_assert.h"
#include "TestCase/test_session.h"
#include "TestCase/test_path.h"
namespace test_name = Test::path;


int main ()
{
	std::cout << "Start Test\n";
	try {

		test_name::test ();
	}
	catch (std::exception e)
	{
		std::cout << std::format ("Test case error:{}", e.what ());
	}
	(void) getchar ();
}