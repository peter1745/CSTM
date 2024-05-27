#include "Test.hpp"

#include <print>

std::vector<TestListing>& get_tests()
{
	static std::vector<TestListing> tests;
	return tests;
}

std::monostate register_test(std::string_view category, std::string_view name, TestEntryPointFn entryPoint)
{
	get_tests().emplace_back(
		category,
		name,
		entryPoint
	);
	return {};
}

int main(int argc, char* argv[])
{
	uint32_t totalPassed = 0;
	uint32_t totalFailed = 0;
	uint32_t totalSkipped = 0;

	bool verbose = false;

	for (int i = 1; i < argc; i++)
	{
		std::string_view arg{ argv[i] };

		if (arg == "--verbose" || arg == "-v")
		{
			verbose = true;
		}
	}

	for (const auto& test : get_tests())
	{
		if (argc > 1)
		{
			bool executeTest = false;

			for (int i = 1; i < argc; i++)
			{
				std::string_view arg{ argv[i] };

				if (arg.starts_with('-'))
				{
					continue;
				}

				if (test.category == argv[i])
				{
					executeTest = true;
					break;
				}
			}

			if (!executeTest)
			{
				totalSkipped++;

				if (verbose)
				{
					std::println("[\u001B[33mSKIP\u001B[0m]: {}", test.name);
				}

				continue;
			}
		}

		std::vector<TestCondition> conditions;
		test.entry_point(conditions);

		bool passed = true;

		for (const auto& cond : conditions)
		{
			if (!cond.pass)
			{
				passed = false;
			}
		}

		if (passed)
		{
			std::println("[\u001B[32mPASS\u001B[0m]: {}", test.name);
			totalPassed++;
		}
		else
		{
			std::println("[\u001B[91mFAIL\u001B[0m]: {}", test.name);

			for (const auto& cond : conditions)
			{
				if (!cond.pass)
				{
					std::println("\t- Expression '{}' on line {} was false", cond.cond, cond.line);
				}
			}

			totalFailed++;
		}
	}

	std::println("{} tests passed, {} failed, {} skipped. (Total {})", totalPassed, totalFailed, totalSkipped, get_tests().size());
}
