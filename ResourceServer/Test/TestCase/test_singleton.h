#pragma once
#include "singleton.hpp"
#include <thread>


namespace Test::singleton {
	class Test_Singleton :public Singleton<Test_Singleton>
	{
		_SLNGLETON_DELETE_COPY_MOVE_ (Test_Singleton);
		friend class Singleton<Test_Singleton>;
		Test_Singleton () = default;
	public:
		~Test_Singleton () = default;
	};

	static void test ()
	{
		Test_Singleton::get_instance ().test ();
		std::cout << Test_Singleton::get_instance ().get_address () << "\n";
	}
}