#pragma once

#include "Types.hpp"
#include "CodePointIterator.hpp"

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
		bool contains(const std::ranges::contiguous_range auto& chars) const
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

		[[nodiscard]]
		bool starts_with(const std::ranges::contiguous_range auto& chars) const
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

			for (size_t i = 0; i < charsSize; i++)
			{
				if (chars[i] != str[i])
				{
					return false;
				}
			}

			return true;
		}

		[[nodiscard]]
		bool starts_with_any(const std::ranges::contiguous_range auto& chars) const
			requires(std::same_as<std::ranges::range_value_t<decltype(chars)>, char>)
		{
			const byte* str = data();

			for (const auto c : chars)
			{
				if (c == str[0])
				{
					return true;
				}
			}

			return false;
		}

		[[nodiscard]]
		bool starts_with_any_code_point(const std::ranges::contiguous_range auto& codePoints) const
			requires(std::same_as<std::ranges::range_value_t<decltype(codePoints)>, uint32_t>)
		{
			bool result = false;

			CodePointIterator{ *this }.each([&](const uint32_t codePoint)
			{
				for (auto c : codePoints)
				{
					if (c == codePoint)
					{
						result = true;
						return IterAction::Break;
					}
				}

				// NOTE(Peter): Only check the very first code point
				return IterAction::Break;
			});

			return result;
		}

		[[nodiscard]]
		bool ends_with_any_code_point(const std::ranges::contiguous_range auto& codePoints) const
			requires(std::same_as<std::ranges::range_value_t<decltype(codePoints)>, uint32_t>)
		{
			bool result = false;

			CodePointReverseIterator{ *this }.each([&](const uint32_t codePoint)
			{
				for (auto c : codePoints)
				{
					if (c == codePoint)
					{
						result = true;
						return IterAction::Break;
					}
				}

				// NOTE(Peter): Only check the very last code point
				return IterAction::Break;
			});

			return result;
		}

	};

}
