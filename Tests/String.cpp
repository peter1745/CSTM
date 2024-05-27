#include "Test.hpp"

#include <String.hpp>
#include <CodePointIterator.hpp>

#include <print>

using namespace CSTM;

DeclTest(string, large_small_string)
{
	const auto smallString = String::create("Hello, World!");
	const auto largeString = String::create("Hello, World! My name is Bob!");
	Cond(Eq, smallString.is_large_string(), false);
	Cond(Eq, largeString.is_large_string(), true);
}

DeclTest(string, ref_count)
{
	const auto smallString = String::create("Hello, World!");
	const auto smallStringCopy = smallString;
	Cond(Eq, smallString.ref_count(), 1);
	Cond(Eq, smallStringCopy.ref_count(), 1);

	const auto largeString = String::create("Hello, Cruel World!");
	Cond(Eq, largeString.ref_count(), 1);

	const auto largeStringCopy = largeString;
	Cond(Eq, largeString.ref_count(), 2);
	Cond(Eq, largeStringCopy.ref_count(), 2);

	{
		const auto largeString1 = String::create("Hello, Cruel World!");
		Cond(Eq, largeString.ref_count(), 3);
		Cond(Eq, largeStringCopy.ref_count(), 3);
		Cond(Eq, largeString1.ref_count(), 3);
	}

	Cond(Eq, largeString.ref_count(), 2);
	Cond(Eq, largeStringCopy.ref_count(), 2);
}

DeclTest(string, equals)
{
	// Small-string - Small-string comparison
	const auto smallString = String::create("Hello, World!");
	const auto smallString1 = String::create("Hello, World!");
	const auto smallString2 = String::create("Goodbye, World!");
	Cond(Eq, smallString, smallString1);
	Cond(Eq, smallString1, smallString);
	Cond(NotEq, smallString, smallString2);
	Cond(NotEq, smallString1, smallString2);

	// Large-string - Large-string comparison
	const auto largeString = String::create("Hello, Cruel World!");
	const auto largeString1 = String::create("Hello, Cruel World!");
	const auto largeString2 = String::create("Goodbye, Cruel World!");
	Cond(Eq, largeString, largeString1);
	Cond(Eq, largeString1, largeString);
	Cond(NotEq, largeString, largeString2);
	Cond(NotEq, largeString1, largeString2);
}

DeclTest(string, code_point_iterator)
{
	const auto str = String::create("Hello, World");
	constexpr auto codePoints = std::array{ 72u, 101u, 108u, 108u, 111u, 44u, 32u, 87u, 111u, 114u, 108u, 100u };

	CondManual(CodePointIterator{str}.each([&](const size_t i, const uint32_t cp)
	{
		if (codePoints[i] != cp)
		{
			pass = false;
			return IterAction::Break;
		}

		pass = true;
		return IterAction::Continue;
	}), true);
}
