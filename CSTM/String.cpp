#include "Assert.hpp"
#include "String.hpp"
#include "StringView.hpp"
#include "Unicode.hpp"

#include <algorithm>
#include <utility>

namespace CSTM {

	String String::create(const char* str)
	{
		// NOTE(Peter): Lets face it, it's increadibly unlikely for char
		// to be anything other than 8 bits, but just in case I'll leave this here
		static_assert(CHAR_BIT == 8);
		String string;
		string.allocate_from(reinterpret_cast<const byte*>(str), std::char_traits<char>::length(str));
		return string;
	}

	String String::create(std::string_view str)
	{
		String string;
		string.allocate_from(reinterpret_cast<const byte*>(str.data()), str.length());
		return string;
	}

	String String::create(Span<uint32_t> codePoints)
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

		String string;
		string.allocate_from(codePointBytes.data(), codePointBytes.size());
		return string;
	}

	String String::create(Span<byte> bytes)
	{
		String string;
		string.allocate_from(bytes.begin(), bytes.byte_count());
		return string;
	}

	String::String(const String& other) noexcept
		: m_byte_count(other.m_byte_count)
	{
		if (other.is_large_string())
		{
			m_large_storage = other.m_large_storage;
			m_large_storage->ref_count++;
		}
		else
		{
			std::copy_n(other.m_small_storage, other.m_byte_count, m_small_storage);
		}
	}

	String::String(String&& other) noexcept
		: m_byte_count(std::exchange(other.m_byte_count, 0))
	{
		if (is_large_string())
		{
			m_large_storage = std::exchange(other.m_large_storage, nullptr);
		}
		else
		{
			std::copy_n(other.m_small_storage, m_byte_count, m_small_storage);
			std::fill_n(other.m_small_storage, SmallStringLength, 0);
		}
	}

	String::~String() noexcept
	{
		try_decrease_ref_count();
	}

	String& String::operator=(const String& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}

		try_decrease_ref_count();

		m_byte_count = other.m_byte_count;

		if (other.is_large_string())
		{
			m_large_storage = other.m_large_storage;
			m_large_storage->ref_count++;
		}
		else
		{
			std::fill_n(m_small_storage, SmallStringLength, 0);
			std::copy_n(other.m_small_storage, other.m_byte_count, m_small_storage);
		}

		return *this;
	}

	String& String::operator=(String&& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}

		try_decrease_ref_count();

		m_byte_count = std::exchange(other.m_byte_count, 0);

		if (is_large_string())
		{
			m_large_storage = std::exchange(other.m_large_storage, nullptr);
		}
		else
		{
			std::fill_n(m_small_storage, SmallStringLength, 0);
			std::copy_n(other.m_small_storage, m_byte_count, m_small_storage);
			std::fill_n(other.m_small_storage, SmallStringLength, 0);
		}

		return *this;
	}

	bool String::operator==(const String& other) const noexcept
	{
		if (this == &other)
		{
			return true;
		}

		if (m_byte_count != other.m_byte_count)
		{
			return false;
		}

		if (is_large_string())
		{
			// If we're a large string we can do simple pointer comparison
			return m_large_storage == other.m_large_storage;
		}

		// Otherwise we have to do full string comparison
		return std::equal(std::begin(m_small_storage), std::end(m_small_storage), std::begin(other.m_small_storage));
	}

	Result<StringView, StringError> String::view(size_t offset, size_t length) const noexcept
	{
		if (offset >= m_byte_count)
		{
			return StringError::InvalidOffset;
		}

		if (length == ~0)
		{
			length = m_byte_count - offset;
		}

		if (length > m_byte_count - offset)
		{
			return StringError::InvalidLength;
		}

		return StringView{ data() + offset, length };
	}

	void String::try_decrease_ref_count() const noexcept
	{
		if (!is_large_string())
		{
			return;
		}

		if (--m_large_storage->ref_count > 0)
		{
			return;
		}

		StringPool.remove(m_large_storage->hash_code);

		delete[] m_large_storage->data;
		delete m_large_storage;
	}

	void String::allocate_from(const byte* bytes, size_t byteCount)
	{
		CSTM_Assert(m_byte_count == 0);

		m_byte_count = byteCount;

		if (is_large_string())
		{
			const size_t hash = SecureHash<std::u8string_view>{}(std::u8string_view{ reinterpret_cast<const char8_t*>(bytes), byteCount });

			if (!StringPool.contains(hash))
			{
				m_large_storage = new LargeStorage();
				m_large_storage->data = new byte[m_byte_count];
				m_large_storage->ref_count = 1;
				m_large_storage->hash_code = hash;

				StringPool.insert(hash, m_large_storage);
			}
			else
			{
				m_large_storage = StringPool[hash];
				m_large_storage->ref_count++;
				return;
			}
		}

		std::copy_n(bytes, m_byte_count, data_mut());
	}

}
