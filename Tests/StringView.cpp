#include "Test.hpp"

#include <String.hpp>
#include <StringView.hpp>

using namespace CSTM;

DeclTest(string_view, create)
{
	const auto str = String::create("Hello, World!");
	const auto view = str.view();
	Cond(Eq, view.has_value(), true);
	Cond(Eq, str.data(), view.value_or({}).data());
}
