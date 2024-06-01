#pragma once

#include "Types.hpp"

#include <algorithm>
#include <ranges>

namespace CSTM {

	class StringBase
	{
	public:
		virtual ~StringBase() = default;

		[[nodiscard]]
		virtual const byte* data() const noexcept = 0;

		[[nodiscard]]
		virtual size_t byte_count() const noexcept = 0;

	public:
		[[nodiscard]]
		bool contains_all(const std::ranges::contiguous_range auto& chars) const
			requires(std::same_as<std::ranges::range_value_t<decltype(chars)>, char>)
		{
			const byte* str = data();
			const size_t length = byte_count();

			size_t charsSize = std::ranges::size(chars);

			if (chars[charsSize - 1] == '\0')
			{
				charsSize--;
			}

			if (length < charsSize)
			{
				return false;
			}

			const auto begin = std::ranges::begin(chars);

			return std::ranges::contains_subrange(
				str, str + length,
				begin, std::ranges::next(begin, charsSize)
			);
		}

		[[nodiscard]]
		bool contains_any(const std::ranges::contiguous_range auto& chars) const
			requires(std::same_as<std::ranges::range_value_t<decltype(chars)>, char>)
		{
			const byte* str = data();
			const size_t length = byte_count();

			for (size_t i = 0; i < length; i++)
			{
				if (std::ranges::contains(chars, str[i]))
				{
					return true;
				}
			}

			return false;
		}
	};

}
