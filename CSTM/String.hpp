#pragma once

#include <cstddef>
#include <atomic>

#include "Types.hpp"
#include "HashMap.hpp"
#include "Result.hpp"
#include "StringBase.hpp"
#include "Span.hpp"

namespace CSTM {

	class StringView;

	enum class StringError
	{
		InvalidOffset,
		InvalidLength
	};

	class String : public StringBase
	{
		struct LargeStorage
		{
			byte* data;
			std::atomic_size_t ref_count;
			size_t hash_code;
		};

		static constexpr size_t SmallStringLength = 16 * sizeof(byte);
		inline static HashMap<size_t, LargeStorage*> StringPool;

	public:
		static String create(const char* str);
		static String create(std::string_view str);
		static String create(Span<uint32_t> codePoints);
		static String create(Span<byte> bytes);

	public:
		[[nodiscard]]
		bool is_empty() const noexcept { return m_byte_count == 0; }

		[[nodiscard]]
		bool is_large_string() const noexcept { return m_byte_count > SmallStringLength; }

		[[nodiscard]]
		const byte* data() const noexcept override { return is_large_string() ? m_large_storage->data : m_small_storage; }

		[[nodiscard]]
		size_t byte_count() const noexcept override { return m_byte_count; }

		[[nodiscard]]
		byte byte_at(const size_t index) const { return data()[index]; }

		[[nodiscard]]
		bool operator==(const String& other) const noexcept;

		[[nodiscard]]
		bool operator==(const std::ranges::contiguous_range auto& str) const noexcept
			requires(std::same_as<std::ranges::range_value_t<decltype(str)>, char>)
		{
			size_t strLength = std::ranges::size(str);

			if (str[strLength - 1] == '\0')
			{
				strLength--;
			}

			if (m_byte_count != strLength)
			{
				return false;
			}

			return std::equal(data(), data() + m_byte_count, str);
		}

		[[nodiscard]]
		size_t ref_count() const noexcept { return is_large_string() ? m_large_storage->ref_count.load() : 1; }

		[[nodiscard]]
		Result<StringView, StringError> view(size_t offset = 0, size_t length = ~0) const noexcept;

		[[nodiscard]]
		Result<String, StringError> remove(const std::ranges::contiguous_range auto& str) const noexcept
			requires(std::same_as<std::ranges::range_value_t<decltype(str)>, char>)
		{
			std::vector<byte> originalChars;
			originalChars.resize(m_byte_count);

			std::ranges::copy(data(), data() + m_byte_count, originalChars.begin());

			auto it = std::ranges::find(originalChars, str[0]);

			if (it == originalChars.end())
			{
				return *this;
			}

			size_t strLength = std::ranges::size(str);

			if (str[strLength - 1] == '\0')
			{
				strLength--;
			}

			if (m_byte_count < strLength)
			{
				return *this;
			}

			auto strBegin = std::ranges::begin(str);
			auto strEnd = std::next(strBegin, strLength - 1);

			while (it != originalChars.end())
			{
				auto end = std::ranges::find(it, originalChars.end(), str[strLength - 1]);

				if (end == originalChars.end())
				{
					return *this;
				}

				if (std::distance(it, end) != strLength - 1)
				{
					// Character range can't possibly contain the given string, keep searching
					it = std::ranges::find(std::next(it), originalChars.end(), str[0]);
					continue;
				}

				if (std::equal(it, end, strBegin, strEnd))
				{
					it = originalChars.erase(it, std::next(end));
				}
			}

			return create(Span<byte>(originalChars));
		}

		[[nodiscard]]
		Result<String, StringError> remove_any(const std::ranges::contiguous_range auto& chars) const noexcept
			requires(std::same_as<std::ranges::range_value_t<decltype(chars)>, char>)
		{
			std::vector<byte> originalChars;
			originalChars.resize(m_byte_count);

			std::ranges::copy(data(), data() + m_byte_count, originalChars.begin());

			auto toErase = std::ranges::remove_if(originalChars, [&](byte c)
			{
				return std::ranges::contains(chars, c);
			});

			originalChars.erase(toErase.begin(), toErase.end());

			return create(Span<byte>(originalChars));
		}

		[[nodiscard]]
		Result<String, StringError> remove_leading_code_points(const std::ranges::contiguous_range auto& codePoints) const noexcept
		{
			return remove_code_points_impl<CodePointIterator>(codePoints);
		}

		[[nodiscard]]
		Result<String, StringError> remove_trailing_code_points(const std::ranges::contiguous_range auto& codePoints) const noexcept
		{
			return remove_code_points_impl<CodePointReverseIterator>(codePoints);
		}

		[[nodiscard]]
		Result<String, StringError> append_code_points(const std::ranges::contiguous_range auto& codePoints) const noexcept
			requires(std::same_as<std::ranges::range_value_t<decltype(codePoints)>, uint32_t>)
		{
			std::vector<uint32_t> originalCodePoints;
			CodePointIterator{ *this }.store(originalCodePoints);
			originalCodePoints.insert(originalCodePoints.end(), std::ranges::begin(codePoints), std::ranges::end(codePoints));
			return create(Span<uint32_t>{ originalCodePoints });
		}

	private:
		template<std::derived_from<CodePointIteratorBase> Iterator>
		[[nodiscard]]
		Result<String, StringError> remove_code_points_impl(const std::ranges::contiguous_range auto& codePoints) const noexcept
		{
			std::vector<uint32_t> originalCodePoints;
			Iterator{ *this }.store(originalCodePoints);

			auto it = originalCodePoints.begin();
			while (it != originalCodePoints.end() && std::ranges::contains(codePoints, *it))
			{
				it++;
			}

			if (it == originalCodePoints.begin() || it == originalCodePoints.end())
			{
				return *this;
			}

			originalCodePoints.erase(originalCodePoints.begin(), it);

			if constexpr (std::same_as<CodePointReverseIterator, Iterator>)
			{
				std::ranges::reverse(originalCodePoints);
			}

			return create(Span<uint32_t>(originalCodePoints));
		}

	public:
		String() noexcept = default;
		String(const String& other) noexcept;
		String(String&& other) noexcept;
		~String() noexcept override;

		String& operator=(const String& other) noexcept;
		String& operator=(String&& other) noexcept;

	private:
		void try_decrease_ref_count() const noexcept;
		void allocate_from(const byte* data, size_t byteCount);

		[[nodiscard]]
		byte* data_mut() { return is_large_string() ? m_large_storage->data : m_small_storage; }

	private:
		union
		{
			byte m_small_storage[SmallStringLength]{};
			LargeStorage* m_large_storage;
		};

		size_t m_byte_count = 0;
	};

}
