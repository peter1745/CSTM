#include "CodePointIterator.hpp"
#include "Assert.hpp"
#include "Unicode.hpp"
#include "String.hpp"

namespace CSTM {

	CodePointIteratorBase::CodePointIteratorBase(const byte* current, const byte* end)
		: m_current(current), m_end(end)
	{
	}

	void CodePointIteratorBase::compute_current_code_point()
	{
		uint32_t codePointByteCount = 0;
		const auto codePoint = utf8_to_utf32({ m_current[0], m_current[1], m_current[2], m_current[3] }, codePointByteCount);
		CSTM_Assert(codePoint.has_value());
		m_code_point.value = codePoint.value();
		m_code_point.byteCount = codePointByteCount;
	}

	CodePointIterator::CodePointIterator(const String& str)
		: CodePointIteratorBase(str.data(), str.data() + str.byte_count())
	{
	}

	bool CodePointIterator::advance()
	{
		if (m_current + m_code_point.byteCount >= m_end)
		{
			return false;
		}

		m_current += m_code_point.byteCount;

		CSTM_Assert(is_leading_byte(*m_current));

		compute_current_code_point();
		return true;
	}

}
