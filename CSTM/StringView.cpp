#include "StringView.hpp"

#include <algorithm>
#include <string>

namespace CSTM {

	StringView::StringView() noexcept
			: m_data(nullptr), m_byte_count(0)
	{}

	StringView::StringView(const byte* data, size_t byteCount) noexcept
		: m_data(data), m_byte_count(byteCount)
	{}

	StringView::~StringView() noexcept
	{
		m_data = nullptr;
		m_byte_count = 0;
	}

	bool StringView::operator==(StringView other) const noexcept
	{
		if (this == &other || m_data == other.m_data)
		{
			return true;
		}

		if (m_byte_count != other.m_byte_count)
		{
			return false;
		}

		return std::equal(m_data, m_data + m_byte_count, other.m_data);
	}

	bool StringView::operator==(const char* str) const noexcept
	{
		if (std::char_traits<char>::length(str) != m_byte_count)
		{
			return false;
		}

		return std::equal(m_data, m_data + m_byte_count, str);
	}

}
