#include "Test.hpp"

#include <Result.hpp>
#include <print>

using namespace CSTM;

enum class ErrorCode
{
	None,
	SomeError,
	AnotherError
};

DeclTest(result, default_constructible)
{
	const Result<std::pair<int&, int&>, ErrorCode> r1;
	std::pair pair{50, 10};
	const Result<std::pair<int&, int&>, ErrorCode> r2{pair};

	Cond(Eq, r1.is_empty(), true);
	Cond(Eq, r2.is_empty(), false);
}

DeclTest(result, storage_type)
{
	constexpr Result<int, ErrorCode> r0;
	constexpr Result<int&, ErrorCode> r1;
	constexpr Result<int*, ErrorCode> r2;

	Cond(Eq, r0.storage_type(), ResultStorageType::Value);
	Cond(Eq, r1.storage_type(), ResultStorageType::Reference);
	Cond(Eq, r2.storage_type(), ResultStorageType::Pointer);
}

DeclTest(result, has_value)
{
	constexpr Result<int, ErrorCode> r0;
	constexpr Result<int, ErrorCode> r1{ 50 };

	Cond(Eq, r0.has_value(), false);
	Cond(Eq, r1.has_value(), true);
}

DeclTest(result, value)
{
	constexpr Result<int, ErrorCode> result{ 50 };
	Cond(Eq, result.value(), 50);
}

DeclTest(result, value_or)
{
	constexpr Result<int, ErrorCode> r0;
	constexpr Result<int, ErrorCode> r1{ 50 };
	Cond(Eq, r0.value_or(10), 10);
	Cond(Eq, r1.value_or(10), 50);
}

DeclTest(result, error)
{
	constexpr Result<int, ErrorCode> result{ ErrorCode::SomeError };
	Cond(Eq, result.error(), ErrorCode::SomeError);
}

DeclTest(result, error_or)
{
	constexpr Result<int, ErrorCode> r0;
	constexpr Result<int, ErrorCode> r1{ ErrorCode::SomeError };
	Cond(Eq, r0.error_or(ErrorCode::None), ErrorCode::None);
	Cond(Eq, r1.error_or(ErrorCode::None), ErrorCode::SomeError);
}

DeclTest(result, match)
{
	constexpr Result<int, ErrorCode> r0;
	constexpr Result<int, ErrorCode> r1{ ErrorCode::SomeError };
	constexpr Result<int, ErrorCode> r2{ 10 };

	CondManual(r0.match(
		[&]{ pass = false; },
		[&]{ pass = false; }
	), false);

	CondManual(r1.match(
		[&]{ pass = false; },
		[&]{ pass = true; }
	), true);

	CondManual(r2.match(
		[&]{ pass = true; },
		[&]{ pass = false; }
	), true);
}

DeclTest(result, match_ret)
{
	const auto r0 = Result<int, ErrorCode>{}.match(
		[&]{ return Result<int, ErrorCode>{ 20 }; },
		[&]{ return Result<int, ErrorCode>{ ErrorCode::SomeError }; }
	);

	const auto r1 = Result<int, ErrorCode>{ ErrorCode::SomeError }.match(
		[&]{ return Result<int, ErrorCode>{ 20 }; },
		[&]{ return Result<int, ErrorCode>{ ErrorCode::AnotherError }; }
	);

	const auto r2 = Result<int, ErrorCode>{ 10 }.match(
		[&]{ return Result<int, ErrorCode>{ 20 }; },
		[&]{ return Result<int, ErrorCode>{ ErrorCode::AnotherError }; }
	);

	Cond(Eq, r0.is_empty(), true);
	Cond(Eq, r1.error(), ErrorCode::AnotherError);
	Cond(Eq, r2.value(), 20);
}

DeclTest(result, and_then)
{
	const auto r0 = Result<int, ErrorCode>{}.and_then([]{});
	const auto r1 = Result<int, ErrorCode>{ ErrorCode::SomeError }.and_then([](const int){});
	const auto r2 = Result<int, ErrorCode>{ 10 }.and_then([](const int value){ return value * 10; });

	Cond(Eq, r0.is_empty(), true);
	Cond(Eq, r1.has_error(), true);
	Cond(Eq, r2.value(), 100);
}

DeclTest(result, or_else)
{
	enum class DummyError { None, SomeError, AnotherError };

	const auto r0 = Result<int, ErrorCode>{}.or_else([]{ return DummyError::SomeError; });
	const auto r1 = Result<int, ErrorCode>{ ErrorCode::SomeError }.or_else([](const ErrorCode code)
	{
		switch (code)
		{
		case ErrorCode::None: return DummyError::None;
		case ErrorCode::SomeError: return DummyError::SomeError;
		case ErrorCode::AnotherError: return DummyError::AnotherError;
		}

		return DummyError::None;
	});
	const auto r2 = Result<int, ErrorCode>{ 10 }.or_else([]{ return DummyError::SomeError; });

	Cond(Eq, r0.is_empty(), true);
	Cond(Eq, r1.error(), DummyError::SomeError);
	Cond(Eq, r2.value(), 10);
}

DeclTest(result, throw_on_error)
{
	constexpr Result<int, ErrorCode> r0;
	constexpr Result<int, ErrorCode> r1{ ErrorCode::SomeError };
	constexpr Result<int, ErrorCode> r2{ 10 };

	struct DummyException {};

	CondManual([&]
	{
		try
		{
			r0.throw_on_value<DummyException>().throw_on_error<DummyException>();
		}
		catch (...)
		{
			pass = false;
		}
	}(), false);

	CondManual([&]
	{
		try
		{
			r1.throw_on_error<DummyException>();
		}
		catch (const DummyException&)
		{
			pass = true;
		}
		catch (...)
		{
			pass = false;
		}
	}(), true);

	CondManual([&]
	{
		try
		{
			r2.throw_on_value<DummyException>();
		}
		catch (const DummyException&)
		{
			pass = true;
		}
		catch (...)
		{
			pass = false;
		}
	}(), true);
}
