// Copyright David Pol 2013
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef LUNATIC_LUA_FUNCTION_HPP_INCLUDED
#define LUNATIC_LUA_FUNCTION_HPP_INCLUDED

#include <cassert>
#include <string>
#include <tuple>

#include <lua.hpp>

namespace lunatic {

namespace detail {

//----------------------------------------------------------------
// Function template to automate pushing values to the Lua stack.
//----------------------------------------------------------------
int push_lua_args(lua_State*)
{
	return 0;
}

template<typename... Args>
int push_lua_args(lua_State* L, bool arg, Args... rest)
{
	lua_pushboolean(L, arg);
	return 1 + push_lua_args(L, rest...);
}

template<typename... Args>
int push_lua_args(lua_State* L, lua_Integer arg, Args... rest)
{
	lua_pushinteger(L, arg);
	return 1 + push_lua_args(L, rest...);
}

template<typename... Args>
int push_lua_args(lua_State*L, lua_Number arg, Args... rest)
{
	lua_pushnumber(L, arg);
	return 1 + push_lua_args(L, rest...);
}

template<typename... Args>
int push_lua_args(lua_State* L, const std::string& arg, Args... rest)
{
	lua_pushstring(L, arg.c_str());
	return 1 + push_lua_args(L, rest...);
}

//---------------------------------------------------------------------
// Function template to automate retrieving values from the Lua stack.
//---------------------------------------------------------------------
template<typename T>
T get_lua_result(lua_State* L, int index = -1);

template<>
bool get_lua_result<bool>(lua_State* L, int index)
{
	assert(lua_isboolean(L, index));
	return lua_toboolean(L, index) != 0;
}

template<>
lua_Integer get_lua_result<lua_Integer>(lua_State* L, int index)
{
	assert(lua_isnumber(L, index));
	return static_cast<int>(lua_tonumber(L, index));
}

template<>
lua_Number get_lua_result<lua_Number>(lua_State* L, int index)
{
	assert(lua_isnumber(L, index));
	return lua_tonumber(L, index);
}

template<>
std::string get_lua_result<std::string>(lua_State* L, int index)
{
	assert(lua_isstring(L, index));
	return lua_tostring(L, index);
}

//------------------------------------------------------------------------
// Function template to automate retrieving multiple values from the Lua
// stack into a std::tuple<>.
//------------------------------------------------------------------------
template<typename First, typename... Rest>
class get_lua_results_helper
{
public:
	static std::tuple<First, Rest...> get_lua_results(lua_State* L)
	{
		const auto f = get_lua_result<First>(L, -static_cast<int>(sizeof...(Rest)) - 1);
		const auto t = std::make_tuple(f);
		return std::tuple_cat(t, get_lua_results_helper<Rest...>::get_lua_results(L));
	}
};

template<typename T>
class get_lua_results_helper<T>
{
public:
	static std::tuple<T> get_lua_results(lua_State* L)
	{
		const auto t = get_lua_result<T>(L);
		return std::make_tuple(t);
	}
};

// Base class from which the actual lua_function<> derives. This is
// just an implementation convenience.
class lua_function_base
{
public:
	lua_function_base(lua_State* L, const char* name)
		: L_(L), name_(name) {}

	// Disable copying.
	lua_function_base(const lua_function_base&) = delete;
	lua_function_base& operator=(const lua_function_base&) = delete;

protected:
	lua_State* L_;
	const char* name_;
};

} // namespace detail

/*
* The lua_function<> type abstracts the calling of a Lua
* function from C++ in a generic, type-safe way.
*
* TODO: - Support for more types.
*       - Better error handling.
*/
template<typename Ret, typename... Args>
class lua_function : public detail::lua_function_base
{
public:
	lua_function(lua_State* L, const char* name)
		: lua_function_base(L, name) {}

	Ret operator()(Args... args)
	{
		lua_getglobal(L_, name_);
		const auto num_args = detail::push_lua_args(L_, args...);
		assert(lua_pcall(L_, num_args, 1, 0) == 0);
		const auto ret = detail::get_lua_result<Ret>(L_);
		lua_pop(L_, 1);
		assert(lua_gettop(L_) == 0);
		return ret;
	}
};

template<typename... Args>
class lua_function<void, Args...> : public detail::lua_function_base
{
public:
	lua_function(lua_State*L, const char* name)
		: lua_function_base(L, name) {}

	void operator()(Args... args)
	{
		lua_getglobal(L_, name_);
		const auto num_args = detail::push_lua_args(L_, args...);
		assert(lua_pcall(L_, num_args, 1, 0) == 0);
	}
};

template<typename... Ret, typename... Args>
class lua_function<std::tuple<Ret...>, Args...> : public detail::lua_function_base
{
public:
	lua_function(lua_State*L, const char* name)
		: lua_function_base(L, name) {}

	std::tuple<Ret...> operator()(Args... args)
	{
		lua_getglobal(L_, name_);
		const auto num_args = detail::push_lua_args(L_, args...);
		assert(lua_pcall(L_, num_args, sizeof...(Ret), 0) == 0);
		const auto ret = detail::get_lua_results_helper<Ret...>::get_lua_results(L_);
		lua_pop(L_, static_cast<int>(sizeof...(Ret)));
		assert(lua_gettop(L_) == 0);
		return ret;
	}
};

} // namespace lunatic

#endif // LUNATIC_LUA_FUNCTION_HPP_INCLUDED
