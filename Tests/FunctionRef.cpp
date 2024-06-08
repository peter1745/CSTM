#include "Test.hpp"

#include <FunctionRef.hpp>

using namespace CSTM;

constexpr int add(const int x, const int y)
{
	return x + y;
}

DeclTest(function_ref, free_function)
{
	FunctionRef<int(int, int)> f{ add };
	Cond(Eq, f.is_empty(), false);
	Cond(Eq, f(10, 10), 20);
}

DeclTest(function_ref, lambda)
{
	FunctionRef<int(int, int)> f
	{
		[](const int x, const int y)
		{
			return x + y;
		}
	};

	Cond(Eq, f.is_empty(), false);
	Cond(Eq, f(10, 10), 20);
}

DeclTest(function_ref, member_function)
{
	struct Dummy
	{
		int my_member_function(const int x, const int y)
		{
			return x + y;
		}
	};

	Dummy dummy;
	FunctionRef<int(int, int)> f { nontype<&Dummy::my_member_function>, dummy };
	Cond(Eq, f.is_empty(), false);
	Cond(Eq, f(10, 10), 20);
}

DeclTest(function_ref, const_member_function)
{
	struct Dummy
	{
		int my_member_function(const int x, const int y)
		{
			return x + y;
		}

		int my_const_member_function(const int x, const int y) const
		{
			return x + y;
		}
	};

	const Dummy dummy;
	// The line below shouldn't compile, there's no good way of testing this afaik
	// FunctionRef<int(int, int)> f { nontype<&Dummy::my_member_function>, dummy };
	FunctionRef<int(int, int)> f { nontype<&Dummy::my_const_member_function>, dummy };
	Cond(Eq, f.is_empty(), false);
	Cond(Eq, f(10, 10), 20);
}

