#include "Test.hpp"

#include <Utf8String.hpp>

#include <set>

using namespace CSTM;

DeclTest(utf8_string, create)
{
	auto smallString = Utf8String::from_chars("Hello, SSO!");
	auto largeString = Utf8String::from_chars("Hello, Large Mode Engaged!");

	Cond(Eq, smallString.large_storage_engaged(), false);
	Cond(Eq, largeString.large_storage_engaged(), true);
}

DeclTest(utf8_string, equal_check_small_string)
{
	auto str1 = Utf8String::from_chars("Small string");
	auto str2 = Utf8String::from_chars("Small strin");
	auto str3 = Utf8String::from_chars("Small string");

	Cond(NotEq, str1, str2);
	Cond(NotEq, str2, str1);
	Cond(Eq, str1, str3);
	Cond(Eq, str3, str1);
}

DeclTest(utf8_string, equal_check_large_string)
{
	auto str1 = Utf8String::from_chars("This is some large string that should hit the string pool");
	auto str2 = Utf8String::from_chars("This is some large string that shouldn't equal str1");
	auto str3 = Utf8String::from_chars("This is some large string that should hit the string pool");

	Cond(NotEq, str1, str2);
	Cond(NotEq, str2, str1);
	Cond(Eq, str1, str3);
	Cond(Eq, str3, str1);
}

DeclTest(utf8_string, ref_count)
{
	{
		auto smallString = Utf8String::from_chars("Hello, SSO!");
		Cond(Eq, smallString.ref_count(), 1);

		const Utf8String copy(smallString);
		Cond(Eq, smallString.ref_count(), 1);
		Cond(Eq, copy.ref_count(), 1);
		Cond(Eq, smallString, copy);
	}

	{
		auto largeString = Utf8String::from_chars("Hello, Large Mode Engaged!");
		Cond(Eq, largeString.ref_count(), 1);

		const Utf8String copy(largeString);
		Cond(Eq, largeString.ref_count(), 2);
		Cond(Eq, copy.ref_count(), 2);
		Cond(Eq, largeString.data(), copy.data());
	}
}

DeclTest(utf8_string, unicode_byte_count)
{
	auto str = Utf8String::from_chars(u8"きみのないわ");
	Cond(Eq, str.byte_count(), 18);
}

DeclTest(utf8_string, view)
{
	auto str = Utf8String::from_chars(u8"きみのないわ");
	auto view = str.view();

	Cond(Eq, view.ptr, str.data());
	Cond(Eq, view.byte_count, 18);
}

DeclTest(utf8_string, unicode_code_points)
{
	const char8_t* asciiChars = u8"Hello, World!";
	const char8_t* jpChars = u8"きみのないわ";

	auto asciiStr = Utf8String::from_chars(asciiChars);
	auto jpStr = Utf8String::from_chars(jpChars);

	const auto asciiCharsCodePoints = std::vector<uint32_t>
	{
		0x0048, 0x0065, 0x006C, 0x006C, 0x006F, 0x002C, 0x0020, 0x0057, 0x006F, 0x0072, 0x006C, 0x0064, 0x0021
	};

	const auto jpCharsCodePoints = std::vector<uint32_t>
	{
		0x304D, 0x307F, 0x306E, 0x306A, 0x3044, 0x308F
	};

	size_t i = 0;
	for (auto cp : asciiStr)
	{
		Cond(Eq, cp, asciiCharsCodePoints[i++]);
	}

	i = 0;
	for (auto cp : jpStr)
	{
		Cond(Eq, cp, jpCharsCodePoints[i++]);
	}
}

DeclTest(utf8_string, unicode_reverse_code_points)
{
	const char8_t* asciiChars = u8"Hello, World!";
	const char8_t* jpChars = u8"きみのないわ";

	auto asciiStr = Utf8String::from_chars(asciiChars);
	auto jpStr = Utf8String::from_chars(jpChars);

	const auto asciiCharsCodePoints = std::vector<uint32_t>
	{
		0x0048, 0x0065, 0x006C, 0x006C, 0x006F, 0x002C, 0x0020, 0x0057, 0x006F, 0x0072, 0x006C, 0x0064, 0x0021
	} | std::views::reverse;

	const auto jpCharsCodePoints = std::vector<uint32_t>
	{
		0x304D, 0x307F, 0x306E, 0x306A, 0x3044, 0x308F
	} | std::views::reverse;

	size_t i = 0;
	for (auto it = asciiStr.rbegin(); it != asciiStr.rend(); ++it)
	{
		Cond(Eq, *it, asciiCharsCodePoints[i++]);
	}

	i = 0;
	for (auto it = jpStr.rbegin(); it != jpStr.rend(); ++it)
	{
		Cond(Eq, *it, jpCharsCodePoints[i++]);
	}
}

DeclTest(utf8_string, unicode_starts_with)
{
	auto asciiStr = Utf8String::from_chars(u8"Hello, World!");
	auto jpStr = Utf8String::from_chars(u8"きみのないわ");

	Cond(Eq, asciiStr.starts_with(u8"Hello"), true);
	Cond(Eq, asciiStr.starts_with(u8", World!"), false);

	Cond(Eq, jpStr.starts_with(u8"きみの"), true);
	Cond(Eq, jpStr.starts_with(u8"ないわ"), false);
}

DeclTest(utf8_string, unicode_starts_with_any)
{
	const auto codePoints = std::set<uint32_t>
	{
		0x308F, // わ
		0x304D, // き
		0x0021, // !
		0x0048, // H
	};

	auto asciiStr = Utf8String::from_chars(u8"Hello, World!");
	auto asciiStr2 = Utf8String::from_chars(u8"This includes Hello, World! but not at the start");

	auto jpStr = Utf8String::from_chars(u8"きみのないわ");
	auto jpStr2 = Utf8String::from_chars(u8"This includes きみのないわ but not at the start");

	Cond(Eq, asciiStr.starts_with_any(codePoints), true);
	Cond(Eq, asciiStr2.starts_with_any(codePoints), false);

	Cond(Eq, jpStr.starts_with_any(codePoints), true);
	Cond(Eq, jpStr2.starts_with_any(codePoints), false);
}

DeclTest(utf8_string, unicode_ends_with)
{
	auto asciiStr = Utf8String::from_chars(u8"Hello, World!");
	auto jpStr = Utf8String::from_chars(u8"きみのないわ");

	Cond(Eq, asciiStr.ends_with(u8"World!"), true);
	Cond(Eq, asciiStr.ends_with(u8"Hello,"), false);

	Cond(Eq, jpStr.ends_with(u8"ないわ"), true);
	Cond(Eq, jpStr.ends_with(u8"きみの"), false);
}

DeclTest(utf8_string, unicode_ends_with_any)
{
	const auto codePoints = std::set<uint32_t>
	{
		0x308F, // わ
		0x304D, // き
		0x0021, // !
		0x0048, // H
	};

	auto asciiStr = Utf8String::from_chars(u8"Hello, World!");
	auto asciiStr2 = Utf8String::from_chars(u8"This includes Hello, World! but not at the start");

	auto jpStr = Utf8String::from_chars(u8"きみのないわ");
	auto jpStr2 = Utf8String::from_chars(u8"This includes きみのないわ but not at the start");

	Cond(Eq, asciiStr.ends_with_any(codePoints), true);
	Cond(Eq, asciiStr2.ends_with_any(codePoints), false);

	Cond(Eq, jpStr.ends_with_any(codePoints), true);
	Cond(Eq, jpStr2.ends_with_any(codePoints), false);
}

DeclTest(utf8_string, unicode_erase_leading)
{
	auto asciiStr = Utf8String::from_chars(u8"Hello, World!");
	auto jpStr = Utf8String::from_chars(u8"きみのないわ");

	const auto asciiCharsCodePoints = std::vector<uint32_t>
	{
		0x0048, // H
		0x0065, // e
		0x006C, // l
		0x002C, // ,
		0x0021  // !
	};

	const auto jpCharsCodePoints = std::vector<uint32_t>
	{
		0x304D, // き
		0x307F, // み
		0x3044, // い
		0x308F  // わ
	};

	{
		auto expected = Utf8String::from_chars(u8"o, World!");
		auto result = asciiStr.erase(asciiCharsCodePoints, EraseMode::Leading);
		Cond(Eq, result, expected);
	}

	{
		auto expected = Utf8String::from_chars(u8"のないわ");
		auto result = jpStr.erase(jpCharsCodePoints, EraseMode::Leading);
		Cond(Eq, result, expected);
	}
}

DeclTest(utf8_string, unicode_erase_trailing)
{
	auto asciiStr = Utf8String::from_chars(u8"Hello, World!");
	auto jpStr = Utf8String::from_chars(u8"きみのないわ");

	const auto asciiCharsCodePoints = std::vector<uint32_t>
	{
		0x0048, // H
		0x0065, // e
		0x006C, // l
		0x002C, // ,
		0x0021  // !
	};

	const auto jpCharsCodePoints = std::vector<uint32_t>
	{
		0x304D, // き
		0x307F, // み
		0x3044, // い
		0x308F  // わ
	};

	{
		auto expected = Utf8String::from_chars(u8"Hello, World");
		auto result = asciiStr.erase(asciiCharsCodePoints, EraseMode::Trailing);
		Cond(Eq, result, expected);
	}

	{
		auto expected = Utf8String::from_chars(u8"きみのな");
		auto result = jpStr.erase(jpCharsCodePoints, EraseMode::Trailing);
		Cond(Eq, result, expected);
	}
}
