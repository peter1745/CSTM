#pragma once

#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <cstdint>

struct TestCondition
{
	std::string_view cond;
	uint32_t line;
	bool pass;
	std::string value;
	std::string expected;
};

using TestEntryPointFn = void(*)(std::vector<TestCondition>& outConditions);

struct TestListing
{
	std::string_view category;
	std::string_view name;
	TestEntryPointFn entry_point;
};

std::vector<TestListing>& get_tests();
std::monostate register_test(std::string_view category, std::string_view name, TestEntryPointFn entryPoint);

#define DeclTest(category, name)\
	void category##_##name##_test_main(std::vector<TestCondition>& outConditions);\
	static auto category##_##name##_test_state = register_test(#category, #category "_" #name, category##_##name##_test_main);\
	void category##_##name##_test_main(std::vector<TestCondition>& outConditions)

#define CondStr(v0, v1, op) #v0#op#v1
#define Cond(op, v0, v1)\
	do\
	{\
		auto& condState = outConditions.emplace_back();\
		condState.cond = CondStr(v0, v1, op);\
		condState.line = __LINE__;\
		bool pass = false;\
		try {\
			pass = (v0 op v1);\
		} catch(...) {\
			pass = false;\
		}\
		condState.pass = pass;\
	} while(false)

#define CondManual(expr, passValue)\
	do\
	{\
		auto& condState = outConditions.emplace_back();\
		condState.cond = CondStr(pass, passValue, ==);\
		condState.line = __LINE__;\
		bool pass = false;\
		try {\
			expr;\
		} catch(...) {\
			pass = !passValue;\
		}\
		condState.pass = pass == passValue;\
	} while(false)

#define Eq ==
#define NotEq !=
