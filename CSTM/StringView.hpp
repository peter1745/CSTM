#pragma once

#include "Types.hpp"

#include <cstddef>

namespace CSTM {

	class String;

	class StringView
	{
	public:
		StringView() noexcept;
		StringView(const byte* data, size_t byteCount) noexcept;

		[[nodiscard]]
		const byte* data() const noexcept { return m_data; }

	private:
		const byte* m_data;
		size_t m_byte_count;
	};

}
