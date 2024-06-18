#pragma once

#include "Assert.hpp"
#include "Utility.hpp"

namespace CSTM {

	CSTM_TagType(null_t);

	constexpr auto null = null_t{};

	template<typename T>
	concept nullable = requires(T t)
	{
		{ t.is_null() } -> std::same_as<bool>;
	};

	template<typename T>
	requires(nullable<T>)
	void ensure_not_null(T&& value)
	{
		CSTM_Assert(!value.is_null());
	}

	template<typename T>
	requires(std::is_pointer_v<T>)
	void ensure_not_null(T value)
	{
		CSTM_Assert(value != nullptr);
	}

}
