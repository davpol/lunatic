# lunatic

A collection of utility types for code that mixes C++ and Lua. It should work on any compiler with reasonable support for C++11.

## lua_function<>

This type makes it possible to call a Lua function from C++ in a generic, type-safe way without the need to directly manipulate the Lua stack. If we have defined this function in Lua:

	function secret_of_life()
		return 42
	end

All we need to do to call it from C++ is to

	#include <lua_function.hpp>

and then write:

	lunatic::lua_function<int> secret_of_life(L, "secret_of_life");
	const auto secret = secret_of_life();
	assert(secret == 42);

The assumption here is that L points to a valid lua_State object.

You can also pass parameters to the Lua function. For example, if we have:

	function sum(x, y)
		return x + y
	end

We can do:

	lunatic::lua_function<int, int, int> sum(L, "sum");
	const auto result = sum(3, 9);
	assert(result == 12);

Finally, you can also retrieve multiple return values by using std::tuple<>. Imagine we have defined this function in Lua:

	function nice_cities()
		return "Palma de Mallorca", "Tampere"
	end

We can call this function from C++ like this:

	lunatic::lua_function<std::tuple<std::string, std::string>> nice_cities(L, "nice_cities_to_visit");
	const auto cities = nice_cities();
    assert(std::get<0>(cities) == "Palma de Mallorca");
	assert(std::get<1>(cities) == "Tampere");

There is no fixed limit on the number of parameters we can pass to lua_function<> or the number of return values we can get from it, as the implementation makes use of variadic templates.
