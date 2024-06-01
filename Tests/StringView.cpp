#include "Test.hpp"

#include <String.hpp>
#include <StringView.hpp>

using namespace CSTM;

DeclTest(string_view, create)
{
	const auto str = String::create("Hello, World!");
	const auto strView = str.view();
	Cond(Eq, strView.has_value(), true);
	Cond(Eq, str.data(), strView.value().data());
	Cond(Eq, str.byte_count(), strView.value().byte_count());
}

DeclTest(string_view, substr)
{
	const char* expectedSubstr = "ello, World!";
	const auto str = String::create("Hello, World!");
	const auto strView = str.view(1);
	Cond(Eq, strView.has_value(), true);
	Cond(Eq, strView.value(), expectedSubstr);
}

DeclTest(string_view, substr_length)
{
	const char* expectedSubstr = "Hello";
	const auto str = String::create("Hello, World!");
	const auto strView = str.view(0, 5);
	Cond(Eq, strView.has_value(), true);
	Cond(Eq, strView.value(), expectedSubstr);
}

DeclTest(string_view, contains_all)
{
	const auto str = String::create("Hello, World");
	const auto strView = str.view().value_or({});
	Cond(Eq, strView.contains_all("Hello"), true);
	Cond(Eq, strView.contains_all(std::string{"Hello"}), true);
	Cond(Eq, strView.contains_all(std::string_view{"Hello"}), true);
	Cond(Eq, strView.contains_all("olleH"), false);
}

DeclTest(string_view, contains_any)
{
	const auto str = String::create("Hello, World");
	const auto strView = str.view().value_or({});
	Cond(Eq, strView.contains_any("Wd"), true);
	Cond(Eq, strView.contains_any(std::string{"Wd"}), true);
	Cond(Eq, strView.contains_any(std::string_view{"Wd"}), true);
	Cond(Eq, strView.contains_any(std::string_view{"Abc"}), false);
}
