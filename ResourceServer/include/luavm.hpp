#include "lua.hpp"
#include <iostream>

void test_luajit ()
{
	std::cout << "call test_luajit\n";
	lua_State* lua = luaL_newstate ();
	luaL_openlibs (lua);
	auto result = luaL_dostring (lua, "print('test_luajit')");
	if (!result)
		std::cout << "call test_luajit failed!!!\n";
	else std::cout << "call test_luajit success\n";
	lua_close (lua);
	lua = nullptr;
}