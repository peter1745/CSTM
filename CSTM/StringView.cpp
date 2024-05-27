#include "StringView.hpp"

namespace CSTM {

	StringView::StringView() noexcept
			: m_data(nullptr), m_byte_count(0)
	{}

	StringView::StringView(const byte* data, size_t byteCount) noexcept
		: m_data(data), m_byte_count(byteCount)
	{}

}
