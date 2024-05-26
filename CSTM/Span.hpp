#pragma once

#include <ranges>
#include <initializer_list>

namespace CSTM {

	template<typename T>
	class Span
	{
	public:
		Span(std::initializer_list<const T> values)
			: m_begin(values.begin()), m_end(values.end())
		{
		}

		Span(std::ranges::contiguous_range auto&& range)
			: m_begin(range.data()), m_end(range.data() + range.size())
		{
		}

		Span(const T* begin, const T* end)
			: m_begin(begin), m_end(end)
		{
		}

		[[nodiscard]]
		size_t count() const { return m_end - m_begin; }

		[[nodiscard]]
		size_t byte_count() const { return (m_end - m_begin) * sizeof(T); }

		const T* begin() const { return m_begin; }
		const T* end() const { return m_end; }

		[[nodiscard]]
		decltype(auto) operator[](this auto&& self, size_t index)
		{
			using Self = decltype(self);
			TheiaAssert(index < std::forward<Self>(self).count());
			return std::forward<Self>(self).m_begin[index];
		}

	private:
		const T* m_begin;
		const T* m_end;
	};

}
