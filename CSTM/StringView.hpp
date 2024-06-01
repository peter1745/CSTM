#pragma once

#include "Types.hpp"
#include "StringBase.hpp"

#include <cstddef>

namespace CSTM {

	class String;

	class StringView : public StringBase
	{
	public:
		StringView() noexcept;
		StringView(const byte* data, size_t byteCount) noexcept;
		~StringView() noexcept override;

	public:
		[[nodiscard]]
		bool operator==(StringView other) const noexcept;

		[[nodiscard]]
		bool operator==(const char* str) const noexcept;

		[[nodiscard]]
		bool is_empty() const noexcept { return m_byte_count == 0; }

		[[nodiscard]]
		const byte* data() const noexcept override { return m_data; }

		[[nodiscard]]
		size_t byte_count() const noexcept override { return m_byte_count; }

	private:
		const byte* m_data;
		size_t m_byte_count;
	};

}
