#pragma once

#include "Assert.hpp"
#include "HashMap.hpp"
#include "Span.hpp"
#include "Types.hpp"
#include "EnumFlags.hpp"
#include "Unicode.hpp"

#include <cstdint>
#include <cstddef>
#include <utility>
#include <ranges>
#include <algorithm>

namespace CSTM {

	// Utf8String represents an immutable UTF-8 string
	// that's designed to operate on Unicode code points
	// instead of normal text characters

	enum class EraseMode
	{
		All = 1 << 0,
		Leading = 1 << 1,
		Trailing = 1 << 2,
	};
	void enum_flags(EraseMode);

	class CodePointIteratorBase
	{
	public:
		virtual ~CodePointIteratorBase() = default;

		bool operator==(const CodePointIteratorBase& other) const;
		CodePointIteratorBase& operator++();
		uint32_t operator*() const;

		[[nodiscard]]
		const byte* data() const { return m_data; }

		template<template<typename V, typename...> typename T>
		requires std::ranges::range<T<byte>>
		[[nodiscard]]
		typename T<byte>::const_iterator to_iter() const
		{
			return typename T<byte>::const_iterator(m_data);
		}

	protected:
		CodePointIteratorBase(const byte* data, const byte* end)
			: m_data(data), m_end(end)
		{
		}

		uint32_t compute_current_code_point(uint32_t& outCodePointByteCount) const;

		virtual void advance_and_read() = 0;

	protected:
		const byte* m_data;
		const byte* m_end;

		struct CodePoint
		{
			uint32_t value = ~0u;
			uint32_t byteCount = 0;
		} m_code_point{};
	};

	class CodePointIterator final : public CodePointIteratorBase
	{
	public:
		CodePointIterator(const byte* data, const byte* end)
			: CodePointIteratorBase(data, end)
		{
			if (data != end)
			{
				CodePointIterator::advance_and_read();
			}
		}

	private:
		void advance_and_read() override;
	};

	class CodePointReverseIterator final : public CodePointIteratorBase
	{
	public:
		CodePointReverseIterator(const byte* data, const byte* end)
			: CodePointIteratorBase(data, end)
		{
			if (data != end)
			{
				CodePointReverseIterator::advance_and_read();
			}
		}

	private:
		void advance_and_read() override;
	};

	struct StringData
	{
		byte* storage;
		size_t ref_count;
		size_t byte_count;
		size_t hash_code;
	};

	class Utf8String;

	struct Utf8View
	{
		const byte* ptr = nullptr;
		size_t byte_count = 0;

		Utf8View(const Utf8String& string);
		Utf8View(const char8_t* string);
		Utf8View(const char* string);

		[[nodiscard]]
		CodePointIterator begin() const { return { ptr, ptr + byte_count }; }

		[[nodiscard]]
		CodePointReverseIterator rbegin() const { return { ptr + byte_count, ptr - 1 }; }

		[[nodiscard]]
		CodePointIterator end() const { return { ptr + byte_count, ptr + byte_count }; }

		[[nodiscard]]
		CodePointReverseIterator rend() const { return { ptr - 1, ptr - 1 }; }

		[[nodiscard]]
		std::vector<uint32_t> get_code_points() const;
	};

	class Utf8String
	{
		static constexpr size_t SmallStringLength = 15 * sizeof(char8_t);
		inline static HashMap<size_t, StringData*> StringPool;

	public:
		static Utf8String from_chars(const char8_t* str);
		static Utf8String from_chars(const char* str);
		static Utf8String from_bytes(Span<byte> bytes);
		static Utf8String from_code_points(Span<uint32_t> codePoints);

	public:
		bool is_empty() const { return m_byte_count == 0; }

		[[nodiscard]]
		Utf8View view() const { return Utf8View{ *this }; }

		[[nodiscard]]
		size_t ref_count() const noexcept { return large_storage_engaged() ? m_large_storage->ref_count : 1; }

		[[nodiscard]]
		size_t byte_count() const noexcept { return m_byte_count; }

		[[nodiscard]]
		size_t code_point_count() const noexcept
		{
			return get_code_points().size();
		}

		[[nodiscard]]
		bool large_storage_engaged() const noexcept
		{
			return m_byte_count > SmallStringLength;
		}

		[[nodiscard]]
		const byte* data() const noexcept
		{
			if (large_storage_engaged())
			{
				return m_large_storage->storage;
			}

			return m_small_storage;
		}

		[[nodiscard]]
		CodePointIterator begin() const { return { data(), data() + m_byte_count }; }

		[[nodiscard]]
		CodePointReverseIterator rbegin() const { return { data() + m_byte_count, data() - 1 }; }

		[[nodiscard]]
		CodePointIterator end() const { return { data() + m_byte_count, data() + m_byte_count }; }

		[[nodiscard]]
		CodePointReverseIterator rend() const { return { data() - 1, data() - 1 }; }

		bool operator==(const Utf8String& other) const
		{
			if (this == &other)
			{
				return true;
			}

			if (large_storage_engaged())
			{
				// Large storage utilizes string pooling which means that the comparison
				// simply requires comparing the pointers of the two strings
				return m_large_storage == other.m_large_storage;
			}

			// In the event that we're using small storage
			// we should also guarantee that the strings are
			// the same length before doing byte-to-byte comparison
			if (m_byte_count != other.m_byte_count)
			{
				return false;
			}

			// In the event that we're using small string storage
			// we do byte-to-byte comparison
			for (size_t i = 0; i < m_byte_count; i++)
			{
				if (m_small_storage[i] != other.m_small_storage[i])
				{
					return false;
				}
			}

			return true;
		}

		bool operator==(Utf8View other) const
		{
			if (data() == other.ptr)
			{
				return true;
			}

			const auto codePoints = get_code_points();
			const auto otherCodePoints = other.get_code_points();

			if (codePoints.size() != otherCodePoints.size())
			{
				return false;
			}

			for (size_t i = 0; i < codePoints.size(); i++)
			{
				if (codePoints[i] != otherCodePoints[i])
				{
					return false;
				}
			}

			return true;
		}

		[[nodiscard]]
		std::vector<uint32_t> get_code_points() const;

		[[nodiscard]]
		bool starts_with(Utf8View str) const;

		template<template<typename V, typename... Args> typename T, typename... Args>
		requires std::ranges::range<T<uint32_t, Args...>>
		[[nodiscard]]
		bool starts_with_any(const T<uint32_t, Args...>& codePoints) const
		{
			return std::ranges::contains(codePoints, *begin());
		}

		[[nodiscard]]
		bool ends_with(Utf8View str) const;

		template<template<typename V, typename... Args> typename T, typename... Args>
		requires std::ranges::range<T<uint32_t, Args...>>
		[[nodiscard]]
		bool ends_with_any(const T<uint32_t, Args...>& codePoints) const
		{
			return std::ranges::contains(codePoints, *rbegin());
		}

		template<template<typename V, typename... Args> typename T, typename... Args>
		requires std::ranges::range<T<uint32_t, Args...>>
		[[nodiscard]]
		bool contains_any(const T<uint32_t, Args...>& searchFor) const
		{
			auto codePoints = get_code_points();

			for (auto codePoint : searchFor)
			{
				if (std::ranges::contains(codePoints, codePoint))
				{
					return true;
				}
			}

			return false;
		}

		[[nodiscard]]
		bool contains_any(Utf8View searchFor) const
		{
			return contains_any(searchFor.get_code_points());
		}

		template<template<typename V, typename... Args> typename T, typename... Args>
		requires std::ranges::range<T<uint32_t, Args...>>
		[[nodiscard]]
		Utf8String erase(const T<uint32_t, Args...>& toErase, EraseMode mode = EraseMode::All) const
		{
			// FIXME(Peter): Would be nice to avoid a copy if we end up not mutating the string
			auto codePoints = get_code_points();

			bool didErase = false;

			if (mode & EraseMode::All)
			{
				for (auto it = codePoints.rbegin(); it != codePoints.rend(); ++it)
				{
					if (!std::ranges::contains(toErase, *it))
					{
						continue;
					}

					codePoints.erase(std::next(it).base());
					didErase = true;
				}
			}
			else
			{
				if (mode & EraseMode::Leading)
				{
					auto it = codePoints.begin();

					for (; it != codePoints.end(); ++it)
					{
						if (!std::ranges::contains(toErase, *it))
						{
							break;
						}

						didErase = true;
					}

					codePoints.erase(codePoints.begin(), it);
				}

				if (mode & EraseMode::Trailing)
				{
					auto it = codePoints.rbegin();

					for (; it != codePoints.rend(); ++it)
					{
						if (!std::ranges::contains(toErase, *it))
						{
							break;
						}

						didErase = true;
					}

					codePoints.erase(it.base(), codePoints.end());
				}
			}

			/*if (!didErase)
				return *this;*/

			return from_code_points(codePoints);
		}

		[[nodiscard]]
		Utf8String erase(Utf8View toErase, EraseMode mode = EraseMode::All) const
		{
			return erase(toErase.get_code_points(), mode);
		}

		[[nodiscard]]
		uint32_t code_point_at(size_t index) const
		{
			const auto codePoints = get_code_points();
			TheiaAssert(index < codePoints.size());
			return codePoints.at(index);
		}

	public:
		Utf8String() noexcept;
		Utf8String(const Utf8String& other) noexcept;
		~Utf8String() noexcept;

		Utf8String& operator=(const Utf8String& other) noexcept;

	private:
		auto data_ptr(this auto&& self) noexcept
		{
			using Self = decltype(self);

			if (std::forward<Self>(self).large_storage_engaged())
			{
				return std::forward_like<Self>(self.m_large_storage)->storage;
			}

			return std::forward_like<Self>(self.m_small_storage);
		}

		void try_decrease_ref_count() const noexcept;

		void allocate_from(const byte* data, size_t byteCount);

	private:
		union
		{
			StringData* m_large_storage;
			byte m_small_storage[SmallStringLength]{};
		};

		size_t m_byte_count = 0;
	};
}

