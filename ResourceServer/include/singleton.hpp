#pragma once
#include <macro.hpp>

#ifdef _DEBUG
#include <iostream>
#endif // _DEBUG

#ifndef __SLNGLETON_DELETE_COPY_MOVE__
#define _SLNGLETON_DELETE_COPY_MOVE_(classname) __NOCOPY_MOVE__(classname)
#endif // !__SLNGLETON_DELETE_COPY_MOVE__

#ifndef DEFINE_SINGLETON
#define DEFINE_SINGLETON(classname) _SLNGLETON_DELETE_COPY_MOVE_(classname);\
friend class Singleton<classname>;
#endif // DEFINE_SINGLETON



template<typename T>
class Singleton
{
protected:
	Singleton ()
	{
#ifdef _DEBUG
		std::clog << typeid(T).name () << " singleton structer\n";
#endif // _DEBUG
	}
	_SLNGLETON_DELETE_COPY_MOVE_ (Singleton);
public:
	[[nodiscard]] static T& get_instance ()
	{
		static T sp_instance;
		return sp_instance;
	}

	[[nodiscard]] std::size_t get_address () const
	{
		return reinterpret_cast<std::size_t>(std::addressof (get_instance ()));
	}

	~Singleton ()
	{
#ifdef _DEBUG
		std::clog << typeid(T).name () << " singleton destruct\n";
#endif // _DEBUG
	}

#ifdef __MYTEST__
	void test ()
	{
		std::cout << "Test_run\n";
	}
#endif // __MYTEST__



};

