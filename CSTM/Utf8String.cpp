#include "Utf8String.hpp"
#include "Assert.hpp"
#include "Utility.hpp"

#include <algorithm>
#include <array>
#include <string_view>

namespace CSTM {

	bool CodePointIteratorBase::operator==(const CodePointIteratorBase& other) const
	{
		return m_data == other.m_data;
	}

	CodePointIteratorBase& CodePointIteratorBase::operator++()
	{
		advance_and_read();
		return *this;
	}

	uint32_t CodePointIteratorBase::operator*() const
	{
		return m_code_point.value;
	}

	uint32_t CodePointIteratorBase::compute_current_code_point(uint32_t& outCodePointByteCount) const
	{
		const auto codePoint = utf8_to_utf32({ m_data[0], m_data[1], m_data[2], m_data[3] }, outCodePointByteCount);
		TheiaAssert(codePoint.has_value());
		return codePoint.value();
	}

	void CodePointIterator::advance_and_read()
	{
		TheiaAssert(m_data + m_code_point.byteCount <= m_end);
		m_data += m_code_point.byteCount;
		TheiaAssert(is_leading_byte(*m_data));
		m_code_point.value = compute_current_code_point(m_code_point.byteCount);
	}

	void CodePointReverseIterator::advance_and_read()
	{
		// 1. Search backwards from m_data until we find a leading byte
		size_t searchOffset = 1;
		while (!is_leading_byte(*(m_data - searchOffset)))
		{
			searchOffset++;
		}

		TheiaAssert(m_data - searchOffset >= m_end);

		// 2. Point m_data to the leading byte
		m_data -= searchOffset;

		// 3. Read the leading byte + trailing bytes
		m_code_point.value = compute_current_code_point(m_code_point.byteCount);
	}

	Utf8View::Utf8View(const Utf8String& string)
			: ptr(string.data()), byte_count(string.byte_count())
	{
	}

	Utf8View::Utf8View(const char8_t* string)
		: ptr(std::bit_cast<const byte*>(string)), byte_count(std::char_traits<char8_t>::length(string))
	{
	}

	Utf8View::Utf8View(const char* string)
		: ptr(std::bit_cast<const byte*>(string)), byte_count(std::char_traits<char>::length(string))
	{
	}

	std::vector<uint32_t> Utf8View::get_code_points() const
	{
		// FIXME(Peter): Consider caching this in the event that it becomes a bottleneck
		std::vector<uint32_t> result;

		for (auto cp : *this)
		{
			result.push_back(cp);
		}

		return result;
	}

	Utf8String Utf8String::from_chars(const char8_t* str)
	{
		Utf8String string;
		string.allocate_from(reinterpret_cast<const byte*>(str), std::char_traits<char8_t>::length(str));
		return string;
	}

	Utf8String Utf8String::from_chars(const char* str)
	{
		static_assert(sizeof(char) == 1);
		return from_chars(reinterpret_cast<const char8_t*>(str));
	}

	Utf8String Utf8String::from_bytes(Span<byte> str)
	{
		Utf8String string;
		string.allocate_from(str.begin(), str.count());
		return string;
	}

	Utf8String Utf8String::from_code_points(Span<uint32_t> codePoints)
	{
		std::vector<byte> codePointBytes;

		for (auto codePoint : codePoints)
		{
			uint32_t byteCount = 0;
			auto bytes = utf32_to_utf8(codePoint, byteCount);

			for (uint32_t i = 0; i < byteCount; i++)
			{
				codePointBytes.push_back(bytes[i]);
			}
		}

		return from_bytes(codePointBytes);
	}

	Utf8String::Utf8String() noexcept {}

	Utf8String::Utf8String(const Utf8String& other) noexcept
		: m_byte_count(other.m_byte_count)
	{
		if (other.large_storage_engaged())
		{
			m_large_storage = other.m_large_storage;
			m_large_storage->ref_count++;
		}
		else
		{
			std::copy_n(other.m_small_storage, other.m_byte_count, m_small_storage);
		}
	}

	Utf8String::~Utf8String() noexcept
	{
		try_decrease_ref_count();
	}

	Utf8String& Utf8String::operator=(const Utf8String& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}

		try_decrease_ref_count();

		m_byte_count = other.m_byte_count;

		if (other.large_storage_engaged())
		{
			m_large_storage = other.m_large_storage;
			m_large_storage->ref_count++;
		}
		else
		{
			std::copy_n(other.m_small_storage, other.m_byte_count, m_small_storage);
		}

		return *this;
	}

	void Utf8String::try_decrease_ref_count() const noexcept
	{
		if (!large_storage_engaged())
		{
			return;
		}

		if (--m_large_storage->ref_count > 0)
		{
			return;
		}

		StringPool.remove(m_large_storage->hash_code);

		delete[] m_large_storage->storage;
		delete m_large_storage;
	}

	void Utf8String::allocate_from(const byte* data, size_t byteCount)
	{
		TheiaAssert(m_byte_count == 0);

		m_byte_count = byteCount;

		if (large_storage_engaged())
		{
			const size_t hash = SecureHash<std::u8string_view>{}(std::u8string_view{reinterpret_cast<const char8_t*>(data)});

			if (!StringPool.contains(hash))
			{
				m_large_storage = new StringData();
				m_large_storage->storage = new byte[m_byte_count];
				m_large_storage->ref_count = 1;
				m_large_storage->byte_count = m_byte_count;
				m_large_storage->hash_code = hash;

				StringPool.insert(hash, m_large_storage);
			}
			else
			{
				m_large_storage = StringPool[hash];
				m_large_storage->ref_count++;
			}
		}

		std::copy_n(data, m_byte_count, data_ptr());
	}

	std::vector<uint32_t> Utf8String::get_code_points() const
	{
		return view().get_code_points();
	}

	bool Utf8String::starts_with(Utf8View str) const
	{
		if (m_byte_count < str.byte_count)
		{
			return false;
		}

		const byte* startByte = data();
		const byte* endByte = startByte + str.byte_count;
		return std::equal(startByte, endByte, str.ptr);
	}

	bool Utf8String::ends_with(Utf8View str) const
	{
		if (m_byte_count < str.byte_count)
		{
			return false;
		}

		size_t start = m_byte_count - str.byte_count;
		const byte* startByte = data() + start;
		const byte* endByte = startByte + str.byte_count;

		return std::equal(startByte, endByte, str.ptr);
	}

}
